#include <SDL2/SDL.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <ctime>
#include <cmath>

// Point structure
struct Point {
    float x, y;
    int label;
};

// Function to generate the dataset and save it to a CSV file
void generateCSV(const std::string& filename, int num_points, float separation_ratio = 8.0f) {
    std::ofstream file(filename);
    if (!file) {
        std::cerr << "Failed to create file: " << filename << std::endl;
        return;
    }

    file << "x,y,label\n";  // Header row

    int num_class_0 = num_points / 2;  // Class 0 points
    int num_class_1 = num_points - num_class_0;  // Class 1 points

    // Seed for randomness
    std::srand(std::time(nullptr));

    // Generate points for class 0
    for (int i = 0; i < num_class_0; ++i) {
        // Class 0 points will be clustered around the origin (0, 0)
        float x = static_cast<float>(std::rand() % 200 - 100) / 10.0f;
        float y = static_cast<float>(std::rand() % 200 - 100) / 10.0f;
        int label = 0;
        file << x << "," << y << "," << label << "\n";
    }

    // Generate points for class 1, with the separation controlled by `separation_ratio`
    for (int i = 0; i < num_class_1; ++i) {
        // Class 1 points will be shifted to the right based on the separation ratio
        float x = static_cast<float>(std::rand() % 200 - 100) / 10.0f + separation_ratio;  // Increase separation with ratio
        float y = static_cast<float>(std::rand() % 200 - 100) / 10.0f;
        int label = 1;
        file << x << "," << y << "," << label << "\n";
    }

    file.close();
    std::cout << "Dataset generated: " << filename << std::endl;
}

// Load dataset from CSV file
std::vector<Point> loadDataset(const std::string& filename) {
    std::vector<Point> dataset;
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return dataset;
    }

    std::string line;
    std::getline(file, line);  // Skip header
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string x_str, y_str, label_str;
        std::getline(ss, x_str, ',');
        std::getline(ss, y_str, ',');
        std::getline(ss, label_str, ',');

        Point p{std::stof(x_str), std::stof(y_str), std::stoi(label_str)};
        dataset.push_back(p);
    }

    return dataset;
}

// Function to draw points
void drawPoints(SDL_Renderer* renderer, const std::vector<Point>& dataset) {
    for (const auto& p : dataset) {
        if (p.label == 0) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);  // Red for class 0
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);  // Blue for class 1
        }

        // Larger points (5x5 pixels)
        int screenX = static_cast<int>(p.x * 40 + 600);  // Scale and center
        int screenY = static_cast<int>(p.y * -40 + 400); // Scale and invert y
        for (int dx = -2; dx <= 2; ++dx) {
            for (int dy = -2; dy <= 2; ++dy) {
                SDL_RenderDrawPoint(renderer, screenX + dx, screenY + dy);
            }
        }
    }
}

// Function to draw the line y = ax + b
void drawLine(SDL_Renderer* renderer, float a, float b) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);  // Green for the line
    for (int x = 0; x < 1200; ++x) {
        float normalizedX = (x - 600) / 40.0f;
        float normalizedY = a * normalizedX + b;
        int y = static_cast<int>(-normalizedY * 40 + 400);
        if (y >= 0 && y < 800) {
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
}

int main() {
    // Step 1: Generate dataset (uncomment if you want to generate a new file)
    generateCSV("dataset.csv", 100, 2.5f);  // Generate 100 points, with a separation of 2.5 for class 1

    // Step 2: Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Dynamic Line Visualization", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 800, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Load dataset
    auto dataset = loadDataset("dataset.csv");
    if (dataset.empty()) {
        SDL_Quit();
        return -1;
    }

    float a = 1.0f, b = 2.0f;  // Initial values for a and b
    bool running = true;
    SDL_Event event;

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Update values of a and b (example: oscillation)
        a += 0.01f * std::sin(SDL_GetTicks() / 500.0f);
        b += 0.01f * std::cos(SDL_GetTicks() / 500.0f);

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black background
        SDL_RenderClear(renderer);
        drawPoints(renderer, dataset);
        drawLine(renderer, a, b);
        SDL_RenderPresent(renderer);

        SDL_Delay(16);  // ~60 FPS
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}