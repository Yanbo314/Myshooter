#include "raylib.h"
#include <vector>
#include <algorithm>

enum GameState {
	MENU,
	PLAYING,
	GAMEOVER
};
bool DrawButton(const char* text, int x, int y, int w, int h){
	Rectangle buttonRect = { (float)x, (float)y, (float)w, (float)h };
	bool hover = CheckCollisionPointRec(GetMousePosition(), buttonRect);
	DrawRectangleRec(buttonRect, hover ? LIGHTGRAY : GRAY);
	DrawRectangleLinesEx(buttonRect, 2, WHITE);
	int textWidth = MeasureText(text, 22);
	DrawText(text, x + w / 2 - textWidth / 2, y + h / 2 - 11, 22, BLACK);
	return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
struct Player {
	float x, y, speed, size;
};
struct Enemy {
	float x, y, speed;
	int size;
	bool active;
	float lifeTimer;
};
struct Bullet {
	float x, y, speed;
	bool  active;
};
float gameTime = 60.0f;

int main()
{
	InitWindow(1200, 1000, "My Shooter");
	SetTargetFPS(60);
	GameState state = MENU;
	int score = 0;
	int killedEnemies = 0;

	Player player = { 600, 500, 600.0f, 20 };
	std::vector<Enemy> enemies;
	float enemySpawnTimer = 0.0f;
	float enemySpawnInterval = 1.5f;
	int totalEnemies = 0;
	std::vector<Bullet> bullets;

	while (!WindowShouldClose()) {
		float deltaTime = GetFrameTime();
		BeginDrawing();
		ClearBackground(BLACK);
		if (state == MENU) {
			int px = 400, py = 270;
			DrawRectangle(px, py, 400, 300, DARKGRAY);
			DrawRectangleLinesEx({ (float)px, (float)py, 400, 300 }, 2, WHITE);
			int textWidth = MeasureText("MY SHOOTER", 40);
			DrawText("MY SHOOTER", px + 200 - textWidth / 2, py + 40, 40, WHITE);
			if (DrawButton("Start", px + 90, py + 130, 220, 60)) {
				state = PLAYING;
			}
			if (DrawButton("Exit", px + 90, py + 210, 220, 60)) {
				CloseWindow();
				break;
			}
		}
		else if (state == PLAYING) {

			gameTime -= deltaTime;
			if (gameTime <= 0) {
				gameTime = 0; state = GAMEOVER;
			}
			enemySpawnTimer += deltaTime;
			if (enemySpawnTimer >= enemySpawnInterval && (int)enemies.size() < 3) {
				enemySpawnTimer = 0.0f;
				Enemy enemy;
				enemy.x = (float)GetRandomValue(0, 1170);
				enemy.y = -50;
				enemy.speed = 100.0f;
				enemy.size = 30;
				enemy.active = true;
				enemy.lifeTimer = 0.0f;
				enemies.push_back(enemy);
				totalEnemies++;
			}
			if (IsKeyDown(KEY_W)) player.y -= player.speed * deltaTime;
			if (IsKeyDown(KEY_S)) player.y += player.speed * deltaTime;
			if (IsKeyDown(KEY_A)) player.x -= player.speed * deltaTime;
			if (IsKeyDown(KEY_D)) player.x += player.speed * deltaTime;
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				Bullet bullet;
				bullet.x = player.x + player.size / 2;
				bullet.y = player.y;
				bullet.speed = 400.0f;
				bullet.active = true;
				bullets.push_back(bullet);
			}
			for (auto& bullet : bullets) {
				if (bullet.active) {
					bullet.y -= bullet.speed * deltaTime * 1.5;
					if (bullet.y < 0) bullet.active = false;
				}
			}
			for (int i = bullets.size() - 1; i >= 0; i--) {
				if (!bullets[i].active) bullets.erase(bullets.begin() + i);
			}
			for (auto& enemy : enemies) {
				if (!enemy.active) continue;
				enemy.y += enemy.speed * deltaTime;
				enemy.lifeTimer += deltaTime;
				if (enemy.y > 1000 || enemy.lifeTimer > 6.0f) enemy.active = false;
			}
			for (int i = enemies.size() - 1; i >= 0; i--) {
				if (!enemies[i].active) enemies.erase(enemies.begin() + i);
			}
			for (auto& bullet : bullets) {
				if (!bullet.active) continue;
				for (auto& enemy : enemies) {
					if (!enemy.active) continue;
					Rectangle bulletRect = { bullet.x, bullet.y, 5, 10 };
					Rectangle enemyRect = { enemy.x, enemy.y, (float)enemy.size, (float)enemy.size };

					if (CheckCollisionRecs(bulletRect, enemyRect)) {
						bullet.active = false;
						enemy.active = false;
						score += 10;
						killedEnemies++;
						break;
					}
				} //
			} //

			DrawRectangle((int)player.x, (int)player.y, (int)player.size, (int)player.size, GRAY);
			for (auto& enemy : enemies) {
				if (!enemy.active) continue;
				DrawRectangle((int)enemy.x, (int)enemy.y, enemy.size, enemy.size, BLUE);
				float ratio = 1.0f - enemy.lifeTimer / 6.0f;
				DrawRectangle((int)enemy.x, (int)enemy.y - 10, enemy.size * ratio, 6, DARKGRAY);
				DrawRectangle((int)enemy.x, (int)enemy.y - 10, (int)(enemy.size * ratio), 6, GREEN);
			}
			for (auto& bullet : bullets) {
				if (bullet.active) DrawRectangle((int)bullet.x, (int)bullet.y, 5, 10, RED);
			}
			DrawText("WASD to move, Mouse to shoot", 10, 10, 20, SKYBLUE);
			DrawText(TextFormat("Score: %d", score), 10, 40, 24, WHITE);
			DrawText(TextFormat("Killed: %d/%d", killedEnemies, totalEnemies), 10, 70, 22, YELLOW);
			int mins = (int)(gameTime / 60);
			int secs = (int)(gameTime) % 60;
			Color timeColor = gameTime <= 10.0f ? RED : WHITE;
			DrawText(TextFormat("Time: %02d:%02d", mins, secs), 10, 100, 24, timeColor);
		}
		else if (state == GAMEOVER) {
			int px = 400, py = 270;
			DrawRectangle(px, py, 400, 460, DARKGRAY);
			DrawRectangleLinesEx({ (float)px, (float)py, 400, 450 }, 2, WHITE);

			int textWidth = MeasureText("GAME OVER", 40);
			DrawText(TextFormat("Score: %d", score),
				px + 40, py + 110, 28, WHITE);
			float ratio = totalEnemies > 0 ? (float)killedEnemies / totalEnemies * 100.0f : 0;
			DrawText(TextFormat("Kill Ratio: %d/%d (%.0f%%)", killedEnemies, totalEnemies, ratio), px + 40, py + 160, 22, YELLOW);
			if (DrawButton("Play Again", px + 80, py + 250, 240, 60)) {
				score = 0; killedEnemies = 0;
				totalEnemies = 0; gameTime = 60.0f;
				enemySpawnTimer = 0;
				bullets.clear(); enemies.clear();
				player = { 200, 400, 400.0f, 20 };
				state = PLAYING;
			}
			if (DrawButton("Exit", px + 80, py + 350, 240, 60)) {
				break;
			}
		}
		EndDrawing();
	}
	return 0;
}