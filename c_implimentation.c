#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

typedef struct {
    float x, y;
    int label;
} Point;

typedef struct {
    float w0, w1, w2;
} Perceptron;

void perceptron_init(Perceptron *p) {
    p->w0 = 0;
    p->w1 = 1;
    p->w2 = -1.5;
}

int perceptron_classify(const Perceptron *p, float x, float y) {
    float sum = p->w0 + p->w1 * x + p->w2 * y;
    return sum > 0 ? 1 : 0;
}

void perceptron_updateWeights(Perceptron *p, const Point *point) {
    int prediction = perceptron_classify(p, point->x, point->y);
    int error = point->label - prediction;

    if (error != 0) {
        p->w0 += error * 0.1f;
        p->w1 += error * point->x * 0.1f;
        p->w2 += error * point->y * 0.1f;
    }
}

void generateCSV(const char *filename, int num_points, float separation_ratio_0, float separation_ratio_1,
                 float a0, float b0, float a1, float b1) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Failed to create file: %s\n", filename);
        return;
    }

    fprintf(file, "x,y,label\n"); // Header row

    srand(time(NULL));

    for (int i = 0; i < num_points / 2; ++i) {
        float x = (rand() % 200 - 100) / 10.0f;
        float y = a0 * x + b0 + (rand() % 200 - 100) / 100.0f * separation_ratio_0;
        fprintf(file, "%f,%f,0\n", x, y);
    }

    for (int i = 0; i < num_points / 2; ++i) {
        float x = (rand() % 200 - 100) / 10.0f;
        float y = a1 * x + b1 + (rand() % 200 - 100) / 100.0f * separation_ratio_1;
        fprintf(file, "%f,%f,1\n", x, y);
    }

    fclose(file);
    printf("Dataset generated: %s\n", filename);
}

int loadDataset(const char *filename, Point **points) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open file: %s\n", filename);
        return 0;
    }

    char line[256];
    int count = 0;
    fgets(line, sizeof(line), file); // Skip header

    while (fgets(line, sizeof(line), file)) {
        count++;
    }

    rewind(file);
    fgets(line, sizeof(line), file); // Skip header again

    *points = (Point *)malloc(count * sizeof(Point));
    int idx = 0;
    while (fgets(line, sizeof(line), file)) {
        sscanf(line, "%f,%f,%d", &(*points)[idx].x, &(*points)[idx].y, &(*points)[idx].label);
        idx++;
    }

    fclose(file);
    return count;
}

void visualize(SDL_Renderer *renderer, Point *points, int num_points, const Perceptron *perceptron) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (int i = 0; i < num_points; ++i) {
        int screenX = (int)((points[i].x + 10) / 20.0 * WINDOW_WIDTH);
        int screenY = (int)((-points[i].y + 10) / 20.0 * WINDOW_HEIGHT);

        if (points[i].label == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
        }

        SDL_Rect rect = {screenX - 2, screenY - 2, 4, 4};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    for (int x = 0; x < WINDOW_WIDTH; ++x) {
        float worldX = (x / (float)WINDOW_WIDTH) * 20 - 10;
        float worldY = -(perceptron->w1 / perceptron->w2 * worldX + perceptron->w0 / perceptron->w2);
        int screenY = (int)((-worldY + 10) / 20.0 * WINDOW_HEIGHT);
        if (screenY >= 0 && screenY < WINDOW_HEIGHT) {
            SDL_RenderDrawPoint(renderer, x, screenY);
        }
    }

    SDL_RenderPresent(renderer);
}

int main(int argc, char *argv[]) {
    const char *filename = "dataset.csv";
    generateCSV(filename, 900, 3.0f, 3.0f, 1.0f, 1.0f, 1.09f, 9.0f);

    Point *points;
    int num_points = loadDataset(filename, &points);
    Perceptron perceptron;
    perceptron_init(&perceptron);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        free(points);
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Perceptron Visualization", 500, 200, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        free(points);
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_DestroyWindow(window);
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        free(points);
        return 1;
    }

    int epoch = 0;
    int solutionFound = 0;
    SDL_Event event;
    int running = 1;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = 0;
            }
        }

        int misclassified = 0;

        for (int i = 0; i < num_points; ++i) {
            if (perceptron_classify(&perceptron, points[i].x, points[i].y) != points[i].label) {
                perceptron_updateWeights(&perceptron, &points[i]);
                misclassified++;
            }
        }

        float accuracy = 1.0f - (float)misclassified / num_points;
        printf("Epoch: %d | Accuracy: %.2f%% | Misclassified: %d\n", epoch, accuracy * 100, misclassified);

        visualize(renderer, points, num_points, &perceptron);

        if (misclassified == 0) {
            printf("Solution found after %d epochs!\n", epoch);
            solutionFound = 1;
            
            continue;
        }

        epoch++;
        SDL_Delay(100);
    }

    if (!solutionFound) {
        printf("Training ended without finding a perfect solution.\n");
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    free(points);

    return 0;
}