#define NOMINMAX
#include "SceneStage01.h"
#include "crashDecision/crashDecision.h"

#include <algorithm>

SceneStage01::SceneStage01(kEngine *system) {
	/// =========== システム初期化 ============///
	system_ = system;
	debugCamera_ = new DebugCamera(system);
	camera_ = new Camera;
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));

	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	playerModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/charater/simplePlayer.obj");
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
	player_->mainPosition.transform.scale = Vector3(0.5f, 0.5f, 0.5f);
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

	enemy_ = new Enemy(system, Vector3(10.0f, 0.0f, 10.0f));
	enemy_->CreateModelData(playerModelHandle_);
	enemy_->mainPosition.transform.scale = Vector3(0.5f, 0.5f, 0.5f);

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
}

SceneStage01::~SceneStage01() {
	delete camera_;
	delete debugCamera_;

	delete player_;
	delete skydome_;
	delete sprite_;
	delete sprite2_;
	delete enemy_;
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

	enemy_->Update(usingCamera_);

	// ▼▼▼▼▼▼▼▼▼ AABB版：腕の衝突判定（スケール対応版） ▼▼▼▼▼▼▼▼▼

    // 引数に「attackerScale（攻撃主のスケール）」を追加しました
	auto CheckArmCollision = [&](Object* targetObj, const AABB& targetAABB, Vector3 armPos, float armScale, float attackerScale) {

		// 1. 腕のAABBを作成
		// 解説: 
		// - 0.5f : モデル自体の半径（1mの立方体の半分）
		// - armScale : 腕自体の膨張率（1.0 ~ 5.0）
		// - attackerScale : プレイヤーや敵の全体スケール（0.5 や 1.0 など）
		float armRealRadius = 0.5f * armScale * attackerScale;

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

			float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
			float len = std::sqrt(distSq);

			// 4. ノックバック適用
			if (len > 0.0f) {
				Vector3 pushDir = { diff.x / len, diff.y / len, diff.z / len };

				// 威力計算
				// 威力もスケールに合わせて調整したい場合は、ここにも attackerScale を掛けると良いです
				// （小さいキャラの攻撃は弱く、大きいキャラは強くなる）
				float basePower = 8.0f;

				// 例: 体のサイズも威力に反映させるなら
				// float power = basePower * (armScale * armScale) * (attackerScale / 0.5f); 
				// ※とりあえず今は元のままにしておきます
				float power = basePower * (armScale * armScale);

				Vector3 knockbackForce = {
					pushDir.x * power,
					0.0f,
					pushDir.z * power
				};

				if (targetObj == player_) {
					player_->AddKnockback(knockbackForce);
				} else if (targetObj == enemy_) {
					enemy_->AddKnockback(knockbackForce);
				}
			}
		}
		};

	// --- 実際の判定実行 ---

	// 準備
	AABB playerBodyAABB = player_->GetAABB(player_);
	AABB enemyBodyAABB = enemy_->GetAABB(enemy_);

	// 現在のスケールを取得 (XYZ均等と仮定してXを使う)
	float pScale = player_->mainPosition.transform.scale.x;
	float eScale = enemy_->mainPosition.transform.scale.x;

	// 1. 敵からの攻撃 (引数の最後に eScale を追加)
	if (enemy_->IsAttackingR()) {
		CheckArmCollision(player_, playerBodyAABB, enemy_->GetArmPositionR(), enemy_->GetArmScaleR(), eScale);
	}
	if (enemy_->IsAttackingL()) {
		CheckArmCollision(player_, playerBodyAABB, enemy_->GetArmPositionL(), enemy_->GetArmScaleL(), eScale);
	}

	// 2. プレイヤーからの攻撃 (引数の最後に pScale を追加)
	if (player_->IsAttackingR()) {
		CheckArmCollision(enemy_, enemyBodyAABB, player_->GetArmPositionR(), player_->GetArmScaleR(), pScale);
	}
	if (player_->IsAttackingL()) {
		CheckArmCollision(enemy_, enemyBodyAABB, player_->GetArmPositionL(), player_->GetArmScaleL(), pScale);
	}

	// ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲

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

	dashWindEffect_->SetGenerationPos(playerPos);
	dashWindEffect_->SetGenerationPos(playerPos + forward * 1.0f + Vector3(0.0f, 0.5f, 0.0f));
	dashWindEffect_->SetTargetForward(forward);

	dashWindEffect_->Update(usingCamera_);

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

	enemy_->Draw();

	for (int i = 0; i < 4; i++) {
		wall_[i]->Draw();
	}

	dashWindEffect_->Draw();

#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void SceneStage01::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
	} else {
		// プレイヤーの情報を取得
		Vector3 playerPos = player_->mainPosition.transform.translate;
		float playerRotY = player_->mainPosition.transform.rotate.y;

		// 腕の大きさに応じた距離と高さの計算(ズーム処理)
		float maxArmScale = std::max(player_->GetArmScaleL(), player_->GetArmScaleR());
		// 腕が大きくなるほど距離(baseDistance)を増やす
		// 例：基本 + 腕の余剰分 * 倍率
		// ここの数値をいじると「引き」の強さが変わる
		float baseDistance = cameraDistance_ + (maxArmScale - 1.0f) * cameraDistanceMultiply_;
		float baseHeight = cameraHeight_ + (maxArmScale - 1.0f) * cameraHeightMultiply_;

		// カメラの注視点と位置を計算
		Vector3 eye = {
			playerPos.x + std::sin(playerRotY) * baseDistance,
			playerPos.y + baseHeight,
			playerPos.z + std::cos(playerRotY) * baseDistance
		};

		// Transform構造体を作成して値をセットする
		Transform cameraTransform = CreateDefaultTransform();
		cameraTransform.translate = eye;

		// 回転の設定
		// 前進方向が {-sin, -cos} なので、
		// 基本の向き(0)が Z+方向なら、180度(3.14f)足すと Z-方向（前進方向）を向く。
		cameraTransform.rotate.x = 0.5f;             // 少し見下ろす角度
		cameraTransform.rotate.y = playerRotY + 3.14159f; // プレイヤーの背中を追う向き
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
		ImGui::SliderFloat("H_RealValues", &cameraHeight_, 1.0f, 20.0f);
		ImGui::SliderFloat("H_Multiply", &cameraHeightMultiply_, 1.0f, 50.0f);
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
			ImGui::SliderFloat("ArmScaleL", &player_->GetRefArmScaleL(), 0.0f, 5.0f);
			ImGui::SliderFloat("ArmScaleR", &player_->GetRefArmScaleR(), 0.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Params")) {
			ImGui::SliderFloat("DamageCD", &player_->GetRefDamageCD(), 0.0f, 5.0f);
			ImGui::SliderFloat("DamageTimer", &player_->GetRefDamageTimer(), 0.0f, 5.0f);
			ImGui::SliderFloat("KnockbackCD", &player_->GetRefKnockbackCD(), 0.0f, 5.0f);
			ImGui::SliderFloat("KnockbackTimer", &player_->GetRefKnockbackTimer(), 0.0f, 5.0f);
			ImGui::Checkbox("isDamage", &player_->GetRefIsDamage());
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("State")) {
			ImGui::Text("isAttackR: %d", player_->IsAttackingR());
			ImGui::Text("isAttackL: %d", player_->IsAttackingL());
			ImGui::TreePop();
		}
		ImGui::End();
	}

	{
		// ウィンドウの初期位置を右に設定
		ImGui::SetNextWindowPos(ImVec2(950, 300), ImGuiCond_Once);
		ImGui::Begin("Enemy");
		if (ImGui::Button("ResetPos")) {
			enemy_->mainPosition.transform.translate = Vector3(10.0f, 0.0f, 10.0f);
		}
		if (ImGui::TreeNode("Transform")) {
			ImGui::SliderFloat3("Pos", &enemy_->mainPosition.transform.translate.x, -50.0f, 50.0f);
			ImGui::SliderFloat3("Rotate", &enemy_->mainPosition.transform.rotate.x, -6.5f, 6.5f);
			ImGui::SliderFloat3("Scale", &enemy_->mainPosition.transform.scale.x, 0.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("ArmScale")) {
			ImGui::SliderFloat("ArmScaleL", &enemy_->GetRefArmScaleL(), 0.0f, 5.0f);
			ImGui::SliderFloat("ArmScaleR", &enemy_->GetRefArmScaleR(), 0.0f, 5.0f);
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Params")) {
			ImGui::SliderFloat("DamageCD", &enemy_->GetRefDamageCD(), 0.0f, 5.0f);
			ImGui::SliderFloat("DamageTimer", &enemy_->GetRefDamageTimer(), 0.0f, 5.0f);
			ImGui::SliderFloat("KnockbackCD", &enemy_->GetRefKnockbackCD(), 0.0f, 5.0f);
			ImGui::SliderFloat("KnockbackTimer", &enemy_->GetRefKnockbackTimer(), 0.0f, 5.0f);
			ImGui::Checkbox("isDamage", &enemy_->GetRefIsDamage());
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("State")) {
			ImGui::Text("isAttackR: %d", enemy_->IsAttackingR());
			ImGui::Text("isAttackL: %d", enemy_->IsAttackingL());
			ImGui::TreePop();
		}
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