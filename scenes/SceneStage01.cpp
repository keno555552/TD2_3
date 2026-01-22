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
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("State")) {
			ImGui::Text("isDamage:  %d", enemy_->IsDamage());
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

void SceneStage01::CheckAttackCollision() {
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

	// ▼▼▼▼▼▼▼▼▼ 腕 vs 腕 の衝突判定 ▼▼▼▼▼▼▼▼▼

	// objA: A本体, posA: Aの腕位置, armScaleA: Aの腕倍率, bodyScaleA: Aの体倍率
	// objB: B本体, ... (同上)
	auto CheckArmToArm = [&](
		Object* objA, Vector3 posA, float armScaleA, float bodyScaleA, bool isAtkA,
		Object* objB, Vector3 posB, float armScaleB, float bodyScaleB, bool isAtkB) {
			// 1. Aの腕のAABB作成（体スケールも考慮）
			float radiusA = 0.5f * armScaleA * bodyScaleA;
			AABB boxA;
			boxA.min = { posA.x - radiusA, posA.y - radiusA, posA.z - radiusA };
			boxA.max = { posA.x + radiusA, posA.y + radiusA, posA.z + radiusA };

			// 2. Bの腕のAABB作成
			float radiusB = 0.5f * armScaleB * bodyScaleB;
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

				// ★★★ 状態による勝敗判定 ★★★

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
					if (scaleDiff <= drawTolerance) {
						draw = true; // 引き分け
					} else if (armScaleA == 1.0f || armScaleB == 1.0f) {
						draw = true; // どちらかが最小サイズなら引き分け
					} else if (armScaleA > armScaleB) {
						winA = true;
					} else {
						winB = true;
					}
				}

				// --- 結果の適用 ---

				// Aが勝った場合（Bを吹き飛ばす）
				if (winA) {
					Vector3 diff = {
						objB->mainPosition.transform.translate.x - posA.x,
						objB->mainPosition.transform.translate.y - posA.y,
						objB->mainPosition.transform.translate.z - posA.z
					};
					float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
					if (len > 0.0f) {
						Vector3 dir = { diff.x / len, diff.y / len, diff.z / len };

						// カウンター（状態勝ち）なら威力ボーナスを与えても面白いです
						// ここでは単純にAのサイズパワーで計算
						float power = basePower * (armScaleA * armScaleA);

						Vector3 knockbackForce = { dir.x * power, 5.0f, dir.z * power };
						if (objB == player_) player_->AddKnockback(knockbackForce);
						else if (objB == enemy_) enemy_->AddKnockback(knockbackForce);
					}
				}
				// Bが勝った場合（Aを吹き飛ばす）
				else if (winB) {
					Vector3 diff = {
						objA->mainPosition.transform.translate.x - posB.x,
						objA->mainPosition.transform.translate.y - posB.y,
						objA->mainPosition.transform.translate.z - posB.z
					};
					float len = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);
					if (len > 0.0f) {
						Vector3 dir = { diff.x / len, diff.y / len, diff.z / len };
						float power = basePower * (armScaleB * armScaleB);
						Vector3 knockbackForce = { dir.x * power, 5.0f, dir.z * power };

						if (objA == player_) player_->AddKnockback(knockbackForce);
						else if (objA == enemy_) enemy_->AddKnockback(knockbackForce);
					}
				}
				// 引き分け（両方吹っ飛ぶ）
				else if (draw) {
					Vector3 centerA = objA->mainPosition.transform.translate;
					Vector3 centerB = objB->mainPosition.transform.translate;
					Vector3 diffBA = { centerA.x - centerB.x, centerA.y - centerB.y, centerA.z - centerB.z };
					float len = std::sqrt(diffBA.x * diffBA.x + diffBA.y * diffBA.y + diffBA.z * diffBA.z);
					if (len > 0.0f) {
						Vector3 dirA = { diffBA.x / len, diffBA.y / len, diffBA.z / len };
						Vector3 dirB = { -dirA.x, -dirA.y, -dirA.z };

						float reboundPowerA = basePower * (armScaleA * armScaleA) * 0.8f;
						float reboundPowerB = basePower * (armScaleB * armScaleB) * 0.8f;

						Vector3 forceA = { dirA.x * reboundPowerA, 5.0f, dirA.z * reboundPowerA };
						if (objA == player_) player_->AddKnockback(forceA);
						else if (objA == enemy_) enemy_->AddKnockback(forceA);

						Vector3 forceB = { dirB.x * reboundPowerB, 5.0f, dirB.z * reboundPowerB };
						if (objB == player_) player_->AddKnockback(forceB);
						else if (objB == enemy_) enemy_->AddKnockback(forceB);
					}
				}
			}
		};

	// --- 実際の判定実行 ---

	// 準備
	AABB playerBodyAABB = player_->GetAABB(player_);
	AABB enemyBodyAABB = enemy_->GetAABB(enemy_);

	// 体のスケール取得 (XYZ均等と仮定してXを使う)
	float pScale = player_->mainPosition.transform.scale.x;
	float eScale = enemy_->mainPosition.transform.scale.x;

	// 腕の情報取得（何度も呼ぶので変数に入れておくと見やすいです）
	Vector3 pArmPosR = player_->GetArmPositionR();
	float   pArmSclR = player_->GetArmScaleR();
	bool    pAtkR = player_->IsAttackingR();
	Vector3 pArmPosL = player_->GetArmPositionL();
	float   pArmSclL = player_->GetArmScaleL();
	bool    pAtkL = player_->IsAttackingL();

	Vector3 eArmPosR = enemy_->GetArmPositionR();
	float   eArmSclR = enemy_->GetArmScaleR();
	bool	eAtkR = enemy_->IsAttackingR();
	Vector3 eArmPosL = enemy_->GetArmPositionL();
	float   eArmSclL = enemy_->GetArmScaleL();
	bool	eAtkL = enemy_->IsAttackingL();

	// 判定処理開始
	// 1. 敵からの攻撃 (引数の最後に eScale を追加)
	if (enemy_->IsAttackingR()) {
         // 敵・右 vs プレイヤー・本体
		CheckArmCollision(player_, playerBodyAABB, eArmPosR, eArmSclR, eScale);
	}

	if (enemy_->IsAttackingL()) {
		// 敵・左 vs プレイヤー・本体
		CheckArmCollision(player_, playerBodyAABB, eArmPosL, eArmSclL, eScale);
	}

	// 2. プレイヤーからの攻撃 (引数の最後に pScale を追加)
	if (player_->IsAttackingR()) {
		// 自・右 vs 敵・本体
		CheckArmCollision(enemy_, enemyBodyAABB, pArmPosR, pArmSclR, pScale);
	}

	if (player_->IsAttackingL()) {
		// 自・左 vs 敵・本体
		CheckArmCollision(enemy_, enemyBodyAABB, pArmPosL, pArmSclL, pScale);
	}
	// 3. 腕 vs 腕 の判定
	// 自・右 vs 敵・右
	CheckArmToArm(player_, pArmPosR, pArmSclR, pScale, pAtkR, enemy_, eArmPosR, eArmSclR, eScale, eAtkR);
	// 自・右 vs 敵・左
	CheckArmToArm(player_, pArmPosR, pArmSclR, pScale, pAtkR, enemy_, eArmPosL, eArmSclL, eScale, eAtkL);
	// 自・左 vs 敵・右
	CheckArmToArm(player_, pArmPosL, pArmSclL, pScale, pAtkL, enemy_, eArmPosR, eArmSclR, eScale, eAtkR);
	// 自・左 vs 敵・左
	CheckArmToArm(player_, pArmPosL, pArmSclL, pScale, pAtkL, enemy_, eArmPosL, eArmSclL, eScale, eAtkL);

	// ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲
}