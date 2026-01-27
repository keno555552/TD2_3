#include "P_PlaneReduction.h"
#include <numbers>

P_PlaneReduction::P_PlaneReduction(kEngine* system) {
	Object::IntObject(system);
	mainPosition.transform = CreateDefaultTransform();
	mainPosition.transform.scale = { 2.0f,2.0f,2.0f };
	mainPosition.transform.rotate.x = std::numbers::pi_v<float> / 2.0f;
	isActive_ = false;
}

void P_PlaneReduction::Update(Camera* camera) {
	
	if (!isActive_) { 
		mainPosition.transform.scale.x = targetScale_ * 0.5f;
		mainPosition.transform.scale.y = targetScale_ * 0.5f;
		return;
	}

	mainPosition.transform.scale.x -= targetScale_ / 60.0f;
	mainPosition.transform.scale.y -= targetScale_ / 60.0f;
	//mainPosition.transform.scale.z -= 0.03f;

	if (mainPosition.transform.scale.x <= 0.3f &&
		mainPosition.transform.scale.y <= 0.3f) {
		mainPosition.transform.scale.x = targetScale_ * 0.5f;
		mainPosition.transform.scale.y = targetScale_ * 0.5f;
	}

	mainPosition.transform.translate = targetPos_;

	Object::Update(camera);
}
