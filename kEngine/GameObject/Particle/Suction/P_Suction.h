#pragma once
#include "../Particle.h"
#include <random>

class P_Suction :public Particle {
public:

	P_Suction(kEngine* system);

	void Update(Camera* camera)override;
	void Draw()override;

	void SetLeftEmitting(bool on) { isLeftEmitting_ = on; } // 左側のパーティクルを出すか否か
	void SetRightEmitting(bool on) { isRightEmitting_ = on; } // 右側のパーティクルを出すか否か
	void SetTargetForward(const Vector3& forward) { targetForward_ = forward; }
	void SetTargetPosLeft(const Vector3& pos) { targetPosLeft_ = pos; }
	void SetTargetPosRight(const Vector3& pos) { targetPosRight_ = pos; }
	void SetArmScaleL(float scale) { armScaleL_ = scale; }
	void SetArmScaleR(float scale) { armScaleR_ = scale; }


private:
	void CreateSuction();
	void UpdateSuction();
	void DeleteSuction();

private:
	struct EmitterDesc {
		bool* isEmitting; // 左右それぞれのON/OFF
		Vector3* targetPos;
		float* armScale;
	};

private:
	bool isLeftEmitting_ = true;
	bool isRightEmitting_ = true;
	Vector3 targetForward_{ 0.0f,0.0f,1.0f };
	Vector3 targetPosLeft_{};
	Vector3 targetPosRight_{};

	float lifeTime_ = 0.1f;
	float speed_ = 0.1f;
	float spread_ = 0.00f;      // 横に散らす量
	uint32_t createNumber_ = 3; // 1フレームでのパーティクル生成数
	int   maxParticleNum_ = 400;

	int defaultTextureHandle_ = -1;
	int defaultModelHandle_ = -1;

	std::random_device seedGenerator;
	std::mt19937 randomEngine{ seedGenerator() };

	float radius_ = 0.5f;

	// 対象の腕のスケール
	float armScaleL_ = 1.0f;
	float armScaleR_ = 1.0f;
};