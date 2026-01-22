#pragma once
#include "Object/Object.h"
#include "kEngine.h"
#include "Vector3.h"

class P_PlaneReduction :public Object{
public:
	P_PlaneReduction(kEngine* system);

	void Update(Camera* camera) override;

	bool GetIsActive() { return isActive_; }

	void SetIsAction(bool on) { isActive_ = on; }
	void SetTargetPos(Vector3 pos) { targetPos_ = pos; }
	void SetTargetScale(float scale) { targetScale_ = scale; }

private:
	kEngine* system_ = nullptr;

	bool isActive_ = false;

	Vector3 targetPos_{};
	float targetScale_ = 1.0f;
};

