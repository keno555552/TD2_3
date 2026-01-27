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
        "resources/gate/model/door_left.obj",
        "resources/gate/model/door_right.obj",
        "resources/gate/model/door_peace.obj",
        "resources/gate/model/roof.obj",
        "resources/gate/model/signboard.obj",
        "resources/gate/model/simenawa.obj",
        "resources/gate/model/tourou.obj",
        "resources/gate/model/wall_stone.obj",
        "resources/gate/model/wall_wood.obj",
          "resources/gate/model/ground.obj",
          
          
    };

    const char* texturePaths[kGatePartCount] = {
        "resources/gate/model/door.png",
        "resources/gate/model/door.png",
        "resources/gate/model/door_peace.png",
        "resources/gate/model/roof.png",
        "resources/gate/model/door.png",
        "resources/gate/model/simenawa.png",
        "resources/gate/model/tourou.png",
        "resources/gate/model/wall_stone.png",
        "resources/gate/model/wall_wood.png",
         "resources/gate/model/ground.png",
        
    };

    for (int i = 0; i < kGatePartCount; i++) {
        if (!modelPaths[i] || !texturePaths[i]) {
            continue;
        }

        gateModels_[i] = system_->SetModelObj(modelPaths[i]);
        gateTextures_[i] = system_->LoadTextrue(texturePaths[i]);

        gates_[i] = new Object;
        gates_[i]->IntObject(system_);

        // ★ 別プロジェクト寄せ：最低1パーツ確保
        gates_[i]->CreateDefaultData();
        gates_[i]->CreateModelData(gateModels_[i]);

        gates_[i]->mainPosition.transform.scale = Vector3(0.1f, 0.1f, 0.1f);

        // ★ モデルごとにテクスチャを明示設定
        for (auto& part : gates_[i]->objectParts_) {
            if (!part.materialConfig) {
                part.materialConfig = std::make_shared<MaterialConfig>();
            }

            part.materialConfig->useModelTexture = false;
            part.materialConfig->textureHandle = gateTextures_[i];

            // ★ ライティングは有効化（ライトで色味を作る）
            part.materialConfig->enableLighting = true;

            // ★ textureColor は触らない（ここ重要）
        }

        gates_[i]->Update(usingCamera_);
    }


    // =====================
// こぶしモデル（門とは別）
// =====================
 // =====================
// こぶしモデル（門とは別）
// =====================
    punchModelHandle_ = system_->SetModelObj("resources/gate/model/punch.obj");
    punchTexture_ = system_->LoadTextrue("resources/gate/model/ground.png");

    punch_ = new Object;
    punch_->IntObject(system_);
    punch_->CreateDefaultData();

    // ★ ここは int を渡す
    punch_->CreateModelData(punchModelHandle_);

    // 仮配置
    punch_->mainPosition.transform.scale =
        Vector3(0.1f, 0.1f, 0.1f);

    punch_->mainPosition.transform.translate =
        Vector3(0.0f, 0.0f, 2.0f);

    // マテリアル設定
    for (auto& part : punch_->objectParts_) {
        if (!part.materialConfig) {
            part.materialConfig = std::make_shared<MaterialConfig>();
        }

        part.materialConfig->useModelTexture = false;
        part.materialConfig->textureHandle = punchTexture_;
        part.materialConfig->enableLighting = true;
    }

    punch_->Update(usingCamera_);
    punchPos_ = { 0.0f, 0.0f, 2.0f };
    punchRot_ = { 0.0f, 0.0f, 0.0f };
    punchScale_ = { 0.1f, 0.1f, 0.1f };

    punch_->mainPosition.transform.translate = punchPos_;
    punch_->mainPosition.transform.rotate = punchRot_;
    punch_->mainPosition.transform.scale = punchScale_;

    // 黒マスク
    irisTex_ = system_->LoadTextrue("resources/gate/model/punch.png");

    irisMask_ = new Object;
    irisMask_->IntObject(system_);
    irisMask_->CreateDefaultData();

    for (auto& p : irisMask_->objectParts_) {
        if (!p.materialConfig) p.materialConfig = std::make_shared<MaterialConfig>();
        p.materialConfig->textureHandle = irisTex_;
        p.materialConfig->enableLighting = false;
    }

    // 最初はフル黒
    irisMask_->mainPosition.transform.translate = { 0,0,0 };
    irisMask_->mainPosition.transform.scale = { irisMaxScale_, irisMaxScale_, 1 };

    irisMask_->Update(usingCamera_);

}

SceneTitle::~SceneTitle() {
    for (int i = 0; i < kGatePartCount; i++) {
        delete gates_[i];
        gates_[i] = nullptr;
    }
    delete sprite_;
    sprite_ = nullptr;

    delete camera_;
    camera_ = nullptr;

    usingCamera_ = nullptr;
}

void SceneTitle::Update() {
    CameraControl();
    usingCamera_->Update();

    // =====================================================
    // ★ 念のため：毎フレ同じ夕方ライトを再適用したい場合はここ
    // （エンジン側でライトが毎回初期化されるタイプなら効く）
    // =====================================================
    {
        DirectionalLight eveningLight{};
        eveningLight.color = { 1.00f, 0.78f, 0.55f, 1.00f };
        eveningLight.direction = { -0.35f, -0.65f, 0.25f };
        eveningLight.intensity = 0.85f;

        system_->SetDirectionalLight(&eveningLight);
    }

    for (int i = 0; i < kGatePartCount; i++) {
        if (gates_[i]) {
            gates_[i]->Update(usingCamera_);
        }
    }

    // ----------------------
    // Transform 反映
    // ----------------------
    if (punch_) {
        punch_->mainPosition.transform.translate = punchPos_;
        punch_->mainPosition.transform.rotate = punchRot_;
        punch_->mainPosition.transform.scale = punchScale_;
        punch_->Update(usingCamera_);
    }


    // Update内
    if (!irisPlaying_ && system_->GetTriggerOn(DIK_RETURN)) {
        irisPlaying_ = true;
        irisT_ = 0.0f;
    }
    if (irisPlaying_) {
        irisT_ += system_->GetDeltaTime() * irisSpeed_;
        if (irisT_ > 1.0f) irisT_ = 1.0f;

        float s = irisMaxScale_ * (1.0f - irisT_);
        irisMask_->mainPosition.transform.scale = { s, s, 1.0f };
        irisMask_->Update(usingCamera_);

        if (irisT_ >= 1.0f) {
            irisPlaying_ = false; // 完全に開いた
        }
    }


    sprite_->Update(usingCamera_);

    if (system_->GetTriggerOn(DIK_SPACE)) {
        ChangeNextStage(SceneNum::S_STAGE_01);
    }
}

void SceneTitle::Draw() {
  /*  for (int i = 0; i < kGatePartCount; i++) {
        if (gates_[i]) {
            gates_[i]->Draw();
        }
    }*/
#ifdef _DEBUG
    ImGui::Begin("Punch Debug");

    ImGui::Checkbox("Enable Punch Debug", &showPunchDebug_);

    if (showPunchDebug_) {
        ImGui::DragFloat3("Position", &punchPos_.x, 0.05f);
        ImGui::DragFloat3("Rotation", &punchRot_.x, 1.0f);
        ImGui::DragFloat3("Scale", &punchScale_.x, 0.01f);
    }

    ImGui::End();
#endif
    // ★ こぶし（門より前）
    if (punch_) {
        punch_->Draw();
    }
    sprite_->Draw();
    if (irisMask_) irisMask_->Draw();
}

void SceneTitle::CameraControl()
{
    Vector3 move{ 0,0,0 };

    if (system_->GetIsPush(DIK_W)) move.z += cameraMoveSpeed_;
    if (system_->GetIsPush(DIK_S)) move.z -= cameraMoveSpeed_;

    if (system_->GetIsPush(DIK_A)) move.x -= cameraMoveSpeed_;
    if (system_->GetIsPush(DIK_D)) move.x += cameraMoveSpeed_;

    if (system_->GetIsPush(DIK_Q)) move.y += cameraMoveSpeed_;
    if (system_->GetIsPush(DIK_E)) move.y -= cameraMoveSpeed_;

    camera_->Move(move);
}
