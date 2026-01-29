#include "SceneResult.h"

SceneResult::SceneResult(kEngine* system) {
	system_ = system;

	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.0f, -5.0f));
	camera_->Update();

	testTextResult = system_->LoadTextrue("resources/texture/sceneTest/result.png");

	sprite_ = new SimpleSprite;
	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = testTextResult;
}

SceneResult::~SceneResult() {
	delete sprite_;
	delete camera_;
}

void SceneResult::Update() {

	// ★ カメラ更新（必須）
	camera_->Update();

	if (system_->GetTriggerOn(DIK_SPACE)) {
		ChangeNextStage(SceneNum::S_TITLE);
	}
}

void SceneResult::Draw() {
	sprite_->Draw();
}
