#ifndef GAME_H
#define GAME_H

#include "game_types.h"

void GameInit(Game *game);
void GameShutdown(Game *game);
void GameUpdate(Game *game, float dt);
void GameDraw(const Game *game);

int GameSpawnEnemy(Game *game, EnemyType type, Vector2 position);
void GameSpawnProjectile(Game *game, Vector2 position, Vector2 velocity, bool fromPlayer, float radius, float life, EnemyType ownerType);
void GameSpawnExplosion(Game *game, Vector2 position, Color color, int count, float speedScale);
void GameSpawnImpact(Game *game, Vector2 position, Color color);
void GameSpawnFloatingText(Game *game, Vector2 position, const char *text, Color color);
void GameAddScore(Game *game, int amount, Vector2 position);
void GameDamageEnemy(Game *game, int enemyIndex, int damage);
void GameKillEnemy(Game *game, int enemyIndex);
void GameKillPlayer(Game *game, Vector2 position);
bool GameUseSmartBomb(Game *game);
bool GameUseHyperspace(Game *game);
int GameLivingHumanCount(const Game *game);
int GameActiveEnemyCount(const Game *game);

#endif
