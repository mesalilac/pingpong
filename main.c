#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH  800
#define HEIGHT 600

#define BACKGROUND_COLOR 50, 50, 50, 255

#define MAX_SCORE 10

#define PLAYER_SPEED     6
#define PLAYER_WIDTH     10
#define PLAYER_HEIGHT    120
#define PLAYER_X_PADDING 10
#define PLAYER_Y_PADDING 10
#define PLAYER_COLOR     250, 250, 250, 255

#define BALL_SPEED  4
#define BALL_WIDTH  20
#define BALL_HEIGHT 20
#define BALL_COLOR  200, 40, 40, 255

#define FONT_NAME "./FiraCode-Regular.ttf"

typedef enum
{
    PLAYING,
    WIN
} GameState;

typedef struct
{
    int x;
    int y;
} Vector;

typedef struct
{
    bool up;
    bool down;
} Keys;

typedef struct
{
    SDL_Rect rect;
    int score;
    Keys keys;
} Player;

typedef struct
{
    SDL_Rect rect;
    Vector velocity;
} Ball;

void render_score(SDL_Renderer *ren, TTF_Font *font, int player_num, int score)
{
    SDL_Color color = {255, 255, 255, 255};

    char *text = malloc(50 * sizeof(char));
    sprintf(text, "%i", score);

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(ren, surface);
    SDL_Rect score_rect  = {
         .x = WIDTH / 2,
         .y = 0,
         .w = surface->w,
         .h = surface->h,
    };

    if (player_num == 1)
        score_rect.x = (score_rect.x - surface->w) - 20;
    else if (player_num == 2)
        score_rect.x = score_rect.x + 20;

    SDL_RenderCopy(ren, texture, NULL, &score_rect);
    SDL_SetRenderDrawColor(ren, BACKGROUND_COLOR);
    SDL_RenderDrawRect(ren, &score_rect);

    free(text);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void init_game_state(
    Player *player1,
    Player *player2,
    Ball *ball,
    GameState *game_state,
    int *winning_player
)
{
    player1->score  = 0;
    player2->score  = 0;
    *winning_player = 0;
    *game_state     = PLAYING;

    // Reset position
    player1->rect.x    = PLAYER_X_PADDING;
    player1->rect.y    = PLAYER_HEIGHT + PLAYER_Y_PADDING;
    player1->keys.up   = false;
    player1->keys.down = false;

    player2->rect.x    = WIDTH - PLAYER_WIDTH - PLAYER_X_PADDING;
    player2->rect.y    = PLAYER_HEIGHT + PLAYER_Y_PADDING;
    player2->keys.up   = false;
    player2->keys.down = false;

    ball->rect.x     = WIDTH / 2 - BALL_WIDTH;
    ball->rect.y     = HEIGHT / 2 - BALL_HEIGHT;
    ball->velocity.x = -1;
    ball->velocity.y = -1;
}

int main()
{
    if (SDL_Init(SDL_INIT_VIDEO) == -1)
    {
        fprintf(stderr, "ERROR: Failed to Init video: %s", SDL_GetError());
        exit(1);
    }

    SDL_Window *win = SDL_CreateWindow("Ping Pong", 0, 0, WIDTH, HEIGHT, 0);
    if (win == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create window: %s", SDL_GetError());
        exit(1);
    }

    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);
    if (ren == NULL)
    {
        fprintf(stderr, "ERROR: Failed to create renderer: %s", SDL_GetError());
        exit(1);
    }

    if (TTF_Init() == -1)
    {
        fprintf(stderr, "ERROR: Failed to init TTF: %s", SDL_GetError());
        exit(1);
    }

    TTF_Font *score_font = TTF_OpenFont(FONT_NAME, 18);
    if (score_font == NULL)
    {
        fprintf(
            stderr,
            "ERROR: Failed to open font %s: %s",
            FONT_NAME,
            SDL_GetError()
        );
        exit(1);
    }

    TTF_Font *title_font = TTF_OpenFont(FONT_NAME, 24);
    if (title_font == NULL)
    {
        fprintf(
            stderr,
            "ERROR: Failed to open font %s: %s",
            FONT_NAME,
            SDL_GetError()
        );
        exit(1);
    }

    TTF_Font *tip_font = TTF_OpenFont(FONT_NAME, 18);
    if (tip_font == NULL)
    {
        fprintf(
            stderr,
            "ERROR: Failed to open font %s: %s",
            FONT_NAME,
            SDL_GetError()
        );
        exit(1);
    }

    bool is_running      = true;
    GameState game_state = PLAYING;
    int winning_player   = 0;
    SDL_Event event;

    Player player1 = {
        .rect =
            {.x = PLAYER_X_PADDING,
                   .y = PLAYER_HEIGHT + PLAYER_Y_PADDING,
                   .w = PLAYER_WIDTH,
                   .h = PLAYER_HEIGHT},
        .score = 0,
        .keys  = {.up = false,    .down = false                 }
    };

    Player player2 = {
        .rect =
            {.x = WIDTH - PLAYER_WIDTH - PLAYER_X_PADDING,
                   .y = PLAYER_HEIGHT + PLAYER_Y_PADDING,
                   .w = PLAYER_WIDTH,
                   .h = PLAYER_HEIGHT},
        .score = 0,
        .keys  = {.up = false,                           .down = false                 }
    };

    Ball ball = {
        .rect =
            {.x = WIDTH / 2 - BALL_WIDTH,
                   .y = HEIGHT / 2 - BALL_HEIGHT,
                   .w = BALL_WIDTH,
                   .h = BALL_HEIGHT},
        .velocity = {.x = -1,            .y = -1           }
    };

    while (is_running)
    {
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                case SDL_QUIT:
                    is_running = false;
                    break;
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == 'w')
                        player1.keys.up = true;
                    if (event.key.keysym.sym == 's')
                        player1.keys.down = true;

                    if (event.key.keysym.sym == SDLK_UP)
                        player2.keys.up = true;
                    if (event.key.keysym.sym == SDLK_DOWN)
                        player2.keys.down = true;

                    if (event.key.keysym.sym == SDLK_SPACE && game_state == WIN)
                    {
                        init_game_state(
                            &player1,
                            &player2,
                            &ball,
                            &game_state,
                            &winning_player
                        );
                    }

                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym == 'w')
                        player1.keys.up = false;
                    if (event.key.keysym.sym == 's')
                        player1.keys.down = false;

                    if (event.key.keysym.sym == SDLK_UP)
                        player2.keys.up = false;
                    if (event.key.keysym.sym == SDLK_DOWN)
                        player2.keys.down = false;

                    break;
            }
        }

        SDL_SetRenderDrawColor(ren, BACKGROUND_COLOR);
        SDL_RenderClear(ren);

        if (game_state == PLAYING)
        {
            // Draw divider
            SDL_SetRenderDrawColor(ren, 30, 30, 30, 255);
            SDL_RenderDrawLine(ren, WIDTH / 2, 0, WIDTH / 2, HEIGHT);

            if (player1.keys.up)
                player1.rect.y -= PLAYER_SPEED;
            if (player1.keys.down)
                player1.rect.y += PLAYER_SPEED;

            if (player2.keys.up)
                player2.rect.y -= PLAYER_SPEED;
            if (player2.keys.down)
                player2.rect.y += PLAYER_SPEED;

            if (SDL_HasIntersection(&ball.rect, &player1.rect))
            {
                if (ball.rect.x > player1.rect.x)
                    ball.velocity.x = 1;
            }
            if (SDL_HasIntersection(&ball.rect, &player2.rect))
            {
                if (ball.rect.x < player2.rect.x)
                    ball.velocity.x = -1;
            }

            // ball movement
            if (ball.rect.x < 0)
            {
                ball.velocity.x = 1;
                player2.score++;
                if (player2.score == MAX_SCORE)
                {
                    game_state     = WIN;
                    winning_player = 2;
                }
            }
            if (ball.rect.x + BALL_WIDTH > WIDTH)
            {
                ball.velocity.x = -1;
                player1.score++;
                if (player1.score == MAX_SCORE)
                {
                    game_state     = WIN;
                    winning_player = 1;
                }
            }
            if (ball.rect.y < 0)
                ball.velocity.y = 1;
            if (ball.rect.y + BALL_HEIGHT > HEIGHT)
                ball.velocity.y = -1;

            ball.rect.x += ball.velocity.x * BALL_SPEED;
            ball.rect.y += ball.velocity.y * BALL_SPEED;

            render_score(ren, score_font, 1, player1.score);
            render_score(ren, score_font, 2, player2.score);

            SDL_SetRenderDrawColor(ren, PLAYER_COLOR);
            SDL_RenderFillRect(ren, &player1.rect);

            SDL_SetRenderDrawColor(ren, PLAYER_COLOR);
            SDL_RenderFillRect(ren, &player2.rect);

            SDL_SetRenderDrawColor(ren, BALL_COLOR);
            SDL_RenderFillRect(ren, &ball.rect);
        }
        else if (game_state == WIN)
        {

            {
                SDL_Color title_color = {255, 255, 255, 255};

                char *text = malloc(50 * sizeof(char));
                sprintf(text, "Player %i won!", winning_player);

                SDL_Surface *surface =
                    TTF_RenderText_Solid(title_font, text, title_color);
                SDL_Texture *texture =
                    SDL_CreateTextureFromSurface(ren, surface);
                SDL_Rect title_rect = {
                    .x = WIDTH / 2 - surface->w / 2,
                    .y = HEIGHT / 2 - surface->h / 2,
                    .w = surface->w,
                    .h = surface->h,
                };

                SDL_RenderCopy(ren, texture, NULL, &title_rect);
                SDL_SetRenderDrawColor(ren, BACKGROUND_COLOR);
                SDL_RenderDrawRect(ren, &title_rect);

                free(text);
                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }

            {
                SDL_Color tip_color = {200, 200, 200, 255};

                SDL_Surface *surface = TTF_RenderText_Solid(
                    tip_font, "Press Space to play again!", tip_color
                );
                SDL_Texture *texture =
                    SDL_CreateTextureFromSurface(ren, surface);
                SDL_Rect tip_rect = {
                    .x = WIDTH / 2 - surface->w / 2,
                    .y = HEIGHT - surface->h,
                    .w = surface->w,
                    .h = surface->h,
                };

                SDL_RenderCopy(ren, texture, NULL, &tip_rect);
                SDL_SetRenderDrawColor(ren, BACKGROUND_COLOR);
                SDL_RenderDrawRect(ren, &tip_rect);

                SDL_FreeSurface(surface);
                SDL_DestroyTexture(texture);
            }
        }

        SDL_RenderPresent(ren);
    }

    TTF_CloseFont(score_font);
    TTF_CloseFont(title_font);
    TTF_CloseFont(tip_font);
    SDL_DestroyWindow(win);
    SDL_DestroyRenderer(ren);
    SDL_Quit();
}
