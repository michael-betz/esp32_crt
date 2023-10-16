// demo the drawing engine on a PC using SDL2
#include <stdbool.h>
#include <SDL2/SDL.h>
#include <time.h>
#include <math.h>
#include <limits.h>
#include "draw.h"
#include "fast_sin.h"

#define DISPLAY_WIDTH 1024
#define DISPLAY_HEIGHT DISPLAY_WIDTH
#define ZOOM 1

#define MAX_DL_SIZE 4096
static draw_list_t dl[MAX_DL_SIZE];
static unsigned n_dl = 0;

SDL_Renderer *rr = NULL;
SDL_Window* window = NULL;

void read_csv(char *fName)
{
    FILE* f = fopen(fName, "r");
    int a, b, c, d;
    draw_list_t *p = dl;
    char line[1024];
    char *tmp = NULL;

    n_dl = 0;
    while (fgets(line, 1024, f)) {
        if (line[0] == '#' || strlen(line) < 8)
            continue;

        tmp = strtok(line, ",");
        if (tmp == NULL) continue;
        a = atoi(tmp);

        tmp = strtok(NULL, ",");
        if (tmp == NULL) continue;
        b = atoi(tmp);

        tmp = strtok(NULL, ",");
        if (tmp == NULL) continue;
        c = atoi(tmp);

        tmp = strtok(NULL, ",");
        if (tmp == NULL) continue;
        d = atoi(tmp);

        // printf("%d %d %d %d\n", a, b, c, d);
        p->type = a;
        p->density = b;
        p->x = c << FP;
        p->y = d << FP;
        p++;
        n_dl++;

        if (n_dl >= MAX_DL_SIZE)
            break;
    }
    printf("read %d entries\n", n_dl);
    fclose(f);
}

// Draw into framebuffer
void push_sample(int16_t val_a, int16_t val_b, int16_t val_c, int16_t val_d)
{
    int x = (val_a + 0x4000) >> FP;
    int y = (-val_b + 0x4000) >> FP;

    SDL_Rect rect = {x / 2, y / 2, 3, 3};
    SDL_RenderFillRect(rr, &rect);
}

static void init_sdl()
{
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "could not initialize sdl2: %s\n", SDL_GetError());
        return;
    }

    if (SDL_CreateWindowAndRenderer(
        DISPLAY_WIDTH, DISPLAY_HEIGHT, SDL_WINDOW_RESIZABLE, &window, &rr
    )) {
        fprintf(stderr, "could not create window: %s\n", SDL_GetError());
        return;
    }

    SDL_RenderSetScale(rr, ZOOM, ZOOM);
    SDL_SetRenderDrawBlendMode(rr, SDL_BLENDMODE_ADD);
}

int main(int argc, char* args[])
{
    if (argc != 2) {
        printf("try %s draw_list.csv\n", args[0]);
        return -1;
    }
    printf("reading %s\n", args[1]);
    read_csv(args[1]);

    init_lut();
    init_sdl();

    while (1) {
        SDL_Event e;
        bool isExit = false;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    isExit = true;
                    break;
                // case SDL_KEYDOWN:
                //     switch(e.key.keysym.sym) {
                //         case SDLK_LEFT:
                //             break;
                //         case SDLK_RIGHT:
                //             break;
                //         case SDLK_DOWN:
                //             break;
                //         case SDLK_UP:
                //             break;
                //     }
                //     break;
            }
        }
        if (isExit)
            break;

        SDL_SetRenderDrawColor(rr, 0x00, 0x00, 0x00, 0xFF);
        SDL_RenderClear(rr);

        SDL_SetRenderDrawColor(rr, 0x00, 0xFF, 0x00, 0x60);
        push_list(dl, n_dl);

        SDL_RenderPresent(rr);
        SDL_Delay(500);
    }

    SDL_DestroyRenderer(rr);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

