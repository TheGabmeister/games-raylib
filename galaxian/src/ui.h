#ifndef UI_H
#define UI_H

struct Game;
void DrawHUD(struct Game *game);
void DrawTitleScreen(void);
void DrawGameOverScreen(int score);

#endif /* UI_H */
