#ifndef MAIN_H
#define MAIN_H

#include "raylib.h"

#define NUM_SHOOTS 5
#define NUM_MAX_ENEMIES 50
#define FIRST_WAVE 50
#define GRID_WIDTH 10
#define GRID_HEIGHT 5
#define GRID_SPACING_X 50  // Adjust this value for desired horizontal spacing
#define GRID_SPACING_Y 50  // Adjust this value for desired vertical spacing
#define GRID_OFFSET_X 65
#define GRID_OFFSET_Y 120

typedef struct Player{
    Rectangle rec;
    Vector2 speed;
    Color color;
} Player;

typedef struct Enemy{
    Rectangle rec;
    Vector2 speed;
    float baseSpeed;
    bool active;
    Color color;
} Enemy;

typedef struct Shoot{
    Rectangle rec;
    Vector2 speed;
    bool active;
    Color color;
} Shoot;

static const int screenWidth = 600;
static const int screenHeight = 800;

static bool gameOver = false;
static bool pause =  false;
static int score = 0;
static int highScore = 0;
static bool victory = false;

static Player player = { 0 };
static Texture2D playerTexture;
static Enemy enemy[NUM_MAX_ENEMIES] = { 0 };
static Shoot shoot[NUM_SHOOTS] = { 0 };

static int shootRate = 0;
static float alpha = 0.0f;

static int activeEnemies = 0;
static int enemiesKill = 0;
static bool smooth = false;

static void InitGame(void);         
static void UpdateGame(void);       
static void DrawGame(void);         
static void UnloadGame(void);       
static void UpdateDrawFrame(void);  

#endif // MAIN_H