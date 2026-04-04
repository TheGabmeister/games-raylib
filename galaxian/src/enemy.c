#include "enemy.h"
#include "draw_utils.h"

Color EnemyColor(EnemyType type)
{
    switch (type) {
        case ENEMY_BLUE:     return COLOR_BLUE_ENEMY;
        case ENEMY_PURPLE:   return COLOR_PURPLE_ENEMY;
        case ENEMY_RED:      return COLOR_RED_ENEMY;
        case ENEMY_FLAGSHIP: return COLOR_FLAGSHIP;
        default:             return WHITE;
    }
}

float EnemyCollisionRadius(EnemyType type)
{
    switch (type) {
        case ENEMY_BLUE:     return COLLISION_BLUE;
        case ENEMY_PURPLE:   return COLLISION_PURPLE;
        case ENEMY_RED:      return COLLISION_RED;
        case ENEMY_FLAGSHIP: return COLLISION_FLAGSHIP;
        default:             return 10.0f;
    }
}

int EnemyScoreFormation(EnemyType type)
{
    switch (type) {
        case ENEMY_BLUE:     return 30;
        case ENEMY_PURPLE:   return 40;
        case ENEMY_RED:      return 50;
        case ENEMY_FLAGSHIP: return 60;
        default:             return 30;
    }
}

int EnemyScoreDiving(Enemy *e)
{
    switch (e->type) {
        case ENEMY_BLUE:   return 60;
        case ENEMY_PURPLE: return 80;
        case ENEMY_RED:    return 100;
        case ENEMY_FLAGSHIP: {
            /* Special flagship scoring */
            if (e->escort_count > 0 && e->escorts_killed == e->escort_count)
                return 800;
            int alive_escorts = e->escort_count - e->escorts_killed;
            if (alive_escorts >= 2) return 300;
            if (alive_escorts >= 1) return 200;
            return 150;
        }
        default: return 60;
    }
}

void EnemyDraw(Enemy *e)
{
    if (!e->alive || e->state == ENEMY_INACTIVE) return;

    /* Skip enemies still waiting to enter (parked offscreen) */
    if (e->state == ENEMY_ENTERING && e->enter_timer < e->enter_delay) return;

    Color c = EnemyColor(e->type);

    switch (e->type) {
        case ENEMY_BLUE:     DrawBlueEnemy(e->position, e->rotation, c);     break;
        case ENEMY_PURPLE:   DrawPurpleEnemy(e->position, e->rotation, c);   break;
        case ENEMY_RED:      DrawRedEnemy(e->position, e->rotation, c);      break;
        case ENEMY_FLAGSHIP: DrawFlagshipEnemy(e->position, e->rotation, c); break;
    }
}
