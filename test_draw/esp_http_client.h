// mockup file to simulate the functionality of the esp_http_client

#ifndef _ESP_HTTP_CLIENT_H
#define _ESP_HTTP_CLIENT_H

typedef int esp_err_t;
#define ESP_OK          0       /*!< esp_err_t value indicating success (no error) */
#define ESP_FAIL        -1      /*!< Generic esp_err_t code indicating failure */

typedef struct esp_http_client *esp_http_client_handle_t;
typedef struct esp_http_client_event *esp_http_client_event_handle_t;

struct esp_http_client {
    int bla;
};

/**
 * @brief   HTTP Client events id
 */
typedef enum {
    HTTP_EVENT_ERROR = 0,       /*!< This event occurs when there are any errors during execution */
    HTTP_EVENT_ON_CONNECTED,    /*!< Once the HTTP has been connected to the server, no data exchange has been performed */
    HTTP_EVENT_HEADERS_SENT,     /*!< After sending all the headers to the server */
    HTTP_EVENT_HEADER_SENT = HTTP_EVENT_HEADERS_SENT, /*!< This header has been kept for backward compatability
                                                           and will be deprecated in future versions esp-idf */
    HTTP_EVENT_ON_HEADER,       /*!< Occurs when receiving each header sent from the server */
    HTTP_EVENT_ON_DATA,         /*!< Occurs when receiving data from the server, possibly multiple portions of the packet */
    HTTP_EVENT_ON_FINISH,       /*!< Occurs when finish a HTTP session */
    HTTP_EVENT_DISCONNECTED,    /*!< The connection has been disconnected */
    HTTP_EVENT_REDIRECT,        /*!< Intercepting HTTP redirects to handle them manually */
} esp_http_client_event_id_t;


/**
 * @brief      HTTP Client events data
 */
typedef struct esp_http_client_event {
    esp_http_client_event_id_t event_id;    /*!< event_id, to know the cause of the event */
    esp_http_client_handle_t client;        /*!< esp_http_client_handle_t context */
    void *data;                             /*!< data of the event */
    int data_len;                           /*!< data length of data */
    void *user_data;                        /*!< user_data context, from esp_http_client_config_t user_data */
    char *header_key;                       /*!< For HTTP_EVENT_ON_HEADER event_id, it's store current http header key */
    char *header_value;                     /*!< For HTTP_EVENT_ON_HEADER event_id, it's store current http header value */
} esp_http_client_event_t;

#endif
