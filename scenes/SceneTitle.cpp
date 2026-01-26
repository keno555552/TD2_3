#include "SceneTitle.h"

SceneTitle::SceneTitle(kEngine* system) {
    system_ = system;

    // ===== カメラ（SceneTest基準）=====
    camera_ = new Camera;
    camera_->Move(Vector3(0.0f, 0.0f, -10.0f));
    usingCamera_ = camera_;
    usingCamera_->Update();

    // ===== タイトルスプライト =====
    testTextTitle = system_->LoadTextrue("resources/texture/sceneTest/title.png");

    sprite_->IntObject(system_);
    sprite_->CreateDefaultData();

    // ★ materialConfig が null の可能性に備える
    if (!sprite_->objectParts_[0].materialConfig) {
        sprite_->objectParts_[0].materialConfig = std::make_shared<MaterialConfig>();
    }
    sprite_->objectParts_[0].materialConfig->textureHandle = testTextTitle;

    sprite_->Update(usingCamera_);

  // =====================
		// 門モデル一覧（画像の通り）
		// =====================
	const char* modelPaths[kGatePartCount] = {
		"resources/gate/model/tourou.obj"
		/*"resources/gate/model/door_left.obj",*/
	/*	"resources/gate/model/door_right.obj",
		"resources/gate/model/door_peace.obj",
		"resources/gate/model/roof.obj",
		"resources/gate/model/signboard.obj",
		"resources/gate/model/simenawa.obj",
		"resources/gate/model/tourou.obj",
		"resources/gate/model/wall_stone.obj",
		"resources/gate/model/wall_wood.obj",*/
	};

	const char* texturePaths[kGatePartCount] = {
		"resources/gate/model/tourou.png"
		/*"resources/gate/model/door.png",*/
	/*	"resources/gate/model/door.png",
		"resources/gate/model/door_peace.png",
		"resources/gate/model/signboard.png",
		"resources/gate/model/simenawa.png",
		"resources/gate/model/tourou.png",
		"resources/gate/model/wall_stone.png",
		"resources/gate/model/wall_wood.png",*/
	};
    for (int i = 0; i < kGatePartCount; i++) {
        gateModels_[i] = system_->SetModelObj(modelPaths[i]);
        gateTextures_[i] = system_->LoadTextrue(texturePaths[i]);

        gates_[i] = new Object;
        gates_[i]->IntObject(system_);

        // ★ 別プロジェクト寄せ：最低1パーツ確保
        gates_[i]->CreateDefaultData();
        gates_[i]->CreateModelData(gateModels_[i]);

        for (auto& part : gates_[i]->objectParts_) {
            if (!part.materialConfig) {
                part.materialConfig = std::make_shared<MaterialConfig>();
            }
            part.materialConfig->useModelTexture = false;
            part.materialConfig->textureHandle = gateTextures_[i];
        }

        gates_[i]->Update(usingCamera_);
    }
}

SceneTitle::~SceneTitle() {
    for (int i = 0; i < kGatePartCount; i++) {
        delete gates_[i];
    }
    delete sprite_;
    delete camera_;
}

void SceneTitle::Update() {
    CameraControl();        // ← 追加
    usingCamera_->Update();

    for (int i = 0; i < kGatePartCount; i++) {
        gates_[i]->Update(usingCamera_);
    }

    sprite_->Update(usingCamera_);

    if (system_->GetTriggerOn(DIK_SPACE)) {
        ChangeNextStage(SceneNum::S_STAGE_01);
    }
}

void SceneTitle::Draw() {
    for (int i = 0; i < kGatePartCount; i++) {
        gates_[i]->Draw();
    }
    sprite_->Draw();
}
void SceneTitle::CameraControl()
{
    Vector3 move{ 0,0,0 };

    // 前後（Z）
    if (system_->GetKey(DIK_W)) move.z += cameraMoveSpeed_;
    if (system_->GetKey(DIK_S)) move.z -= cameraMoveSpeed_;

    // 左右（X）
    if (system_->GetKey(DIK_A)) move.x -= cameraMoveSpeed_;
    if (system_->GetKey(DIK_D)) move.x += cameraMoveSpeed_;

    // 上下（Y）
    if (system_->GetKey(DIK_Q)) move.y += cameraMoveSpeed_;
    if (system_->GetKey(DIK_E)) move.y -= cameraMoveSpeed_;

    camera_->Move(move);
}
