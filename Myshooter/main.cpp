#include "raylib.h"
#include <vector>
#include <algorithm>
#include <raymath.h>
#include <cmath>

enum class GameState {MENU, PLAYING, GAMEOVER, EXIT};

struct Player { float x, y, speed, size; };
struct Enemy { float x, y, speed, size; bool active; float lifeTimer; };
struct Bullet { float x, y, speed; bool  active; };

bool DrawButton(const char* text, int x, int y, int w, int h){
	Rectangle buttonRect = { (float)x, (float)y, (float)w, (float)h };
	bool hover = CheckCollisionPointRec(GetMousePosition(), buttonRect);
	DrawRectangleRec(buttonRect, hover ? LIGHTGRAY : GRAY);
	DrawRectangleLinesEx(buttonRect, 2, WHITE);
	int textWidth = MeasureText(text, 22);
	DrawText(text, x + w / 2 - textWidth / 2, y + h / 2 - 11, 22, BLACK);
	return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

void ResetGame(Player& player, std::vector<Bullet>& bullets,
	std::vector<Enemy>& enemies, int& score,
	int& killed, int& total, float& gameTime, float& spawnTimer) {
	player = {.x = 600, .y = 500, .speed = 600.0f, .size = 20.0f };
	bullets.clear();
	enemies.clear();
	score = 0; killed = 0; total = 0;
	gameTime = 60.0f; spawnTimer = 0;
}

void UpdatePlayer(Player& player, std::vector<Bullet>& bullets, float dt) {
	Vector2 input = { 0, 0 };

	if (IsKeyDown(KEY_W)) input.y -= 1;
	if (IsKeyDown(KEY_S)) input.y += 1;
	if (IsKeyDown(KEY_A)) input.x -= 1;
	if (IsKeyDown(KEY_D)) input.x += 1;

	input = Vector2Normalize(input);

	if(IsGamepadAvailable(0)) {
		float stickX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
		float stickY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);

		const float DEADZONE = 0.2f;
		if (fabsf(stickX) < DEADZONE) stickX = 0;
		if (fabsf(stickY) < DEADZONE) stickY = 0;
		 
		if(stickX != 0 || stickY != 0){
			input.x = stickX;
			input.y = stickY;
		}
	}

	player.x += input.x * player.speed * dt;
	player.y += input.y * player.speed * dt;

	if (player.x < 0) player.x = 0;
	if (player.x > 1200 - player.size) player.x = 1200 - player.size;
	if (player.y < 0) player.y = 0;
	if (player.y > 1000 - player.size) player.y = 1000 - player.size;

	if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
		Bullet bullet = { player.x + player.size / 2, player.y, 600.0f, true };
		bullets.push_back(bullet);
	}
}
void UpdateBullets(std::vector<Bullet>& bullets, float dt) {
	for (auto& b : bullets) {
		if (!b.active) continue;
		b.y -= b.speed * dt;
		if (b.y < 0) b.active = false;
	}
}

void UpdateEnemies(std::vector<Enemy>& enemies, float& spawnTimer,
	float interval, int& total, float dt) {

	spawnTimer += dt;
	if (spawnTimer >= interval && (int)enemies.size() < 3) {
		spawnTimer = 0;
		Enemy e = { (float)GetRandomValue(0, 1170), -50.0f, 100.0f, 30.0f, true, 0 };
		enemies.push_back(e);
		total++;
	}
	for (auto& e : enemies) {
		if (!e.active) continue;
		e.y += e.speed * dt;
		e.lifeTimer += dt;
		if (e.y > 1000 || e.lifeTimer > 6.0f) e.active = false;
	}
}
void CheckCollisions(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies,
	int& score, int& killed) {
	for (auto& b : bullets) {
		if (!b.active) continue;
		Rectangle br = { b.x, b.y, 5, 10 };
		for (auto& e : enemies) {
			if (!e.active) continue;
			Rectangle er = { e.x, e.y, (float)e.size, (float)e.size };

			if (CheckCollisionRecs(br, er)) {
				b.active = false;
				e.active = false;
				score += 10;
				killed++;
				break;
			}
		}
	}
}

void CleanupDead(std::vector<Bullet>& bullets, std::vector<Enemy>& enemies) {
	bullets.erase(
		std::remove_if(bullets.begin(), bullets.end(),
			[](const Bullet& b) { return !b.active; }),
		bullets.end());

	enemies.erase(
		std::remove_if(enemies.begin(), enemies.end(),
			[](const Enemy& e) { return !e.active; }),
		enemies.end());
}

void DrawGame(Player& player, std::vector<Enemy>& enemies,
	std::vector<Bullet>& bullets,
	int score, int killed, int total, float gameTime) {

	DrawRectangle((int)player.x, (int)player.y,
		(int)player.size, (int)player.size, GRAY);

	for (auto& e : enemies) {
		if (!e.active) continue;
		DrawRectangle((int)e.x, (int)e.y, e.size, e.size, BLUE);

		float ratio = 1.0f - e.lifeTimer / 6.0f;
		DrawRectangle((int)e.x, (int)e.y - 10, e.size, 6, DARKGRAY);
		DrawRectangle((int)e.x, (int)e.y - 10, (int)(e.size * ratio), 6, GREEN);
	}
	for (auto& b : bullets)
		DrawRectangle((int)b.x, (int)b.y, 5, 10, RED);

	DrawText("WASD to move, Mouse to shoot", 10, 10, 20, SKYBLUE);
	DrawText(TextFormat("Score: %d", score), 10, 40, 24, WHITE);
	DrawText(TextFormat("Killed: %d/%d", killed, total), 10, 70, 22, YELLOW);

	int mins = (int)(gameTime / 60), secs = (int)gameTime % 60;
	DrawText(TextFormat("Time: %02d:%02d", mins, secs),
		10, 100, 24, gameTime <= 10.0f ? RED : WHITE);
}
GameState DrawMenu() {
	int px = 400, py = 350;
	DrawRectangle(px, py, 400, 300, DARKGRAY);
	DrawRectangleLinesEx({ (float)px, (float)py, 400, 300 }, 2, WHITE);
	int tw = MeasureText("MY SHOOTER", 40);
	DrawText("MY SHOOTER", px + 200 - tw / 2, py + 40, 40, WHITE);
	if (DrawButton("Start", px + 90, py + 130, 220, 60)) return GameState::PLAYING;
	if (DrawButton("Exit", px + 90, py + 210, 220, 60)) return GameState::EXIT;
	return GameState::MENU;
}

GameState DrawGameOver(int& score, int& killed, int& total,
	Player& player, std::vector<Bullet>& bullets,
	std::vector<Enemy>& enemies,
	float& gameTime, float& spawnTimer) {
	int px = 400, py = 270;
	DrawRectangle(px, py, 400, 460, DARKGRAY);
	DrawRectangleLinesEx({ (float)px, (float)py, 400, 460 }, 2, WHITE);
	int tw = MeasureText("GAME OVER", 40);
	DrawText("GAME OVER", px + 200 - tw / 2, py + 30, 40, RED);
	DrawText(TextFormat("Score: %d", score), px + 40, py + 110, 28, WHITE);
	float ratio = total > 0 ? (float)killed / total * 100.0f : 0;
	DrawText(TextFormat("Kill Ratio: %d/%d (%.0f%%)", killed, total, ratio),
		px + 40, py + 160, 22, YELLOW);
	if (DrawButton("Play Again", px + 80, py + 250, 240, 60)) {
		ResetGame(player, bullets, enemies,
			score, killed, total, gameTime, spawnTimer);
		return GameState::PLAYING;
	}
	if (DrawButton("Exit", px + 80, py + 350, 240, 60)) return GameState::EXIT;
	return GameState::GAMEOVER;
}
int main() {
	InitWindow(1200, 1000, "My Shooter");
	SetTargetFPS(60);

	GameState state = GameState::MENU;
	Player player;
	std::vector<Bullet> bullets;
	std::vector<Enemy>  enemies;
	bullets.reserve(200);
	enemies.reserve(20);
	int   score = 0, killedEnemies = 0, totalEnemies = 0;
	float gameTime = 60.0f, spawnTimer = 0, spawnInterval = 1.5f;

	ResetGame(player, bullets, enemies,
		score, killedEnemies, totalEnemies, gameTime, spawnTimer);

	while (!WindowShouldClose() && state != GameState::EXIT) {
		float dt = GetFrameTime();

		if (state == GameState::PLAYING) {
			gameTime -= dt;
			if (gameTime <= 0) { gameTime = 0; state = GameState::GAMEOVER; }
			UpdatePlayer(player, bullets, dt);
			UpdateBullets(bullets, dt);
			UpdateEnemies(enemies, spawnTimer, spawnInterval, totalEnemies, dt);
			CheckCollisions(bullets, enemies, score, killedEnemies);
			CleanupDead(bullets, enemies);
		}

		BeginDrawing();
		ClearBackground(BLACK);
		if (state == GameState::MENU)
			state = DrawMenu();
		else if (state == GameState::PLAYING)
			DrawGame(player, enemies, bullets,
				score, killedEnemies, totalEnemies, gameTime);
		else if (state == GameState::GAMEOVER)
			state = DrawGameOver(score, killedEnemies, totalEnemies,
				player, bullets, enemies,
				gameTime, spawnTimer);
		EndDrawing();
	}
	CloseWindow();
	return 0;
}