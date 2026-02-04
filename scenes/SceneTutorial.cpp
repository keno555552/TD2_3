#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "SceneTutorial.h"
#include "SceneManager.h" // 次のシーンへ遷移する場合に必要
#include <numbers>

SceneTutorial::SceneTutorial(kEngine* system) {
    system_ = system;

    // ■ カメラ生成
    camera_ = system_->CreateCamera();
    camera_->Move({ 0.0f, 10.0f, -15.0f }); // 見下ろし視点
    camera_->SetFarClip(200000.0f);
    usingCamera_ = camera_;
    system_->SetCamera(usingCamera_);

	int skydomeModelHandle = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	skydome_ = new Object;
	skydome_->IntObject(system_);
	skydome_->CreateModelData(skydomeModelHandle);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;

    // ■ プレイヤー生成
	playerModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/charater/simplePlayer.obj");
	player_ = new Player(system_, Vector3(0.0f, 0.0f, 5.0f));
	player_->CreateModelData(playerModelHandle_);
	player_->mainPosition.transform.scale = kBaseActorScale_;

    // ■ 説明用テキストオブジェクト（最初は「移動」の説明）
    textObj_ = new Object();
    textObj_->IntObject(system_);
    // ★重要: ここで各ステップの3D文字モデルをロードしてください
    // 例: "resources/tutorial/text_move.obj"
    ChangeTextModel("resources/TemplateResource/object/plane/plane.obj");

    // テキストの位置調整（画面奥や上部に配置）
    textObj_->mainPosition.transform.translate = { 0.0f, 5.0f, 10.0f };
    textObj_->mainPosition.transform.scale = { 2.0f, 2.0f, 2.0f };

	int tatamiHandle = system_->SetModelObj("resources/game/model/tatami.obj");
	tatami_ = new Object();
	tatami_->IntObject(system_);
	tatami_->CreateModelData(tatamiHandle);
	tatami_->mainPosition.transform.translate = { 0.0f, 0.0f, 10.0f };
	tatami_->mainPosition.transform.scale = { 2.0f, 1.0f, 3.0f };
	
	// ■ 進捗バーの生成
	// ★適当な白い四角画像を読み込むか、既存の画像を使用してください
	// ここでは仮に "white.png" としていますが、単色の四角があれば何でもOKです
	int barTex = system_->LoadTexture("resources/TemplateResource/texture/white5x5.png");
	// 背景（枠）：黒やグレーなどで表示
	barFrame_ = new SimpleSprite();
	barFrame_->IntObject(system_);
	barFrame_->CreateDefaultData();
	barFrame_->objectParts_[0].materialConfig->textureHandle = barTex;
	barFrame_->mainPosition.transform.translate ={ kBarPos_.x - 8.0f, kBarPos_.y - 3.0f, 0.0f };
	barFrame_->mainPosition.transform.scale = { kBarScale_.x + 3.0f, kBarScale_.y + 4.0f, 1.0f };
	barFrame_->objectParts_[0].materialConfig->textureColor = {0.0f, 0.0f, 0.0f, 0.4f};
	barFrame_->objectParts_[0].anchorPoint = { 0.0f, 0.0f };
	// ※枠なので少し大きく、色はグレーに

	// 空（ゲージ）：白などで表示
	barBrank_ = new SimpleSprite();
	barBrank_->IntObject(system_);
	barBrank_->CreateDefaultData();
	barBrank_->objectParts_[0].materialConfig->textureHandle = barTex;
	barBrank_->mainPosition.transform.translate = { kBarPos_.x, kBarPos_.y + 7.0f, 0.0f };
	barBrank_->mainPosition.transform.scale = { kBarScale_.x, kBarScale_.y, 1.0f };
	barBrank_->objectParts_[0].materialConfig->textureColor = { 1.0f, 1.0f, 1.0f, 0.6f };
	barBrank_->objectParts_[0].anchorPoint = { 0.0f, 0.0f };

	// 中身（ゲージ）：緑や黄色で表示
	barFill_ = new SimpleSprite();
	barFill_->IntObject(system_);
	barFill_->CreateDefaultData();
	barFill_->objectParts_[0].materialConfig->textureHandle = barTex;
	barFill_->mainPosition.transform.translate = { kBarPos_.x, kBarPos_.y + 7.0f, 0.0f };
	barFill_->mainPosition.transform.scale = { 0.0f, kBarScale_.y, 1.0f };
	barFill_->objectParts_[0].materialConfig->textureColor = {0.0f, 1.0f, 0.0f, 0.6f};
	barFill_->objectParts_[0].anchorPoint = { 0.0f, 0.0f };
	// 初期サイズは横幅0、色は緑
}

SceneTutorial::~SceneTutorial() {
    if (camera_) system_->DestroyCamera(camera_);
	usingCamera_ = nullptr;

	delete skydome_;
	if (tatami_) delete tatami_;
    if (player_) delete player_;
    if (textObj_) delete textObj_;
	if (barFrame_) delete barFrame_;
	if (barBrank_) delete barBrank_;
	if (barFill_) delete barFill_;

	for (auto& data : enemies_) {
		delete data.enemy;
	}
    enemies_.clear();
}

void SceneTutorial::Update() {
    // 共通の更新
	skydome_->Update(usingCamera_);
	if (tatami_) tatami_->Update(usingCamera_);
    if (textObj_) textObj_->Update(usingCamera_);

	if (player_) {
		player_->Update(usingCamera_);

		float limitX = 15.0f - 0.25f;
		float limitZ = 15.0f - 0.25f;
		float minLimit = -15.0f; // 下限

		if (player_->mainPosition.transform.translate.x > limitX) {
			player_->mainPosition.transform.translate.x = limitX;
		}
		if (player_->mainPosition.transform.translate.x < minLimit) {
			player_->mainPosition.transform.translate.x = minLimit;
		}
		if (player_->mainPosition.transform.translate.z > limitZ) {
			player_->mainPosition.transform.translate.z = limitZ;
		}

		if (player_->mainPosition.transform.translate.z < minLimit) {
			player_->mainPosition.transform.translate.z = minLimit;
		}
	}

	CameraPart();

	if (system_->GetTriggerOn(DIK_RETURN) || system_->GetGamepadIsPush(VK_PAD_START)) {
		outcome_ = SceneOutcome::NEXT;
	}

	// ■ バーの更新処理
	barFrame_->mainPosition.materialConfig->textureColor = { 0.2f, 0.2f, 0.2f, 1.0f };
	if (barFill_) {
		barFill_->mainPosition.materialConfig->textureColor = { 0.0f, 1.0f, 0.0f, 1.0f };
		float ratio = GetProgressRatio();

		// 中身のスケール（横幅）を更新
		float currentWidth = kBarScale_.x * ratio;

		barFill_->mainPosition.transform.scale.x = currentWidth;
	}

    // ステップ完了後の待機時間処理
    if (isStepCompleted_) {
        stepWaitTimer_ += system_->GetDeltaTime();

		for (int i = 0; i < enemies_.size(); i++) {

			// 参照で受ける（書き換えられるように auto& にする）
			auto& data = enemies_[i];

			// 敵の更新（倒されていてもノックバック処理等のために更新は続ける）
			data.enemy->Update(usingCamera_);
		}
        if (stepWaitTimer_ > 1.5f) { // 1.5秒待って次へ
            stepWaitTimer_ = 0.0f;
            isStepCompleted_ = false;

            // 次のステップへ移行
            switch (currentStep_) {
            case TutorialStep::Move:
                currentStep_ = TutorialStep::ArmExpand;
                ChangeTextModel("resources/TemplateResource/object/arrow/arrow.obj"); // 腕の説明へ
                break;
            case TutorialStep::ArmExpand:
                currentStep_ = TutorialStep::Dash;
                ChangeTextModel("resources/TemplateResource/object/plane/plane.obj"); // 加速の説明へ
                break;
            case TutorialStep::Dash:
                currentStep_ = TutorialStep::Battle;
                ChangeTextModel("resources/TemplateResource/object/crown/crown.obj"); // 戦闘の説明へ
                // 敵をスポーンさせる
                {
					// 敵を生成してリストに追加
                    Enemy* e1 = new Enemy(system_, Vector3(-10.0f, 20.0f, 10.0f));
					e1->CreateModelData(playerModelHandle_);
					e1->SetIsTutorial(true);
					e1->mainPosition.transform.scale = kBaseActorScale_;
                    Enemy* e2 = new Enemy(system_, Vector3(10.0f, 20.0f, 10.0f));
					e2->CreateModelData(playerModelHandle_);
					e2->SetIsTutorial(true);
					e2->mainPosition.transform.scale = kBaseActorScale_;

					// 構造体を作って登録
					TutorialEnemyData data1;
					data1.enemy = e1;
					data1.isDefeated = false;
					// ★ アニメーション設定
					data1.isSpawning = true;
					data1.spawnTimer = 0.0f;
					data1.spawnStartPos = { -10.0f, 20.0f, 10.0f }; // 開始
					data1.spawnEndPos = { -10.0f,  0.0f, 10.0f }; // 着地
					TutorialEnemyData data2;
					data2.enemy = e2;
					data2.isDefeated = false;
					// ★ アニメーション設定
					data2.isSpawning = true;
					data2.spawnTimer = 0.0f;
					data2.spawnStartPos = { 10.0f, 20.0f, 10.0f };
					data2.spawnEndPos = { 10.0f,  0.0f, 10.0f };

                    enemies_.push_back(data1);
                    enemies_.push_back(data2);
                }
                break;
            case TutorialStep::Battle:
				// ▼▼▼ カメラアニメーションの開始設定 ▼▼▼
				isCameraTransiting_ = true;
				cameraTransitionTimer_ = 0.0f;

				// 1. 今のカメラの位置・回転を「開始地点」として保存
				Transform currentT = camera_->GetTransform();
				camAnimStartPos_ = currentT.translate;
				camAnimStartRot_ = currentT.rotate;

				// 2. 最終的な目標位置「終了地点」を設定 (CameraPartのif文の中身と同じ値)
				camAnimEndPos_ = { 0.0f, 5.0f, -10.0f };
				camAnimEndRot_ = { 0.1f, std::numbers::pi_v<float> *2.0f, 0.0f };
				// ▲▲▲ ここまで ▲▲▲

                currentStep_ = TutorialStep::Caution;
                ChangeTextModel("resources/TemplateResource/object/plane/plane.obj"); // 注意事項へ
                break;
            case TutorialStep::Caution:
                currentStep_ = TutorialStep::Finished;
                break;
            }
        }
        return; // ウェイト中は操作判定しない
    }

    // 各ステップの更新
    switch (currentStep_) {
    case TutorialStep::Move:      UpdateStepMove(); break;
    case TutorialStep::ArmExpand: UpdateStepExpand(); break;
    case TutorialStep::Dash:      UpdateStepDash(); break;
    case TutorialStep::Battle:    UpdateStepBattle(); break;
    case TutorialStep::Caution:   UpdateStepCaution(); break;
    case TutorialStep::Finished:
        // 完了したらタイトルやステージ1へ
        outcome_ = SceneOutcome::NEXT;
        break;
    }
}

void SceneTutorial::Draw() {
	skydome_->Draw();
	if (tatami_) tatami_->Draw();
    if (player_) player_->Draw();

	for (auto& data : enemies_) {
		data.enemy->Draw();
	}

    if (textObj_) textObj_->Draw();

	if (currentStep_ != TutorialStep::Finished && currentStep_ != TutorialStep::Caution) {
		// UIは最後に描画（最前面）
		if (barFrame_) barFrame_->Draw();
		if (barBrank_) barBrank_->Draw();
		if (barFill_) barFill_->Draw();
	}
}

Camera* SceneTutorial::GetCamera() {
	return usingCamera_;   // ★ 絶対に camera_ じゃない
}

void SceneTutorial::CameraPart() {
	// 最終ステップ（Caution / Finished）の場合
	if (currentStep_ == TutorialStep::Caution || currentStep_ == TutorialStep::Finished) {
		// 最短経路で角度を補間する関数
		auto LerpAngle = [&](float a, float b, float t) {
			float diff = b - a;
			// 差分を -PI ~ PI (-180度 ~ 180度) の範囲に収める
			while (diff > std::numbers::pi_v<float>) diff -= 2.0f * std::numbers::pi_v<float>;
			while (diff < -std::numbers::pi_v<float>) diff += 2.0f * std::numbers::pi_v<float>;
			return a + diff * t;
			};
		// イージング関数 (EaseInOutCubic: 最初と最後がゆっくり)
		auto EaseInOutCubic = [&](float x) {
			return x < 0.5f ? 4.0f * x * x * x : 1.0f - std::pow(-2.0f * x + 2.0f, 3.0f) / 2.0f;
			};

		Transform cameraTransform = CreateDefaultTransform();

		// ■ アニメーション中
		if (isCameraTransiting_) {
			cameraTransitionTimer_ += system_->GetDeltaTime();

			// 進行度 (0.0 -> 1.0)
			float t = cameraTransitionTimer_ / kCameraTransitionDuration_;
			if (t >= 1.0f) {
				t = 1.0f;
				isCameraTransiting_ = false; // 完了
			}

			// イージング適用
			float easeT = EaseInOutCubic(t);

			// 座標の線形補間 (Lerp)
			cameraTransform.translate = camAnimStartPos_ * (1.0f - easeT) + camAnimEndPos_ * easeT;

			// 回転の補間 (角度対応)
			cameraTransform.rotate.x = std::lerp(camAnimStartRot_.x, camAnimEndRot_.x, easeT);
			cameraTransform.rotate.y = LerpAngle(camAnimStartRot_.y, camAnimEndRot_.y, easeT); // Y軸は最短回転
			cameraTransform.rotate.z = std::lerp(camAnimStartRot_.z, camAnimEndRot_.z, easeT);
		}
		// ■ アニメーション完了後（固定位置）
		else {
			cameraTransform.translate = camAnimEndPos_;
			cameraTransform.rotate = camAnimEndRot_;
		}

		camera_->SetCamera(cameraTransform);
		usingCamera_ = camera_;

	} else {
		// ---------------------------------------------------------
		// 1. 手動ズーム入力の処理 (W:接近 / S:離れる)
		// ---------------------------------------------------------
		float dt = system_->GetDeltaTime();

		// Sキー: ズームアウト（距離を足す）
		if (system_->GetIsPush(DIK_S) || system_->GetGamepadIsPush(VK_PAD_A)) {
			manualZoomOffset_ += kManualZoomSpeed_ * dt;
		}
		// Wキー: ズームイン（距離を引く）
		else if (system_->GetIsPush(DIK_W) || system_->GetGamepadIsPush(VK_PAD_B)) {
			manualZoomOffset_ -= kManualZoomSpeed_ * dt;
		}

		// 行き過ぎないように制限 (Clamp)
		manualZoomOffset_ = std::clamp(manualZoomOffset_, kMinManualZoom_, kMaxManualZoom_);


		// ---------------------------------------------------------
		// 2. カメラ座標の計算
		// ---------------------------------------------------------

		// プレイヤーの情報を取得
		Vector3 playerPos = player_->mainPosition.transform.translate;
		float playerRotY = player_->mainPosition.transform.rotate.y;

		// ★★★ 旧版から統合：回転値を [-PI, PI] に正規化 ★★★
		const float PI = 3.14159265359f;
		const float TWO_PI = 6.28318530718f;
		while (playerRotY > PI) playerRotY -= TWO_PI;
		while (playerRotY < -PI) playerRotY += TWO_PI;

		// 腕の大きさに応じた距離と高さの計算(ズーム処理)
		float maxArmScale = std::max(player_->GetArmScaleL(), player_->GetArmScaleR());
		// 腕が大きくなるほど距離(baseDistance)を増やす
		// 例：基本 + 腕の余剰分 * 倍率
		// ここの数値をいじると「引き」の強さが変わる
		float autoDistance = cameraDistance_ + (maxArmScale - 1.0f) * cameraDistanceMultiply_;
		float autoHeight = cameraHeight_ + (maxArmScale - 1.0f) * cameraHeightMultiply_;

		// ▼▼▼ 最終決定: 自動値 + 手動オフセット ▼▼▼

		// 距離: 自動値 + 手動オフセット
		// ※近すぎるとプレイヤーの中に入り込むので std::max で最低距離(2.0f程度)を保証すると安全です
		float finalDistance = std::max(2.0f, autoDistance + manualZoomOffset_);

		// 高さ: 自動値 + (手動オフセット * 比率)
		// ※離れるほど少し高くすると、プレイヤーが画面中央に留まりやすくなります
		float finalHeight = autoHeight + (manualZoomOffset_ * kManualZoomHeightRatio_);

		// カメラの注視点と位置を計算
		Vector3 eye = {
			playerPos.x + std::sin(playerRotY) * finalDistance,
			playerPos.y + finalHeight,
			playerPos.z + std::cos(playerRotY) * finalDistance
		};

		// Transform構造体を作成して値をセットする
		Transform cameraTransform = CreateDefaultTransform();
		cameraTransform.translate = eye;

		// 回転の設定
		// 前進方向が {-sin, -cos} なので、
		// 基本の向き(0)が Z+方向なら、180度(3.14f)足すと Z-方向（前進方向）を向く。
		cameraTransform.rotate.x = 0.5f;             // 少し見下ろす角度
		cameraTransform.rotate.y = playerRotY + std::numbers::pi_v<float>; // プレイヤーの背中を追う向き
		cameraTransform.rotate.z = 0.0f;

		camera_->SetCamera(cameraTransform);
		usingCamera_ = camera_;
	}

	system_->SetCamera(usingCamera_);
}

// ---------------------------------------------------------
// 各ステップのロジック
// ---------------------------------------------------------

// 1. 基本移動（左右旋回）
void SceneTutorial::UpdateStepMove() {
	// 移動を停止
	player_->SetIsTutorial(true);

    // Aキー or Dキー（あるいは左スティック）が入力されているか
    bool isRotating = false;


	if (system_->GetIsPush(DIK_A) && system_->GetIsPush(DIK_D)) {
		isRotating = false;
	} else if (system_->GetIsPush(DIK_A) || system_->GetIsPush(DIK_D)) {
        isRotating = true;
    }
    // パッドの判定等もあればここに追加
	if (system_->GetGamepadIsPush(VK_PAD_LSHOULDER) && system_->GetGamepadIsPush(VK_PAD_RSHOULDER)) {
		isRotating = false;
	} else if (system_->GetGamepadIsPush(VK_PAD_LSHOULDER) || system_->GetGamepadIsPush(VK_PAD_RSHOULDER)) {
		isRotating = true;
	}

    if (isRotating) {
        rotateTimer_ += system_->GetDeltaTime();
    }

    // 一定時間旋回したら次へ
    if (rotateTimer_ >= kRotateQuota_) {
        isStepCompleted_ = true;
        // 演出：テキストを少し跳ねさせたり色を変えたりしても良い
		player_->ResetArm(false);
		player_->ResetArm(true);
    }
}

// 2. 腕の仕様（拡大）
void SceneTutorial::UpdateStepExpand() {
    // プレイヤーの腕サイズを取得して判定
    // ※Playerクラスに GetArmScale() や GetRightArmScale() がある前提
    float scaleL = player_->GetArmScaleL();
    float scaleR = player_->GetArmScaleR();

    // 一定以上（例: 2.0倍）になったら次へ
    if (scaleL > 2.0f || scaleR > 2.0f) {
        isStepCompleted_ = true;
    }
}

// 3. 加速（チャージして離す）
void SceneTutorial::UpdateStepDash() {
    // 現在のチャージ状態（キーを押しているか）
    bool isCharging = player_->GetIsCharging();
    
    // 「押していた」のに「離した」瞬間、かつ「ある程度チャージされていた」場合
	bool isAttacking = (player_->IsAttackingL() || player_->IsAttackingR());
    if (isAttacking && !wasDashing_) {
		wasDashing_ = isAttacking;
    }

    // 「大きく膨らんでいる状態」から「急に小さくなった（＝発射した）」を検知する
    if (wasCharging_ && !isCharging) {
        // 離して発射したとみなす
		// 移動を停止
		player_->SetIsTutorial(false);
        dashSuccessCount_++;
    }
    // チャージ中フラグ更新
	wasCharging_ = isCharging;

    if (dashSuccessCount_ >= kDashQuota_) {
        isStepCompleted_ = true;
    }
}

// 4. 腕の無敵と無防備（敵を殴る）
void SceneTutorial::UpdateStepBattle() {
	auto EaseOutCubic = [&](float x) {
		return 1.0f - std::pow(1.0f - x, 3.0f);
		};
	// ■ 1. 敵のループ処理
	// イテレータを使うか、インデックスでアクセス
	for (int i = 0; i < enemies_.size(); i++) {

		// 参照で受ける（書き換えられるように auto& にする）
		auto& data = enemies_[i];

		// ▼▼▼ 出現アニメーション処理 ▼▼▼
		if (data.isSpawning) {
			// タイマー進行 (1.0秒で着地とする場合)
			data.spawnTimer += system_->GetDeltaTime();
			float t = data.spawnTimer / 1.0f; // 1.0f は所要時間

			if (t >= 1.0f) {
				t = 1.0f;
				data.isSpawning = false; // アニメーション終了
			}

			// イージング適用 (0.0 -> 1.0)
			float easeT = EaseOutCubic(t);

			// 座標更新 (Lerp計算: start + (end - start) * t)
			Vector3 currentPos;
			currentPos.x = data.spawnStartPos.x + (data.spawnEndPos.x - data.spawnStartPos.x) * easeT;
			currentPos.y = data.spawnStartPos.y + (data.spawnEndPos.y - data.spawnStartPos.y) * easeT;
			currentPos.z = data.spawnStartPos.z + (data.spawnEndPos.z - data.spawnStartPos.z) * easeT;

			data.enemy->mainPosition.transform.translate = currentPos;

			// 落下中は向きやアニメーションを更新するだけにして、攻撃判定などはスキップ
			data.enemy->Update(usingCamera_);
			continue;
		}
		// ▲▲▲ ここまで ▲▲▲

		// 敵の更新（倒されていてもノックバック処理等のために更新は続ける）
		data.enemy->Update(usingCamera_);

		// 既に倒されているなら、衝突判定はスキップする
		if (data.isDefeated) {
			// 必要ならここで「やられた状態」の演出（色を変える、縮むなど）
			data.enemy->ResetArm(false);
			data.enemy->ResetArm(true);
			continue;
		}

		// 常にチャージ状態に見せる（倒される前）
		data.enemy->SetArmScaleL(2.0f);
		data.enemy->SetArmScaleR(2.0f);

		// ■ 2. 攻撃当たり判定
		// ここで「プレイヤーの攻撃がこの敵(data.enemy)に当たったか」をチェックします
		// ※ 内部でヒット処理
		CheckAttackCollision();

		bool isHit = data.enemy->IsDamage();
		if (isHit) {
			// 倒した扱いにする
			data.isDefeated = true;
			killCount_++;
			data.enemy->Update(usingCamera_);
		}
	}

	// ■ 3. 終了判定
	if (killCount_ >= kKillQuota_) {
		isStepCompleted_ = true;
	}
}

// 5. 注意事項（テキスト表示のみ）
void SceneTutorial::UpdateStepCaution() {
	player_->SetIsTutorial(true);
    // 読む時間を確保するため、ボタン入力で終了にする
    if (system_->GetIsPush(DIK_SPACE) || system_->GetGamepadIsPush(VK_PAD_A)) {
        isStepCompleted_ = true;
    }
}

float SceneTutorial::GetProgressRatio() {
	float ratio = 0.0f;

	switch (currentStep_) {
	case TutorialStep::Move:
		// 現在の時間 / ノルマ時間
		ratio = rotateTimer_ / kRotateQuota_;
		break;

	case TutorialStep::ArmExpand:
	{
		if (isStepCompleted_) {
			ratio = 1.0f;
			break;
		}

		// 現在の腕の大きさ / 目標の大きさ (1.0fからスタートなので補正)
		float maxScale = std::max(player_->GetArmScaleL(), player_->GetArmScaleR());
		// 1.0(通常) ～ 2.0(目標) の間を 0.0～1.0 に正規化
		float target = 2.0f;
		ratio = (maxScale - 1.0f) / (target - 1.0f);
		break;
	}

	case TutorialStep::Dash:
		// 成功回数 / ノルマ回数
		ratio = (float)dashSuccessCount_ / (float)kDashQuota_;
		break;

	case TutorialStep::Battle:
		// 撃破数 / ノルマ数
		ratio = (float)killCount_ / (float)kKillQuota_;
		break;

	case TutorialStep::Caution:
		// 確認待ちなので、完了するまでは0、押したら完了扱いになるが画面遷移するので0固定でもOK
		ratio = 0.0f;
		break;

	case TutorialStep::Finished:
		ratio = 1.0f;
		break;
	}

	// 0.0f ～ 1.0f の範囲に収める
	return std::clamp(ratio, 0.0f, 1.0f);
}

void SceneTutorial::ChangeTextModel(const std::string& modelPath) {
	// 安全策: 既存のオブジェクトがあれば一度削除して作り直す
	if (textObj_) {
		delete textObj_;
	}

	textObj_ = new Object();
	textObj_->IntObject(system_);

	// 位置やスケールの再設定（メンバ変数などで管理していればそれを使う）
	textObj_->mainPosition.transform.translate = { 0.0f, 5.0f, 10.0f };
	textObj_->mainPosition.transform.scale = { 2.0f, 2.0f, 2.0f };

	// モデルロードと適用
	int handle = system_->SetModelObj(modelPath);
	textObj_->CreateModelData(handle);

	// 設定適用
	if (!textObj_->objectParts_.empty()) {
		textObj_->objectParts_[0].materialConfig->enableLighting = false;

		// もし plane.obj にテクスチャが無いなら、白くするために以下も必要かもしれません
		// textObj_->objectParts_[0].materialConfig->textureColor = {1,1,1,1};
	}
}

void SceneTutorial::CheckAttackCollision() {
	// ▼▼▼▼▼▼▼▼▼ AABB版：腕の衝突判定（スケール対応版） ▼▼▼▼▼▼▼▼▼

	// 引数に「attackerScale（攻撃主のスケール）」を追加しました
	auto CheckArmCollision = [&](Object* targetObj, const AABB& targetAABB, Vector3 armPos, float armScale, float attackerScale) {
		// ▼▼▼ 追加修正1: 既にダメージを受けている相手は無視する ▼▼▼
		// これがないと、触れている間ずっとスローがかかり続けます
		if (targetObj == player_) {
			if (player_->IsDamage()) return;
		} else {
			Enemy* e = dynamic_cast<Enemy*>(targetObj);
			if (e && e->IsDamage()) return;
		}

		// 1. 腕のAABBを作成
		// 解説: 
		// - kBaseActorScale_ : モデル自体の半径（1mの立方体の半分）
		// - armScale : 腕自体の膨張率（1.0 ~ 5.0）
		// - attackerScale : プレイヤーや敵の全体スケール（0.5 や 1.0 など）
		float armRealRadius = kBaseActorScale_.x * armScale * attackerScale;

		AABB armAABB;
		armAABB.min = { armPos.x - armRealRadius, armPos.y - armRealRadius, armPos.z - armRealRadius };
		armAABB.max = { armPos.x + armRealRadius, armPos.y + armRealRadius, armPos.z + armRealRadius };

		// 2. 衝突判定 (AABB vs AABB)
		if (crashDecision(armAABB, targetAABB)) {
			float duration = 0.3f;

			// 3. 衝突ベクトル計算
			Vector3 targetPos = targetObj->mainPosition.transform.translate;
			Vector3 diff = {
				targetPos.x - armPos.x,
				targetPos.y - armPos.y,
				targetPos.z - armPos.z
			};

			float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
			// 4. ノックバック適用
			if (len > 0.0f) {
				// 【パンチ挙動】 腕の位置から、体の位置へ向かって吹っ飛ばす
				Vector3 pushDir = { diff.x / len, diff.y / len, diff.z / len };

				// ▼▼▼ 追加修正2: 最小ノックバック量の保証 ▼▼▼
				// 計算上の威力
				float rawPower = 8.0f * (armScale * armScale);
				// どんなに弱くても「10.0f」の力は保証する（値は調整してください）
				// これにより、確実に相手と距離が離れます
				float finalPower = std::max(rawPower, 10.0f);

				Vector3 knockbackForce = { pushDir.x * finalPower, 0.0f, pushDir.z * finalPower };

				// ターゲットに適用（Player/Enemyで分岐）
				// ※Objectクラスに仮想関数 AddKnockback があれば分岐不要になります
				if (targetObj == player_) {

					player_->AddKnockback(knockbackForce);
				} else {
					// targetObjが敵リストの中にいるか確認、あるいはキャストして適用
					// ここでは簡易的に Enemy* へのキャストで対応
					Enemy* e = dynamic_cast<Enemy*>(targetObj);
					if (e) {
						e->AddKnockback(knockbackForce);
					}
				}
			}
		}
		};

	// ▼▼▼▼▼▼▼▼▼ 腕 vs 腕 の衝突判定 ▼▼▼▼▼▼▼▼▼

	// objA: A本体, posA: Aの腕位置, armScaleA: Aの腕倍率, bodyScaleA: Aの体倍率
	// objB: B本体, ... (同上)
	auto CheckArmToArm = [&](
		Object* objA, Vector3 posA, float armScaleA, float bodyScaleA, bool isRightA, bool isAtkA,
		Object* objB, Vector3 posB, float armScaleB, float bodyScaleB, bool isRightB, bool isAtkB) {
			// 1. Aの腕のAABB作成（体スケールも考慮）
			float radiusA = kBaseActorScale_.x * armScaleA * bodyScaleA;
			AABB boxA;
			boxA.min = { posA.x - radiusA, posA.y - radiusA, posA.z - radiusA };
			boxA.max = { posA.x + radiusA, posA.y + radiusA, posA.z + radiusA };

			// 2. Bの腕のAABB作成
			float radiusB = kBaseActorScale_.x * armScaleB * bodyScaleB;
			AABB boxB;
			boxB.min = { posB.x - radiusB, posB.y - radiusB, posB.z - radiusB };
			boxB.max = { posB.x + radiusB, posB.y + radiusB, posB.z + radiusB };

			// 3. 衝突判定
			if (crashDecision(boxA, boxB)) {
				// 定数定義
				float basePower = 8.0f;
				float minKnockback = 10.0f; // 最小ノックバック量

				float duration = 0.3f;

				// 引き分けとみなすサイズ差の許容範囲
				// 例えば 0.8 に設定すれば、サイズ 3.0 vs 3.5 でも引き分けになります。
				// 将来スケールが巨大になるなら、ここを「大きい方の10%」のように割合にしても良いです。
				float drawTolerance = 0.8f;

				float scaleDiff = std::abs(armScaleA - armScaleB); // サイズ差の絶対値

				// 勝者決定ロジック
				bool winA = false;
				bool winB = false;
				bool draw = false;

				// --- 勝敗判定ロジック ---

				// パターン1: Aが攻撃中で、Bはチャージ中（無防備） -> Aの勝ち
				if (isAtkA && !isAtkB) {
					winA = true;
				}
				// パターン2: Bが攻撃中で、Aはチャージ中（無防備） -> Bの勝ち
				else if (!isAtkA && isAtkB) {
					winB = true;
				}
				// パターン3: 状態が同じ（両方攻撃中、または両方チャージ中） -> サイズ勝負
				else {
					// (どちらかがサイズ1.0なら「ぶつかっただけ」として引き分けにする)
					if (armScaleA <= 1.01f || armScaleB <= 1.01f) {
						draw = true;
					} else if (scaleDiff <= drawTolerance) {
						draw = true;
					} else if (armScaleA > armScaleB) {
						winA = true;
					} else {
						winB = true;
					}
				}

				bool isCheckInPlayer = false;
				if (objA == player_ || objB == player_) {
					isCheckInPlayer = true;
				}


				// --- ノックバック適用 ---a
				// --- 結果の適用 ---

				if (winA) {
					// Aが勝った場合（Bを吹き飛ばす）

					// ★追加: Bが既にダメージ中なら処理しない
					Enemy* eB = dynamic_cast<Enemy*>(objB);
					if (objB == player_ && player_->IsDamage()) return;
					if (eB && eB->IsDamage()) return;

					Vector3 diff = {
						objB->mainPosition.transform.translate.x - posA.x,
						objB->mainPosition.transform.translate.y - posA.y,
						objB->mainPosition.transform.translate.z - posA.z
					};
					float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
					if (len > 0.0f) {
						Vector3 dir = { diff.x / len, diff.y / len, diff.z / len };

						// ★修正: 最小値保証
						float power = std::max(basePower * (armScaleA * armScaleA), minKnockback);


						Vector3 force = { dir.x * power, 0.0f, dir.z * power };

						if (eB) {
							eB->AddKnockback(force);
							eB->ResetArm(isRightB);
						}
						if (objB == player_) {
							player_->AddKnockback(force);
							player_->ResetArm(isRightB);
						}
					}
				} else if (winB) {
					// Bが勝った場合（Aを吹き飛ばす）
					// ★追加: Aが既にダメージ中なら処理しない
					Enemy* eA = dynamic_cast<Enemy*>(objA);
					if (objA == player_ && player_->IsDamage()) return;
					if (eA && eA->IsDamage()) return;

					Vector3 diff = {
						objA->mainPosition.transform.translate.x - posB.x,
						objA->mainPosition.transform.translate.y - posB.y,
						objA->mainPosition.transform.translate.z - posB.z
					};
					float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
					if (len > 0.0f) {
						Vector3 dir = { diff.x / len, diff.y / len, diff.z / len };
						// ★修正: 最小値保証
						float power = std::max(basePower * (armScaleB * armScaleB), minKnockback);

						Vector3 force = { dir.x * power, 0.0f, dir.z * power };

						if (eA) {
							eA->AddKnockback(force);
							eA->ResetArm(isRightA);
						}
						if (objA == player_) {
							player_->AddKnockback(force);
							player_->ResetArm(isRightA);
						}
					}
				} else if (draw) {
					// 引き分け（両方吹っ飛ぶ）
					Vector3 centerA = objA->mainPosition.transform.translate;
					Vector3 centerB = objB->mainPosition.transform.translate;
					Vector3 diffBA = { centerA.x - centerB.x, centerA.y - centerB.y, centerA.z - centerB.z };
					float len = std::sqrt(diffBA.x * diffBA.x + diffBA.y * diffBA.y + diffBA.z * diffBA.z);
					if (len > 0.0f) {
						Vector3 dirA = { diffBA.x / len, diffBA.y / len, diffBA.z / len };
						Vector3 dirB = { -dirA.x, -dirA.y, -dirA.z };

						// Aへの適用
						bool ignoreA = (objA == player_ && player_->IsDamage()) || (dynamic_cast<Enemy*>(objA) && dynamic_cast<Enemy*>(objA)->IsDamage());
						if (!ignoreA) {
							float powerA = std::max(basePower * (armScaleA * armScaleA) * 0.8f, minKnockback); // ★最小値
							// ... AddKnockback(powerA) ...
							Vector3 forceA = { dirA.x * powerA, 5.0f, dirA.z * powerA };
							if (objA == player_) {
								player_->AddKnockback(forceA);
							} else {
								Enemy* e = dynamic_cast<Enemy*>(objA);
								if (e) {
									e->AddKnockback(forceA);
								}
							}
						}

						// Bへの適用
						bool ignoreB = (objB == player_ && player_->IsDamage()) || (dynamic_cast<Enemy*>(objB) && dynamic_cast<Enemy*>(objB)->IsDamage());
						if (!ignoreB) {
							float powerB = std::max(basePower * (armScaleB * armScaleB) * 0.8f, minKnockback); // ★最小値
							// ... AddKnockback(powerB) ...
							Vector3 forceB = { dirB.x * powerB, 5.0f, dirB.z * powerB };
							if (objB == player_) {
								player_->AddKnockback(forceB);
							} else {
								Enemy* e = dynamic_cast<Enemy*>(objB);
								if (e) {
									e->AddKnockback(forceB);
								}
							}
						}
					}
				}
			}
		};

	// --- 実際の判定実行 ---

	// プレイヤーの情報（ループ外で一度だけ取得）
	AABB pBodyAABB = player_->GetAABB(player_);
	float pBodyScale = player_->mainPosition.transform.scale.x;
	ArmInfo pArms[2] = {
		{ false, player_->GetArmPositionL(), player_->GetArmScaleL(), player_->IsAttackingL() }, // 左
		{ true,  player_->GetArmPositionR(), player_->GetArmScaleR(), player_->IsAttackingR() }  // 右
	};

	// 全エネミーに対してループ処理
	for (int enemyIdx = 0; enemyIdx < enemies_.size(); enemyIdx++) {
		// 参照で受ける（書き換えられるように auto& にする）
		auto& data = enemies_[enemyIdx];
		// エネミーの情報
		AABB eBodyAABB = data.enemy->GetAABB(data.enemy);
		float eBodyScale = data.enemy->mainPosition.transform.scale.x;
		// 敵の左右の情報（敵が複数の場合はループ内で）
		ArmInfo eArms[2] = {
			{ false, data.enemy->GetArmPositionL(), data.enemy->GetArmScaleL(), data.enemy->IsAttackingL() },
			{ true,  data.enemy->GetArmPositionR(), data.enemy->GetArmScaleR(), data.enemy->IsAttackingR() }
		};

		for (int armIdx = 0; armIdx < 2; armIdx++) {
			// 1. 敵の攻撃 vs プレイヤーの体
			if (eArms[armIdx].isAtk) CheckArmCollision(player_, pBodyAABB, eArms[armIdx].pos, eArms[armIdx].scale, eBodyScale);
			// 2. プレイヤーの攻撃 vs 敵の体
			if (pArms[armIdx].isAtk) CheckArmCollision(data.enemy, eBodyAABB, pArms[armIdx].pos, pArms[armIdx].scale, pBodyScale);
			for (int j = 0; j < 2; j++) {
				// 3. 腕同士の衝突
				CheckArmToArm(
					player_, pArms[armIdx].pos, pArms[armIdx].scale, pBodyScale, pArms[armIdx].isRight, pArms[armIdx].isAtk,
					data.enemy, eArms[j].pos, eArms[j].scale, eBodyScale, eArms[j].isRight, eArms[j].isAtk);
			}
		}
	}
}