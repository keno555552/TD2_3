#define NOMINMAX
#include "SceneStage01.h"
#include "crashDecision/crashDecision.h"

#include <algorithm>
#include <numbers>

SceneStage01::SceneStage01(kEngine *system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));

	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	playerModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/charater/simplePlayer.obj");
	guideArrowModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/arrow/arrow.obj");
	wallModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/cube/cube.obj");

	boxTextureHandle_ = system_->LoadTextrue("resources/texture/testBox.png");
	tryTextureHandle_ = system_->LoadTextrue("resources/texture/Tryer.png");
	uvTextureHandle_ = system_->LoadTextrue("resources/TemplateResource/texture/uvChecker.png");

	//skydome_->CreateDefaultData();
	skydome_ = new Object;
	skydome_->IntObject(system_);
	skydome_->CreateModelData(skydomeModelHandle_);
	skydome_->objectParts_[0].materialConfig->enableLighting = false;

	player_ = new Player(system, Vector3(5.0f, 0.0f, 5.0f));
	player_->CreateModelData(playerModelHandle_);
	player_->mainPosition.transform.scale = kBaseActorScale_;
	//player_->mainPosition.transform.translate = Vector3(0.0f, 0.0f, 0.0f);
	//player_->CreateDefaultData();
	//player_->modelHandle_ = playerModelHandle_;

	sprite_ = new SimpleSprite;
	sprite_->IntObject(system_);
	sprite_->CreateDefaultData();
	sprite_->objectParts_[0].materialConfig->textureHandle = uvTextureHandle_;

	sprite2_ = new SimpleSprite;
	sprite2_->IntObject(system_);
	sprite2_->CreateDefaultData();
	sprite2_->objectParts_[0].materialConfig->textureHandle = uvTextureHandle_;

	// --- エネミーの複数生成 ---
	// テスト用に3体ほど配置
	std::vector<Vector3> spawnPositions = {
		{ 20.0f, 0.0f,  20.0f}, // 右奥
		{ 20.0f, 0.0f,  10.0f}, // 右手前
		{  5.0f, 0.0f, 20.0f}   // 前
	};

	for (const auto& pos : spawnPositions) {
		Enemy* newEnemy = new Enemy(system_, pos);
		// モデルデータ共有（読み込み回数を減らすためハンドルを使い回す）
		newEnemy->CreateModelData(playerModelHandle_);
		newEnemy->mainPosition.transform.scale = kBaseActorScale_;

		// リストに追加
		enemies_.push_back(newEnemy);
	}

	// ガイド矢印
	guideArrow_ = new Object;
	guideArrow_->IntObject(system_);
	guideArrow_->CreateModelData(guideArrowModelHandle_);
	guideArrow_->mainPosition.transform.scale = Vector3(0.0f, 0.0f, 0.0f); // 最初は非表示

	// 王冠生成(初期化時はターゲットなし)
	crown_ = new Crown(system_);

	for (int i = 0; i < 4; i++) {
		wall_[i] = new Object;
		wall_[i]->IntObject(system_);
		wall_[i]->CreateModelData(wallModelHandle_);
		wall_[i]->mainPosition.transform.scale = Vector3(1.0f, 1.0f, 1.0f);
		//wall_[i]->mainPosition.transform.scale = Vector3(0.05f, 0.05f, 0.05f);
		wall_[i]->mainPosition.transform.translate = Vector3(0.0f, 0.0f, 0.0f);
	}
	wall_[0]->mainPosition.transform.translate = Vector3(0.0f, 0.0f, 50.0f);
	wall_[0]->mainPosition.transform.scale = Vector3(1.0f, 1.0f, 100.0f);
	wall_[1]->mainPosition.transform.translate = Vector3(50.0f, 0.0f, 100.0f);
	wall_[1]->mainPosition.transform.scale = Vector3(100.0f, 1.0f, 1.0f);
	wall_[2]->mainPosition.transform.translate = Vector3(100.0f, 0.0f, 50.0f);
	wall_[2]->mainPosition.transform.scale = Vector3(1.0f, 1.0f, 100.0f);
	wall_[3]->mainPosition.transform.translate = Vector3(50.0f, 0.0f, 0.0f);
	wall_[3]->mainPosition.transform.scale = Vector3(100.0f, 1.0f, 1.0f);

	dashWindEffect_ = new P_DashWind(system_);

	suctionEffect_ = new P_Suction(system_);

	planeReductionEffectL_ = new P_PlaneReduction(system_);
	planeReductionEffectL_->CreateModelData(system_->SetModelObj("resources/TemplateResource/object/plane/plane.obj"));
	planeReductionEffectR_ = new P_PlaneReduction(system_);
	planeReductionEffectR_->CreateModelData(system_->SetModelObj("resources/TemplateResource/object/plane/plane.obj"));

}

SceneStage01::~SceneStage01() {
	delete camera_;
	delete debugCamera_;

	delete player_;
	delete skydome_;
	delete sprite_;
	delete sprite2_;
	// リスト内のエネミーを全て解放
	for (auto* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();
	if (guideArrow_) {
		delete guideArrow_;
	}
	if (crown_) {
		delete crown_;
	}
	for (int i = 0; i < 4; i++) {
		delete wall_[i];
        }
}

void SceneStage01::Update() {
	/// カメラ処理
	CameraPart();

	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// player更新
	player_->Update(usingCamera_);

	sprite_->Update(usingCamera_);
	//sprite2_->Update(usingCamera_);

	// リスト内のエネミーを全て更新
	for (auto* enemy : enemies_) {
		enemy->Update(usingCamera_);
	}


	/// 衝突判定処理

	// 攻撃当たり判定処理
	CheckAttackCollision();

	for (int i = 0; i < 4; i++) {
		wall_[i]->Update(usingCamera_);
		if (crashDecision(wall_[i]->GetAABB(wall_[i]), player_->GetAABB(player_))) {
			// 衝突処理
			if (i == 0) {
				player_->mainPosition.transform.translate.x = wall_[i]->mainPosition.transform.translate.x + wall_[i]->mainPosition.transform.scale.x / 2.0f + player_->mainPosition.transform.scale.x / 2.0f;
			}
			if (i == 1) {
				player_->mainPosition.transform.translate.z = wall_[i]->mainPosition.transform.translate.z - wall_[i]->mainPosition.transform.scale.z / 2.0f - player_->mainPosition.transform.scale.z / 2.0f;
			}
			if (i == 2) {
				player_->mainPosition.transform.translate.x = wall_[i]->mainPosition.transform.translate.x - wall_[i]->mainPosition.transform.scale.x / 2.0f - player_->mainPosition.transform.scale.x / 2.0f;
			}
			if(i == 3) {
				player_->mainPosition.transform.translate.z = wall_[i]->mainPosition.transform.translate.z + wall_[i]->mainPosition.transform.scale.z / 2.0f + player_->mainPosition.transform.scale.z / 2.0f;
			}

		}
	}

	Vector3 playerPos = player_->mainPosition.transform.translate;
	Vector3 forward = player_->GetForward();

	dashWindEffect_->SetGenerationPos(player_->GetArmPositionL() + forward + Vector3(0.0f, player_->GetArmScaleL()-0.5f,0.0f));
	dashWindEffect_->SetTargetForward(forward);
	dashWindEffect_->SetArmScale(player_->GetArmScaleL());
	dashWindEffect_->Update(usingCamera_);
	dashWindEffect_->SetEmitting(false);
	
	if (player_->IsAttackingL()) {
		dashWindEffect_->SetEmitting(true);
	}

	suctionEffect_->SetArmScaleL(player_->GetArmScaleL());
	suctionEffect_->SetTargetPosLeft(player_->GetArmPositionL());
	suctionEffect_->SetTargetPosRight(player_->GetArmPositionR());
	suctionEffect_->SetLeftEmitting(false);
	suctionEffect_->SetRightEmitting(false);
	if (system_->GetIsPush(DIK_A)) {
		suctionEffect_->SetLeftEmitting(true);
	}
	if (system_->GetIsPush(DIK_D)) {
		suctionEffect_->SetRightEmitting(true);
	}

	suctionEffect_->Update(usingCamera_);


	planeReductionEffectL_->SetIsAction(system_->GetIsPush(DIK_A));
	planeReductionEffectL_->SetTargetPos(player_->GetArmPositionL());
	planeReductionEffectL_->SetTargetScale(player_->GetArmScaleL());
	planeReductionEffectL_->Update(camera_);

	planeReductionEffectR_->SetIsAction(system_->GetIsPush(DIK_D));
	planeReductionEffectR_->SetTargetPos(player_->GetArmPositionR());
	planeReductionEffectR_->SetTargetScale(player_->GetArmScaleR());
	planeReductionEffectR_->Update(camera_);

	/// ガイド矢印更新
	UpdateGuideArrow();
	/// 王冠の更新
	UpdateCrown();

	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}

	if (system_->GetTriggerOn(DIK_SPACE)) {
		ChangeNextStage(SceneNum::S_Result);
	}
}


void SceneStage01::Draw() {

	/// 実体処理
	//system_->DrawModel(&skydome_->objectParts[0].transformationMatrix, &(skydome_->objectParts[0].materialConfig.get()), skydomeModelHandle_);
	skydome_->Draw();
	player_->Draw();
	//sprite_->Draw();
	//sprite2_->Draw();
	//system_->Draw3D(skydome_);
	//system_->Draw3D(player_);
	//system_->Draw3D(model_);

	for (auto* enemy : enemies_) {
		enemy->Draw();
	}

	guideArrow_->Draw();

	if (crown_) {
		crown_->Draw();
	}

	for (int i = 0; i < 4; i++) {
		wall_[i]->Draw();
	}

	// 自機が進む際のダッシュエフェクト
	dashWindEffect_->Draw();
	// 自機の腕がでかくなる際の吸い込みエフェクト
	suctionEffect_->Draw();

	if (planeReductionEffectL_->GetIsActive())planeReductionEffectL_->Draw();
	if (planeReductionEffectR_->GetIsActive())planeReductionEffectR_->Draw();

#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void SceneStage01::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		// ---------------------------------------------------------
		// 1. 手動ズーム入力の処理 (W:接近 / S:離れる)
		// ---------------------------------------------------------
		float dt = system_->GetDeltaTime();

		// Sキー: ズームアウト（距離を足す）
		if (system_->GetIsPush(DIK_S)) {
			manualZoomOffset_ += kManualZoomSpeed_ * dt;
		}
		// Wキー: ズームイン（距離を引く）
		else if (system_->GetIsPush(DIK_W)) {
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
	usingCamera_->Update();
}

#ifdef USE_IMGUI
void SceneStage01::ImguiPart() {
	ImGui::Begin("DebugCamera");
	ImGui::Checkbox("isUse", &useDebugCamera);
	ImGui::End();

	{
        Transform camTransform = usingCamera_->GetTransform();
		// ウィンドウの初期位置を左上に設定
        ImGui::SetNextWindowPos(ImVec2(10, 0), ImGuiCond_Once);
		ImGui::Begin("GameCamera");
        ImGui::Text("Transform");
		ImGui::SliderFloat3("Pos", &camTransform.translate.x, -1.0f, 1.0f);
		ImGui::SliderFloat3("Rotate", &camTransform.rotate.x, -1.0f, 1.0f);
		ImGui::Separator();
		ImGui::Text("Distance");
		ImGui::SliderFloat("D_RealValues", &cameraDistance_, 1.0f, 30.0f);
		ImGui::SliderFloat("D_Multiply", &cameraDistanceMultiply_, 1.0f, 50.0f);
		ImGui::Separator();
		ImGui::Text("Height");
		ImGui::SliderFloat("H_RealValues", &cameraHeight_, 1.0f, 20.0f);
		ImGui::SliderFloat("H_Multiply", &cameraHeightMultiply_, 1.0f, 50.0f);
		ImGui::Separator();
		ImGui::Text("Zoom");
		ImGui::SliderFloat("Offset", &manualZoomOffset_, kMinManualZoom_, kMaxManualZoom_);
		ImGui::End();
	}

	{
		// ウィンドウの初期位置を右上に設定
		ImGui::SetNextWindowPos(ImVec2(950, 0), ImGuiCond_Once);
		ImGui::Begin("PlayerPos");
		if (ImGui::Button("ResetPos")) {
			player_->mainPosition.transform.translate = Vector3(2.0f, 0.0f, 2.0f);
		}
		if (ImGui::TreeNode("Transform")) {
			ImGui::SliderFloat3("Pos", &player_->mainPosition.transform.translate.x, -50.0f, 50.0f);
			ImGui::SliderFloat3("Rotate", &player_->mainPosition.transform.rotate.x, -6.5f, 6.5f);
			ImGui::SliderFloat3("Scale", &player_->mainPosition.transform.scale.x, 0.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("ArmScale")) {
			ImGui::SliderFloat("ArmScaleL", &player_->GetRefArmScaleL(), 1.0f, 50.0f);
			ImGui::SliderFloat("ArmScaleR", &player_->GetRefArmScaleR(), 1.0f, 50.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Params")) {
			ImGui::SliderFloat("DamageCD", &player_->GetRefDamageCD(), 0.0f, 5.0f);
			ImGui::SliderFloat("DamageTimer", &player_->GetRefDamageTimer(), 0.0f, 5.0f);
			ImGui::SliderFloat("KnockbackCD", &player_->GetRefKnockbackCD(), 0.0f, 5.0f);
			ImGui::SliderFloat("KnockbackTimer", &player_->GetRefKnockbackTimer(), 0.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("State")) {
			ImGui::Text("isDamage:  %d", player_->IsDamage());
			ImGui::Text("isAttackR: %d", player_->IsAttackingR());
			ImGui::Text("isAttackL: %d", player_->IsAttackingL());
			ImGui::TreePop();
		}
		ImGui::End();
	}

	{
		// ウィンドウ位置設定
		ImGui::SetNextWindowPos(ImVec2(950, 300), ImGuiCond_Once);
		ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Once); // サイズも指定

		ImGui::Begin("Enemy Manager");

		// 全体リセットボタン
		if (ImGui::Button("Reset All Positions")) {
			// 初期位置に戻す（簡易的にY=0にする、あるいは初期座標を記憶しておくなどの拡張が可能）
			// ここでは単純に少し離して整列させ直す例
			for (int i = 0; i < enemies_.size(); ++i) {
				float angle = (float)i * (6.28f / enemies_.size());
				enemies_[i]->mainPosition.transform.translate = {
					10.0f + std::sin(angle) * 10.0f, 0.0f, 10.0f + std::cos(angle) * 10.0f
				};
			}
		}

		ImGui::Separator();

		// リスト内のエネミーごとにUIを生成
		for (int i = 0; i < enemies_.size(); ++i) {
			Enemy* e = enemies_[i];

			// IDをプッシュ（これがないと全部のスライダーが同期して動いてしまう）
			ImGui::PushID(i);

			// "Enemy 0", "Enemy 1" ... というヘッダーを作る
			std::string headerName = "Enemy " + std::to_string(i);

			if (ImGui::CollapsingHeader(headerName.c_str())) {

				// 個別リセット
				if (ImGui::Button("Reset Pos")) {
					e->mainPosition.transform.translate = Vector3(10.0f * (i + 1), 0.0f, 10.0f);
				}

				if (ImGui::TreeNode("Transform")) {
					ImGui::DragFloat3("Pos", &e->mainPosition.transform.translate.x, 0.1f);
					ImGui::SliderFloat3("Rotate", &e->mainPosition.transform.rotate.x, -6.5f, 6.5f);
					ImGui::SliderFloat3("Scale", &e->mainPosition.transform.scale.x, 0.0f, 5.0f);
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("ArmScale")) {
					// テスト用に強制的に値を書き換えるスライダー
					// 実際はUpdateで変動するので、一時停止中などに有効
					ImGui::SliderFloat("ArmScaleL", &e->GetRefArmScaleL(), 0.0f, 50.0f);
					ImGui::SliderFloat("ArmScaleR", &e->GetRefArmScaleR(), 0.0f, 50.0f);
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Params")) {
					ImGui::SliderFloat("DamageCD", &e->GetRefDamageCD(), 0.0f, 5.0f);
					ImGui::SliderFloat("DamageTimer", &e->GetRefDamageTimer(), 0.0f, 5.0f);
					ImGui::SliderFloat("KnockbackCD", &e->GetRefKnockbackCD(), 0.0f, 5.0f);
					ImGui::SliderFloat("KnockbackTimer", &e->GetRefKnockbackTimer(), 0.0f, 5.0f);
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("State")) {
					ImGui::Text("isDamage:  %d", e->IsDamage());
					ImGui::Text("isAttackR: %d", e->IsAttackingR());
					ImGui::Text("isAttackL: %d", e->IsAttackingL());
					ImGui::TreePop();
				}
			}

			// IDをポップ（忘れるとエラーになります）
			ImGui::PopID();
		}

		ImGui::End();
	}

	{
		// ウィンドウの初期位置を右下に設定
        ImGui::SetNextWindowPos(ImVec2(950, 600), ImGuiCond_Once);
		ImGui::Begin("GuideArrow");
		ImGui::SliderFloat3("Pos", &guideArrow_->mainPosition.transform.translate.x, -50.0f, 50.0f);
		ImGui::SliderFloat3("Rotate", &guideArrow_->mainPosition.transform.rotate.x, -6.5f, 6.5f);
		ImGui::SliderFloat3("Scale", &guideArrow_->mainPosition.transform.scale.x, 0.0f, 5.0f);
		ImGui::End();
	}

	{
		ImGui::Begin("SpritePos");
		ImGui::SliderFloat3("Pos", &sprite_->mainPosition.transform.translate.x, 0.0f, 1280.0f);
		ImGui::SliderFloat3("Rotate", &sprite_->mainPosition.transform.rotate.x, 0.0f, 6.5f);
		ImGui::SliderFloat3("Scale", &sprite_->mainPosition.transform.scale.x, 0.0f, 5.0f);

		ImGui::SliderFloat3("AnchorPoint", &sprite_->mainPosition.anchorPoint.x, -500.0f, 500.0f);
		ImGui::SliderFloat2("CropLT", &sprite_->objectParts_[0].cropLT.x, -500.0f, 500.0f);
		ImGui::SliderFloat2("CropSize", &sprite_->objectParts_[0].cropSize.x, -500.0f, 500.0f);

		ImGui::Text("MaterialConfig");
		ImGui::SliderFloat3("uvTranslate", &sprite_->objectParts_[0].materialConfig->uvTranslate.x, -5.0f, 5.0f);
		ImGui::SliderFloat3("uvScale", &sprite_->objectParts_[0].materialConfig->uvScale.x, -5.0f, 5.0f);
		ImGui::SliderFloat3("uvRotate", &sprite_->objectParts_[0].materialConfig->uvRotate.x, -6.5f, 6.5f);
		ImGui::ColorEdit4("Color", &sprite_->objectParts_[0].materialConfig->textureColor.x);

		ImGui::End();
	}
}
#endif

void SceneStage01::CheckAttackCollision() {
	// ▼▼▼▼▼▼▼▼▼ AABB版：腕の衝突判定（スケール対応版） ▼▼▼▼▼▼▼▼▼

	// 引数に「attackerScale（攻撃主のスケール）」を追加しました
	auto CheckArmCollision = [&](Object* targetObj, const AABB& targetAABB, Vector3 armPos, float armScale, float attackerScale) {

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

				// 威力: 腕の大きさの2乗
				float power = 8.0f * (armScale * armScale);
				Vector3 knockbackForce = { pushDir.x * power, 0.0f, pushDir.z * power };

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
		Object* objA, Vector3 posA, float armScaleA, float bodyScaleA, bool isAtkA,
		Object* objB, Vector3 posB, float armScaleB, float bodyScaleB, bool isAtkB) {
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
				float basePower = 8.0f; // 基本威力

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

				// --- ノックバック適用 ---

				if (winA) {
					// Aが勝った場合（Bを吹き飛ばす）
					Vector3 diff = {
						objB->mainPosition.transform.translate.x - posA.x,
						objB->mainPosition.transform.translate.y - posA.y,
						objB->mainPosition.transform.translate.z - posA.z
					};
					float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
					if (len > 0.0f) {
						Vector3 dir = { diff.x / len, diff.y / len, diff.z / len };

						// ここでは単純にAのサイズパワーで計算
						float power = basePower * (armScaleA * armScaleA);

						Vector3 knockbackForce = { dir.x * power, 0.0f, dir.z * power };
						Enemy* e = dynamic_cast<Enemy*>(objB);
						if (e) {
							e->AddKnockback(knockbackForce);
						}
						if (objB == player_) {
							player_->AddKnockback(knockbackForce);
						}
					}
				} else if (winB) {
					// Bが勝った場合（Aを吹き飛ばす）
					Vector3 diff = {
						objA->mainPosition.transform.translate.x - posB.x,
						objA->mainPosition.transform.translate.y - posB.y,
						objA->mainPosition.transform.translate.z - posB.z
					};
					float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
					if (len > 0.0f) {
						Vector3 dir = { diff.x / len, diff.y / len, diff.z / len };
						float power = basePower * (armScaleB * armScaleB);
						Vector3 knockbackForce = { dir.x * power, 0.0f, dir.z * power };

						Enemy* e = dynamic_cast<Enemy*>(objA);
						if (e) {
							e->AddKnockback(knockbackForce);
						}
						if (objA == player_) {
							player_->AddKnockback(knockbackForce);
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

						float powerA = basePower * (armScaleA * armScaleA) * 0.8f;
						float powerB = basePower * (armScaleB * armScaleB) * 0.8f;

						Vector3 forceA = { dirA.x * powerA, 5.0f, dirA.z * powerA };
						if (objA == player_) player_->AddKnockback(forceA);
						else {
							Enemy* e = dynamic_cast<Enemy*>(objA);
							if (e) {
								e->AddKnockback(forceA);
							}
						}
						Vector3 forceB = { dirB.x * powerB, 5.0f, dirB.z * powerB };
						if (objB == player_) player_->AddKnockback(forceB);
						else {
							Enemy* e = dynamic_cast<Enemy*>(objB);
							if (e) {
								e->AddKnockback(forceB);
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
	Vector3 pPosR = player_->GetArmPositionR(); float pSclR = player_->GetArmScaleR(); bool pAtkR = player_->IsAttackingR();
	Vector3 pPosL = player_->GetArmPositionL(); float pSclL = player_->GetArmScaleL(); bool pAtkL = player_->IsAttackingL();

	// 全エネミーに対してループ処理
	for (auto* enemy : enemies_) {

		// エネミーの情報
		AABB eBodyAABB = enemy->GetAABB(enemy);
		float eBodyScale = enemy->mainPosition.transform.scale.x;
		Vector3 ePosR = enemy->GetArmPositionR(); float eSclR = enemy->GetArmScaleR(); bool eAtkR = enemy->IsAttackingR();
		Vector3 ePosL = enemy->GetArmPositionL(); float eSclL = enemy->GetArmScaleL(); bool eAtkL = enemy->IsAttackingL();


		// 1. 敵の攻撃 vs プレイヤーの体
		if (eAtkR) CheckArmCollision(player_, pBodyAABB, ePosR, eSclR, eBodyScale);
		if (eAtkL) CheckArmCollision(player_, pBodyAABB, ePosL, eSclL, eBodyScale);

		// 2. プレイヤーの攻撃 vs 敵の体
		if (pAtkR) CheckArmCollision(enemy, eBodyAABB, pPosR, pSclR, pBodyScale);
		if (pAtkL) CheckArmCollision(enemy, eBodyAABB, pPosL, pSclL, pBodyScale);

		// 3. 腕同士の衝突 (プレイヤーの腕の状態に関わらず判定してOK、細かい条件は関数内で処理)
		// 右 vs 右
		CheckArmToArm(player_, pPosR, pSclR, pBodyScale, pAtkR, enemy, ePosR, eSclR, eBodyScale, eAtkR);
		// 右 vs 左
		CheckArmToArm(player_, pPosR, pSclR, pBodyScale, pAtkR, enemy, ePosL, eSclL, eBodyScale, eAtkL);
		// 左 vs 右
		CheckArmToArm(player_, pPosL, pSclL, pBodyScale, pAtkL, enemy, ePosR, eSclR, eBodyScale, eAtkR);
		// 左 vs 左
		CheckArmToArm(player_, pPosL, pSclL, pBodyScale, pAtkL, enemy, ePosL, eSclL, eBodyScale, eAtkL);
	}
}

void SceneStage01::UpdateGuideArrow() {
	// ▼▼▼▼▼▼▼▼▼ ガイド矢印の制御処理 ▼▼▼▼▼▼▼▼▼

	// 1. プレイヤーの最大腕サイズを取得
	float pMaxArm = std::max(player_->GetArmScaleL(), player_->GetArmScaleR());

	// 2. 敵の中で「プレイヤーより腕が大きく」かつ「最も腕が大きい敵」を探す
	Object* targetEnemy = nullptr;
	float maxEnemyArm = 0.0f;

	for (auto* enemy : enemies_) {
		float eArmL = enemy->GetArmScaleL();
		float eArmR = enemy->GetArmScaleR();
		float currentEnemyMax = std::max(eArmL, eArmR);

		// プレイヤーより大きく、かつ今まで見た敵の中で一番大きいか
		if (currentEnemyMax > pMaxArm && currentEnemyMax > maxEnemyArm) {
			maxEnemyArm = currentEnemyMax;
			targetEnemy = enemy;
		}
	}

	// 3. 表示・非表示と座標計算
	if (targetEnemy != nullptr) {
		// --- ターゲットが見つかった場合（表示） ---

		Vector3 pPos = player_->mainPosition.transform.translate;
		Vector3 ePos = targetEnemy->mainPosition.transform.translate;

		// A. 方向ベクトル計算 (プレイヤー -> 敵)
		float diffX = ePos.x - pPos.x;
		float diffZ = ePos.z - pPos.z;

		// 距離と正規化（方向）
		float dist = std::sqrt(diffX * diffX + diffZ * diffZ);
		Vector3 dir = { 0.0f, 0.0f, 1.0f }; // デフォルト
		if (dist > 0.0f) {
			dir = { diffX / dist, 0.0f, diffZ / dist };
		}

		// B. 矢印の回転 (Y軸回転)
		// atan2(x, z) で角度(ラジアン)を求める
		float angleY = std::atan2(diffX, diffZ) + std::numbers::pi_v<float>;
		guideArrow_->mainPosition.transform.rotate.y = angleY;


		// C. 矢印の配置半径 (プレイヤーの腕サイズに合わせて広げる)
		float baseRadius = 0.5f;
		float radiusFactor = 1.0f;
		float currentRadius = baseRadius + (pMaxArm * radiusFactor);

		// プレイヤーの中心から、敵の方向に currentRadius 分だけ進んだ位置
		guideArrow_->mainPosition.transform.translate = {
			pPos.x + dir.x * currentRadius,
			pPos.y + 0.5f, // 地面より少し浮かせる（高さ調整）
			pPos.z + dir.z * currentRadius
		};


		// D. 矢印の大きさ (プレイヤーの腕サイズに合わせて大きくする)
		// 1. プレイヤーの全体スケールを取得 (0.5f)
		float playerGlobalScale = kBaseActorScale_.x;

		// 2. 「基本サイズ」の割合を決める
		// 0.4f なら「体の40%の大きさ」からスタート
		float startRatio = 0.4f;

		// 3. 「成長スピード」の割合を決める
		// 0.1f なら「腕倍率が+1されるごとに、体の10%分だけ大きくなる」
		// 腕が最大(5.0)になると、(5-1)*0.1 = +0.4 (40%) 増える計算
		float growthRatio = 0.2f;

		// 計算式: ( 基本割合 + (増えた腕倍率 * 成長割合) ) * 全体スケール
		// これなら、最大まで腕を大きくしても矢印は「体の80%くらいの大きさ」に収まります
		float finalScale = (startRatio + (pMaxArm - 1.0f) * growthRatio) * playerGlobalScale;

		guideArrow_->mainPosition.transform.scale = { finalScale, finalScale, finalScale };

	} else {
		// --- ターゲットがいない（自分が最強）場合（非表示） ---
		// スケールを0にして見えなくする（または描画スキップ）
		guideArrow_->mainPosition.transform.scale = { 0.0f, 0.0f, 0.0f };
	}

	// 4. 矢印の更新（行列計算など）
	guideArrow_->Update(camera_); // ←忘れずに呼ぶ
}

void SceneStage01::UpdateCrown() {
	// 1. 全員の中で「最大の腕サイズ」を持つキャラを探す

	// まずプレイヤーを暫定1位にする
	Object* topChar = player_;
	float topArmScale = std::max(player_->GetArmScaleL(), player_->GetArmScaleR());

	// 敵リストを回して比較
	for (auto* enemy : enemies_) {
		float eMax = std::max(enemy->GetArmScaleL(), enemy->GetArmScaleR());

		// 現王者より大きければ更新
		if (eMax > topArmScale) {
			topArmScale = eMax;
			topChar = enemy;
		}
	}

	// 2. 王冠に情報を渡す
	// SetTarget内で「前回と違うキャラならアニメーション開始」を判定しているので
	// 毎フレーム渡してOK
	if (crown_) {
		crown_->SetTarget(topChar, topArmScale);
		crown_->Update(camera_);
	}
}
