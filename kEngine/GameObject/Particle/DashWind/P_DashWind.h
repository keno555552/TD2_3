#pragma once
#include "../Particle.h"
#include <random>

class P_DashWind :public Particle{
public:

	P_DashWind(kEngine* system);
	
	void Update(Camera* camera)override;
	void Draw()override;
	
	void SetEmitting(bool on) { isEmitting_ = on; }
	void SetTargetForward(const Vector3& forward) { targetForward_ = forward; }
	void SetTargetPos(const Vector3& pos) { targetPos_ = pos; }
	void SetGenerationPos(const Vector3& pos) { generationPos_ = pos; }
	void SetArmScale(float scale) { armScale_ = scale; }

private:

	void CreateDashWind();
	void UpdateDashWind();
	void DeleteDashWind();

private:
	bool isEmitting_ = true;
	Vector3 targetForward_{ 0.0f,0.0f,1.0f };
	Vector3 targetPos_{};
	Vector3 generationPos_{};

	float lifeTime_ = 0.5f;
	float speed_ = 0.05f;
	float spread_ = 0.00f;      // 横に散らす量
	float createNumber_ = 20.0f; // 1フレームでのパーティクル生成数
	int   maxParticleNum_ = 400;

	int defaultTextureHandle_ = -1;
	int defaultModelHandle_ = -1;

	std::random_device seedGenerator;
	std::mt19937 randomEngine{ seedGenerator() };

	float radius_ = 1.0f;

	float armScale_ = 1.0f;
};

