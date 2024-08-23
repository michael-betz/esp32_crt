/**
 * \file            lwjson.c
 * \brief           Lightweight JSON format parser
 */

/*
 * Copyright (c) 2024 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of LwJSON - Lightweight JSON format parser.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v1.7.0
 */
#include <string.h>
#include "lwjson.h"
#include <stdio.h>

/**
 * \brief           Token print instance
 */
typedef struct {
    size_t indent; /*!< Indent level for token print */
} lwjson_token_print_t;

#if defined(LWJSON_DEV)
#define DEBUG_STRING_PREFIX_SPACES                                                                                     \
    "                                                                                                "
#define LWJSON_DEBUG(jsp, ...)                                                                                         \
    do {                                                                                                               \
        if ((jsp) != NULL) {                                                                                           \
            printf("%.*s", (int)(4 * (jsp)->stack_pos), DEBUG_STRING_PREFIX_SPACES);                                   \
        }                                                                                                              \
        printf(__VA_ARGS__);                                                                                           \
    } while (0)

/* Strings for debug */
static const char* const type_strings[] = {
    [LWJSON_STREAM_TYPE_NONE] = "none",
    [LWJSON_STREAM_TYPE_OBJECT] = "object",
    [LWJSON_STREAM_TYPE_OBJECT_END] = "object_end",
    [LWJSON_STREAM_TYPE_ARRAY] = "array",
    [LWJSON_STREAM_TYPE_ARRAY_END] = "array_end",
    [LWJSON_STREAM_TYPE_KEY] = "key",
    [LWJSON_STREAM_TYPE_STRING] = "string",
    [LWJSON_STREAM_TYPE_TRUE] = "true",
    [LWJSON_STREAM_TYPE_FALSE] = "false",
    [LWJSON_STREAM_TYPE_NULL] = "null",
    [LWJSON_STREAM_TYPE_NUMBER] = "number",
};
#else
#define LWJSON_DEBUG(jsp, ...)
#endif /* defined(LWJSON_DEV) */

/**
 * \brief           Sends an event to user for further processing
 *
 */
#define SEND_EVT(jsp, type)                                                                                            \
    if ((jsp) != NULL && (jsp)->evt_fn != NULL) {                                                                      \
        (jsp)->evt_fn((jsp), (type));                                                                                  \
    }

/**
 * \brief           Check if character is a space character (with extended chars)
 * \param[in]       c: Character to check
 * \return          `1` if considered extended space, `0` otherwise
 */
#define prv_is_space_char_ext(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n' || (c) == '\f')


/**
 * \brief           Internal string object
 */
typedef struct {
    const char* start; /*!< Original pointer to beginning of JSON object */
    size_t len;        /*!< Total length of input json string */
    const char* p;     /*!< Current char pointer */
} lwjson_int_str_t;

/**
 * \brief           Print token value
 * \param[in]       prt: Token print instance
 * \param[in]       token: Token to print
 */
static void
prv_print_token(lwjson_token_print_t* prt, const lwjson_token_t* token) {
#define print_indent() printf("%.*s", (int)((prt->indent)), "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");

    if (token == NULL) {
        return;
    }

    /* Check if token has a name */
    print_indent();
    if (token->token_name != NULL) {
        printf("\"%.*s\":", (int)token->token_name_len, token->token_name);
    }

    /* Print different types */
    switch (token->type) {
        case LWJSON_TYPE_OBJECT:
        case LWJSON_TYPE_ARRAY: {
            printf("%c", token->type == LWJSON_TYPE_OBJECT ? '{' : '[');
            if (token->u.first_child != NULL) {
                printf("\n");
                ++prt->indent;
                for (const lwjson_token_t* t = lwjson_get_first_child(token); t != NULL; t = t->next) {
                    prv_print_token(prt, t);
                }
                --prt->indent;
                print_indent();
            }
            printf("%c", token->type == LWJSON_TYPE_OBJECT ? '}' : ']');
            break;
        }
        case LWJSON_TYPE_STRING: {
            printf("\"%.*s\"", (int)lwjson_get_val_string_length(token), lwjson_get_val_string(token, NULL));
            break;
        }
        case LWJSON_TYPE_NUM_INT: {
            printf("%lld", (long long)lwjson_get_val_int(token));
            break;
        }
        case LWJSON_TYPE_NUM_REAL: {
            printf("%f", (double)lwjson_get_val_real(token));
            break;
        }
        case LWJSON_TYPE_TRUE: {
            printf("true");
            break;
        }
        case LWJSON_TYPE_FALSE: {
            printf("false");
            break;
        }
        case LWJSON_TYPE_NULL: {
            printf("NULL");
            break;
        }
        default: break;
    }
    if (token->next != NULL) {
        printf(",");
    }
    printf("\n");
}

/**
 * \brief           Prints and outputs token data to the stream output
 * \note            This function is not re-entrant
 * \param[in]       token: Token to print
 */
void
lwjson_print_token(const lwjson_token_t* token) {
    lwjson_token_print_t prt = {0};
    prv_print_token(&prt, token);
}

/**
 * \brief           Prints and outputs full parsed LwJSON instance
 * \note            This function is not re-entrant
 * \param[in]       lwobj: LwJSON instance to print
 */
void
lwjson_print_json(const lwjson_t* lwobj) {
    lwjson_token_print_t prt = {0};
    prv_print_token(&prt, lwjson_get_first_token(lwobj));
}

/**
 * \brief           Allocate new token for JSON block
 * \param[in]       lwobj: LwJSON instance
 * \return          Pointer to new token
 */
static lwjson_token_t*
prv_alloc_token(lwjson_t* lwobj) {
    if (lwobj->next_free_token_pos < lwobj->tokens_len) {
        LWJSON_MEMSET(&lwobj->tokens[lwobj->next_free_token_pos], 0x00, sizeof(*lwobj->tokens));
        return &lwobj->tokens[lwobj->next_free_token_pos++];
    }
    return NULL;
}

/**
 * \brief           Skip all characters that are considered *blank* as per RFC4627
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_skip_blank(lwjson_int_str_t* pobj) {
    while (pobj->p != NULL && *pobj->p != '\0' && (size_t)(pobj->p - pobj->start) < pobj->len) {
        if (*pobj->p == ' ' || *pobj->p == '\t' || *pobj->p == '\r' || *pobj->p == '\n' || *pobj->p == '\f') {
            ++pobj->p;
#if LWJSON_CFG_COMMENTS
            /* Check for comments and remove them */
        } else if (*pobj->p == '/') {
            ++pobj->p;
            if (pobj->p != NULL && *pobj->p == '*') {
                ++pobj->p;
                while (pobj->p != NULL && *pobj->p != '\0' && (size_t)(pobj->p - pobj->start) < pobj->len) {
                    if (*pobj->p == '*') {
                        ++pobj->p;
                        if (*pobj->p == '/') {
                            ++pobj->p;
                            break;
                        }
                    }
                    ++pobj->p;
                }
            }
#endif /* LWJSON_CFG_COMMENTS */
        } else {
            break;
        }
    }
    if (pobj->p != NULL && *pobj->p != '\0' && (size_t)(pobj->p - pobj->start) < pobj->len) {
        return lwjsonOK;
    }
    return lwjsonERRJSON;
}

/**
 * \brief           Parse JSON string that must start end end with double quotes `"` character
 * It just parses length of characters and does not perform any decode operation
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \param[out]      pout: Pointer to pointer to string that is set where string starts
 * \param[out]      poutlen: Length of string in units of characters is stored here
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_parse_string(lwjson_int_str_t* pobj, const char** pout, size_t* poutlen) {
    lwjsonr_t res;
    size_t len = 0;

    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    if (*pobj->p++ != '"') {
        return lwjsonERRJSON;
    }
    *pout = pobj->p;
    /* Parse string but take care of escape characters */
    for (;; ++pobj->p, ++len) {
        if (pobj->p == NULL || *pobj->p == '\0' || (size_t)(pobj->p - pobj->start) >= pobj->len) {
            return lwjsonERRJSON;
        }
        /* Check special characters */
        if (*pobj->p == '\\') {
            ++pobj->p;
            ++len;
            switch (*pobj->p) {
                case '"':  /* fallthrough */
                case '\\': /* fallthrough */
                case '/':  /* fallthrough */
                case 'b':  /* fallthrough */
                case 'f':  /* fallthrough */
                case 'n':  /* fallthrough */
                case 'r':  /* fallthrough */
                case 't': break;
                case 'u':
                    ++pobj->p;
                    for (size_t i = 0; i < 4; ++i, ++len) {
                        if (!((*pobj->p >= '0' && *pobj->p <= '9') || (*pobj->p >= 'a' && *pobj->p <= 'f')
                              || (*pobj->p >= 'A' && *pobj->p <= 'F'))) {
                            return lwjsonERRJSON;
                        }
                        if (i < 3) {
                            ++pobj->p;
                        }
                    }
                    break;
                default: return lwjsonERRJSON;
            }
        } else if (*pobj->p == '"') {
            ++pobj->p;
            break;
        }
    }
    *poutlen = len;
    return res;
}

/**
 * \brief           Parse property name that must comply with JSON string format as in RFC4627
 * Property string must be followed by colon character ":"
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \param[out]      t: Token instance to write property name to
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_parse_property_name(lwjson_int_str_t* pobj, lwjson_token_t* t) {
    lwjsonr_t res;

    /* Parse property string first */
    res = prv_parse_string(pobj, &t->token_name, &t->token_name_len);
    if (res != lwjsonOK) {
        return res;
    }
    /* Skip any spaces */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    /* Must continue with colon */
    if (*pobj->p++ != ':') {
        return lwjsonERRJSON;
    }
    /* Skip any spaces */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    return lwjsonOK;
}

/**
 * \brief           Parse number as described in RFC4627
 * \param[in,out]   pobj: Pointer to text that is modified on success
 * \param[out]      tout: Pointer to output number format
 * \param[out]      fout: Pointer to output real-type variable. Used if type is REAL.
 * \param[out]      iout: Pointer to output int-type variable. Used if type is INT.
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static lwjsonr_t
prv_parse_number(lwjson_int_str_t* pobj, lwjson_type_t* tout, lwjson_real_t* fout, lwjson_int_t* iout) {
    lwjsonr_t res;
    uint8_t is_minus;
    lwjson_real_t real_num = 0;
    lwjson_int_t int_num = 0;
    lwjson_type_t type = LWJSON_TYPE_NUM_INT;

    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    if (*pobj->p == '\0' || (size_t)(pobj->p - pobj->start) >= pobj->len) {
        return lwjsonERRJSON;
    }
    is_minus = *pobj->p == '-' ? (++pobj->p, 1) : 0;
    if (*pobj->p == '\0'                    /* Invalid string */
        || *pobj->p < '0' || *pobj->p > '9' /* Character outside number range */
        || (*pobj->p == '0'
            && (pobj->p[1] < '0' && pobj->p[1] > '9'))) { /* Number starts with 0 but not followed by dot */
        return lwjsonERRJSON;
    }

    /* Parse number */
    for (int_num = 0; *pobj->p >= '0' && *pobj->p <= '9'; ++pobj->p) {
        int_num = int_num * (lwjson_int_t)10 + (*pobj->p - '0');
    }

    real_num = (lwjson_real_t)int_num;

    if (pobj->p != NULL && *pobj->p == '.') { /* Number has exponent */
        lwjson_real_t exp;
        lwjson_int_t dec_num;

        real_num = (lwjson_real_t)int_num;

        type = LWJSON_TYPE_NUM_REAL;            /* Format is real */
        ++pobj->p;                              /* Ignore comma character */
        if (*pobj->p < '0' || *pobj->p > '9') { /* Must be followed by number characters */
            return lwjsonERRJSON;
        }

        /* Get number after decimal point */
        for (exp = (lwjson_real_t)1, dec_num = 0; *pobj->p >= '0' && *pobj->p <= '9';
             ++pobj->p, exp *= (lwjson_real_t)10) {
            dec_num = dec_num * (lwjson_int_t)10 + (lwjson_int_t)(*pobj->p - '0');
        }

        /* Add decimal part to number */
        real_num += (lwjson_real_t)dec_num / exp;
    }
    if (pobj->p != NULL && (*pobj->p == 'e' || *pobj->p == 'E')) { /* Engineering mode */
        uint8_t is_minus_exp;
        lwjson_int_t exp_cnt;

        type = LWJSON_TYPE_NUM_REAL;                         /* Format is real */
        ++pobj->p;                                           /* Ignore enginnering sing part */
        is_minus_exp = *pobj->p == '-' ? (++pobj->p, 1) : 0; /* Check if negative */
        if (*pobj->p == '+') {                               /* Optional '+' is possible too */
            ++pobj->p;
        }
        if (*pobj->p < '0' || *pobj->p > '9') { /* Must be followed by number characters */
            return lwjsonERRJSON;
        }

        /* Parse exponent number */
        for (exp_cnt = 0; *pobj->p >= '0' && *pobj->p <= '9'; ++pobj->p) {
            exp_cnt = exp_cnt * (lwjson_int_t)10 + (lwjson_int_t)(*pobj->p - '0');
        }

        /* Calculate new value for exponent 10^exponent */
        /* TODO: We could change this to lookup tables... */
        if (is_minus_exp) {
            for (; exp_cnt > 0; real_num /= (lwjson_real_t)10, --exp_cnt) {}
        } else {
            for (; exp_cnt > 0; real_num *= (lwjson_real_t)10, --exp_cnt) {}
        }
    }

    /* Write output values */
    if (tout != NULL) {
        *tout = type;
    }
    if (type == LWJSON_TYPE_NUM_INT) {
        *iout = is_minus ? -int_num : int_num;
    } else {
        *fout = is_minus ? -real_num : real_num;
    }
    return lwjsonOK;
}

/**
 * \brief           Create path segment from input path for search operation
 * \param[in,out]   ppath: Pointer to pointer to input path. Pointer is modified
 * \param[out]      opath: Pointer to pointer to write path segment
 * \param[out]      olen: Pointer to variable to write length of segment
 * \param[out]      is_last: Pointer to write if this is last segment
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_create_path_segment(const char** ppath, const char** opath, size_t* olen, uint8_t* is_last) {
    const char* segment = *ppath;

    *is_last = 0;
    *opath = NULL;
    *olen = 0;

    /* Check input path */
    if (segment == NULL || *segment == '\0') {
        *is_last = 1;
        return 0;
    }

    /*
     * Path must be one of:
     * - literal text
     * - "#" followed by dot "."
     */
    if (*segment == '#') {
        *opath = segment;
        for (*olen = 0;; ++segment, ++(*olen)) {
            if (*segment == '.') {
                ++segment;
                break;
            } else if (*segment == '\0') {
                if (*olen == 1) {
                    return 0;
                } else {
                    break;
                }
            }
        }
        *ppath = segment;
    } else {
        *opath = segment;
        for (*olen = 0; *segment != '\0' && *segment != '.'; ++(*olen), ++segment) {}
        *ppath = segment + 1;
    }
    if (*segment == '\0') {
        *is_last = 1;
    }
    return 1;
}

/**
 * \brief           Input recursive function for find operation
 * \param[in]       parent: Parent token of type \ref LWJSON_TYPE_ARRAY or LWJSON_TYPE_OBJECT
 * \param[in]       path: Path to search for starting this token further
 * \return          Found token on success, `NULL` otherwise
 */
static const lwjson_token_t*
prv_find(const lwjson_token_t* parent, const char* path) {
    const char* segment;
    size_t segment_len;
    uint8_t is_last, res;

    /* Get path segments */
    res = prv_create_path_segment(&path, &segment, &segment_len, &is_last);
    if (res != 0) {
        /* Check if detected an array request */
        if (*segment == '#') {
            /* Parent must be array */
            if (parent->type != LWJSON_TYPE_ARRAY) {
                return NULL;
            }

            /* Check if index requested */
            if (segment_len > 1) {
                const lwjson_token_t* tkn;
                size_t index = 0;

                /* Parse number */
                for (size_t i = 1; i < segment_len; ++i) {
                    if (segment[i] < '0' || segment[i] > '9') {
                        return NULL;
                    } else {
                        index = index * 10 + (segment[i] - '0');
                    }
                }

                /* Start from beginning */
                for (tkn = parent->u.first_child; tkn != NULL && index > 0; tkn = tkn->next, --index) {}
                if (tkn != NULL) {
                    if (is_last) {
                        return tkn;
                    } else {
                        return prv_find(tkn, path);
                    }
                }
                return NULL;
            }

            /* Scan all indexes and get first match */
            for (const lwjson_token_t* tkn = parent->u.first_child; tkn != NULL; tkn = tkn->next) {
                const lwjson_token_t* tmp = prv_find(tkn, path);
                if (tmp != NULL) {
                    return tmp;
                }
            }
        } else {
            if (parent->type != LWJSON_TYPE_OBJECT) {
                return NULL;
            }
            for (const lwjson_token_t* tkn = parent->u.first_child; tkn != NULL; tkn = tkn->next) {
                if (tkn->token_name_len == segment_len && !strncmp(tkn->token_name, segment, segment_len)) {
                    const lwjson_token_t* tmp;
                    if (is_last) {
                        return tkn;
                    }
                    tmp = prv_find(tkn, path);
                    if (tmp != NULL) {
                        return tmp;
                    }
                }
            }
        }
    }
    return NULL;
}

/**
 * \brief           Check for character after opening bracket of array or object
 * \param[in,out]   pobj: JSON string
 * \param[in]       t: Token to check for type
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
static inline lwjsonr_t
prv_check_valid_char_after_open_bracket(lwjson_int_str_t* pobj, lwjson_token_t* t) {
    lwjsonr_t res;

    /* Check next character after object open */
    res = prv_skip_blank(pobj);
    if (res != lwjsonOK) {
        return res;
    }
    if (*pobj->p == '\0' || (t->type == LWJSON_TYPE_OBJECT && (*pobj->p != '"' && *pobj->p != '}'))
        || (t->type == LWJSON_TYPE_ARRAY
            && (*pobj->p != '"' && *pobj->p != ']' && *pobj->p != '[' && *pobj->p != '{' && *pobj->p != '-'
                && (*pobj->p < '0' || *pobj->p > '9') && *pobj->p != 't' && *pobj->p != 'n' && *pobj->p != 'f'))) {
        res = lwjsonERRJSON;
    }
    return res;
}

/**
 * \brief           Setup LwJSON instance for parsing JSON strings
 * \param[in,out]   lwobj: LwJSON instance
 * \param[in]       tokens: Pointer to array of tokens used for parsing
 * \param[in]       tokens_len: Number of tokens
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_init(lwjson_t* lwobj, lwjson_token_t* tokens, size_t tokens_len) {
    LWJSON_MEMSET(lwobj, 0x00, sizeof(*lwobj));
    LWJSON_MEMSET(tokens, 0x00, sizeof(*tokens) * tokens_len);
    lwobj->tokens = tokens;
    lwobj->tokens_len = tokens_len;
    lwobj->first_token.type = LWJSON_TYPE_OBJECT;
    return lwjsonOK;
}

/**
 * \brief           Parse JSON data with length parameter
 * JSON format must be complete and must comply with RFC4627
 * \param[in,out]   lwobj: LwJSON instance
 * \param[in]       json_data: JSON string to parse
 * \param[in]       jsonČlen: JSON data length
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_parse_ex(lwjson_t* lwobj, const void* json_data, size_t json_len) {
    lwjsonr_t res = lwjsonOK;
    lwjson_token_t *t, *to;
    lwjson_int_str_t pobj = {.start = json_data, .len = json_len, .p = json_data};

    /* Check input parameters */
    if (lwobj == NULL || json_data == NULL || json_len == 0) {
        res = lwjsonERRPAR;
        goto ret;
    }

    /* set first token */
    to = &lwobj->first_token;

    /* values from very beginning */
    lwobj->flags.parsed = 0;
    lwobj->next_free_token_pos = 0;
    LWJSON_MEMSET(to, 0x00, sizeof(*to));

    /* First parse */
    res = prv_skip_blank(&pobj);
    if (res != lwjsonOK) {
        goto ret;
    }
    if (*pobj.p == '{') {
        to->type = LWJSON_TYPE_OBJECT;
    } else if (*pobj.p == '[') {
        to->type = LWJSON_TYPE_ARRAY;
    } else {
        res = lwjsonERRJSON;
        goto ret;
    }
    ++pobj.p;
    res = prv_check_valid_char_after_open_bracket(&pobj, to);
    if (res != lwjsonOK) {
        goto ret;
    }

    /* Process all characters as indicated by input user */
    while (pobj.p != NULL && *pobj.p != '\0' && (size_t)(pobj.p - pobj.start) < pobj.len) {
        /* Filter out blanks */
        res = prv_skip_blank(&pobj);
        if (res != lwjsonOK) {
            goto ret;
        }
        if (*pobj.p == ',') {
            ++pobj.p;
            continue;
        }

        /* Check if end of object or array*/
        if (*pobj.p == (to->type == LWJSON_TYPE_OBJECT ? '}' : ']')) {
            lwjson_token_t* parent = to->next;
            to->next = NULL;
            ++pobj.p;

            /* End of string if to == NULL (no parent), check if properly terminated */
            to = parent;
            if (to == NULL) {
                prv_skip_blank(&pobj);
                res = (pobj.p == NULL || *pobj.p == '\0' || (size_t)(pobj.p - pobj.start) == pobj.len) ? lwjsonOK
                                                                                                       : lwjsonERR;
                goto ret;
            }
            continue;
        }

        /* Allocate new token */
        t = prv_alloc_token(lwobj);
        if (t == NULL) {
            res = lwjsonERRMEM;
            goto ret;
        }

        /* If object type is not array, first thing is property that starts with quotes */
        if (to->type != LWJSON_TYPE_ARRAY) {
            if (*pobj.p != '"') {
                res = lwjsonERRJSON;
                goto ret;
            }
            res = prv_parse_property_name(&pobj, t);
            if (res != lwjsonOK) {
                goto ret;
            }
        }

        /* Add element to linked list */
        if (to->u.first_child == NULL) {
            to->u.first_child = t;
        } else {
            lwjson_token_t* c;
            for (c = to->u.first_child; c->next != NULL; c = c->next) {}
            c->next = t;
        }

        /* Check next character to process */
        switch (*pobj.p) {
            case '{':
            case '[':
                t->type = *pobj.p == '{' ? LWJSON_TYPE_OBJECT : LWJSON_TYPE_ARRAY;
                ++pobj.p;

                res = prv_check_valid_char_after_open_bracket(&pobj, t);
                if (res != lwjsonOK) {
                    goto ret;
                }
                t->next = to; /* Temporary saved as parent object */
                to = t;
                break;
            case '"':
                res = prv_parse_string(&pobj, &t->u.str.token_value, &t->u.str.token_value_len);
                if (res == lwjsonOK) {
                    t->type = LWJSON_TYPE_STRING;
                } else {
                    goto ret;
                }
                break;
            case 't':
                /* RFC4627 is lower-case only */
                if (strncmp(pobj.p, "true", 4) == 0) {
                    t->type = LWJSON_TYPE_TRUE;
                    pobj.p += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'f':
                /* RFC4627 is lower-case only */
                if (strncmp(pobj.p, "false", 5) == 0) {
                    t->type = LWJSON_TYPE_FALSE;
                    pobj.p += 5;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            case 'n':
                /* RFC4627 is lower-case only */
                if (strncmp(pobj.p, "null", 4) == 0) {
                    t->type = LWJSON_TYPE_NULL;
                    pobj.p += 4;
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
            default:
                if (*pobj.p == '-' || (*pobj.p >= '0' && *pobj.p <= '9')) {
                    if (prv_parse_number(&pobj, &t->type, &t->u.num_real, &t->u.num_int) != lwjsonOK) {
                        res = lwjsonERRJSON;
                        goto ret;
                    }
                } else {
                    res = lwjsonERRJSON;
                    goto ret;
                }
                break;
        }

        /* Below code is used to check characters after valid tokens */
        if (t->type == LWJSON_TYPE_ARRAY || t->type == LWJSON_TYPE_OBJECT) {
            continue;
        }

        /*
         * Check what are values after the token value
         *
         * As per RFC4627, every token value may have one or more
         * blank characters, followed by one of below options:
         *  - Comma separator for next token
         *  - End of array indication
         *  - End of object indication
         */
        res = prv_skip_blank(&pobj);
        if (res != lwjsonOK) {
            goto ret;
        }
        /* Check if valid string is availabe after */
        if (pobj.p == NULL || *pobj.p == '\0' || (*pobj.p != ',' && *pobj.p != ']' && *pobj.p != '}')) {
            res = lwjsonERRJSON;
            goto ret;
        } else if (*pobj.p == ',') { /* Check to advance to next token immediatey */
            ++pobj.p;
        }
    }
    if (to != &lwobj->first_token || (to != NULL && to->next != NULL)) {
        res = lwjsonERRJSON;
        to = NULL;
    }
    if (to != NULL) {
        if (to->type != LWJSON_TYPE_ARRAY && to->type != LWJSON_TYPE_OBJECT) {
            res = lwjsonERRJSON;
        }
        to->token_name = NULL;
        to->token_name_len = 0;
    }
ret:
    if (res == lwjsonOK) {
        lwobj->flags.parsed = 1;
    }
    return res;
}

/**
 * \brief           Parse input JSON format
 * JSON format must be complete and must comply with RFC4627
 * \param[in,out]   lwobj: LwJSON instance
 * \param[in]       json_str: JSON string to parse
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_parse(lwjson_t* lwobj, const char* json_str) {
    return lwjson_parse_ex(lwobj, json_str, strlen(json_str));
}

/**
 * \brief           Free token instances (specially used in case of dynamic memory allocation)
 * \param[in,out]   lwobj: LwJSON instance
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_free(lwjson_t* lwobj) {
    LWJSON_MEMSET(lwobj->tokens, 0x00, sizeof(*lwobj->tokens) * lwobj->tokens_len);
    lwobj->flags.parsed = 0;
    return lwjsonOK;
}

/**
 * \brief           Find first match in the given path for JSON entry
 * JSON must be valid and parsed with \ref lwjson_parse function
 * \param[in]       lwobj: JSON instance with parsed JSON string
 * \param[in]       path: Path with dot-separated entries to search for the JSON key to return
 * \return          Pointer to found token on success, `NULL` if token cannot be found
 */
const lwjson_token_t*
lwjson_find(lwjson_t* lwobj, const char* path) {
    if (lwobj == NULL || !lwobj->flags.parsed || path == NULL) {
        return NULL;
    }
    return prv_find(lwjson_get_first_token(lwobj), path);
}

/**
 * \brief           Find first match in the given path for JSON path
 * JSON must be valid and parsed with \ref lwjson_parse function
 *
 * \param[in]       lwobj: JSON instance with parsed JSON string
 * \param[in]       token: Root token to start search at.
 *                      Token must be type \ref LWJSON_TYPE_OBJECT or \ref LWJSON_TYPE_ARRAY.
 *                      Set to `NULL` to use root token of LwJSON object
 * \param[in]       path: path with dot-separated entries to search for JSON key
 * \return          Pointer to found token on success, `NULL` if token cannot be found
 */
const lwjson_token_t*
lwjson_find_ex(lwjson_t* lwobj, const lwjson_token_t* token, const char* path) {
    if (lwobj == NULL || !lwobj->flags.parsed || path == NULL) {
        return NULL;
    }
    if (token == NULL) {
        token = lwjson_get_first_token(lwobj);
    }
    if (token == NULL || (token->type != LWJSON_TYPE_ARRAY && token->type != LWJSON_TYPE_OBJECT)) {
        return NULL;
    }
    return prv_find(token, path);
}


// -----------
//   STREAM
// -----------
/**
 * \brief           Push "parent" state to the artificial stack
 * \param           jsp: JSON stream parser instance
 * \param           type: Stream type to be pushed on stack
 * \return          `1` on success, `0` otherwise
 */
static uint8_t
prv_stack_push(lwjson_stream_parser_t* jsp, lwjson_stream_type_t type) {
    if (jsp->stack_pos < LWJSON_ARRAYSIZE(jsp->stack)) {
        jsp->stack[jsp->stack_pos].type = type;
        jsp->stack[jsp->stack_pos].meta.index = 0;
        LWJSON_DEBUG(jsp, "Pushed to stack: %s\r\n", type_strings[type]);
        jsp->stack_pos++;
        return 1;
    }
    return 0;
}

/**
 * \brief           Pop value from stack (remove it) and return its value
 * \param           jsp: JSON stream parser instance
 * \return          Member of \ref lwjson_stream_type_t enumeration
 */
static lwjson_stream_type_t
prv_stack_pop(lwjson_stream_parser_t* jsp) {
    if (jsp->stack_pos > 0) {
        lwjson_stream_type_t type = jsp->stack[--jsp->stack_pos].type;

        jsp->stack[jsp->stack_pos].type = LWJSON_STREAM_TYPE_NONE;
        LWJSON_DEBUG(jsp, "Popped from stack: %s\r\n", type_strings[type]);

        /* Take care of array to indicate number of entries */
        if (jsp->stack_pos > 0 && jsp->stack[jsp->stack_pos - 1].type == LWJSON_STREAM_TYPE_ARRAY) {
            jsp->stack[jsp->stack_pos - 1].meta.index++;
        }
        return type;
    }
    return LWJSON_STREAM_TYPE_NONE;
}

/**
 * \brief           Get top type value currently on the stack
 * \param           jsp: JSON stream parser instance
 * \return          Member of \ref lwjson_stream_type_t enumeration
 */
static lwjson_stream_type_t
prv_stack_get_top(lwjson_stream_parser_t* jsp) {
    if (jsp->stack_pos > 0) {
        return jsp->stack[jsp->stack_pos - 1].type;
    }
    return LWJSON_STREAM_TYPE_NONE;
}

/**
 * \brief           Initialize LwJSON stream object before parsing takes place
 * \param[in,out]   jsp: Stream JSON structure
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_stream_init(lwjson_stream_parser_t* jsp, lwjson_stream_parser_callback_fn evt_fn) {
    LWJSON_MEMSET(jsp, 0x00, sizeof(*jsp));
    jsp->parse_state = LWJSON_STREAM_STATE_WAITINGFIRSTCHAR;
    jsp->evt_fn = evt_fn;
    jsp->user_data = NULL;
    return lwjsonOK;
}

/**
 * \brief           Reset LwJSON stream structure
 *
 * \param[in,out]   jsp: LwJSON stream parser
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_stream_reset(lwjson_stream_parser_t* jsp) {
    jsp->parse_state = LWJSON_STREAM_STATE_WAITINGFIRSTCHAR;
    jsp->stack_pos = 0;
    return lwjsonOK;
}

/**
 * \brief           Set user_data in stream parser
 *
 * \param[in,out]   jsp: LwJSON stream parser
 * \param[in]       user_data: user data
 * \return          \ref lwjsonOK on success, member of \ref lwjsonr_t otherwise
 */
lwjsonr_t
lwjson_stream_set_user_data(lwjson_stream_parser_t* jsp, void* user_data) {
    jsp->user_data = user_data;
    return lwjsonOK;
}

/**
 * \brief           Get user_data in stream parser
 *
 * \param[in]       jsp: LwJSON stream parser
 * \return          pointer to user data
 */
void*
lwjson_stream_get_user_data(lwjson_stream_parser_t* jsp) {
    return jsp->user_data;
}

/**
 * \brief           Parse JSON string in streaming mode
 * \param[in,out]   jsp: Stream JSON structure
 * \param[in]       chr: Character to parse
 * \return          \ref lwjsonSTREAMWAITFIRSTCHAR when stream did not start parsing since no valid start character has been received
 * \return          \ref lwjsonSTREAMINPROG if parsing is in progress and no hard error detected
 * \return          \ref lwjsonSTREAMDONE when valid JSON was detected and stack level reached back `0` level
 * \return          \ref One of enumeration otherwise
 */
lwjsonr_t
lwjson_stream_parse(lwjson_stream_parser_t* jsp, char chr) {
    /* Get first character first */
    if (jsp->parse_state == LWJSON_STREAM_STATE_WAITINGFIRSTCHAR && chr != '{' && chr != '[') {
        return prv_is_space_char_ext(chr) ? lwjsonSTREAMWAITFIRSTCHAR : lwjsonERRJSON;
    }

start_over:
    /* Determine what to do from parsing state */
    switch (jsp->parse_state) {

        /*
         * Waiting for very first valid characters,
         * that is used to indicate start of JSON stream
         */
        case LWJSON_STREAM_STATE_WAITINGFIRSTCHAR:
        case LWJSON_STREAM_STATE_EXPECTING_COMMA_OR_END:
        case LWJSON_STREAM_STATE_PARSING: {
            /* Ignore whitespace chars */
            if (prv_is_space_char_ext(chr)) {
                break;

                /* Determine value separator */
            } else if (chr == ',') {
                if (jsp->parse_state == LWJSON_STREAM_STATE_EXPECTING_COMMA_OR_END) {
                    jsp->parse_state = LWJSON_STREAM_STATE_PARSING;
                } else {
                    LWJSON_DEBUG(jsp, "ERROR - ',' can only follow value\r\n");
                    return lwjsonERRJSON;
                }

                /* Determine end of object or an array */
            } else if (chr == '}' || chr == ']') {
                lwjson_stream_type_t type = prv_stack_get_top(jsp);

                /*
                 * If it is a key last entry on closing area,
                 * it is an error - an example: {"key":}
                 */
                if (type == LWJSON_STREAM_TYPE_KEY) {
                    LWJSON_DEBUG(jsp, "ERROR - key should not be followed by ] without value for a key\r\n");
                    return lwjsonERRJSON;
                }

                /*
                 * Check if closing character matches stack value
                 * Avoid cases like: {"key":"value"] or ["v1", "v2", "v3"}
                 */
                if ((chr == '}' && type != LWJSON_STREAM_TYPE_OBJECT)
                    || (chr == ']' && type != LWJSON_STREAM_TYPE_ARRAY)) {
                    LWJSON_DEBUG(jsp, "ERROR - closing character '%c' does not match stack element \"%s\"\r\n", chr,
                                 type_strings[type]);
                    return lwjsonERRJSON;
                }

                /* Now remove the array or object from stack */
                if (prv_stack_pop(jsp) == LWJSON_STREAM_TYPE_NONE) {
                    return lwjsonERRJSON;
                }

                /*
                 * Check if above is a key type
                 * and remove it too as we finished with processing of potential case.
                 *
                 * {"key":{"abc":1}} - remove "key" part
                 */
                if (prv_stack_get_top(jsp) == LWJSON_STREAM_TYPE_KEY) {
                    prv_stack_pop(jsp);
                }
                SEND_EVT(jsp, chr == '}' ? LWJSON_STREAM_TYPE_OBJECT_END : LWJSON_STREAM_TYPE_ARRAY_END);

                /* If that is the end of JSON */
                if (jsp->stack_pos == 0) {
                    lwjson_stream_reset(jsp);
                    return lwjsonSTREAMDONE;
                }

                jsp->parse_state = LWJSON_STREAM_STATE_EXPECTING_COMMA_OR_END;

                /* If comma or end was expected, then it is already error here */
            } else if (jsp->parse_state == LWJSON_STREAM_STATE_EXPECTING_COMMA_OR_END) {
                LWJSON_DEBUG(jsp, "ERROR - ',', '}' or ']' was expected\r\n");
                return lwjsonERRJSON;

                /* Determine start of string - can be key or regular string (in array or after key) */
            } else if (chr == '"') {
#if defined(LWJSON_DEV)
                lwjson_stream_type_t type = prv_stack_get_top(jsp);
                if (type == LWJSON_STREAM_TYPE_OBJECT) {
                    LWJSON_DEBUG(jsp, "Start of string parsing - expected key name in an object\r\n");
                } else if (type == LWJSON_STREAM_TYPE_KEY) {
                    LWJSON_DEBUG(jsp,
                                 "Start of string parsing - string value associated to previous key in an object\r\n");
                } else if (type == LWJSON_STREAM_TYPE_ARRAY) {
                    LWJSON_DEBUG(jsp, "Start of string parsing - string entry in an array\r\n");
                }
#endif /* defined(LWJSON_DEV) */
                jsp->parse_state = LWJSON_STREAM_STATE_PARSING_STRING;
                LWJSON_MEMSET(&jsp->data.str, 0x00, sizeof(jsp->data.str));

            } else if (prv_stack_get_top(jsp) == LWJSON_STREAM_TYPE_OBJECT) {
                /* Key must be before value */
                LWJSON_DEBUG(jsp, "ERROR - key must be before value\r\n");
                return lwjsonERRJSON;

                /* Determine start of object or an array */
            } else if (chr == '{' || chr == '[') {
                /* Reset stack pointer if this character came from waiting for first character */
                if (jsp->parse_state == LWJSON_STREAM_STATE_WAITINGFIRSTCHAR) {
                    jsp->stack_pos = 0;
                }
                if (!prv_stack_push(jsp, chr == '{' ? LWJSON_STREAM_TYPE_OBJECT : LWJSON_STREAM_TYPE_ARRAY)) {
                    LWJSON_DEBUG(jsp, "Cannot push object/array to stack\r\n");
                    return lwjsonERRMEM;
                }
                jsp->parse_state = LWJSON_STREAM_STATE_PARSING;
                SEND_EVT(jsp, chr == '{' ? LWJSON_STREAM_TYPE_OBJECT : LWJSON_STREAM_TYPE_ARRAY);

                /* Check if this is start of number or "true", "false" or "null" */
            } else if (chr == '-' || (chr >= '0' && chr <= '9') || chr == 't' || chr == 'f' || chr == 'n') {
                if (prv_stack_get_top(jsp) == LWJSON_STREAM_TYPE_OBJECT) {
                    LWJSON_DEBUG(jsp, "ERROR - key must be before value (primitive)\r\n");
                    /* Key must be before value */
                    return lwjsonERRJSON;
                }

                LWJSON_DEBUG(jsp, "Start of primitive parsing parsing - %s, First char: %c\r\n",
                             (chr == '-' || (chr >= '0' && chr <= '9')) ? "number" : "true,false,null", chr);
                jsp->parse_state = LWJSON_STREAM_STATE_PARSING_PRIMITIVE;
                LWJSON_MEMSET(&jsp->data.prim, 0x00, sizeof(jsp->data.prim));
                jsp->data.prim.buff[jsp->data.prim.buff_pos++] = chr;

                /* Wrong char */
            } else {
                LWJSON_DEBUG(jsp, "ERROR - wrong char %c\r\n", chr);
                return lwjsonERRJSON;
            }
            break;
        }

        /* Check for end of key character i.e. name separator ':' */
        case LWJSON_STREAM_STATE_EXPECTING_COLON:
            /* Ignore whitespace chars */
            if (prv_is_space_char_ext(chr)) {
                break;
            } else if (chr == ':') {
                jsp->parse_state = LWJSON_STREAM_STATE_PARSING;
            } else {
                LWJSON_DEBUG(jsp, "ERROR - expecting ':'\r\n");
                return lwjsonERRJSON;
            }
            break;

        /*
         * Parse any type of string in a sequence
         *
         * It is used for key or string in an object or an array
         */
        case LWJSON_STREAM_STATE_PARSING_STRING: {
            lwjson_stream_type_t type = prv_stack_get_top(jsp);

            /*
             * Quote character may trigger end of string,
             * or if backslasled before - it is part of string
             *
             * TODO: Handle backslash
             */
            if (chr == '"' && jsp->prev_c != '\\') {
#if defined(LWJSON_DEV)
                if (type == LWJSON_STREAM_TYPE_OBJECT) {
                    LWJSON_DEBUG(jsp, "End of string parsing - object key name: \"%s\"\r\n", jsp->data.str.buff);
                } else if (type == LWJSON_STREAM_TYPE_KEY) {
                    LWJSON_DEBUG(
                        jsp, "End of string parsing - string value associated to previous key in an object: \"%s\"\r\n",
                        jsp->data.str.buff);
                } else if (type == LWJSON_STREAM_TYPE_ARRAY) {
                    LWJSON_DEBUG(jsp, "End of string parsing - an array string entry: \"%s\"\r\n", jsp->data.str.buff);
                }
#endif /* defined(LWJSON_DEV) */

                /* Set is_last to 1 as this is the last part of this string token */
                jsp->data.str.is_last = 1;

                /*
                 * When top of stack is object - string is treated as a key
                 * When top of stack is a key - string is a value for a key - notify user and pop the value for key
                 * When top of stack is an array - string is one type - notify user and don't do anything
                 */
                jsp->parse_state = LWJSON_STREAM_STATE_EXPECTING_COMMA_OR_END;
                if (type == LWJSON_STREAM_TYPE_OBJECT) {
                    SEND_EVT(jsp, LWJSON_STREAM_TYPE_KEY);
                    if (prv_stack_push(jsp, LWJSON_STREAM_TYPE_KEY)) {
                        size_t len = jsp->data.str.buff_pos;
                        if (len > (sizeof(jsp->stack[0].meta.name) - 1)) {
                            len = sizeof(jsp->stack[0].meta.name) - 1;
                        }
                        LWJSON_MEMCPY(jsp->stack[jsp->stack_pos - 1].meta.name, jsp->data.str.buff, len);
                        jsp->stack[jsp->stack_pos - 1].meta.name[len] = '\0';
                    } else {
                        LWJSON_DEBUG(jsp, "Cannot push key to stack\r\n");
                        return lwjsonERRMEM;
                    }
                    jsp->parse_state = LWJSON_STREAM_STATE_EXPECTING_COLON;
                } else if (type == LWJSON_STREAM_TYPE_KEY) {
                    SEND_EVT(jsp, LWJSON_STREAM_TYPE_STRING);
                    prv_stack_pop(jsp);
                    /* Next character to wait for is either space or comma or end of object */
                } else if (type == LWJSON_STREAM_TYPE_ARRAY) {
                    SEND_EVT(jsp, LWJSON_STREAM_TYPE_STRING);
                    jsp->stack[jsp->stack_pos - 1].meta.index++;
                }
            } else {
                /* TODO: Check other backslash elements */
                jsp->data.str.buff[jsp->data.str.buff_pos++] = chr;
                jsp->data.str.buff_total_pos++;

                /* Handle buffer "overflow" */
                if (jsp->data.str.buff_pos >= (LWJSON_CFG_STREAM_STRING_MAX_LEN - 1)) {
                    jsp->data.str.buff[jsp->data.str.buff_pos] = '\0';

                    /*
                     * - For array or key types - following one is always string
                     * - For object type - character is key
                     */
                    SEND_EVT(jsp, (type == LWJSON_STREAM_TYPE_KEY || type == LWJSON_STREAM_TYPE_ARRAY)
                                      ? LWJSON_STREAM_TYPE_STRING
                                      : LWJSON_STREAM_TYPE_KEY);
                    jsp->data.str.buff_pos = 0;
                }
            }
            break;
        }

        /*
         * Parse any type of primitive that is not a string.
         *
         * true, false, null or any number primitive
         */
        case LWJSON_STREAM_STATE_PARSING_PRIMITIVE: {
            /* Any character except space, comma, or end of array/object are valid */
            if (!prv_is_space_char_ext(chr) && chr != ',' && chr != ']' && chr != '}') {
                if (jsp->data.prim.buff_pos < sizeof(jsp->data.prim.buff) - 1) {
                    jsp->data.prim.buff[jsp->data.prim.buff_pos++] = chr;
                } else {
                    LWJSON_DEBUG(jsp, "Buffer overflow for primitive\r\n");
                    return lwjsonERRJSON;
                }
            } else {
                lwjson_stream_type_t type = prv_stack_get_top(jsp);

#if defined(LWJSON_DEV)
                if (type == LWJSON_STREAM_TYPE_KEY) {
                    LWJSON_DEBUG(
                        jsp,
                        "End of primitive parsing - string value associated to previous key in an object: \"%s\"\r\n",
                        jsp->data.prim.buff);
                } else if (type == LWJSON_STREAM_TYPE_ARRAY) {
                    LWJSON_DEBUG(jsp, "End of primitive parsing - an array string entry: \"%s\"\r\n",
                                 jsp->data.prim.buff);
                }
#endif /* defined(LWJSON_DEV) */

                /*
                 * This is the end of primitive parsing
                 *
                 * It is assumed that buffer for primitive can handle at least
                 * true, false, null or all number characters (that being real or int number)
                 */
                if (jsp->data.prim.buff_pos == 4 && strncmp(jsp->data.prim.buff, "true", 4) == 0) {
                    LWJSON_DEBUG(jsp, "Primitive parsed as %s\r\n", "true");
                    SEND_EVT(jsp, LWJSON_STREAM_TYPE_TRUE);
                } else if (jsp->data.prim.buff_pos == 4 && strncmp(jsp->data.prim.buff, "null", 4) == 0) {
                    LWJSON_DEBUG(jsp, "Primitive parsed as %s\r\n", "null");
                    SEND_EVT(jsp, LWJSON_STREAM_TYPE_NULL);
                } else if (jsp->data.prim.buff_pos == 5 && strncmp(jsp->data.prim.buff, "false", 5) == 0) {
                    LWJSON_DEBUG(jsp, "Primitive parsed as %s\r\n", "false");
                    SEND_EVT(jsp, LWJSON_STREAM_TYPE_FALSE);
                } else if (jsp->data.prim.buff[0] == '-'
                           || (jsp->data.prim.buff[0] >= '0' && jsp->data.prim.buff[0] <= '9')) {
                    LWJSON_DEBUG(jsp, "Primitive parsed - number\r\n");
                    SEND_EVT(jsp, LWJSON_STREAM_TYPE_NUMBER);
                } else {
                    LWJSON_DEBUG(jsp, "Invalid primitive type. Got: %s\r\n", jsp->data.prim.buff);
                    return lwjsonERRJSON;
                }
                if (type == LWJSON_STREAM_TYPE_KEY) {
                    prv_stack_pop(jsp);
                } else if (type == LWJSON_STREAM_TYPE_ARRAY) {
                    jsp->stack[jsp->stack_pos - 1].meta.index++;
                }

                /*
                 * Received character is not part of the primitive and must be processed again
                 */
                jsp->parse_state = LWJSON_STREAM_STATE_EXPECTING_COMMA_OR_END;
                goto start_over;
            }
            break;
        }

        default: break;
    }
    jsp->prev_c = chr; /* Save current c as previous for next round */
    return lwjsonSTREAMINPROG;
}
