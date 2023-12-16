#include <raylib.h>
#include <stdlib.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450

#define GRAVITY 0.5f
#define PLAYER_JUMP_FORCE 10.0f

#define PLATFORMS_COUNT 20

#define min(a, b) ((a) < (b) ? (a) : (b))

typedef struct {
    Rectangle rect;
    Vector2 velocity;
    bool isJumping;
} Player;

typedef struct {
    Rectangle rect;
    bool isActive;
} Platform;

typedef struct {
    Rectangle rect;
    bool isActive;
} Objective;

typedef enum {
    MAIN_MENU,
    GAMEPLAY,
    GAME_OVER
} GameState;

void UpdatePlayer(Player* player, Platform* platforms, int platformCount, Objective* objectives, int* score, int* highScore, GameState* gameState) {
    // Apply gravity
    player->velocity.y += GRAVITY;

    // Move player horizontally
    if (IsKeyDown(KEY_LEFT)) {
        player->velocity.x = -5.0f;
    } else if (IsKeyDown(KEY_RIGHT)) {
        player->velocity.x = 5.0f;
    } else {
        player->velocity.x = 0.0f;
    }
    player->rect.x += player->velocity.x;

    // Check collision with platforms
    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(player->rect, platforms[i].rect)) {
            // Colliding with a platform
            player->velocity.y = 0;
            player->isJumping = false;
            player->rect.y = platforms[i].rect.y - player->rect.height;
        }
    }

    // Move player vertically
    player->rect.y += player->velocity.y;

    // Check collision with platforms again after vertical movement
    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(player->rect, platforms[i].rect)) {
            // Colliding with a platform
            player->velocity.y = 0;
            player->isJumping = false;
            player->rect.y = platforms[i].rect.y - player->rect.height;
        }
    }

    // Check collision with objectives
    for (int i = 0; i < platformCount; i++) {
        if (CheckCollisionRecs(player->rect, objectives[i].rect) && objectives[i].isActive) {
            // Colliding with an objective
            objectives[i].isActive = false;
            (*score)++;

            // Generate a new objective
            int platformIndex = GetRandomValue(0, platformCount - 1);
            objectives[i].rect.x = GetRandomValue(platforms[platformIndex].rect.x, platforms[platformIndex].rect.x + platforms[platformIndex].rect.width - objectives[i].rect.width);
            objectives[i].rect.y = platforms[platformIndex].rect.y - objectives[i].rect.height;
            objectives[i].isActive = true;
        }
    }

    // Check if player touches the ground
    if (player->rect.y + player->rect.height >= SCREEN_HEIGHT - 50) {
        *gameState = GAME_OVER;
        if (*score > *highScore) {
            *highScore = *score;
        }
    }
}

void DrawGameOverScreen(int score, int highScore) {
    DrawText("Game Over", SCREEN_WIDTH / 2 - MeasureText("Game Over", 40) / 2, SCREEN_HEIGHT / 2 - 40, 40, BLACK);
    DrawText(TextFormat("Score: %d", score), SCREEN_WIDTH / 2 - MeasureText(TextFormat("Score: %d", score), 20) / 2, SCREEN_HEIGHT / 2, 20, BLACK);
    DrawText(TextFormat("High Score: %d", highScore), SCREEN_WIDTH / 2 - MeasureText(TextFormat("High Score: %d", highScore), 20) / 2, SCREEN_HEIGHT / 2 + 40, 20, BLACK);
    DrawText("Press R to Restart", SCREEN_WIDTH / 2 - MeasureText("Press R to Restart", 20) / 2, SCREEN_HEIGHT / 2 + 80, 20, BLACK);
    DrawText("Press M to Main Menu", SCREEN_WIDTH / 2 - MeasureText("Press M to Main Menu", 20) / 2, SCREEN_HEIGHT / 2 + 120, 20, BLACK);
    DrawText("Press ESCAPE to Exit", SCREEN_WIDTH / 2 - MeasureText("Press ESCAPE to Exit", 20) / 2, SCREEN_HEIGHT / 2 + 160, 20, BLACK);
}

int main() {
    // Initialization
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Game Platformer");
    SetTargetFPS(60);

    // Initialize player
    Platform platforms[PLATFORMS_COUNT] = { 0 };
    int platformCount = PLATFORMS_COUNT; // Declare and initialize platformCount
    Player player = { { 100, platforms[0].rect.y - 40, 40, 40 }, { 0, 0 }, false };

    // Generate platforms
    int previousPlatformHeight = 300; // Initial platform height
    for (int i = 0; i < platformCount; i++) {
        platforms[i].rect.width = GetRandomValue(50, 200); // Random width
        platforms[i].rect.height = 20; // Fixed height
        platforms[i].rect.x = i * 200; // Each platform is 200 units apart
        platforms[i].rect.y = GetRandomValue(previousPlatformHeight - 100, min(previousPlatformHeight + 100, SCREEN_HEIGHT - 50 - platforms[i].rect.height)); // Random height within 100 units of the previous platform's height, but not lower than the ground
        platforms[i].isActive = true;

        // Update the previous platform height
        previousPlatformHeight = platforms[i].rect.y;
    }

    // Initialize objectives
    Objective objectives[PLATFORMS_COUNT] = { 0 };
    for (int i = 0; i < platformCount; i++) {
        objectives[i].rect.width = 20; // Fixed width
        objectives[i].rect.height = 20; // Fixed height

        // Generate random platform index
        int platformIndex = GetRandomValue(0, platformCount - 1);

        // Set objective position on top of the platform
        objectives[i].rect.x = GetRandomValue(platforms[platformIndex].rect.x, platforms[platformIndex].rect.x + platforms[platformIndex].rect.width - objectives[i].rect.width);
        objectives[i].rect.y = platforms[platformIndex].rect.y - objectives[i].rect.height;

        objectives[i].isActive = true;
    }

    // Declare a Camera2D variable
    Camera2D camera = { 0 };
    camera.target = (Vector2){ player.rect.x + player.rect.width / 2, player.rect.y + player.rect.height / 2 };
    camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // Initialize ground
    Rectangle ground = { 0, SCREEN_HEIGHT - 50, SCREEN_WIDTH, 50 };

    // Initialize game state
    GameState gameState = MAIN_MENU;

    // Initialize score
    int score = 0;
    int highScore = 0;

    // Main game loop
    while (!WindowShouldClose()) {
        switch (gameState) {
            case MAIN_MENU:
                if (IsKeyPressed(KEY_ENTER)) {
                    gameState = GAMEPLAY;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    CloseWindow();
                    return 0;
                }

                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawText("Game Platformer", SCREEN_WIDTH / 2 - MeasureText("Game Platformer", 40) / 2, SCREEN_HEIGHT / 2 - 40, 40, BLACK);
                DrawText("Press ENTER to Start", SCREEN_WIDTH / 2 - MeasureText("Press ENTER to Start", 20) / 2, SCREEN_HEIGHT / 2, 20, BLACK);
                DrawText("Press ESCAPE to Exit", SCREEN_WIDTH / 2 - MeasureText("Press ESCAPE to Exit", 20) / 2, SCREEN_HEIGHT / 2 + 40, 20, BLACK);
                EndDrawing();
                break;

            case GAMEPLAY:
                // Update
                if (IsKeyPressed(KEY_SPACE) && !player.isJumping) {
                    player.velocity.y = -PLAYER_JUMP_FORCE;
                    player.isJumping = true;
                }
                UpdatePlayer(&player, platforms, platformCount, objectives, &score, &highScore, &gameState);

                // Update camera to follow player
                camera.target = (Vector2){ player.rect.x + player.rect.width / 2, player.rect.y + player.rect.height / 2 };

                // Draw
                BeginDrawing();
                ClearBackground(RAYWHITE);

                BeginMode2D(camera); // Begin 2D mode with camera

                // Draw platforms
                for (int i = 0; i < platformCount; i++) {
                    if (platforms[i].isActive) {
                        DrawRectangleRec(platforms[i].rect, DARKGRAY);
                    }
                }

                // Draw objectives
                for (int i = 0; i < platformCount; i++) {
                    if (objectives[i].isActive) {
                        DrawRectangleRec(objectives[i].rect, BLUE);
                    }
                }

                // Draw player
                DrawRectangleRec(player.rect, RED);

                EndMode2D(); // End 2D mode

                // Draw scoreboard
                DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
                DrawText(TextFormat("High Score: %d", highScore), 10, 40, 20, BLACK);

                EndDrawing();

                // Generate new platforms if needed
                for (int i = 0; i < platformCount; i++) {
                    if (platforms[i].rect.x + platforms[i].rect.width < camera.target.x - GetScreenWidth() / 2) {
                        platforms[i].rect.x += platformCount * 200;
                        platforms[i].rect.y = GetRandomValue(previousPlatformHeight - 100, min(previousPlatformHeight + 100, SCREEN_HEIGHT - 50 - platforms[i].rect.height));
                        platforms[i].isActive = true;
                        previousPlatformHeight = platforms[i].rect.y;
                    }
                }

                break;

            case GAME_OVER:
                if (IsKeyPressed(KEY_R)) {
                    // Reset game
                    score = 0;
                    gameState = GAMEPLAY;
                    player.rect.x = platforms[0].rect.x; // Spawn player on the far left platform
                    player.rect.y = platforms[0].rect.y - player.rect.height;
                    player.velocity = (Vector2){ 0, 0 };
                    player.isJumping = false;
                    for (int i = 0; i < platformCount; i++) {
                        objectives[i].isActive = true;
                    }
                }
                if (IsKeyPressed(KEY_M)) {
                    // Go back to main menu
                    gameState = MAIN_MENU;
                }
                if (IsKeyPressed(KEY_ESCAPE)) {
                    CloseWindow();
                    return 0;
                }

                BeginDrawing();
                ClearBackground(RAYWHITE);
                DrawGameOverScreen(score, highScore);
                EndDrawing();
                break;
        }
    }

    // Cleanup
    CloseWindow();

    return 0;
}
