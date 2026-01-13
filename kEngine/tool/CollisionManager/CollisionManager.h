#pragma once
#include "MathsIncluder.h"
#include "tool/MapChip/MapChipField.h"
#include <vector>

class Enemy;
class Bullet;
class Player;
class BackPoint;
class CollisionManager
{
public:
	
	void Update(float deltaTime);

	void InputPlayer(Player* player);
	void InputEnemyList(std::vector<Enemy*>* enemyList);


private:
	void MapCollisionCircle();
	void PlayerMapCollision();
	void PlayerEnemyCollision();
	void EnemyMapCollision();

private:
	Player* player_ = nullptr;
	MapChipField* mapChipField_ = nullptr;
	std::vector<Bullet*>* bulletList_ = nullptr;
	std::vector<Enemy*>* enemyList_ = nullptr;

	float deltaTime_ = 0.0f;

private:
	bool CheckIsWall(MapChipType type);
	bool CheckIsNotWall(MapChipType type);
};

