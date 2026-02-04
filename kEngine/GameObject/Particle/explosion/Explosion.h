#pragma once
#include "../Particle.h"
#include <random>

class Explosion :public Particle {
public:
	Explosion(kEngine* system);

	void Update(Camera* camera)override;
	void Draw()override;

	void SetIsEmitting(bool on) { isEmitting_ = on; }
	void SetPos(Vector3 pos) { pos_ = pos; }

private:

	void CreateExplosion();
	void UpdateExplosion();
	void DeleteExplosion();

private:
	bool isEmitting_ = false;
	Vector3 pos_;

	std::random_device seedGenerator;
	std::mt19937 randomEngine{ seedGenerator() };

	int defaultTextureHandle_ = -1;
	int defaultModelHandle_ = -1;

	int createNumber_ = 50; // 1フレームで生成する数
	int maxParticleNum_ = 400; // 最大数

	float lifeTime_ = 1.0f;
	
	float coolTimer_ = 0.0f;
};

