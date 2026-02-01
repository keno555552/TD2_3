#define NOMINMAX
#include "SceneStage01.h"
#include "crashDecision/crashDecision.h"

#include <algorithm>
#include <numbers>

SceneStage01::SceneStage01(kEngine *system) {
	/// =========== システム初期化 ============///
	system_ = system;

	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));
	camera_->SetFarClip(20000.0f);
	debugCamera_->SetFarClip(20000.0f);
	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	playerModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/charater/simplePlayer.obj");
	guideArrowModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/arrow/arrow.obj");
	wallModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/cube/cube.obj");

	boxTextureHandle_ = system_->LoadTexture("resources/texture/testBox.png");
	tryTextureHandle_ = system_->LoadTexture("resources/texture/Tryer.png");
	uvTextureHandle_ = system_->LoadTexture("resources/TemplateResource/texture/uvChecker.png");


	// =====================
   // 門モデル一覧（画像の通り）
   // =====================
	const char* DoujouModelPaths[kDoujouPartCount] = {
	//掛け軸
		"resources/game/model/doujou_kakeziku.obj",
		"resources/game/model/doujou_kakeziku_flame.obj",
		"resources/game/model/doujou_kakeziku_mozi.obj",
	
		//白い壁
		"resources/game/model/doujou_wall_roof.obj",
		"resources/game/model/doujou_wall_back.obj",
		"resources/game/model/doujou_wall_front.obj",
		"resources/game/model/doujou_wall_right.obj",
		"resources/game/model/doujou_wall_left.obj",
		//木の部分
		"resources/game/model/doujou_roof.obj",
		"resources/game/model/doujou_wood_back.obj",
		"resources/game/model/doujou_wood_front.obj",
		"resources/game/model/doujou_wood_right.obj",
		"resources/game/model/doujou_wood_left.obj",
		//畳（床）
		"resources/game/model/tatami.obj",
		"resources/game/model/tatami_flame.obj",
		

	};


	const char* DoujouTexturePaths[kDoujouPartCount] = {
		"resources/game/texture/paper.png",
		"resources/game/texture/ohuda_flame.png",
		"resources/gate/model/punch.png",

		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",

		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",

		"resources/game/texture/tatami.png",
		"resources/game/texture/tatami_flame.png",
		
	};

	for (int i = 0; i < kDoujouPartCount; i++) {
		if (!DoujouModelPaths[i] || !DoujouTexturePaths[i]) {
			continue;
		}

		DoujouModels_[i] = system_->SetModelObj(DoujouModelPaths[i]);
		DoujouTextures_[i] = system_->LoadTexture(DoujouTexturePaths[i]);

		Doujous_[i] = new Object;
		Doujous_[i]->IntObject(system_);

		// ★ 別プロジェクト寄せ：最低1パーツ確保
		Doujous_[i]->CreateDefaultData();
		Doujous_[i]->CreateModelData(DoujouModels_[i]);
		Doujous_[i]->mainPosition.transform.translate = Vector3(49.9f, 18.05f, 48.15f);

		Doujous_[i]->mainPosition.transform.scale = Vector3(0.205f, 0.2f, 0.2f);

		// ★ モデルごとにテクスチャを明示設定
		for (auto& part : Doujous_[i]->objectParts_) {
			if (!part.materialConfig) {
				part.materialConfig = std::make_shared<MaterialConfig>();
			}

			part.materialConfig->useModelTexture = false;
			part.materialConfig->textureHandle = DoujouTextures_[i];

			// ★ ライティングは有効化（ライトで色味を作る）
			part.materialConfig->enableLighting = true;

			// ★ textureColor は触らない（ここ重要）
		}

		//Doujous_[i]->Update(usingCamera_);
	

	}
	
	for (int i = 0; i < kDoujouPartCount; i++) {
		if (!Doujous_[i]) continue;

		DoujouPos_[i] = Doujous_[i]->mainPosition.transform.translate;
		DoujouRot_[i] = Doujous_[i]->mainPosition.transform.rotate;
		DoujouScale_[i] = Doujous_[i]->mainPosition.transform.scale;
	}

	constexpr int IDX_TATAMI = 13;
	constexpr int IDX_TATAMI_FRAME = 14;

	constexpr int TATAMI_X = 4;
	constexpr int TATAMI_Z = 7;

	const float TATAMI_SCALE = 0.3;

	// tatami.obj の実寸（Blender基準。合ってなければここだけ調整）
	const Vector3 TATAMI_SIZE = { 97.0f, 0.0f, 48.0f }; // X短辺 / Z長辺

	// =====================
// 畳タイル配置（畳 + フレーム）
// =====================

// 基準位置（好きに変えろ）
	Vector3 origin = { 15.0f, 0.0f, 7.5f };

	// 1枚あたりの間隔（モデル実寸 * スケール）
	// ※畳の向きが違うなら x/z を入れ替え
	float stepX = TATAMI_SIZE.x * TATAMI_SCALE;
	float stepZ = TATAMI_SIZE.z * TATAMI_SCALE;

	// 生成関数（畳とフレームを同じTransformで作る）
	auto CreateTatamiPair = [&](const Vector3& pos) {
		// 畳
		{
			Object* t = new Object();
			t->IntObject(system_);
			t->CreateDefaultData();
			t->CreateModelData(DoujouModels_[IDX_TATAMI]);

			t->mainPosition.transform.translate = pos;
			t->mainPosition.transform.scale = { TATAMI_SCALE, TATAMI_SCALE, TATAMI_SCALE };

			for (auto& part : t->objectParts_) {
				if (!part.materialConfig) part.materialConfig = std::make_shared<MaterialConfig>();
				part.materialConfig->useModelTexture = false;
				part.materialConfig->textureHandle = DoujouTextures_[IDX_TATAMI];
				part.materialConfig->enableLighting = true;
			}

			tatamiTiles_.push_back(t);
		}

		// フレーム（座標・回転・スケール同じ）
		{
			Object* f = new Object();
			f->IntObject(system_);
			f->CreateDefaultData();
			f->CreateModelData(DoujouModels_[IDX_TATAMI_FRAME]);

			f->mainPosition.transform.translate = pos;
			f->mainPosition.transform.scale = { TATAMI_SCALE, TATAMI_SCALE, TATAMI_SCALE };

			for (auto& part : f->objectParts_) {
				if (!part.materialConfig) part.materialConfig = std::make_shared<MaterialConfig>();
				part.materialConfig->useModelTexture = false;
				part.materialConfig->textureHandle = DoujouTextures_[IDX_TATAMI_FRAME];
				part.materialConfig->enableLighting = true;
			}

			tatamiFrames_.push_back(f);
		}
		};

	// タイル敷き
	for (int z = 0; z < TATAMI_Z; ++z) {
		for (int x = 0; x < TATAMI_X; ++x) {
			Vector3 p = {
				origin.x + stepX * x,
				origin.y,
				origin.z + stepZ * z
			};
			CreateTatamiPair(p);
		}
	}

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

	explosion_ = new Explosion(system_);


	/// --- チュートリアルお試し ---
	tutorial_ = new Tutorial(system_);

	// ■■■ まとめて登録（リスト化） ■■■
	// フォルダ内のファイルを順番に登録するイメージです
	std::vector<std::string> fileList = {
		"resources/TemplateResource/object/plane/plane.obj"
		// 例
		//"resources/tutorial/text_move.obj",   // 0: 移動説明
		//"resources/tutorial/text_attack.obj", // 1: 攻撃説明
		//"resources/tutorial/text_skill.obj",  // 2: スキル説明
		//"resources/tutorial/text_win.obj"     // 3: 勝利条件
	};

	for (const auto& path : fileList) {
		tutorial_->AddPage(path);
	}
	/// -------------------------------------------
}

SceneStage01::~SceneStage01() {
	system_->DestroyCamera(camera_);
	system_->DestroyCamera(debugCamera_);

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


	for (auto* t : tatamiTiles_)  delete t;
	for (auto* f : tatamiFrames_) delete f;
	tatamiTiles_.clear();
	tatamiFrames_.clear();


	delete tutorial_;
}

void SceneStage01::Update() {
	tutorial_->Update(usingCamera_);

	// チュートリアル中はゲームを止める場合
	if (tutorial_->IsActive()) {
		return;
	}

	// ゲーム開始時やイベント発生時に呼び出し
	if (system_->GetTriggerOn(DIK_T)) {
		tutorial_->Open(0); // 最初から再生
	}

	//// 特定の条件で途中から再生(イメージ)
	//if (isSkillUnlocked) {
	//	tutorial_->Open(2); // スキル説明(2番)だけを表示
	//	isSkillUnlocked = false; // 一度だけ出すフラグ管理など
	//}

	// ▼▼▼ ヒットストップの管理処理 (Updateの冒頭推奨) ▼▼▼
	if (hitStopTimer_ > 0.0f) {
		// タイマーを減らす
		// ★重要: ここは「実時間 (GetDeltaTime)」を使う！
		hitStopTimer_ -= system_->GetDeltaTime();

		// 時間切れで元に戻す
		if (hitStopTimer_ <= 0.0f) {
			hitStopTimer_ = 0.0f;
			system_->SetTimeScale(1.0f); // 等速に戻す
		}
	}
	// ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲

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

		Vector3 difference = player_->mainPosition.transform.translate - enemy->mainPosition.transform.translate;
		float distance = sqrtf(difference.x * difference.x + difference.y * difference.y + difference.z * difference.z);
		float nearDistance = distance;
		Vector3 nearPos = player_->mainPosition.transform.translate;
		for (auto* other : enemies_) {
			if (other == enemy) continue; // 自身は除外

			// 差分を計算
			difference = other->mainPosition.transform.translate - enemy->mainPosition.transform.translate;
			// 距離を計算
			distance = sqrtf(difference.x * difference.x + difference.y * difference.y + difference.z * difference.z);

			if (distance < nearDistance) {
				nearDistance = distance; // 一番近い距離の変更
				nearPos = other->mainPosition.transform.translate; // 一番近い敵の位置の変更
			}
		}


		enemy->SetTargetPos(nearPos);

		enemy->Update(usingCamera_);
	}


	/// 衝突判定処理

	// 攻撃当たり判定処理
	CheckAttackCollision();
	// ■■■ 壁と自機の当たり判定（範囲制限版） ■■■
    // プレイヤーの半径（半幅）
	float pRadiusX = player_->mainPosition.transform.scale.x / 2.0f;
	float pRadiusZ = player_->mainPosition.transform.scale.z / 2.0f;

	for (int i = 0; i < 4; i++) {
		// 壁情報の更新と取得
		wall_[i]->Update(usingCamera_);
		Vector3 wPos = wall_[i]->mainPosition.transform.translate;
		Vector3 wScale = wall_[i]->mainPosition.transform.scale;
		float wRadiusX = wScale.x / 2.0f;
		float wRadiusZ = wScale.z / 2.0f;

		// 現在のプレイヤー位置
		Vector3 pPos = player_->mainPosition.transform.translate;

		// ■ 1. まずは大まかな衝突判定 (AABB)
		// 高速移動ですり抜けないよう、本来のサイズより少し大きめに判定枠を取る
		// (壁の範囲 + プレイヤー半径) の範囲内にプレイヤー中心がいれば衝突とみなす
		bool isHitX = (pPos.x >= wPos.x - wRadiusX - pRadiusX) && (pPos.x <= wPos.x + wRadiusX + pRadiusX);
		bool isHitZ = (pPos.z >= wPos.z - wRadiusZ - pRadiusZ) && (pPos.z <= wPos.z + wRadiusZ + pRadiusZ);

		if (isHitX && isHitZ) {
			// ■ 2. 衝突した場合、「X方向」と「Z方向」どちらのめり込みが浅いか調べる
			// 浅い方に押し出すことで、自然な挙動になります。

			float diffX = pPos.x - wPos.x;
			float diffZ = pPos.z - wPos.z;

			// めり込みの深さ（絶対値が小さいほど、その面に近い）
			// (半径の合計) - (実際の距離)
			float overlapX = (wRadiusX + pRadiusX) - std::abs(diffX);
			float overlapZ = (wRadiusZ + pRadiusZ) - std::abs(diffZ);

			// --- X軸方向の補正 (横壁に当たった) ---
			if (overlapX < overlapZ) {
				// プレイヤーが壁より「左」にいるなら、左側の境界へ戻す
				if (diffX < 0) {
					player_->mainPosition.transform.translate.x = (wPos.x - wRadiusX - pRadiusX) - 0.001f;
					// 右方向への速度成分があれば消す（壁に吸われないように）
					player_->OnWallHit(Vector3(1.0f, 0.0f, 0.0f));
				}
				// プレイヤーが壁より「右」にいるなら、右側の境界へ戻す
				else {
					player_->mainPosition.transform.translate.x = (wPos.x + wRadiusX + pRadiusX) + 0.001f;
					// 左方向への速度成分があれば消す
					player_->OnWallHit(Vector3(-1.0f, 0.0f, 0.0f));
				}
			}
			// --- Z軸方向の補正 (奥・手前の壁に当たった) ---
			else {
				// 手前 (Zマイナス)
				if (diffZ < 0) {
					player_->mainPosition.transform.translate.z = (wPos.z - wRadiusZ - pRadiusZ) - 0.001f;
					player_->OnWallHit(Vector3(0.0f, 0.0f, 1.0f));
				}
				// 奥 (Zプラス)
				else {
					player_->mainPosition.transform.translate.z = (wPos.z + wRadiusZ + pRadiusZ) + 0.001f;
					player_->OnWallHit(Vector3(0.0f, 0.0f, -1.0f));
				}
			}
		}
	}
	// ...上記のループ処理の後、あるいはループ内で「明らかに部屋の外に出た」場合の保険...

	float limitX = 99.0f - pRadiusX; // 壁の位置 - プレイヤーサイズ
	if (player_->mainPosition.transform.translate.x > limitX) {
		player_->mainPosition.transform.translate.x = limitX;
	}
	if (player_->mainPosition.transform.translate.x < 1) {
		player_->mainPosition.transform.translate.x = 1;
	}
	float limitZ = 99.0f - pRadiusZ; // 壁の位置 - プレイヤーサイズ
	if (player_->mainPosition.transform.translate.z > limitZ) {
		player_->mainPosition.transform.translate.z = limitZ;
	}
	if (player_->mainPosition.transform.translate.z < 1) {
		player_->mainPosition.transform.translate.z = 1;
	}

#pragma region 自機用パーティクル
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


	if (system_->GetTriggerOn(DIK_F)) {
		explosion_->SetIsEmitting(true);
		explosion_->SetPos(player_->mainPosition.transform.translate);
	}
	explosion_->Update(usingCamera_);
#pragma endregion

	/// ガイド矢印更新
	UpdateGuideArrow();
	/// 王冠の更新
	UpdateCrown();

	//背景オブジェクト
	for (int i = 0; i < kDoujouPartCount; i++) {
		if (!Doujous_[i]) continue;

		Doujous_[i]->mainPosition.transform.translate = DoujouPos_[i];
		Doujous_[i]->mainPosition.transform.rotate = DoujouRot_[i];
		Doujous_[i]->mainPosition.transform.scale = DoujouScale_[i];
	}

	for (int i = 0; i < kDoujouPartCount; i++) {
		if (Doujous_[i]) {
			Doujous_[i]->Update(usingCamera_);
		}
	}
	//畳（床）
	for (auto* t : tatamiTiles_)  t->Update(usingCamera_);
	for (auto* f : tatamiFrames_) f->Update(usingCamera_);

	



	if (system_->GetTriggerOn(DIK_0)) {
		if (useDebugCamera)useDebugCamera = false;
		else useDebugCamera = true;
	}

	if (system_->GetTriggerOn(DIK_SPACE)) {
		outcome_ = SceneOutcome::NEXT;
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

	explosion_->Draw();
	//畳（床）
	for (auto* t : tatamiTiles_)  t->Draw();
	for (auto* f : tatamiFrames_) f->Draw();

	//背景オブジェクト
	for (int i = 0; i < kDoujouPartCount-2; i++) {
		if (Doujous_[i]) {
			Doujous_[i]->Draw();
		}
	}


	// チュートリアルお試し
	tutorial_->Draw(); // 最前面に描画

#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void SceneStage01::CameraPart() {
	if (useDebugCamera) {
		usingCamera_ = debugCamera_;
		debugCamera_->MouseControlUpdate();
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

				if (ImGui::Checkbox("Test Expand", &e->IsTestExpand()));

				if (ImGui::TreeNode("Transform")) {
					ImGui::DragFloat3("Pos", &e->mainPosition.transform.translate.x, 0.1f);
					ImGui::SliderFloat3("Rotate", &e->mainPosition.transform.rotate.x, -6.5f, 6.5f);
					ImGui::SliderFloat3("Scale", &e->mainPosition.transform.scale.x, 0.0f, 5.0f);
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("ArmScale")) {
					// テスト用に強制的に値を書き換えるスライダー
					// 実際はUpdateで変動するので、一時停止中などに有効
					ImGui::SliderFloat("ArmScaleL", &e->GetRefArmScaleL(), 1.0f, 50.0f);
					ImGui::SliderFloat("ArmScaleR", &e->GetRefArmScaleR(), 1.0f, 50.0f);
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

#ifdef _DEBUG
	ImGui::Begin("Gate Debug");

	for (int i = 0; i < kDoujouPartCount; i++) {
		if (!Doujous_[i]) continue;

		ImGui::PushID(i);

		ImGui::Text("Gate %d", i);
		ImGui::DragFloat3("Position", &DoujouPos_[i].x, 0.05f);
		ImGui::DragFloat3("Rotation", &DoujouRot_[i].x, 0.01f);
		ImGui::DragFloat3("Scale", &DoujouScale_[i].x, 0.01f);
	
		ImGui::Separator();
		ImGui::PopID();
	}

	ImGui::End();
#endif


}
#endif

void SceneStage01::CheckAttackCollision() {
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
					// ▼▼▼ 演出追加 ▼▼▼
					// 0.05倍速(ほぼ停止) にして、0.1秒間(実時間) 止める
					// これにより「ガッ！」という重いヒット感が生まれます
					StartHitStop(0.05f, 0.15f);

					player_->AddKnockback(knockbackForce);
					explosion_->SetIsEmitting(true);
					explosion_->SetPos(player_->mainPosition.transform.translate);
				} else {
					// targetObjが敵リストの中にいるか確認、あるいはキャストして適用
					// ここでは簡易的に Enemy* へのキャストで対応
					Enemy* e = dynamic_cast<Enemy*>(targetObj);
					if (e) {
						e->AddKnockback(knockbackForce);
						explosion_->SetIsEmitting(true);
						explosion_->SetPos(player_->mainPosition.transform.translate);
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

						// スロー演出（プレイヤーが関与している場合のみ推奨）
						if (isCheckInPlayer) StartHitStop(0.05f, 0.15f);

						Vector3 force = { dir.x * power, 0.0f, dir.z * power };

						if (eB) {
							eB->AddKnockback(force);
							eB->ResetArm(isRightB);
						}
						if (objB == player_) {
							player_->AddKnockback(force);
							player_->ResetArm(isRightB);
						}
						// エフェクト
						explosion_->SetIsEmitting(true);
						explosion_->SetPos((posA + posB) / 2.0f);
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

						if (isCheckInPlayer) StartHitStop(0.05f, 0.15f);

						Vector3 force = { dir.x * power, 0.0f, dir.z * power };

						if (eA) {
							eA->AddKnockback(force);
							eA->ResetArm(isRightA);
						}
						if (objA == player_) {
							player_->AddKnockback(force);
							player_->ResetArm(isRightA);
						}
						explosion_->SetIsEmitting(true);
						explosion_->SetPos((posA + posB) / 2.0f);
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
								explosion_->SetIsEmitting(true);
								explosion_->SetPos((posA + posB) / 2.0f);
							} else {
								Enemy* e = dynamic_cast<Enemy*>(objA);
								if (e) {
									e->AddKnockback(forceA);
									explosion_->SetIsEmitting(true);
									explosion_->SetPos((posA + posB) / 2.0f);
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
								explosion_->SetIsEmitting(true);
								explosion_->SetPos((posA + posB) / 2.0f);
							} else {
								Enemy* e = dynamic_cast<Enemy*>(objB);
								if (e) {
									e->AddKnockback(forceB);
									explosion_->SetIsEmitting(true);
									explosion_->SetPos((posA + posB) / 2.0f);
								}
							}
						}

						// エフェクトとスローは「どちらかがヒットした」なら実行
						if ((!ignoreA || !ignoreB) && isCheckInPlayer) {
							StartHitStop(0.05f, 0.15f);
							explosion_->SetIsEmitting(true);
							explosion_->SetPos((posA + posB) / 2.0f);
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
	for (auto* enemy : enemies_) {

		// エネミーの情報
		AABB eBodyAABB = enemy->GetAABB(enemy);
		float eBodyScale = enemy->mainPosition.transform.scale.x;
		// 敵の左右の情報（敵が複数の場合はループ内で）
		ArmInfo eArms[2] = {
			{ false, enemy->GetArmPositionL(), enemy->GetArmScaleL(), enemy->IsAttackingL() },
			{ true,  enemy->GetArmPositionR(), enemy->GetArmScaleR(), enemy->IsAttackingR() }
		};

		for (int i = 0; i < 2; i++) {
			// 1. 敵の攻撃 vs プレイヤーの体
			if (eArms[i].isAtk) CheckArmCollision(player_, pBodyAABB, eArms[i].pos, eArms[i].scale, eBodyScale);
			// 2. プレイヤーの攻撃 vs 敵の体
			if (pArms[i].isAtk) CheckArmCollision(enemy, eBodyAABB, pArms[i].pos, pArms[i].scale, pBodyScale);
			for (int j = 0; j < 2; j++) {
				// 3. 腕同士の衝突
				CheckArmToArm(
					player_, pArms[i].pos, pArms[i].scale, pBodyScale, pArms[i].isRight, pArms[i].isAtk, 
					enemy, eArms[j].pos, eArms[j].scale, eBodyScale, eArms[j].isRight, eArms[j].isAtk);
			}
		}
	}
}

void SceneStage01::StartHitStop(float scale, float duration) {
	// 既にスロー中なら、より長い時間を優先するか、上書きするかはお好みで。
	if (hitStopTimer_ <= 0.0f) {
		system_->SetTimeScale(scale);
		hitStopTimer_ = duration;
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
	// 目標スケール変数を定義（初期値は 0.0f = 非表示）
	float targetScale = 0.0f;

	if (targetEnemy != nullptr) {
		// --- ターゲットが見つかった場合（表示） ---

		Vector3 pPos = player_->mainPosition.transform.translate;
		Vector3 ePos = targetEnemy->mainPosition.transform.translate;

		// A. 目標角度の計算
		float diffX = ePos.x - pPos.x;
		float diffZ = ePos.z - pPos.z;

		// ターゲットへの本来の角度 (atan2(x, z) で計算)
		// ※ +PIしているのはモデルの向き合わせのため（元のコード準拠）
		float targetAngleY = std::atan2(diffX, diffZ) + std::numbers::pi_v<float>;
		float currentAngleY = guideArrow_->mainPosition.transform.rotate.y;

		// ■■■ 角度の最短補間処理 ■■■

		// 1. 差分を計算
		float deltaAngle = targetAngleY - currentAngleY;

		// 2. 差分を -PI ～ +PI の範囲に正規化する（最短ルート判定）
		// これをしないと、350度→10度の時にぐるっと遠回りしてしまいます
		while (deltaAngle > std::numbers::pi_v<float>)  deltaAngle -= 2.0f * std::numbers::pi_v<float>;
		while (deltaAngle < -std::numbers::pi_v<float>) deltaAngle += 2.0f * std::numbers::pi_v<float>;

		// 3. 補間係数 (0.0f ～ 1.0f)
		// デルタタイム実装後は、ここを (speed * deltaTime) に置き換えてください。
		// 今は仮の値 (0.1f = 1フレームで差の10%縮まる) を入れます。
		float lerpFactor = 0.1f;

		// 4. 現在の角度に「差分の一部」を足す
		guideArrow_->mainPosition.transform.rotate.y += deltaAngle * lerpFactor;


		// ■■■ 補間後の角度を使って座標を決める ■■■
		// (重要) 補間中の angleY を使って方向ベクトル(dir)を作り直します
		// 元のコードの dir 計算は使わず、今の回転角度から逆算します

		// 現在の矢印の向き（補間中）を取得
		float smoothAngle = guideArrow_->mainPosition.transform.rotate.y;

		// +PI していた分を引いて、純粋な方向計算用角度に戻す
		float directionAngle = smoothAngle - std::numbers::pi_v<float>;

		// 角度から方向ベクトルを作成 (sin, cos)
		Vector3 dir = {
			std::sin(directionAngle),
			0.0f,
			std::cos(directionAngle)
		};

		// C. 矢印の配置半径
		float baseRadius = 0.5f;
		float radiusFactor = 1.0f;
		float currentRadius = baseRadius + (pMaxArm * radiusFactor);

		// プレイヤーの中心から、現在の角度(dir)方向に配置
		guideArrow_->mainPosition.transform.translate = {
			pPos.x + dir.x * currentRadius,
			pPos.y + 0.5f,
			pPos.z + dir.z * currentRadius
		};


		// D. 矢印の大きさ
		float playerGlobalScale = kBaseActorScale_.x;
		float startRatio = 0.4f;
		float growthRatio = 0.2f;
		// ここでは「代入」ではなく「目標値の設定」だけを行う
		targetScale = (startRatio + (pMaxArm - 1.0f) * growthRatio) * playerGlobalScale;

	} else {
		// --- ターゲットなしの場合 ---

		// 目標スケールは 0.0f (初期値のまま)
		targetScale = 0.0f;

		// 【重要】フェードアウト中も矢印がその場に置き去りにならないようにする工夫
		// ターゲットがいなくても、矢印は「直前の位置」あるいは「プレイヤーのそば」にいてほしいです。
		// 簡易的に「プレイヤーの位置 + 直前のオフセット」で更新し続けるか、
		// あるいは何もしなければ「最後にターゲットがいた方向」でその場で縮んで消えます。
		// (今回はシンプルに位置更新はせず、その場で縮む挙動になります)
	}

	// ▼▼▼ 共通処理：サイズの滑らかな更新（フェードイン・アウト） ▼▼▼

	float currentScale = guideArrow_->mainPosition.transform.scale.x;

	// 補間係数 (回転と同じ lerpFactor を使うと統一感が出ます)
	float lerpFactor = 0.1f;

	// 現在値から目標値へ近づける
	float nextScale = currentScale + (targetScale - currentScale) * lerpFactor;

	// 計算誤差で完全に0にならないのを防ぐ（0.001未満なら0にする）
	if (std::abs(nextScale) < 0.001f) {
		nextScale = 0.0f;
	}

	guideArrow_->mainPosition.transform.scale = { nextScale, nextScale, nextScale };


	// 4. 矢印の更新
	guideArrow_->Update(camera_);
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
