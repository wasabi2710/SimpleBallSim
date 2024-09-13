#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#define G_CONSTANT 6.672 // rough estimation
#define GRAVITY 200 // newton's second law acceleration
#define GROUND_MASS 10000000.0

struct Bounds { // set collision bounds
    float x0, y0;
    float x1, y1;
};
struct Ball { // ball's properties
    float x, y;
    float vy;
    float vx;
    int rad;
    float mass;
    float dir;
    float angle;
    float restitution; // coefficient of restitution (bounciness)
};

float gravitional_force(float mass_a, float mass_b, float distance) { // calculating grav force based g_constant estimation (!earth: g force)
    if (distance == 0) return 0;
    return G_CONSTANT * ((mass_a * mass_b) / (distance * distance));
}
float fall_acce(float m1, float m2, float r) { // calculating gravity 
    //float force = gravitional_force(m1, m2, r);
    //return force / m1;
    return GRAVITY; 
}
float weight(float m1, float gforce) {
    return m1 * gforce;
}
void lerp(float *a, float b, float delta) {
    *a = *a + (b - *a) * delta;
}
void rotatePoint(int* x, int* y, int cx, int cy, float angle) {
    float s = sin(angle);
    float c = cos(angle);

    int x_temp = *x - cx;
    int y_temp = *y - cy;

    int xnew = x_temp * c - y_temp * s;
    int ynew = x_temp * s + y_temp * c;

    *x = xnew + cx;
    *y = ynew + cy;
}
void handleCollision(struct Ball* a, struct Ball* b) {
    float dx = b->x - a->x;
    float dy = b->y - a->y;
    float distance = sqrt(dx * dx + dy * dy);

    if (distance < a->rad + b->rad) {
        float nx = dx / distance;
        float ny = dy / distance;
        float tx = -ny;
        float ty = nx;
        float dpTanA = a->vx * tx + a->vy * ty;
        float dpTanB = b->vx * tx + b->vy * ty;
        float dpNormA = a->vx * nx + a->vy * ny;
        float dpNormB = b->vx * nx + b->vy * ny;
        float m1 = (dpNormA * (a->rad - b->rad) + 2 * b->rad * dpNormB) / (a->rad + b->rad);
        float m2 = (dpNormB * (b->rad - a->rad) + 2 * a->rad * dpNormA) / (a->rad + b->rad);
        a->vx = tx * dpTanA + nx * m1;
        a->vy = ty * dpTanA + ny * m1;
        b->vx = tx * dpTanB + nx * m2;
        b->vy = ty * dpTanB + ny * m2;
        a->dir *= -1;
        b->dir *= -1;
        float overlap = 0.5f * (a->rad + b->rad - distance);
        a->x -= overlap * nx;
        a->y -= overlap * ny;
        b->x += overlap * nx;
        b->y += overlap * ny;
    }
}
void drawCircle(int x0, int y0, int radius, float angle, SDL_Renderer* renderer) {
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);
    while (x >= y) {
        int points[8][2] = {
            {x0 + x, y0 + y},
            {x0 + y, y0 + x},
            {x0 - y, y0 + x},
            {x0 - x, y0 + y},
            {x0 - x, y0 - y},
            {x0 - y, y0 - x},
            {x0 + y, y0 - x},
            {x0 + x, y0 - y}
        };
        for (int i = 0; i < 8; ++i) {
            rotatePoint(&points[i][0], &points[i][1], x0, y0, angle);
            SDL_RenderDrawPoint(renderer, points[i][0], points[i][1]);
        }
        if (err <= 0) {
            y++;
            err += dy;
            dy += 2;
        }
        if (err > 0) {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
    int endX = x0 + radius * cos(angle);
    int endY = y0 + radius * sin(angle);
    SDL_RenderDrawLine(renderer, x0, y0, endX, endY);
}

/* main entry */
int main(int argc, char* argv[]) {
    // init video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // bounds
    struct Bounds bound;
    bound.x0 = 0.0f;
    bound.y0 = 0.0f;
    bound.x1 = 800.0f;
    bound.y1 = 300.0f;
    
    // init window 
    SDL_Window *win = SDL_CreateWindow("SpaceShooter", 100, 100, bound.x1, bound.y1, SDL_WINDOW_SHOWN);
    if (win == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // init renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        SDL_DestroyWindow(win);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        return 1;
    }

    // event updates
    SDL_Event e;
    bool running = true;

    // balls
    int lower = 1;
    int upper = 100;
    float dt = 1.0f / 60.0f;
    struct Ball balls[10];
    srand(time(NULL));

    for (int i = 0; i < 10; i++) {
        balls[i].x = (rand() % (upper - lower + 1)) + lower;
        balls[i].y = (rand() % (upper - lower + 1)) + lower;
        balls[i].vx = 500.0f;
        balls[i].vy = 2.0f;
        balls[i].rad = 20;
        balls[i].restitution = 0.8f;
        balls[i].angle = 0.0f;
        balls[i].dir = 0.1f;
    }

    // rendering
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }

        for (int i = 0; i < 10; i++) {
            balls[i].vy += GRAVITY * dt;
            balls[i].y += balls[i].vy * dt;

            if (balls[i].y + balls[i].rad > bound.y1) {
                balls[i].y = bound.y1 - balls[i].rad;
                balls[i].vy = -balls[i].vy * balls[i].restitution;
            }
            if (balls[i].y + balls[i].rad < bound.y0) {
                balls[i].y = bound.y0 + balls[i].rad;
                balls[i].vy = -balls[i].vy * balls[i].restitution;
            }
            if (balls[i].x + balls[i].rad > bound.x1) {
                balls[i].x = bound.x1 - balls[i].rad;
                balls[i].vx = -balls[i].vx * balls[i].restitution;
                balls[i].dir *= -1;
            } 
            if (balls[i].x - balls[i].rad < bound.x0) {
                balls[i].x = bound.x0 + balls[i].rad;
                balls[i].vx = -balls[i].vx * balls[i].restitution;
                balls[i].dir *= -1;
            }

            balls[i].x += balls[i].vx * dt;
            if (balls[i].vx != 0) {
                balls[i].angle += (balls[i].vx * dt / balls[i].rad) + balls[i].dir;
            } else {
                balls[i].angle += balls[i].dir;
            }
            if (balls[i].vx > 0) {
                balls[i].vx -= 0.5f;
                if (balls[i].vx < 0) {
                    balls[i].vx = 0;
                }
                balls[i].dir *= 0.99f;
                if (balls[i].dir < 0.01f) {
                    balls[i].dir = 0;
                }
            } 
        }

        for (int i = 0; i < 5; i++) {
            for (int j = i + 1; j < 5; j++) {
                handleCollision(&balls[i], &balls[j]);
            }
        }

        // main window background
        SDL_SetRenderDrawColor(renderer, 10, 20, 30, 255);
        SDL_RenderClear(renderer); // clear the screen with current draw color 

        // draws
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

        // balls randomizer
        for (int i = 0; i < 5; i++) { 
            drawCircle(balls[i].x, balls[i].y, balls[i].rad, balls[i].angle, renderer);
        }

        SDL_RenderPresent(renderer);
    }

    // destroy and quit
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return 0;
}


