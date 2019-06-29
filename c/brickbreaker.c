#include <stdio.h>
#include <math.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include "./deps/vec/vec.h"

const int SCREEN_WIDTH = 800; 
const int SCREEN_HEIGHT = 600;

const int BALL_RADIUS = 10;
const int BALL_SPEED = 15;

const int PADDLE_WIDTH = 100;
const int PADDLE_THICKNESS = 17;
const int PADDLE_Y_OFFSET = 25;
const int PADDLE_SPEED = 9;

const int MAX_SPEED = 20;
const int MIN_Y_VEL = 4;

const int BRICK_COLS = 10;
const int BRICK_ROWS = 5;
const int BRICK_HEIGHT = 20;
const int BRICK_X_OFFSET = 50;
const int BRICK_Y_OFFSET = 60;
const int BRICK_GAP = 2;

const int COLLISION_TOLERANCE = 7;

typedef struct Point{
    int x;
    int y;
} Point;

struct Point new_point(int x, int y){
    struct Point point;
    point.x = x;
    point.y = y;
    return point;
}

typedef struct Ball{
    struct Point pos;
    struct Point vel;
} Ball;

Ball new_ball(Point pos, Point vel){
    struct Ball ball;
    ball.pos = pos;
    ball.vel = vel;
    return ball;
}

enum Direction{
    Left,
    Right,
    None,
};

typedef vec_t(Point) brick_vec_t; //defines the struct body_vec_t as a vector of bodies

int main(){
    srand(time(NULL));

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;

    int paddle_x = SCREEN_WIDTH/2;

    Point pos = new_point(SCREEN_WIDTH/2, SCREEN_HEIGHT/5 * 3);
    Point vel = new_point((rand() % BALL_SPEED) - BALL_SPEED/2, (rand() % BALL_SPEED) - BALL_SPEED/2);
    if (vel.y <= MIN_Y_VEL) vel.y = MIN_Y_VEL;

    Ball ball = new_ball(pos, vel);

    brick_vec_t bricks;
    vec_init(&bricks);
    
    int brick_total_width = SCREEN_WIDTH - (BRICK_X_OFFSET * 2);
    int brick_width = brick_total_width/BRICK_COLS;

    for (int r = 0; r < BRICK_ROWS; r++){
        for (int c = 0; c < BRICK_COLS; c++){
            Point brick;
            brick.y = BRICK_Y_OFFSET + ((BRICK_HEIGHT + BRICK_GAP) * r);
            brick.x = BRICK_X_OFFSET + ((brick_width + BRICK_GAP) * c);
            
            vec_push(&bricks, brick);
        }
    }


    if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() ); 
    else{
        window = SDL_CreateWindow("SDL Window",
                SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                SCREEN_WIDTH, SCREEN_HEIGHT,
                SDL_WINDOW_SHOWN);
        if (window == NULL){
            printf("Error creating window: %s\n", SDL_GetError());
        }else{//SDL initializes correctly
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

            SDL_Event e;

            enum Direction paddle_direction;


            while(1){
                int ball_top = ball.pos.y - BALL_RADIUS;
                int ball_bottom = ball.pos.y + BALL_RADIUS;
                int ball_left = ball.pos.x - BALL_RADIUS;
                int ball_right = ball.pos.x + BALL_RADIUS;
                int ball_center_x = ball.pos.x;
                int ball_center_y = ball.pos.y;

                paddle_direction = None;

                SDL_PollEvent(&e);
                SDL_PumpEvents();
                if(e.type == SDL_QUIT){
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    return 0;
                }

                const Uint8 *keys = SDL_GetKeyboardState(NULL);

                if (keys[SDL_SCANCODE_A]) { paddle_x -= PADDLE_SPEED; paddle_direction = Left; }
                else if (keys[SDL_SCANCODE_D]) { paddle_x += PADDLE_SPEED; paddle_direction = Right; }

                //wall collisions
                if (ball_top <= 0) {ball.vel.y *= -1;}
                else if (ball_bottom >= SCREEN_HEIGHT){
                    SDL_DestroyWindow(window);
                    SDL_Quit();
                    printf("Game over");
                    return 0;
                }

                if (ball_right >= SCREEN_WIDTH || ball_left <= 0){
                    ball.vel.x *= -1; 
                } 

                //paddle collisions
                if (abs(ball_bottom - (SCREEN_HEIGHT - PADDLE_Y_OFFSET - PADDLE_THICKNESS)) <= COLLISION_TOLERANCE
                        &&( ball_center_x >= paddle_x && ball_center_x <= paddle_x + PADDLE_WIDTH )){ 

                    switch (paddle_direction){
                        case Left: { ball.vel.x -= PADDLE_SPEED/2; } break;
                        case Right: { ball.vel.x += PADDLE_SPEED/2; } break;
                        case None: { ball.vel.x *= .9; ball.vel.y *= 1.1; }
                    }

                    ball.vel.y *= -1;
                }

                //brick collisions and drawing
                int i; Point brick_pos;
                vec_foreach(&bricks, brick_pos, i){
                    int brick_top = brick_pos.y;
                    int brick_bottom = brick_pos.y + BRICK_HEIGHT;
                    int brick_left = brick_pos.x;
                    int brick_right = brick_pos.x + brick_width;
                    int brick_center_x = brick_pos.x + brick_width/2;
                    int brick_center_y = brick_pos.y + BRICK_HEIGHT/2;

                    boxRGBA(renderer, 
                            brick_left,
                            brick_top, 
                            brick_right, 
                            brick_bottom,
                            255,
                            255,
                            255,
                            255);

                    //brick collisions
                    //bottom and top
                    if (((abs(ball_top - brick_bottom) <= COLLISION_TOLERANCE)
                                || abs(ball_bottom - brick_top) <= COLLISION_TOLERANCE)
                            && abs(ball_center_x - brick_center_x) <= brick_width/2 ){
                        printf("Collided V\n");
                        ball.vel.y *= -1;
                        vec_splice(&bricks, i, 1);
                    }

                    if (((abs(ball_right - brick_left) <= COLLISION_TOLERANCE)
                                || abs(ball_left - brick_right) <= COLLISION_TOLERANCE)
                            && abs(ball_center_y - brick_center_y) <= BRICK_HEIGHT/2){
                        printf("Collided H\n");
                        ball.vel.x *= -1;
                        vec_splice(&bricks, i, 1);
                    }
                }

                ball.pos.x += ball.vel.x;
                ball.pos.y += ball.vel.y;

                SDL_RenderPresent(renderer);
                SDL_UpdateWindowSurface(window);

                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                SDL_RenderClear(renderer);

                //ball
                filledCircleRGBA(renderer, ball.pos.x, ball.pos.y, BALL_RADIUS, 255, 255, 255, 255);

                //paddle
                boxRGBA(renderer, //renderer, x1, y1, x2, y2, rgba
                        paddle_x, 
                        SCREEN_HEIGHT - PADDLE_Y_OFFSET, 
                        paddle_x + PADDLE_WIDTH,
                        SCREEN_HEIGHT - PADDLE_Y_OFFSET - PADDLE_THICKNESS,
                        255,
                        255,
                        255,
                        255);

                //remaining time goes into time2 but doesn't get used
                struct timespec time;
                struct timespec time2;
                time.tv_sec = 0;
                time.tv_nsec = 16666670;
                nanosleep(&time, &time2);
            }
        }
    }

    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
