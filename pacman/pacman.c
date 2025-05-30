#include "raylib.h"
#include "math.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Shared Variables Definition (global)
Font font = { 0 };
Music music = { 0 };
Sound fxCoin = { 0 };

static const int screenWidth = 800;
static const int screenHeight = 1000;

typedef struct {
    Vector2 position;   
    Vector2 direction;
    float speed;
    float radius; 
    Texture2D sprite;
} Pacman;

typedef struct {
    Vector2 position;
    Vector2 direction;
    float speed;
    float radius;
    Color color;
} Ghost;

Pacman InitPacman(float x, float y)
{
    Pacman p;
    p.position = (Vector2){ x, y };
    p.direction = (Vector2){ 0, 0 };
    p.speed = 4.0f;
    p.radius = 16.0f;
    p.sprite = LoadTexture(RESOURCES_PATH"/pacman.png");
    return p;
}
Pacman pacman;
Vector2 desiredDirection = {0,0};

#define MAZE_ROWS 31
#define MAZE_COLS 28
#define TILE_SIZE 28
// 0 = empty, 1 = wall, 2 = pellet
int maze[MAZE_ROWS][MAZE_COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1},
    {0,0,0,0,0,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,0,0,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,1,1,1,0,0,1,1,1,0,1,1,2,1,0,0,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,1,0,1,1,2,1,1,1,1,1,1},
    {0,0,0,0,0,0,2,0,0,0,1,0,0,0,0,0,0,1,0,0,0,2,0,0,0,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,1,0,1,1,2,1,1,1,1,1,1},
    {0,0,0,0,0,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,0,0,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,0,0,0,0,0},
    {0,0,0,0,0,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,0,0,0,0,0},
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    {1,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,1,1,2,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,2,1,1,1},
    {1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

#define GHOST_COUNT 4
Ghost ghosts[GHOST_COUNT];

void InitGhosts() {
    Color ghostColors[GHOST_COUNT] = { RED, GREEN, BLUE, MAGENTA };
    for (int i = 0; i < GHOST_COUNT; i++) {
        ghosts[i].position = (Vector2){ 14 * TILE_SIZE + TILE_SIZE/2, 14 * TILE_SIZE + TILE_SIZE/2 + i*10 };
        ghosts[i].direction = (Vector2){ 0, -1 };
        ghosts[i].speed = 3.0f;
        ghosts[i].radius = 16.0f;
        ghosts[i].color = ghostColors[i];
    }
}

// Local Functions Declaration
static void UpdateDrawFrame(void);
void UpdateGhosts();

int main(void)
{
    // Initialization
    //---------------------------------------------------------
    InitWindow(screenWidth, screenHeight, "Pacman");

    InitAudioDevice();      // Initialize audio device

    pacman = InitPacman(
        20 * TILE_SIZE + TILE_SIZE /2, 
        20 * TILE_SIZE + TILE_SIZE /2
    );
    
    InitGhosts();

    font = LoadFont("resources/mecha.png");
    //music = LoadMusicStream("resources/ambient.ogg"); // TODO: Load music
    fxCoin = LoadSound("resources/coin.wav");

    SetMusicVolume(music, 1.0f);
    PlayMusicStream(music);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        UpdateDrawFrame();
    }
#endif

    // Unload global data loaded
    UnloadFont(font);
    UnloadMusicStream(music);
    UnloadSound(fxCoin);
    UnloadTexture(pacman.sprite); 

    CloseAudioDevice();     // Close audio context
    CloseWindow();          // Close window and OpenGL context

    return 0;
}

static void UpdateDrawFrame(void)
{
    // Update
    //----------------------------------------------------------------------------------
    //UpdateMusicStream(music);       
    
    // Keyboard input for Pacman movement
    if (IsKeyDown(KEY_RIGHT)) desiredDirection = (Vector2){ 1, 0 };
    else if (IsKeyDown(KEY_LEFT)) desiredDirection = (Vector2){ -1, 0 };
    else if (IsKeyDown(KEY_UP)) desiredDirection = (Vector2) { 0, -1 };
    else if (IsKeyDown(KEY_DOWN)) desiredDirection = (Vector2) { 0, 1 };

    Vector2 pacmanGridPos = {
        (int)(pacman.position.x / TILE_SIZE),
        (int)(pacman.position.y / TILE_SIZE)
    };

    if (pacmanGridPos.y >= 0 && pacmanGridPos.y < MAZE_ROWS && pacmanGridPos.x >= 0 && pacmanGridPos.x < MAZE_COLS) {
        if (maze[(int)pacmanGridPos.y][(int)pacmanGridPos.x] == 2) {
            maze[(int)pacmanGridPos.y][(int)pacmanGridPos.x] = 0; // Remove pellet
            PlaySound(fxCoin);
        }
    }

    // Calculate center of current grid cell
    Vector2 cellCenter = {
        pacmanGridPos.x * TILE_SIZE + TILE_SIZE / 2.0f,
        pacmanGridPos.y * TILE_SIZE + TILE_SIZE / 2.0f
    };

    // Calculate distance from Pacman to center of grid cell
    float distToCenter = sqrtf(
        (pacman.position.x - cellCenter.x) * (pacman.position.x - cellCenter.x) +
        (pacman.position.y - cellCenter.y) * (pacman.position.y - cellCenter.y)
    );

    if(distToCenter <= 2.0)
    {
        // Check if the next cell in the desired direction is a wall
        int nextRow = (int)pacmanGridPos.y + (int)desiredDirection.y;
        int nextCol = (int)pacmanGridPos.x + (int)desiredDirection.x;
        if (nextRow >= 0 && nextRow < MAZE_ROWS && nextCol >= 0 && nextCol < MAZE_COLS)
        {
            if (maze[nextRow][nextCol] != 1)
            {
                pacman.direction = desiredDirection;
                pacman.position = cellCenter;
            }
            else
            {
                // Only stop if the wall is directly in front of Pacman's current direction
                if ((int)desiredDirection.x == (int)pacman.direction.x && (int)desiredDirection.y == (int)pacman.direction.y)
                {
                    pacman.direction = (Vector2){0, 0};
                }
                // Otherwise, keep moving in the current direction
            }
        }

        else
        {
            pacman.direction = (Vector2){0, 0}; // Stop if out of bounds
        }
    }

    // Update Pacman's position
    pacman.position.x += pacman.direction.x * pacman.speed;
    pacman.position.y += pacman.direction.y * pacman.speed;

    UpdateGhosts();


    // Draw
    //----------------------------------------------------------------------------------
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw Maze
    for (int row = 0; row < MAZE_ROWS; row++)
    {
        for (int col = 0; col < MAZE_COLS; col++)
        {
            int x = col * TILE_SIZE;
            int y = row * TILE_SIZE;
            if (maze[row][col] == 1)
                DrawRectangle(x, y, TILE_SIZE, TILE_SIZE, DARKGRAY);
            else if (maze[row][col] == 2)
                DrawCircle(x + TILE_SIZE/2, y + TILE_SIZE/2, 4, WHITE);
        }
    }

    float scale = 0.3f;
    float rotationAngle = atan2f(pacman.direction.y, pacman.direction.x) * (180.0f / PI);
    if (rotationAngle < 0) {
        rotationAngle += 360.0;
    }

    // Use DrawTexturePro to rotate the sprite along its center
    DrawTexturePro(
        pacman.sprite,
        (Rectangle) { 0, 0, pacman.sprite.width, pacman.sprite.height },
        (Rectangle) { pacman.position.x, pacman.position.y, pacman.sprite.width* scale, pacman.sprite.height* scale }, 
        (Vector2) { (pacman.sprite.width * scale) / 2, (pacman.sprite.height * scale) / 2 }, 
        rotationAngle,
        WHITE
    );

    // For debugging Pacman's position in the grid
    /*if (distToCenter <= 2.0)
    {
        DrawRectangle((int)pacmanGridPos.x * TILE_SIZE, (int)pacmanGridPos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE, RED);
    }
    else
    {
        DrawRectangle((int)pacmanGridPos.x * TILE_SIZE, (int)pacmanGridPos.y * TILE_SIZE, TILE_SIZE, TILE_SIZE, YELLOW);
    }*/
    
    // For debugging:
    // DrawText(TextFormat("Dist: %.2f", distToCenter), 10, 10, 20, WHITE);
     
    // Draw Ghosts
    for (int i = 0; i < GHOST_COUNT; i++) {
        DrawCircleV(ghosts[i].position, ghosts[i].radius, ghosts[i].color);
    }

    //DrawFPS(10, 10);

    EndDrawing();
    //----------------------------------------------------------------------------------
}

void UpdateGhosts()
{
    const float CHASE_DISTANCE = 200.0f; // Only chase if Pacman is within 200 pixels

    // Update Ghosts
    for (int i = 0; i < GHOST_COUNT; i++) 
    {
        // Get ghost's grid position
        Vector2 ghostGridPos = {
            (int)(ghosts[i].position.x / TILE_SIZE),
            (int)(ghosts[i].position.y / TILE_SIZE)
        };

        // Calculate center of current grid cell
        Vector2 cellCenter = {
            ghostGridPos.x * TILE_SIZE + TILE_SIZE / 2.0f,
            ghostGridPos.y * TILE_SIZE + TILE_SIZE / 2.0f
        };

        // Calculate distance from ghost to center of grid cell
        float distToCenter = sqrtf(
            (ghosts[i].position.x - cellCenter.x) * (ghosts[i].position.x - cellCenter.x) +
            (ghosts[i].position.y - cellCenter.y) * (ghosts[i].position.y - cellCenter.y)
        );

        const float CHASE_DISTANCE = 200.0f;
        Vector2 diff = {
            pacman.position.x - ghosts[i].position.x,
            pacman.position.y - ghosts[i].position.y
        };
        float distance = sqrtf(diff.x * diff.x + diff.y * diff.y);

        if (distToCenter <= 2.0f) {
            // Only pick a new direction when at the center of a tile
            Vector2 newDir = ghosts[i].direction;

            if (distance <= CHASE_DISTANCE) {
                // --- BFS shortest path to Pacman ---
                typedef struct { int x, y; } Point;
                Point queue[MAZE_ROWS * MAZE_COLS];
                int front = 0, back = 0;
                int visited[MAZE_ROWS][MAZE_COLS] = {0};
                Point prev[MAZE_ROWS][MAZE_COLS] = {0};

                int startX = (int)ghostGridPos.x, startY = (int)ghostGridPos.y;
                int goalX = (int)(pacman.position.x / TILE_SIZE), goalY = (int)(pacman.position.y / TILE_SIZE);

                queue[back++] = (Point){startX, startY};
                visited[startY][startX] = 1;

                int found = 0;
                while (front < back && !found) {
                    Point cur = queue[front++];
                    Point dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
                    for (int d = 0; d < 4; d++) {
                        int nx = cur.x + dirs[d].x, ny = cur.y + dirs[d].y;
                        if (nx >= 0 && nx < MAZE_COLS && ny >= 0 && ny < MAZE_ROWS &&
                            !visited[ny][nx] && maze[ny][nx] != 1) {
                            queue[back++] = (Point){nx, ny};
                            visited[ny][nx] = 1;
                            prev[ny][nx] = cur;
                            if (nx == goalX && ny == goalY) { found = 1; break; }
                        }
                    }
                }
                // Trace back from Pacman to ghost to find next step
                if (visited[goalY][goalX]) {
                    int cx = goalX, cy = goalY;
                    Point before = {startX, startY};
                    while (!(prev[cy][cx].x == startX && prev[cy][cx].y == startY)) {
                        Point p = prev[cy][cx];
                        cx = p.x; cy = p.y;
                    }
                    int dx = cx - startX, dy = cy - startY;
                    newDir = (Vector2){ dx, dy };
                } else {
                    newDir = ghosts[i].direction; // fallback: keep current direction
                }
            } else {
                // Move in a less random direction if Pacman is too far
                Vector2 possibleDirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };
                int validDirs[4] = {0};
                int validCount = 0;
                int currentDirIndex = -1;

                // Check which directions are valid (not a wall)
                for (int d = 0; d < 4; d++) {
                    int testRow = (int)ghostGridPos.y + (int)possibleDirs[d].y;
                    int testCol = (int)ghostGridPos.x + (int)possibleDirs[d].x;
                    if (testRow >= 0 && testRow < MAZE_ROWS && testCol >= 0 && testCol < MAZE_COLS && maze[testRow][testCol] != 1) {
                        validDirs[validCount++] = d;
                        // Check if this is the current direction
                        if ((int)ghosts[i].direction.x == (int)possibleDirs[d].x && (int)ghosts[i].direction.y == (int)possibleDirs[d].y) {
                            currentDirIndex = d;
                        }
                    }
                }

                if (currentDirIndex != -1) {
                    // Continue in the same direction if possible
                    newDir = possibleDirs[currentDirIndex];
                } else if (validCount > 0) {
                    // Otherwise, pick a new direction (randomly, but less often)
                    int choice = GetRandomValue(0, validCount - 1);
                    newDir = possibleDirs[validDirs[choice]];
                } else {
                    newDir = (Vector2){ 0, 0 }; // No valid moves
                }
            }

            // Check if the next cell in the chosen direction is a wall
            int nextRow = (int)ghostGridPos.y + (int)newDir.y;
            int nextCol = (int)ghostGridPos.x + (int)newDir.x;
            if (nextRow >= 0 && nextRow < MAZE_ROWS && nextCol >= 0 && nextCol < MAZE_COLS && maze[nextRow][nextCol] != 1) {
                ghosts[i].direction = newDir;
                ghosts[i].position = cellCenter; // Snap to center
            } else {
                ghosts[i].direction = (Vector2){ 0, 0 }; // Stop if wall or out of bounds
            }
        }

        // Move ghost along the grid
        ghosts[i].position.x += ghosts[i].direction.x * ghosts[i].speed;
        ghosts[i].position.y += ghosts[i].direction.y * ghosts[i].speed;
    }
}