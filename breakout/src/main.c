#include "raylib.h"
#include <math.h>

#define SCREEN_WIDTH 720
#define SCREEN_HEIGHT 900

#define PADDLE_WIDTH 120
#define PADDLE_HEIGHT 20
#define PADDLE_SPEED 8

#define BALL_RADIUS 10
#define BALL_SPEED 6

#define BRICK_ROWS 6
#define BRICK_COLS 10
#define BRICK_WIDTH 60
#define BRICK_HEIGHT 30
#define BRICK_PADDING 8
#define BRICK_OFFSET_TOP 60
#define BRICK_OFFSET_LEFT 30

typedef struct {
    Rectangle rect;
    bool active;
} Brick;

static Rectangle paddle;
static Vector2 ballPosition;
static Vector2 ballSpeed;
static bool ballActive = false;
static Brick bricks[BRICK_ROWS][BRICK_COLS];
static int lives = 3;
static int score = 0;
static bool gameOver = false;
static bool gameWon = false;

static void InitGame(void);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "classic game: breakout");
    InitGame();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateGame();
        DrawGame();
    }

    UnloadGame();
    CloseWindow();
    return 0;
}

void InitGame(void)
{
    // Paddle
    paddle.width = PADDLE_WIDTH;
    paddle.height = PADDLE_HEIGHT;
    paddle.x = (SCREEN_WIDTH - PADDLE_WIDTH) / 2;
    paddle.y = SCREEN_HEIGHT - 60;

    // Ball
    ballPosition = (Vector2){ paddle.x + PADDLE_WIDTH / 2, paddle.y - BALL_RADIUS - 2 };
    ballSpeed = (Vector2){ BALL_SPEED, -BALL_SPEED };
    ballActive = false;

    // Bricks
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            bricks[r][c].rect.x = BRICK_OFFSET_LEFT + c * (BRICK_WIDTH + BRICK_PADDING);
            bricks[r][c].rect.y = BRICK_OFFSET_TOP + r * (BRICK_HEIGHT + BRICK_PADDING);
            bricks[r][c].rect.width = BRICK_WIDTH;
            bricks[r][c].rect.height = BRICK_HEIGHT;
            bricks[r][c].active = true;
        }
    }

    lives = 3;
    score = 0;
    gameOver = false;
    gameWon = false;
}

void UpdateGame(void)
{
    if (gameOver || gameWon) {
        if (IsKeyPressed(KEY_ENTER)) InitGame();
        return;
    }

    // Paddle movement
    if (IsKeyDown(KEY_LEFT)) paddle.x -= PADDLE_SPEED;
    if (IsKeyDown(KEY_RIGHT)) paddle.x += PADDLE_SPEED;
    if (paddle.x < 0) paddle.x = 0;
    if (paddle.x > SCREEN_WIDTH - paddle.width) paddle.x = SCREEN_WIDTH - paddle.width;

    // Launch ball
    if (!ballActive) {
        ballPosition.x = paddle.x + paddle.width / 2;
        ballPosition.y = paddle.y - BALL_RADIUS - 2;
        if (IsKeyPressed(KEY_SPACE)) ballActive = true;
    }

    // Ball movement
    if (ballActive) {
        ballPosition.x += ballSpeed.x;
        ballPosition.y += ballSpeed.y;

        // Wall collision
        if (ballPosition.x < BALL_RADIUS) {
            ballPosition.x = BALL_RADIUS;
            ballSpeed.x *= -1;
        }
        if (ballPosition.x > SCREEN_WIDTH - BALL_RADIUS) {
            ballPosition.x = SCREEN_WIDTH - BALL_RADIUS;
            ballSpeed.x *= -1;
        }
        if (ballPosition.y < BALL_RADIUS) {
            ballPosition.y = BALL_RADIUS;
            ballSpeed.y *= -1;
        }

        // Paddle collision
        if (CheckCollisionCircleRec(ballPosition, BALL_RADIUS, paddle)) {
            ballPosition.y = paddle.y - BALL_RADIUS - 1;
            ballSpeed.y *= -1;
            
            float hitPos = (ballPosition.x - (paddle.x + paddle.width / 2)) / (paddle.width / 2);
            ballSpeed.x = BALL_SPEED * hitPos;
        }

        // Brick collision
        for (int r = 0; r < BRICK_ROWS; r++) {
            for (int c = 0; c < BRICK_COLS; c++) {
                if (bricks[r][c].active && CheckCollisionCircleRec(ballPosition, BALL_RADIUS, bricks[r][c].rect)) {
                    bricks[r][c].active = false;
                    score += 10;
                    // Simple collision response
                    float bx = ballPosition.x;
                    float by = ballPosition.y;
                    Rectangle brick = bricks[r][c].rect;
                    if (bx < brick.x || bx > brick.x + brick.width) ballSpeed.x *= -1;
                    else ballSpeed.y *= -1;
                }
            }
        }

        // Ball lost
        if (ballPosition.y > SCREEN_HEIGHT) {
            lives--;
            ballActive = false;
            if (lives <= 0) gameOver = true;
        }
    }

    // Win check
    int bricksLeft = 0;
    for (int r = 0; r < BRICK_ROWS; r++)
        for (int c = 0; c < BRICK_COLS; c++)
            if (bricks[r][c].active) bricksLeft++;
    if (bricksLeft == 0) gameWon = true;
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    // Draw bricks
    for (int r = 0; r < BRICK_ROWS; r++) {
        for (int c = 0; c < BRICK_COLS; c++) {
            if (bricks[r][c].active) {
                Color color = (Color){ 200, 200 - r * 30, 100 + r * 20, 255 };
                DrawRectangleRec(bricks[r][c].rect, color);
            }
        }
    }

    // Draw paddle
    DrawRectangleRec(paddle, WHITE);

    // Draw ball
    DrawCircleV(ballPosition, BALL_RADIUS, YELLOW);

    // Draw UI
    DrawText(TextFormat("LIVES: %d", lives), 20, SCREEN_HEIGHT - 40, 24, LIGHTGRAY);
    DrawText(TextFormat("SCORE: %d", score), SCREEN_WIDTH - 180, SCREEN_HEIGHT - 40, 24, LIGHTGRAY);

    if (!ballActive && !gameOver && !gameWon)
        DrawText("PRESS SPACE TO LAUNCH", SCREEN_WIDTH/2 - 160, SCREEN_HEIGHT/2, 28, GRAY);

    if (gameOver)
        DrawText("GAME OVER! PRESS ENTER TO RESTART", SCREEN_WIDTH/2 - 260, SCREEN_HEIGHT/2, 32, RED);

    if (gameWon)
        DrawText("YOU WIN! PRESS ENTER TO RESTART", SCREEN_WIDTH/2 - 220, SCREEN_HEIGHT/2, 32, GREEN);

    EndDrawing();
}

void UnloadGame(void)
{
    // No dynamic resources to unload
}
