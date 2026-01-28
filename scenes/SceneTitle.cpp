#include "SceneTitle.h"

SceneTitle::SceneTitle(kEngine* system) {
    system_ = system;

    // ===== カメラ（SceneTest基準）=====
    camera_ = new Camera;
    camera_->Move(Vector3(-0.6f, 16.0f, -25.0f));
    usingCamera_ = camera_;
    usingCamera_->Update();
    cameraStartPos_ = { -8.8f, 16.0f, -25.0f };
    cameraEndPos_ = { -8.8f, 8.5f, -34.5f };
    cameraPrevPos_ = cameraStartPos_;

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
        gates_[i]->mainPosition.transform.translate = Vector3(4.5f, 0, 0.0f);

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
    gates_[0]->mainPosition.transform.translate = Vector3(-7.350f, 0, 14.150f);
    gates_[1]->mainPosition.transform.translate = Vector3(6.500f, 0, 14.150f);
    gates_[9]->mainPosition.transform.translate = Vector3(4.5f, 0, 12.0f);

    for (int i = 0; i < kGatePartCount; i++) {
        if (!gates_[i]) continue;

        gatePos_[i] = gates_[i]->mainPosition.transform.translate;
        gateRot_[i] = gates_[i]->mainPosition.transform.rotate;
        gateScale_[i] = gates_[i]->mainPosition.transform.scale;
    }
 

    // =====================
    // セレクトモデル一覧（画像の通り）
    // =====================
    const char* modelPaths2[kSelectPartCount] = {
    "resources/gate/model/touro2.obj",
    "resources/gate/model/select_peace.obj",
    "resources/gate/model/select_board.obj",
    "resources/gate/model/ohuda.obj",
    "resources/gate/model/select_roof.obj",
    "resources/gate/model/select_simenawa.obj",
    "resources/gate/model/select_wood.obj",
    "resources/gate/model/shugyou.obj",
    "resources/gate/model/zissenn.obj",
      "resources/gate/model/syuuryou.obj",
         "resources/gate/model/cursor.obj",
             "resources/gate/model/select1.obj",
      "resources/gate/model/select2.obj",
         "resources/gate/model/select3.obj",


    };

    const char* texturePaths2[kSelectPartCount] = {
        "resources/gate/model/tourou.png",
        "resources/gate/model/door_peace.png",
        "resources/gate/model/paper.png",
        "resources/gate/model/ohuda_flame.png",
        "resources/gate/model/roof.png",
        "resources/gate/model/simenawa.png",
        "resources/gate/model/wood_tate.png",
        "resources/gate/model/punch.png",
        "resources/gate/model/punch.png",
         "resources/gate/model/punch.png",
            "resources/gate/model/punch.png",
              "resources/gate/model/punch.png",
         "resources/gate/model/punch.png",
            "resources/gate/model/punch.png",
    };

    for (int i = 0; i < kSelectPartCount; i++) {
        if (!modelPaths2[i] || !texturePaths2[i]) {
            continue;
        }

        selectModels_[i] = system_->SetModelObj(modelPaths2[i]);
        selectTextures_[i] = system_->LoadTextrue(texturePaths2[i]);

        selects_[i] = new Object;
        selects_[i]->IntObject(system_);

        // ★ 別プロジェクト寄せ：最低1パーツ確保
        selects_[i]->CreateDefaultData();
        selects_[i]->CreateModelData(selectModels_[i]);
        selects_[i]->mainPosition.transform.translate = Vector3(5.0f, 0, -5.0f);

        selects_[i]->mainPosition.transform.scale = Vector3(0.1f, 0.1f, 0.1f);

        // ★ モデルごとにテクスチャを明示設定
        for (auto& part : selects_[i]->objectParts_) {
            if (!part.materialConfig) {
                part.materialConfig = std::make_shared<MaterialConfig>();
            }

            part.materialConfig->useModelTexture = false;
            part.materialConfig->textureHandle = selectTextures_[i];

            // ★ ライティングは有効化（ライトで色味を作る）
            part.materialConfig->enableLighting = true;

            // ★ textureColor は触らない（ここ重要）
        }

        selects_[i]->Update(usingCamera_);
    }
    selects_[10]->mainPosition.transform.translate = Vector3(-4.400f, 8.350f, 68.500f);

    
   /* for (int i = 0; i < kSelectPartCount; i++) {
        if (!selects_[i]) continue;

        gatePos_[i] = selects_[i]->mainPosition.transform.translate;
        gateRot_[i] = selects_[i]->mainPosition.transform.rotate;
        gateScale_[i] = selects_[i]->mainPosition.transform.scale;
    }
*/

 // =====================
// こぶしモデル（門とは別）
// =====================
    punchModelHandle_ = system_->SetModelObj("resources/gate/model/punch.obj");
    punchTexture_ = system_->LoadTextrue("resources/gate/model/punch.png");

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
    punchPos_ = { -0.45f, 17.4f, 6.6f };
    punchRot_ = { 0.0f, 0.0f, 0.0f };
    punchScale_ = { 0, 0, 0 };

    punch_->mainPosition.transform.translate = punchPos_;
    punch_->mainPosition.transform.rotate = punchRot_;
    punch_->mainPosition.transform.scale = punchScale_;

    // 穴あきモデル（アイリスイン用）
    irisHoleModelHandle_ = system_->SetModelObj("resources/gate/model/irisin.obj");
    irisHoleTex_ = system_->LoadTextrue("resources/gate/model/paper.png");

    irisHole_ = new Object;
    irisHole_->IntObject(system_);
    irisHole_->CreateDefaultData();
    irisHole_->CreateModelData(irisHoleModelHandle_);

    for (auto& part : irisHole_->objectParts_) {
        if (!part.materialConfig) {
            part.materialConfig = std::make_shared<MaterialConfig>();
        }
        part.materialConfig->useModelTexture = false;
        part.materialConfig->textureHandle = irisHoleTex_;
        part.materialConfig->enableLighting = false;
    }

    // 画面中央・最前面想定
    irisHole_->mainPosition.transform.translate = { -0.5f, 17.0f, 6.4f };
    irisHole_->mainPosition.transform.scale = { 0.3f, 0.3f, 0.2f };
    irisHole_->Update(usingCamera_);

    // =====================
  // 文字モデル一覧（画像の通り）
  // =====================
    const char* mozi_modelPaths[kMoziPartCount] = {
        "resources/gate/model/dai.obj",
        "resources/gate/model/kenn.obj",
        "resources/gate/model/dou.obj",
      


    };

    const char* mozi_texturePaths[kMoziPartCount] = {
        "resources/gate/model/punch.png",
        "resources/gate/model/punch.png",
        "resources/gate/model/punch.png",
     

    };

    for (int i = 0; i < kMoziPartCount; i++) {
        if (!mozi_modelPaths[i] || !mozi_texturePaths[i]) {
            continue;
        }

        moziModels_[i] = system_->SetModelObj(mozi_modelPaths[i]);
        moziTextures_[i] = system_->LoadTextrue(mozi_texturePaths[i]);

        mozis_[i] = new Object;
        mozis_[i]->IntObject(system_);

        // ★ 別プロジェクト寄せ：最低1パーツ確保
        mozis_[i]->CreateDefaultData();
        mozis_[i]->CreateModelData(moziModels_[i]);
        mozis_[i]->mainPosition.transform.translate = Vector3(4.5f + i * 2.05f, 0, 0.0f);

        mozis_[i]->mainPosition.transform.scale = Vector3(0.14f, 0.14f, 0.14f);

        // ★ モデルごとにテクスチャを明示設定
        for (auto& part : mozis_[i]->objectParts_) {
            if (!part.materialConfig) {
                part.materialConfig = std::make_shared<MaterialConfig>();
            }

            part.materialConfig->useModelTexture = false;
            part.materialConfig->textureHandle = moziTextures_[i];

            // ★ ライティングは有効化（ライトで色味を作る）
            part.materialConfig->enableLighting = true;

            // ★ textureColor は触らない（ここ重要）
        }
        moziPos_[i] = mozis_[i]->mainPosition.transform.translate;
        moziRot_[i] = mozis_[i]->mainPosition.transform.rotate;
        moziScale_[i] = mozis_[i]->mainPosition.transform.scale;



        mozis_[i]->Update(usingCamera_);
    }

    for (int i = 0; i < kMoziPartCount; i++) {
        moziOrbit_[i].center = {
            -2.45f + i * 2.0f,   // ★ 文字ごとに中心をずらす
            17.5f,
            0.0f
        };

        moziOrbit_[i].angle = i * 1.2f;   // 初期角度（バラす）
        moziOrbit_[i].radius = moziStartRadius_;
    }
    moziOrbitTime_ = 0.0f;

    // 選択肢0の位置を基準にする
    selectBasePos_ = Vector3(-4.37f, 8.350f, 68.500f);

    // カーソル初期位置（0番目）
    selectIndex_ = 0;
    selects_[10]->mainPosition.transform.translate =
        selectBasePos_ + Vector3(selectSpacing_ * selectIndex_, 0.0f, 0.0f);

}

SceneTitle::~SceneTitle() {
    for (int i = 0; i < kGatePartCount; i++) {
        delete gates_[i];
        gates_[i] = nullptr;
    }
    for (int i = 0; i < kSelectPartCount; i++) {
        delete selects_[i];
        selects_[i] = nullptr;
    }
    for (int i = 0; i < kMoziPartCount; i++) {
        delete mozis_[i];
        mozis_[i] = nullptr;
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
    // =====================
// セレクト操作
// =====================
    bool moveRight =
        system_->GetTriggerOn(DIK_D) ||
        system_->GetTriggerOn(DIK_RIGHT);

    bool moveLeft =
        system_->GetTriggerOn(DIK_A) ||
        system_->GetTriggerOn(DIK_LEFT);

    if (moveRight) {
        selectIndex_++;
        if (selectIndex_ >= selectCount_) {
            selectIndex_ = selectCount_ - 1;
        }
    }

    if (moveLeft) {
        selectIndex_--;
        if (selectIndex_ < 0) {
            selectIndex_ = 0;
        }
    }

    // =====================
    // カーソル位置反映
    // =====================
    if (selects_[10]) {
        Vector3 targetPos =
            selectBasePos_ +
            Vector3(selectSpacing_ * selectIndex_, 0.0f, 0.0f);

        selects_[10]->mainPosition.transform.translate = targetPos;
        if (selectIndex_ ==2) {
            selects_[10]->mainPosition.transform.translate = selectBasePos_ + Vector3(7.95f, 0.0f, 0.0f);
        }
    }
 
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
        if (!gates_[i]) continue;

        gates_[i]->mainPosition.transform.translate = gatePos_[i];
        gates_[i]->mainPosition.transform.rotate = gateRot_[i];
        gates_[i]->mainPosition.transform.scale = gateScale_[i];
    }

    for (int i = 0; i < kGatePartCount; i++) {
        if (gates_[i]) {
            gates_[i]->Update(usingCamera_);
        }
    }
    for (int i = 0; i < kSelectPartCount; i++) {
        if (selects_[i]) {
            selects_[i]->Update(usingCamera_);
        }
    }
 
  // ===== 文字の円運動（回転＋収束）=====
    float dt = system_->GetDeltaTime();

    // エンジンが返す単位を吸収する（sec / ms / us）
    if (dt > 1000.0f) {
        // だいたい 16666 みたいな値が来るなら μs
        dt *= 0.000001f;
    }
    else if (dt > 1.0f) {
        // だいたい 16 みたいな値が来るなら ms
        dt *= 0.001f;
    }

    // 保険：極端なスパイクで一発終了しないように上限
    if (dt > 0.1f) dt = 0.1f;



    // =====================
// カーソル スケールアニメ（常時）
// =====================
    if (selects_[10]) {

        // 時間更新
        cursorScaleTime_ += dt * cursorScaleSpeed_;

        // sin を 0〜1 に正規化
        float s = (sinf(cursorScaleTime_) + 1.0f) * 0.5f;

        // easeInOut
        float ease = s * s * (3.0f - 2.0f * s);

        // スケール補間
        float scale =
            cursorScaleMin_ * (1.0f - ease) +
            cursorScaleMax_ * ease;

        selects_[10]->mainPosition.transform.scale =
            Vector3(scale, scale, 1);
    }


    moziOrbitTime_ += dt;
    float t = moziOrbitTime_ / moziOrbitDuration_;
    if (t > 1.0f) t = 1.0f;

    // easeInOut
    float easeT = t * t * (3.0f - 2.0f * t);

    // 半径収束
    float currentRadius =
        moziStartRadius_ * (1.0f - easeT) +
        moziEndRadius_ * easeT;

    if (!moziOrbitFinished_) {
        for (int i = 0; i < kMoziPartCount; i++) {
            if (!mozis_[i]) continue;

            moziOrbit_[i].angle += moziRotateSpeed_ * dt;

            float a = moziOrbit_[i].angle;
            float r = currentRadius;

            Vector3 pos;
            pos.x = moziOrbit_[i].center.x + cosf(a) * r;
            pos.y = moziOrbit_[i].center.y + sinf(a) * r;
            pos.z = moziOrbit_[i].center.z;

            mozis_[i]->mainPosition.transform.translate = pos;
            mozis_[i]->mainPosition.transform.rotate = { 0.0f, -a, 0.0f };
        }
    }


    // ===== 円運動終了検出 =====
    if (!moziOrbitFinished_ && moziOrbitTime_ >= moziOrbitDuration_) {
        moziOrbitFinished_ = true;

        moziSettlePlaying_ = true;
        moziSettleIndex_ = 0;
        moziSettleT_ = 0.0f;

        for (int i = 0; i < kMoziPartCount; i++) {
            moziStartPos_[i] = mozis_[i]->mainPosition.transform.translate;
            moziStartRot_[i] = mozis_[i]->mainPosition.transform.rotate;
            moziStartScale_[i] = mozis_[i]->mainPosition.transform.scale;
        }
    }


    // ===== 文字配置イージング =====
    if (moziSettlePlaying_) {
        moziSettleT_ += dt;
        float t = moziSettleT_ / moziSettleDuration_;
        if (t > 1.0f) t = 1.0f;

        // easeInOut
        float ease = t * t * (3.0f - 2.0f * t);

        int moziIdx = moziSettleOrder_[moziSettleIndex_];

        // 補間
        mozis_[moziIdx]->mainPosition.transform.translate =
            moziStartPos_[moziIdx] * (1.0f - ease) +
            moziFinalPos_[moziIdx] * ease;

        mozis_[moziIdx]->mainPosition.transform.rotate =
            moziStartRot_[moziIdx] * (1.0f - ease) +
            moziFinalRot_[moziIdx] * ease;

        mozis_[moziIdx]->mainPosition.transform.scale =
            moziStartScale_[moziIdx] * (1.0f - ease) +
            moziFinalScale_[moziIdx] * ease;

        // 1文字分完了
        if (t >= 1.0f) {
            // 最終値で固定
            mozis_[moziIdx]->mainPosition.transform.translate = moziFinalPos_[moziIdx];
            mozis_[moziIdx]->mainPosition.transform.rotate = moziFinalRot_[moziIdx];
            mozis_[moziIdx]->mainPosition.transform.scale = moziFinalScale_[moziIdx];

            // ★ 1文字目だけパンチ演出開始
            if (moziIdx == 1) {
                punchAnimPlaying_ = true;
                punchAnimReturning_ = false;
                punchAnimT_ = 0.0f;

                punchScale_ = punchScaleStart_; // 念のため
            }

            // ★ この文字の位置でスパーク発生
            StartSparkEffect(
                moziFinalPos_[moziIdx], // ← この文字の最終位置
                6,                      // 数
                0.8f,                   // 少し短めが気持ちいい
                0.08f                   // 連続感
            );
            StartSparkEffect2(
                moziFinalPos_[moziIdx], // ← この文字の最終位置
                6,                      // 数
                0.8f,                   // 少し短めが気持ちいい
                0.08f                   // 連続感
            );
            // 次の文字へ
            moziSettleIndex_++;
            moziSettleT_ = 0.0f;

            // 全文字終了
            if (moziSettleIndex_ >= kMoziPartCount) {
                moziSettlePlaying_ = false;
            }
        }

    }

    for (int i = 0; i < kMoziPartCount; i++) {
        if (!mozis_[i]) continue;

   /*     mozis_[i]->mainPosition.transform.translate = moziPos_[i];
        mozis_[i]->mainPosition.transform.rotate = moziRot_[i];
        mozis_[i]->mainPosition.transform.scale = moziScale_[i];*/

        mozis_[i]->Update(usingCamera_);
    }
    // ----------------------
    // Transform 反映
    // ----------------------

    // =====================
// パンチ スケール演出
// =====================
    if (punchAnimPlaying_) {
        punchAnimT_ += dt;
        float t = punchAnimT_ / punchAnimDuration_;
        if (t > 1.0f) t = 1.0f;

        float ease = t * t * (3.0f - 2.0f * t);

        if (!punchAnimReturning_) {
            // =========================
            // 拡大フェーズ
            // =========================
            punchScale_ =
                punchScaleStart_ * (1.0f - ease) +
                punchScalePeak_ * ease;

            // ★ 拡大終了「瞬間」
            if (t >= 1.0f) {

                // ★ ここで iris 開始
                if (!irisPlaying_) {
                    irisPlaying_ = true;
                    irisT_ = 0.0f;
                }

                punchAnimReturning_ = true;
                punchAnimT_ = 0.0f;
            }
        }
        else {
            // =========================
            // 縮小フェーズ
            // =========================
            punchScale_ =
                punchScalePeak_ * (1.0f - ease) +
                punchScaleStart_ * ease;

            if (t >= 1.0f) {
                punchAnimPlaying_ = false;
                punchScale_ = punchScaleStart_;

                cameraMovePlaying_ = true;
                cameraMoveT_ = 0.0f;
                cameraPrevPos_ = cameraStartPos_;
            }
        }
    }



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
    if (irisPlaying_ && irisHole_) {
        irisT_ += system_->GetDeltaTime() * irisSpeed_;
        if (irisT_ > 1.0f) irisT_ = 1.0f;

        float s = irisMaxScale_ * irisT_;   // ★ 0 → MAX
        irisHole_->mainPosition.transform.scale = { s, s, 0.2f };

        if (irisT_ >= 1.0f) {
            irisPlaying_ = false;
        }
    }

    // ★ Update は毎フレ必ず呼ぶ（位置固定・追従しない）
    irisHole_->Update(usingCamera_);


    // Mキーでカメラ移動開始
    if (!cameraMovePlaying_ && system_->GetTriggerOn(DIK_M)) {
        cameraMovePlaying_ = true;
        cameraMoveT_ = 0.0f;
        cameraPrevPos_ = cameraStartPos_;
    }

    // カメラ補間
    if (cameraMovePlaying_) {
        cameraMoveT_ += system_->GetDeltaTime() * cameraMoveSpeed_;
        if (cameraMoveT_ > 1.0f) cameraMoveT_ = 1.0f;

        // easeInOut（滑らか）
        float t = cameraMoveT_;
        float easeT = t * t * (3.0f - 2.0f * t);

        Vector3 currentPos =
            cameraStartPos_ * (1.0f - easeT) +
            cameraEndPos_ * easeT;

        // 差分だけ Move
        Vector3 delta = currentPos - cameraPrevPos_;
        camera_->Move(delta);

        cameraPrevPos_ = currentPos;

        if (cameraMoveT_ >= 1.0f) {
            cameraMovePlaying_ = false;
        }
    }

    if (!gateOpenTriggered_ && system_->GetIsPush(DIK_O)) {
        gateOpenTriggered_ = true;

        gateOpenPlaying_ = true;
        gateOpenT_ = 0.0f;

        gateStartRotY_[0] = gateRot_[0].y;
        gateStartRotY_[1] = gateRot_[1].y;
    }

   

    if (system_->GetTriggerOn(DIK_P)) {
        StartSparkEffect(
            Vector3(0.0f, 17.5f, 0.0f), // 基準位置
            6,        // 1回あたりの数
            1.5f,     // 演出全体時間
            0.15f     // 発生間隔
        );
    }
    UpdateSparkEffect();
    UpdateSparkEffect2();
    if (gateOpenPlaying_) {
        gateOpenT_ += dt;
        float t = gateOpenT_ / gateOpenDuration_;
        if (t > 1.0f) t = 1.0f;

        // easeInOut
        float ease = t * t * (3.0f - 2.0f * t);

        // =========================
        // 門の回転
        // =========================
        gateRot_[0].y =
            gateStartRotY_[0] * (1.0f - ease) +
            gateTargetRotY_[0] * ease;

        gateRot_[1].y =
            gateStartRotY_[1] * (1.0f - ease) +
            gateTargetRotY_[1] * ease;

        // =========================
        // ★ カメラシェイク（滑らか）
        // =========================
        gateShakeTime_ += dt;

        // ★ 開始と終了を0にする係数
        float shakeWeight = ease * (1.0f - ease);
        shakeWeight *= 4.0f; // 山をはっきりさせる（重要）

        float shakeX =
            sinf(gateShakeTime_ * gateShakeFreq_) *
            gateShakeStrength_ * shakeWeight;

        float shakeY =
            cosf(gateShakeTime_ * gateShakeFreq_ * 1.3f) *
            gateShakeStrength_ * 0.6f * shakeWeight;

        camera_->Move({ shakeX, shakeY, 0.0f });

        // =========================
        // 終了
        // =========================
        if (t >= 1.0f) {
            gateRot_[0].y = gateTargetRotY_[0];
            gateRot_[1].y = gateTargetRotY_[1];
            gateOpenPlaying_ = false;
            gateShakeTime_ = 0.0f;

            if (!cameraWalkPlaying_) {
                cameraWalkPlaying_ = true;
                cameraWalkT_ = 0.0f;
                cameraWalkBobTime_ = 0.0f;

                cameraWalkStart_ = cameraPrevPos_; // 今のカメラ位置
            }

        }
    }

    if (cameraWalkPlaying_) {
        cameraWalkT_ += dt;
        float t = cameraWalkT_ / cameraWalkDuration_;
        if (t > 1.0f) t = 1.0f;

        // easeInOut
        float ease = t * t * (3.0f - 2.0f * t);

        // 基本の補間位置
        Vector3 basePos =
            cameraWalkStart_ * (1.0f - ease) +
            cameraWalkEnd_ * ease;

        // 歩行っぽい上下揺れ（最初と最後は0）
        cameraWalkBobTime_ += dt;
        float bobWeight = ease * (1.0f - ease) * 4.0f;
        float bobY =
            sinf(cameraWalkBobTime_ * cameraWalkBobFreq_) *
            cameraWalkBobAmp_ * bobWeight;

        Vector3 finalPos = basePos;
        finalPos.y += bobY;

        // 差分移動
        Vector3 delta = finalPos - cameraPrevPos_;
        camera_->Move(delta);
        cameraPrevPos_ = finalPos;

        if (t >= 1.0f) {
            cameraWalkPlaying_ = false;
            cameraPrevPos_ = cameraWalkEnd_;
        }
    }

    sprite_->Update(usingCamera_);

    if (system_->GetTriggerOn(DIK_SPACE)) {
        ChangeNextStage(SceneNum::S_STAGE_01);
    }
}

void SceneTitle::Draw() {
    for (int i = 0; i < kGatePartCount; i++) {
        if (gates_[i]) {
            gates_[i]->Draw();
        }
    }
    for (int i = 0; i < 11; i++) {
        if (selects_[i]) {
            selects_[i]->Draw();
        }
    }
    for (int i = 11; i < 14; i++) {
        if (selectIndex_==0) {
            selects_[11]->Draw();
        }
        if (selectIndex_ == 1) {
            selects_[12]->Draw();
        }
        if (selectIndex_ == 2) {
            selects_[13]->Draw();
        }


    }




    for (int i = 0; i < kMoziPartCount; i++) {
        if (mozis_[i]) {
            mozis_[i]->Draw();
        }
    }
    for (auto& s : sparks_) {
        if (s.obj) {
            s.obj->Draw();
        }
    }
#ifdef _DEBUG
    ImGui::Begin("Gate Debug");

    for (int i = 0; i < kGatePartCount; i++) {
        if (!gates_[i]) continue;

        ImGui::PushID(i);

        ImGui::Text("Gate %d", i);
        ImGui::DragFloat3("Position", &gatePos_[i].x, 0.05f);
        ImGui::DragFloat3("Rotation", &gateRot_[i].x, 0.01f);
        ImGui::DragFloat3("Scale", &gateScale_[i].x, 0.01f);

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::End();
#endif

#ifdef _DEBUG
    ImGui::Begin("Punch Debug");

    ImGui::Checkbox("Enable Punch Debug", &showPunchDebug_);

    if (showPunchDebug_) {
        ImGui::DragFloat3("Position", &punchPos_.x, 0.05f);
        ImGui::DragFloat3("Rotation", &punchRot_.x, 1.0f);
        ImGui::DragFloat3("Scale", &punchScale_.x, 0.01f);
    }

       ImGui::DragFloat3("Pwosition", &selects_Pos_[10].x, 0.05f);
      

    ImGui::End();
#endif

#ifdef _DEBUG
    ImGui::Begin("Mozi Debug");

    for (int i = 0; i < 2; i++) {
        ImGui::PushID(i);

        ImGui::Text("Mozi %d", i);
        ImGui::DragFloat3("Position", &moziPos_[i].x, 0.05f);
        ImGui::DragFloat3("Rotation", &moziRot_[i].x, 0.01f);
        ImGui::DragFloat3("Scale", &moziScale_[i].x, 0.01f);

        ImGui::Separator();
        ImGui::PopID();
    }

    ImGui::End();
#endif

    // ★ こぶし（門より前）
    if (punch_) {
        punch_->Draw();
    }
    // 穴あきモデルを最前面に描画
    if (irisHole_) {
        irisHole_->Draw();
    }
    sprite_->Draw();
  
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
void SceneTitle::StartSparkEffect(
    const Vector3& basePos,
    int countPerBurst,
    float duration,
    float interval
) {
    sparkPlaying_ = true;

    // ★ タイマーは「上書き」しない
    sparkTimer_ = 0.0f;
    sparkIntervalTimer_ = 0.0f;

    sparkBasePos_ = basePos;
    sparkCountPerBurst_ = countPerBurst;
    sparkDuration_ = duration;
    sparkInterval_ = interval;

    // ★ sparks_.clear(); ← 削除！！
}

void SceneTitle::UpdateSparkEffect() {

    float dt = system_->GetDeltaTime();

    // sec / ms / us 吸収
    if (dt > 1000.0f) {
        dt *= 0.000001f;
    }
    else if (dt > 1.0f) {
        dt *= 0.001f;
    }

    if (dt > 0.1f) dt = 0.1f;
    if (dt < 0.0f) dt = 0.0f;

    // ★ 演出中のみタイマー更新
    if (sparkPlaying_) {
        sparkTimer_ += dt;
        sparkIntervalTimer_ += dt;
    }

    // =========================
    // 発生（sparkPlaying_ 中のみ）
    // =========================
    if (sparkPlaying_ && sparkIntervalTimer_ >= sparkInterval_) {
        sparkIntervalTimer_ = 0.0f;

        for (int i = 0; i < sparkCountPerBurst_; i++) {
            Spark s{};
            s.obj = new Object;
            s.obj->IntObject(system_);
            s.obj->CreateDefaultData();
            s.obj->CreateModelData(
                system_->SetModelObj("resources/gate/model/spark2.obj")
            );

            float angle = (rand() % 360) * (3.141592f / 180.0f);
            float radius = 1.5f;

            Vector3 dir = { cosf(angle), sinf(angle), 0.0f };

            float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.0001f) {
                float inv = 1.0f / len;
                dir.x *= inv;
                dir.y *= inv;
            }

            float speed = 2.5f + (rand() % 100) * 0.02f;
            s.velocity = dir * speed;

            s.obj->mainPosition.transform.translate =
                sparkBasePos_ + Vector3(dir.x * radius, dir.y * radius, 0.0f);

            s.startScale = { 0.5f, 0.5f, 0.5f };
            s.obj->mainPosition.transform.scale = s.startScale;

            float rotZ = atan2f(dir.y, dir.x) - 3.141592f * 0.5f;
            s.obj->mainPosition.transform.rotate = { 0.0f, 0.0f, rotZ };

            for (auto& part : s.obj->objectParts_) {
                if (!part.materialConfig) {
                    part.materialConfig = std::make_shared<MaterialConfig>();
                }
                part.materialConfig->useModelTexture = false;
                part.materialConfig->textureHandle =
                    system_->LoadTextrue("resources/gate/model/punch.png");
                part.materialConfig->enableLighting = false;
            }

            s.life = 0.0f;
            s.lifeMax = 0.8f + (rand() % 100) * 0.004f;

            sparks_.push_back(s);
        }
    }

    // =========================
    // 更新（★常に回す）
    // =========================
    for (auto it = sparks_.begin(); it != sparks_.end();) {
        it->life += dt;
        float t = it->life / it->lifeMax;
        if (t > 1.0f) t = 1.0f;

        it->velocity *= (1.0f - 3.0f * dt);
        it->obj->mainPosition.transform.translate += it->velocity * dt;

        Vector3 v = it->velocity;
        float vlen = sqrtf(v.x * v.x + v.y * v.y);
        if (vlen > 0.0001f) {
            float inv = 1.0f / vlen;
            v.x *= inv;
            v.y *= inv;
        }

        float rotZ = atan2f(v.y, v.x) - 3.141592f * 0.5f;
        it->obj->mainPosition.transform.rotate = { 0.0f, 0.0f, rotZ };

        float ease = 1.0f - (t * t * (3.0f - 2.0f * t));
        it->obj->mainPosition.transform.scale = it->startScale * ease;

        it->obj->Update(usingCamera_);

        if (it->life >= it->lifeMax) {
            delete it->obj;
            it = sparks_.erase(it);
        }
        else {
            ++it;
        }
    }

    // =========================
    // 演出終了判定
    // =========================
    if (sparkPlaying_ && sparkTimer_ >= sparkDuration_) {
        sparkPlaying_ = false;
    }
}
void SceneTitle::StartSparkEffect2(
    const Vector3& basePos,
    int countPerBurst,
    float duration,
    float interval
) {
    sparkPlaying_ = true;

    // ★ タイマーは「上書き」しない
    sparkTimer_ = 0.0f;
    sparkIntervalTimer_ = 0.0f;

    sparkBasePos_ = basePos;
    sparkCountPerBurst_ = countPerBurst;
    sparkDuration_ = duration;
    sparkInterval_ = interval;

    // ★ sparks_.clear(); ← 削除！！
}

void SceneTitle::UpdateSparkEffect2() {

    float dt = system_->GetDeltaTime();

    // sec / ms / us 吸収
    if (dt > 1000.0f) {
        dt *= 0.000001f;
    }
    else if (dt > 1.0f) {
        dt *= 0.001f;
    }

    if (dt > 0.1f) dt = 0.1f;
    if (dt < 0.0f) dt = 0.0f;

    // ★ 演出中のみタイマー更新
    if (sparkPlaying_) {
        sparkTimer_ += dt;
        sparkIntervalTimer_ += dt;
    }

    // =========================
    // 発生（sparkPlaying_ 中のみ）
    // =========================
    if (sparkPlaying_ && sparkIntervalTimer_ >= sparkInterval_) {
        sparkIntervalTimer_ = 0.0f;

        for (int i = 0; i < sparkCountPerBurst_; i++) {
            Spark s{};
            s.obj = new Object;
            s.obj->IntObject(system_);
            s.obj->CreateDefaultData();
            s.obj->CreateModelData(
                system_->SetModelObj("resources/gate/model/spark.obj")
            );

            float angle = (rand() % 360) * (3.141592f / 180.0f);
            float radius = 0.8f;

            Vector3 dir = { cosf(angle), sinf(angle), 0.0f };

            float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
            if (len > 0.0001f) {
                float inv = 1.0f / len;
                dir.x *= inv;
                dir.y *= inv;
            }

            float speed = 2.5f + (rand() % 100) * 0.02f;
            s.velocity = dir * speed;

            s.obj->mainPosition.transform.translate =
                sparkBasePos_ + Vector3(dir.x * radius, dir.y * radius, 0.0f);

            s.startScale = { 0.5f, 0.5f, 0.5f };
            s.obj->mainPosition.transform.scale = s.startScale;

            float rotZ = atan2f(dir.y, dir.x) - 3.141592f * 0.5f;
            s.obj->mainPosition.transform.rotate = { 0.0f, 0.0f, rotZ };

            for (auto& part : s.obj->objectParts_) {
                if (!part.materialConfig) {
                    part.materialConfig = std::make_shared<MaterialConfig>();
                }
                part.materialConfig->useModelTexture = false;
                part.materialConfig->textureHandle =
                    system_->LoadTextrue("resources/gate/model/punch.png");
                part.materialConfig->enableLighting = false;
            }

            s.life = 0.0f;
            s.lifeMax = 0.8f + (rand() % 100) * 0.004f;

            sparks_.push_back(s);
        }
    }

    // =========================
    // 更新（★常に回す）
    // =========================
    for (auto it = sparks_.begin(); it != sparks_.end();) {
        it->life += dt;
        float t = it->life / it->lifeMax;
        if (t > 1.0f) t = 1.0f;

        it->velocity *= (1.0f - 3.0f * dt);
        it->obj->mainPosition.transform.translate += it->velocity * dt;

        Vector3 v = it->velocity;
        float vlen = sqrtf(v.x * v.x + v.y * v.y);
        if (vlen > 0.0001f) {
            float inv = 1.0f / vlen;
            v.x *= inv;
            v.y *= inv;
        }

        float rotZ = atan2f(v.y, v.x) - 3.141592f * 0.5f;
        it->obj->mainPosition.transform.rotate = { 0.0f, 0.0f, rotZ };

        float ease = 1.0f - (t * t * (3.0f - 2.0f * t));
        it->obj->mainPosition.transform.scale = it->startScale * ease;

        it->obj->Update(usingCamera_);

        if (it->life >= it->lifeMax) {
            delete it->obj;
            it = sparks_.erase(it);
        }
        else {
            ++it;
        }
    }

    // =========================
    // 演出終了判定
    // =========================
    if (sparkPlaying_ && sparkTimer_ >= sparkDuration_) {
        sparkPlaying_ = false;
    }
}
