#include "P_PlaneReduction.h"
#include <numbers>

P_PlaneReduction::P_PlaneReduction(kEngine* system) {
	Object::IntObject(system);
	mainPosition.transform = CreateDefaultTransform();
	mainPosition.transform.scale = { 1.0f,1.0f,1.0f };
	isActive_ = false;
}

void P_PlaneReduction::Update(Camera* camera) {
	
	if (!isActive_) { 
		mainPosition.transform.scale.x = targetScale_ * 0.5f;
		mainPosition.transform.scale.z = targetScale_ * 0.5f;
		return;
	}

	mainPosition.transform.scale.x -= targetScale_ / 60.0f;
	mainPosition.transform.scale.z -= targetScale_ / 60.0f;
	//mainPosition.transform.scale.z -= 0.03f;

	if (mainPosition.transform.scale.x <= 0.3f &&
		mainPosition.transform.scale.z <= 0.3f) {
		mainPosition.transform.scale.x = targetScale_ * 0.5f;
		mainPosition.transform.scale.z = targetScale_ * 0.5f;
	}

	mainPosition.transform.translate = targetPos_;
	mainPosition.transform.translate.z = targetPos_.z + 0.1f;

	Object::Update(camera);
}
