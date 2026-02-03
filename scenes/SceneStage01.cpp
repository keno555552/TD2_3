#define NOMINMAX
#include "SceneStage01.h"
#include "crashDecision/crashDecision.h"

#include <algorithm>
#include <numbers>
#include "SceneManager.h"

SceneStage01::SceneStage01(kEngine* system) {
	/// =========== システム初期化 ============///
	system_ = system;

	debugCamera_ = system_->CreateDebugCamera();
	camera_ = system_->CreateCamera();
	camera_->Move(Vector3(0.0f, 0.5f, -10.0f));
	camera_->SetFarClip(2000.0f);
	debugCamera_->SetFarClip(2000.0f);
	/// =========== リソースロード ============///
	skydomeModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/skydome/skydome.obj");
	playerModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/charater/simplePlayer.obj");
	guideArrowModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/arrow/arrow.obj");
	wallModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/cube/cube.obj");

	boxTextureHandle_ = system_->LoadTexture("resources/texture/testBox.png");
	tryTextureHandle_ = system_->LoadTexture("resources/texture/Tryer.png");
	uvTextureHandle_ = system_->LoadTexture("resources/TemplateResource/texture/uvChecker.png");

	///順位モデル

	const char* KanjiModelPaths[kKanjiCount] = {
	"resources/game/model/1.obj",
	"resources/game/model/2.obj",
	"resources/game/model/3.obj",
	"resources/game/model/4.obj",
	"resources/game/model/5.obj",
	"resources/game/model/6.obj",
	"resources/game/model/7.obj",
	"resources/game/model/8.obj",
	"resources/game/model/9.obj",
	"resources/game/model/10.obj",
	"resources/game/model/1_1.obj",
	"resources/game/model/number_flame_flame.obj",
	"resources/game/model/number_flame.obj",
	};

	const char* KanjiTexturePaths[kKanjiCount] = {
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/game/texture/ohuda_flame.png",
		"resources/game/texture/paper.png",

	};

	for (int i = 0; i < kKanjiCount; i++) {

		KanjiModels_[i] = system_->SetModelObj(KanjiModelPaths[i]);
		KanjiTextures_[i] = system_->LoadTexture(KanjiTexturePaths[i]);

		KanjiObjs_[i] = new Object;
		KanjiObjs_[i]->IntObject(system_);
		KanjiObjs_[i]->CreateDefaultData();
		KanjiObjs_[i]->CreateModelData(KanjiModels_[i]);

		for (auto& part : KanjiObjs_[i]->objectParts_) {
			if (!part.materialConfig) {
				part.materialConfig = std::make_shared<MaterialConfig>();
			}
			part.materialConfig->useModelTexture = false;
			part.materialConfig->textureHandle = KanjiTextures_[i];
			part.materialConfig->enableLighting = false; // 文字なので基本OFF推奨
		}

		// 初期配置（必要に応じて後で順位別に動かす）
		KanjiObjs_[i]->mainPosition.transform.translate =
			Vector3(49.9f, 20.0f, 48.15f);

		KanjiObjs_[i]->mainPosition.transform.scale =
			Vector3(0.2f, 0.2f, 0.2f);

		// 保存
		KanjiPos_[i] = KanjiObjs_[i]->mainPosition.transform.translate;
		KanjiRot_[i] = KanjiObjs_[i]->mainPosition.transform.rotate;
		KanjiScale_[i] = KanjiObjs_[i]->mainPosition.transform.scale;
	}
	// =====================================================
// Illustration
// =====================================================

	constexpr int kIllustrationPartCount = 10;

	const char* IllustrationModelPaths[kIllustrationPartCount] = {
	"resources/game/model/1_syouzi.obj",
	"resources/game/model/2_syouzi.obj",
	"resources/game/model/3_syouzi.obj",
	"resources/game/model/4_syouzi.obj",
	"resources/game/model/5_syouzi.obj",
	"resources/game/model/6_syouzi.obj",
	"resources/game/model/7_syouzi.obj",
	"resources/game/model/8_syouzi.obj",
	"resources/game/model/9_syouzi.obj",
	"resources/game/model/10_syouzi.obj",
	};

	const char* IllustrationTexturePaths[kIllustrationPartCount] = {
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",
		"resources/gate/model/punch.png",

	};



	for (int i = 0; i < kIllustrationPartCount; i++) {
		IllustrationModels[i] = system_->SetModelObj(IllustrationModelPaths[i]);
		IllustrationTextures[i] = system_->LoadTexture(IllustrationTexturePaths[i]);

		Illustrations[i] = new Object;
		Illustrations[i]->IntObject(system_);
		Illustrations[i]->CreateDefaultData();
		Illustrations[i]->CreateModelData(IllustrationModels[i]);

		for (auto& part : Illustrations[i]->objectParts_) {
			if (!part.materialConfig) {
				part.materialConfig = std::make_shared<MaterialConfig>();
			}
			part.materialConfig->useModelTexture = false;
			part.materialConfig->textureHandle = IllustrationTextures[i];
		}

		Illustrations[i]->mainPosition.transform.translate = Vector3(47.0f, 18.05f, (48.15f - 80 * i));
		Illustrations[i]->mainPosition.transform.scale = Vector3(0.205f, 0.2f, 0.2f);

		IllustrationPos[i] = Illustrations[i]->mainPosition.transform.translate;
		IllustrationRot[i] = Illustrations[i]->mainPosition.transform.rotate;
		IllustrationScale[i] = Illustrations[i]->mainPosition.transform.scale;
	}

	// =====================
   // 背景モデル一覧（画像の通り）
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
		//白い壁
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
		//木の部分
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		//畳（床）
		"resources/game/texture/tatami.png",
		"resources/game/texture/tatami_flame.png",

	};

	Vector3 DoujouModelsForward[kDoujouPartCount] = {
		{0,0,1},
		{0,0,1},
		{0,0,1},

		{},
		{0,0,-1},
		{0,0,1},
		{1,0,0},
		{-1,0,0},

		{},
		{0,0,-1},
		{0,0,1},
		{1,0,0},
		{-1,0,0},

		{},
		{}
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

		Doujous_[i]->objectParts_[0].forward = DoujouModelsForward[i];

		// ★ モデルごとにテクスチャを明示設定
		for (auto& part : Doujous_[i]->objectParts_) {
			if (!part.materialConfig) {
				part.materialConfig = std::make_shared<MaterialConfig>();
			}

			part.materialConfig->useModelTexture = false;
			part.materialConfig->textureHandle = DoujouTextures_[i];



			// ★ textureColor は触らない（ここ重要）
		}



	}

	for (int i = 0; i < kDoujouPartCount; i++) {
		if (!Doujous_[i]) continue;

		DoujouPos_[i] = Doujous_[i]->mainPosition.transform.translate;
		DoujouRot_[i] = Doujous_[i]->mainPosition.transform.rotate;
		DoujouScale_[i] = Doujous_[i]->mainPosition.transform.scale;
	}


	const char* ResultModelPaths[kResultPartCount] = {
	"resources/game/model/result_wall.obj",
	"resources/game/model/result_wood.obj",
	"resources/game/model/result_syouji_left.obj",
	"resources/game/model/result_syouji_right.obj",
	"resources/game/model/result_paper_left.obj",
	"resources/game/model/result_paper_right.obj",
	};

	const char* ResultTexturePaths[kResultPartCount] = {

		"resources/game/texture/paper.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/wood_tate.png",
		"resources/game/texture/paper.png",
		"resources/game/texture/paper.png",
	};



	for (int i = 0; i < kResultPartCount; i++) {
		ResultModels_[i] = system_->SetModelObj(ResultModelPaths[i]);
		ResultTextures_[i] = system_->LoadTexture(ResultTexturePaths[i]);

		Results_[i] = new Object;
		Results_[i]->IntObject(system_);
		Results_[i]->CreateDefaultData();
		Results_[i]->CreateModelData(ResultModels_[i]);

		for (auto& part : Results_[i]->objectParts_) {
			if (!part.materialConfig) {
				part.materialConfig = std::make_shared<MaterialConfig>();
			}
			part.materialConfig->useModelTexture = false;
			part.materialConfig->textureHandle = ResultTextures_[i];

		}

		// 先に設定
		Results_[i]->mainPosition.transform.translate = Vector3(49.9f, 18.05f, 48.15f);
		Results_[i]->mainPosition.transform.scale = Vector3(0.205f, 0.2f, 0.2f);

		// そのあと保存
		ResultPos_[i] = Results_[i]->mainPosition.transform.translate;
		ResultRot_[i] = Results_[i]->mainPosition.transform.rotate;
		ResultScale_[i] = Results_[i]->mainPosition.transform.scale;

	}

	constexpr int IDX_SYOUJI_L = 2;
	constexpr int IDX_SYOUJI_R = 3;

	ResultSet base;

	for (int i = 0; i < kResultPartCount; i++) {
		base.parts.push_back(Results_[i]);
		base.basePos.push_back(ResultPos_[i]);
	}
	ResultSet set;
	set.shojiOpened = false;
	set.shojiOpening = false;
	set.shojiT = 0.0f;

	// ★ メンバ変数に入れる
	resultSets_.push_back(base);


	constexpr int IDX_TATAMI = 13;
	constexpr int IDX_TATAMI_FRAME = 14;

	constexpr int TATAMI_X = 4;
	constexpr int TATAMI_Z = 7;

	constexpr float TATAMI_SCALE = 0.3f;

	// tatami.obj の実寸
	const Vector3 TATAMI_SIZE = { 97.0f, 0.0f, 48.0f };

	// =====================
	// 畳タイル配置（畳 + フレーム）
	// =====================

	// 基準位置（1セット目）
	Vector3 origin = { 15.0f, 0.1f, 7.5f };

	// 1枚あたりの間隔
	float stepX = TATAMI_SIZE.x * TATAMI_SCALE;
	float stepZ = TATAMI_SIZE.z * TATAMI_SCALE;

	// ★ セット全体のサイズ
	float setSizeX = stepX * TATAMI_X;
	float setSizeZ = stepZ * TATAMI_Z;

	// ★ 何セット並べるか（ここだけ変えれば張り替え）
	constexpr int TATAMI_SET_COUNT = 10;

	// =====================
	// セット複製ループ
	// =====================
	for (int set = 0; set < TATAMI_SET_COUNT; ++set) {
		// ★ このセットの基準位置
		Vector3 setOrigin = {
			origin.x,
			origin.y,
			origin.z - setSizeZ * set   // ← Z方向に複製
		};

		// タイル敷き（中身は元コードそのまま）
		for (int z = 0; z < TATAMI_Z; ++z) {
			for (int x = 0; x < TATAMI_X; ++x) {

				Vector3 p = {
					setOrigin.x + stepX * x,
					setOrigin.y,
					setOrigin.z + stepZ * z
				};

				// 畳
				{
					Object* t = new Object();
					t->IntObject(system_);
					t->CreateDefaultData();
					t->CreateModelData(DoujouModels_[IDX_TATAMI]);

					t->mainPosition.transform.translate = p;
					t->mainPosition.transform.scale =
					{ TATAMI_SCALE, TATAMI_SCALE, TATAMI_SCALE };

					for (auto& part : t->objectParts_) {
						if (!part.materialConfig)
							part.materialConfig = std::make_shared<MaterialConfig>();
						part.materialConfig->useModelTexture = false;
						part.materialConfig->textureHandle =
							DoujouTextures_[IDX_TATAMI];

					}

					tatamiTiles_.push_back(t);
				}

				// フレーム
				{
					Object* f = new Object();
					f->IntObject(system_);
					f->CreateDefaultData();
					f->CreateModelData(DoujouModels_[IDX_TATAMI_FRAME]);

					f->mainPosition.transform.translate = p;
					f->mainPosition.transform.scale =
					{ TATAMI_SCALE, TATAMI_SCALE, TATAMI_SCALE };

					for (auto& part : f->objectParts_) {
						if (!part.materialConfig)
							part.materialConfig = std::make_shared<MaterialConfig>();
						part.materialConfig->useModelTexture = false;
						part.materialConfig->textureHandle =
							DoujouTextures_[IDX_TATAMI_FRAME];

					}

					tatamiFrames_.push_back(f);
				}
			}
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
	{ 20.0f, 0.0f, 20.0f}, // 右奥
	{ 20.0f, 0.0f, 10.0f},
	{ 20.0f, 0.0f,  0.0f},

	{ 10.0f, 0.0f, 20.0f},
	{ 10.0f, 0.0f, 10.0f},
	{ 10.0f, 0.0f,  0.0f},

	{  0.0f, 0.0f, 20.0f},
	{  0.0f, 0.0f, 10.0f},
	{  0.0f, 0.0f,  0.0f}
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

	/// --- タイマー関連 ---
	countdownTimer_ = new CountdownTimer(system_);
	startTimer_ = new StartTimer(system_);
	startUpTimer = new Timer();
	startUpTimer->InitM(1.0f, system->GetTimeManager());

	// ■■■ 操作ガイドの初期化 ■■■
	// 画像を2種類読み込む (パスは実際のファイルに合わせてください)
	guideKeyHandle_ = system_->LoadTexture("resources/texture/Tutorial/instruction_keyboad.png");
	guideGamepadHandle_ = system_->LoadTexture("resources/texture/Tutorial/instruction_gamepad.png");

	// --- キーボード用スプライト ---
	guideKey_ = new SimpleSprite();
	guideKey_->IntObject(system_);
	guideKey_->CreateDefaultData();
	guideKey_->objectParts_[0].materialConfig->textureHandle = guideKeyHandle_;
	// 座標設定 (左下)
	// 画面左下に配置 (座標は画面サイズに合わせて調整してください)
	// 例: 画面高さが 720 で 画像高さが 100 なら Y=620 くらい
	// 左端なので X=10 くらい
	float winHeight = config::GetClientHeight();
	guideKey_->mainPosition.transform.translate = Vector3(30.0f, winHeight - 170.0f, 0.0f);

	// サイズ調整（必要なら）
	guideKey_->mainPosition.transform.scale = Vector3(0.15f, 0.15f, 1.0f);

	// --- ゲームパッド用スプライト ---
	guideGamepad_ = new SimpleSprite();
	guideGamepad_->IntObject(system_);
	guideGamepad_->CreateDefaultData();
	guideGamepad_->objectParts_[0].materialConfig->textureHandle = guideGamepadHandle_;
	// 座標設定 (キーボード用と全く同じ位置に重ねる)
	guideGamepad_->mainPosition.transform.translate = guideKey_->mainPosition.transform.translate;

	// サイズ調整（必要なら）

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
	for (int i = 0; i < kKanjiCount; i++) {
		delete KanjiObjs_[i];
		KanjiObjs_[i] = nullptr;
	}
	for (int i = 0; i < kDoujouPartCount; i++) {
		delete Doujous_[i];
		Doujous_[i] = nullptr;
	}
	for (int i = 0; i < kResultPartCount; i++) {
		delete Results_[i];
		Results_[i] = nullptr;
	}
	resultSets_.clear();
	DestroyResultShoji();
	delete dashWindEffect_;
	delete suctionEffect_;
	delete planeReductionEffectL_;
	delete planeReductionEffectR_;
	delete explosion_;

	if (guideKey_) delete guideKey_;
	if (guideGamepad_)  delete guideGamepad_;
	delete tutorial_;
}

void SceneStage01::Update() {

	// ===============================
	// Result中：ゲーム進行を完全停止
	// ===============================
	if (isResultGenerated_) {
		/// カメラ処理
		/*CameraPart();*/
		// ランクアニメを再実行（デバッグ用）
		if (system_->GetTriggerOn(DIK_L)) {

			// アニメを最初からやり直す
			rankAnimT_ = 0.0f;
			isRankAnimating_ = true;

			// ★ 開始・終了位置は「今の設定」をそのまま使う
			// 何も再計算しない（←重要）
		}

		// ■ Result演出とカメラだけは動かす
		UpdateResultShojiAnim();

		for (auto& set : resultSets_) {
			for (auto* obj : set.parts) {
				if (obj) obj->Update(usingCamera_);
			}
		}

		UpdateResultShoji(); // 距離や演出連動があるなら
		UpdateRankModels();
		UpdateSparkEffect2();

		// ★ それ以外は一切通さない
		return;
	}


	tutorial_->Update(usingCamera_);

	// 操作説明切り替え用
	UpdateOperationGuide();

	/// カメラ処理
	CameraPart();

	// チュートリアル中はゲームを止める場合
	if (tutorial_->IsActive()) {
		return;
	}

	startUpTimer->ToZero();

	if (startUpTimer->parameter_ == 0) {
		startTimer_->Start();
	}

	startTimer_->Update(usingCamera_);

	if (startTimer_->IsTimeUp()) {

		countdownTimer_->Start();
	} else {
		return;
	}

	countdownTimer_->Update(usingCamera_);
	countdownTimer_->IsTimeUp();

	// ゲーム開始時やイベント発生時に呼び出し
	//if (system_->GetTriggerOn(DIK_T)) {
	//	tutorial_->Open(0); // 最初から再生
	//}

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


	/// Skydome更新
	skydome_->Update(usingCamera_);

	/// player更新
	player_->Update(usingCamera_);

	sprite_->Update(usingCamera_);
	//sprite2_->Update(usingCamera_);

	int pCount = 0;
	// リスト内のエネミーを全て更新
	for (auto* enemy : enemies_) {
		bool isTargetCharging = false;
		Vector3 difference = player_->mainPosition.transform.translate - enemy->mainPosition.transform.translate;
		float distance = sqrtf(difference.x * difference.x + difference.y * difference.y + difference.z * difference.z);
		enemy->SetDistanceWithPlayer(distance); // パーティクル制御のためのプレイヤーとの距離
		float nearDistance = distance;
		Vector3 nearPos = player_->mainPosition.transform.translate;
		isTargetCharging = player_->GetIsCharging();
		if (distance >= 25.0f || pCount >= 2) { // 自機が一定範囲外なら他の敵を参照||プレイヤーをターゲットにしている数が2以上なら敵をターゲット
			for (auto* other : enemies_) {
				if (other == enemy) continue; // 自身は除外

				// 差分を計算
				difference = other->mainPosition.transform.translate - enemy->mainPosition.transform.translate;
				// 距離を計算
				distance = sqrtf(difference.x * difference.x + difference.y * difference.y + difference.z * difference.z);

				if (distance < nearDistance) {
					nearDistance = distance; // 一番近い距離の変更
					nearPos = other->mainPosition.transform.translate; // 一番近い敵の位置の変更
					isTargetCharging = other->GetIsCharging();
				}
			}
		}

		if (nearPos == player_->mainPosition.transform.translate) {
			pCount++;
		}

		enemy->SetTargetPos(nearPos);
		enemy->SetTargetCharge(isTargetCharging);
		enemy->Update(usingCamera_);
	}

	/// 衝突判定処理

	// 攻撃当たり判定処理
	CheckAttackCollision();
	// ■■■ 壁との当たり判定（共通関数化） ■■■

	// 引数: 対象のオブジェクト (Player または Enemy)
	auto CheckWallCollision = [&](Object* targetObj) {

		// 対象の半径（半幅）
		float radiusX = targetObj->mainPosition.transform.scale.x / 2.0f;
		float radiusZ = targetObj->mainPosition.transform.scale.z / 2.0f;
		Vector3 targetPos = targetObj->mainPosition.transform.translate;

		// 1. 壁との衝突判定ループ
		for (int i = 0; i < 4; i++) {
			wall_[i]->Update(usingCamera_);
			Vector3 wPos = wall_[i]->mainPosition.transform.translate;
			Vector3 wScale = wall_[i]->mainPosition.transform.scale;
			float wRadiusX = wScale.x / 2.0f;
			float wRadiusZ = wScale.z / 2.0f;

			// AABB判定
			bool isHitX = (targetPos.x >= wPos.x - wRadiusX - radiusX) && (targetPos.x <= wPos.x + wRadiusX + radiusX);
			bool isHitZ = (targetPos.z >= wPos.z - wRadiusZ - radiusZ) && (targetPos.z <= wPos.z + wRadiusZ + radiusZ);

			if (isHitX && isHitZ) {
				float diffX = targetPos.x - wPos.x;
				float diffZ = targetPos.z - wPos.z;
				float overlapX = (wRadiusX + radiusX) - std::abs(diffX);
				float overlapZ = (wRadiusZ + radiusZ) - std::abs(diffZ);

				// 速度リセット用のベクトル
				Vector3 hitNormal = { 0,0,0 };

				// 座標補正
				if (overlapX < overlapZ) {
					if (diffX < 0) {
						targetObj->mainPosition.transform.translate.x = (wPos.x - wRadiusX - radiusX) - 0.001f;
						hitNormal = { 1.0f, 0.0f, 0.0f };
					} else {
						targetObj->mainPosition.transform.translate.x = (wPos.x + wRadiusX + radiusX) + 0.001f;
						hitNormal = { -1.0f, 0.0f, 0.0f };
					}
				} else {
					if (diffZ < 0) {
						targetObj->mainPosition.transform.translate.z = (wPos.z - wRadiusZ - radiusZ) - 0.001f;
						hitNormal = { 0.0f, 0.0f, 1.0f };
					} else {
						targetObj->mainPosition.transform.translate.z = (wPos.z + wRadiusZ + radiusZ) + 0.001f;
						hitNormal = { 0.0f, 0.0f, -1.0f };
					}
				}

				// OnWallHitの呼び出し（PlayerかEnemyか判定して呼ぶ）
				if (targetObj == player_) {
					player_->OnWallHit(hitNormal);
				} else {
					Enemy* e = dynamic_cast<Enemy*>(targetObj);
					if (e) e->OnWallHit(hitNormal);
				}

				// 座標更新されたのでローカル変数も更新
				targetPos = targetObj->mainPosition.transform.translate;
			}
		}

		// 2. 部屋の外に出た場合の保険（Clamp処理）
		// ※壁の位置が変わらないなら定数でOKですが、壁の厚みなどを考慮した汎用的な値にするとベターです
		float limitX = 99.0f - radiusX;
		float limitZ = 99.0f - radiusZ;
		float minLimit = 1.0f; // 下限

		if (targetObj->mainPosition.transform.translate.x > limitX) {
			targetObj->mainPosition.transform.translate.x = limitX;
		}
		if (targetObj->mainPosition.transform.translate.x < minLimit) {
			targetObj->mainPosition.transform.translate.x = minLimit;
		}
		if (targetObj->mainPosition.transform.translate.z > limitZ) {
			targetObj->mainPosition.transform.translate.z = limitZ;
		}

		if (targetObj->mainPosition.transform.translate.z < minLimit) {
			targetObj->mainPosition.transform.translate.z = minLimit;
		}
		};

	// ■■■ 実行部分 ■■■

	// 1. プレイヤーの判定
	CheckWallCollision(player_);

	// 2. 全エネミーの判定
	for (auto* enemy : enemies_) {
		CheckWallCollision(enemy);
	}

#pragma region 自機用パーティクル
	Vector3 forward = player_->GetForward();

	dashWindEffect_->SetGenerationPos(player_->GetArmPositionL() + forward + Vector3(0.0f, player_->GetArmScaleL() - 0.5f, 0.0f));
	dashWindEffect_->SetGenerationPos(player_->GetArmPositionL() + (forward * player_->GetArmScaleL() * 2.0f));
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

	if (system_->GetTriggerOn(DIK_P)) {
		ArrangeByArmSize();

		StartResultShoji();
		SpawnResultSets(10, -80.0f);

		isResultGenerated_ = true; // ★ここ

	}

	if (!isResultGenerated_ && countdownTimer_->IsTimeUp()) {

		ArrangeByArmSize();
		StartResultShoji();
		SpawnResultSets(10, -80.0f);

		isResultGenerated_ = true; // ★ 一度だけ
	}

	for (auto& set : resultSets_) {
		for (auto* obj : set.parts) {
			if (obj) obj->Update(usingCamera_);
		}
	}

	for (int i = 0; i < kIllustrationPartCount; i++) {
		if (Illustrations[i]) {
			Illustrations[i]->Update(usingCamera_);
		}
	}
	if (isResultGenerated_) {

		UpdateResultShoji();
	}

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
	// =====================
	// 畳（床）距離カリング描画

	constexpr float TATAMI_DRAW_DIST = 150.0f;
	constexpr float TATAMI_DRAW_DIST_SQ =
		TATAMI_DRAW_DIST * TATAMI_DRAW_DIST;


	Vector3 camPos = usingCamera_->GetPosition();

	// 畳
	for (auto* t : tatamiTiles_) {
		if (!t) continue;

		Vector3 pos = t->mainPosition.transform.translate;

		Vector3 d = pos - camPos;
		float distSq = d.x * d.x + d.y * d.y + d.z * d.z;

		if (distSq > TATAMI_DRAW_DIST_SQ)
			continue;   // ★ 遠いので描画しない

		t->Draw();
	}

	// フレーム
	for (auto* f : tatamiFrames_) {
		if (!f) continue;

		Vector3 pos = f->mainPosition.transform.translate;

		Vector3 d = pos - camPos;
		float distSq = d.x * d.x + d.y * d.y + d.z * d.z;

		if (distSq > TATAMI_DRAW_DIST_SQ)
			continue;

		f->Draw();
	}



	// 背景オブジェクト
	for (int i = 0; i < kDoujouPartCount - 2; i++) {

		if (!Doujous_[i]) continue;

		/// 向きがないのか判定の対象外

		if (Length(Doujous_[i]->objectParts_[0].forward) < 0.0001f) {
			Doujous_[i]->Draw();
			continue;
		}

		Vector3 mainPoint(49.9f, 18.05f, 48.15f);
		Vector3 changePoint{ 0,mainPoint.y,0 };
		/// back
		if (i == 4 || i == 9) { changePoint.x = mainPoint.x; }
		/// front
		if (i <= 2 || i == 5 || i == 10) { changePoint.z = mainPoint.z * 2; }
		/// right
		if (i == 6 || i == 11) { changePoint.x = mainPoint.x * 2; }
		/// leftは要らない

		Doujous_[i]->objectParts_[0].transform.translate = Doujous_[i]->objectParts_[0].transform.translate + changePoint;
		Matrix4x4 worldMatrix = Doujous_[i]->objectParts_[0].UpdateWorldMatrix();
		Doujous_[i]->objectParts_[0].transform.translate = Doujous_[i]->objectParts_[0].transform.translate - changePoint;

		Vector3 worldForward = TransformDirection(Doujous_[i]->objectParts_[0].forward, worldMatrix);
		Vector3 blockWorldPos = { worldMatrix.m[3][0], worldMatrix.m[3][1], worldMatrix.m[3][2] };
		if (usingCamera_->isObjectFaceCamera(worldForward, blockWorldPos, 90.0f)) {
			Doujous_[i]->Draw();
		}
	}
	DrawSparkEffect2(); // ← Spark専用Draw


	//if (!isCameraLocked_) {
	//	Doujous_[4]->Draw();
	//	Doujous_[9]->Draw();
	//}

	constexpr float RESULT_DRAW_DIST = 300.0f;
	constexpr float RESULT_DRAW_DIST_SQ =
		RESULT_DRAW_DIST * RESULT_DRAW_DIST;

	Vector3 resultSets_camPos = usingCamera_->GetPosition();
	for (auto& set : resultSets_) {
		// ★ セット代表位置で距離判定
		Vector3 base = set.basePos[0];
		Vector3 d = base - resultSets_camPos + Vector3(0, 0, -200);
		float distSq = d.x * d.x + d.y * d.y + d.z * d.z;

		if (distSq > RESULT_DRAW_DIST_SQ)
			continue;   // ★ このセット丸ごと描画しない

		// 近いセットだけ描画
		for (auto* obj : set.parts) {
			if (obj) obj->Draw();
		}
	}


	// ===== Result Shoji (演出用) =====
	if (resultShojiL_)        resultShojiL_->Draw();
	if (resultShojiR_)        resultShojiR_->Draw();
	if (resultShojiLPaper_)  resultShojiLPaper_->Draw();
	if (resultShojiRPaper_)  resultShojiRPaper_->Draw();

	// ★順位表示（プレイヤーだけ）
	if (playerRank_ >= 0 && playerRank_ < kKanjiCount) {
		if (KanjiObjs_[playerRank_]) {
			KanjiObjs_[playerRank_]->Draw();
		}
	}
	// ★順位フレーム（常に描画）
	if (KanjiObjs_[kFrameOuterIndex]) {
		KanjiObjs_[kFrameOuterIndex]->Draw();
	}
	if (KanjiObjs_[kFrameInnerIndex]) {
		KanjiObjs_[kFrameInnerIndex]->Draw();
	}
	for (int i = 0; i < kIllustrationPartCount; i++) {
		if (Illustrations[i]) {
			Illustrations[i]->Draw();
		}
	}

	// 操作説明
	// ■■■ ガイド描画切り替え ■■■
	if (isGamepadMode_) {
		// パッドモードならパッド用画像だけ描画
		if (guideGamepad_) guideGamepad_->Draw();
	} else {
		// それ以外ならキーボード用画像だけ描画
		if (guideKey_) guideKey_->Draw();
	}

	// チュートリアルお試し
	tutorial_->Draw(); // 最前面に描画

	if (tutorial_->IsActive()) {
		return;
	} else {
		startTimer_->Draw();
	}

	if (startTimer_->IsTimeUp()) countdownTimer_->Draw();


#ifdef USE_IMGUI
	/// imgui処理
	ImguiPart();
#endif
}

void SceneStage01::CameraPart() {
	//if (isResultShojiActive_) {
	//	system_->SetCamera(usingCamera_);
	//	return;
	//}

	// ★ カメラ追従停止中
	if (isCameraLocked_) {
		if (useDebugCamera) {
			usingCamera_ = debugCamera_;
			debugCamera_->MouseControlUpdate();
		}
		// 位置・回転を一切更新しない
		system_->SetCamera(usingCamera_);
		return;



	}

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

		ImGui::Begin("Rank Debug");

		ImGui::Checkbox("Use Rank Debug", &useRankDebug_);

		ImGui::Separator();
		ImGui::Text("Position");
		ImGui::DragFloat3("Pos", &rankDebugPos_.x, 0.1f);

		ImGui::Separator();
		ImGui::Text("Rotation (rad)");
		ImGui::DragFloat3("Rot", &rankDebugRot_.x, 0.01f);

		ImGui::Separator();
		ImGui::Text("Scale");
		ImGui::DragFloat3("Scale", &rankDebugScale_.x, 0.01f, 0.01f, 10.0f);

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
					// ▼▼▼ 演出追加 ▼▼▼
					// 0.05倍速(ほぼ停止) にして、0.1秒間(実時間) 止める
					// これにより「ガッ！」という重いヒット感が生まれます
					StartHitStop(0.05f, duration);

					player_->AddKnockback(knockbackForce);
					explosion_->SetIsEmitting(true);
					explosion_->SetPos((player_->mainPosition.transform.translate + armPos) / 2.0f);
				} else {
					// targetObjが敵リストの中にいるか確認、あるいはキャストして適用
					// ここでは簡易的に Enemy* へのキャストで対応
					Enemy* e = dynamic_cast<Enemy*>(targetObj);
					if (e) {
						e->AddKnockback(knockbackForce);
						explosion_->SetIsEmitting(true);
						explosion_->SetPos(player_->mainPosition.transform.translate);
						explosion_->SetPos((e->mainPosition.transform.translate + armPos) / 2.0f);
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

						// スロー演出（プレイヤーが関与している場合のみ推奨）
						if (isCheckInPlayer) StartHitStop(0.05f, duration);

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

						if (isCheckInPlayer) StartHitStop(0.05f, duration);

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
							StartHitStop(0.05f, duration);
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

	// ▼▼▼▼▼▼▼▼▼ 【追加】敵同士の総当たり判定 ▼▼▼▼▼▼▼▼▼

	size_t enemyCount = enemies_.size();
	for (size_t i = 0; i < enemyCount; ++i) {
		// j は i + 1 から始めることで、(A, B) を判定したら (B, A) は判定しないようにする（重複防止）
		for (size_t j = i + 1; j < enemyCount; ++j) {

			Enemy* enemyA = enemies_[i];
			Enemy* enemyB = enemies_[j];

			// --- Aの情報取得 ---
			AABB bodyAABB_A = enemyA->GetAABB(enemyA);
			float bodyScaleA = enemyA->mainPosition.transform.scale.x;
			ArmInfo armsA[2] = {
				{ false, enemyA->GetArmPositionL(), enemyA->GetArmScaleL(), enemyA->IsAttackingL() },
				{ true,  enemyA->GetArmPositionR(), enemyA->GetArmScaleR(), enemyA->IsAttackingR() }
			};

			// --- Bの情報取得 ---
			AABB bodyAABB_B = enemyB->GetAABB(enemyB);
			float bodyScaleB = enemyB->mainPosition.transform.scale.x;
			ArmInfo armsB[2] = {
				{ false, enemyB->GetArmPositionL(), enemyB->GetArmScaleL(), enemyB->IsAttackingL() },
				{ true,  enemyB->GetArmPositionR(), enemyB->GetArmScaleR(), enemyB->IsAttackingR() }
			};

			// --- 判定実行 ---

			// 左右の腕についてループ
			for (int aIdx = 0; aIdx < 2; aIdx++) {

				// 1. Aの攻撃 vs Bの体 (AがBを殴った)
				if (armsA[aIdx].isAtk) {
					CheckArmCollision(enemyB, bodyAABB_B, armsA[aIdx].pos, armsA[aIdx].scale, bodyScaleA);
				}

				// 2. Bの攻撃 vs Aの体 (BがAを殴った)
				// ※ループの構造上、ここで逆方向もチェックする必要があります
				// (armsBのループを別途回すより、ここでまとめてチェックした方が効率的です)
				// ただし、腕インデックスを合わせる必要はないので、このループ内でBの左右両方を確認します。
			}

			// B側の攻撃チェック（Aの体に対して）
			for (int bIdx = 0; bIdx < 2; bIdx++) {
				if (armsB[bIdx].isAtk) {
					CheckArmCollision(enemyA, bodyAABB_A, armsB[bIdx].pos, armsB[bIdx].scale, bodyScaleB);
				}
			}

			// 3. 腕同士の衝突 (Aの左右 × Bの左右)
			for (int aIdx = 0; aIdx < 2; aIdx++) {
				for (int bIdx = 0; bIdx < 2; bIdx++) {
					CheckArmToArm(
						enemyA, armsA[aIdx].pos, armsA[aIdx].scale, bodyScaleA, armsA[aIdx].isRight, armsA[aIdx].isAtk,
						enemyB, armsB[bIdx].pos, armsB[bIdx].scale, bodyScaleB, armsB[bIdx].isRight, armsB[bIdx].isAtk
					);
				}
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
void SceneStage01::ArrangeByArmSize() {
	// 順位ごとの固定配置（1位, 2位, 3位, ...）
	static const std::vector<Vector3> rankPositions = {
		{ 49.0f, 0.0f, 60.0f }, // 1位
		{ 49.0f, 0.0f, -40.0f }, // 2位
		{ 49.0f, 0.0f, -120.0f}, // 3位
		{ 49.0f, 0.0f,  -200.5f }, // 4位
		{ 49.0f, 0.0f,  -280.0f}, // 5位
		{ 49.0f, 0.0f, -360.5f }, // 6位
		{ 49.0f, 0.0f, -440.0f }, // 7位
		{ 49.0f, 0.0f, -520.0f }, // 8位
		{ 49.0f, 0.0f, -600.0f } , // 9位
		{ 49.0f, 0.0f, -680.0f }  // 10位
	};



	struct Entry {
		Object* obj;
		float armSize;
	};

	std::vector<Entry> list;

	// --- プレイヤー ---
	{
		float pMaxArm = std::max(
			player_->GetArmScaleL(),
			player_->GetArmScaleR()
		);
		list.push_back({ player_, pMaxArm });
	}

	// --- エネミー ---
	for (auto* enemy : enemies_) {
		float eMaxArm = std::max(
			enemy->GetArmScaleL(),
			enemy->GetArmScaleR()
		);
		list.push_back({ enemy, eMaxArm });
	}

	if (list.empty()) return;

	// --- 腕サイズで降順ソート ---
	std::sort(list.begin(), list.end(),
		[](const Entry& a, const Entry& b) {
			return a.armSize > b.armSize;
		});

	// ★ 並べ始める「固定の場所」
	Vector3 basePos = arrangeOrigin_;

	// 間隔
	const float spacing = 2.5f;

	// --- 配置 ---
	for (int i = 0; i < static_cast<int>(list.size()); ++i) {
		Object* obj = list[i].obj;

		// ★ プレイヤー順位を記録
		if (obj == player_) {
			playerRank_ = i;
		}

		if (i < static_cast<int>(rankPositions.size())) {
			obj->mainPosition.transform.translate = rankPositions[i];
		} else {
			obj->mainPosition.transform.translate = {
				rankPositions.back().x,
				rankPositions.back().y,
				rankPositions.back().z - 2.5f * (i - rankPositions.size() + 1)
			};
		}

		obj->mainPosition.transform.rotate.y = std::numbers::pi_v<float>;
	}


	// ★ 並べた瞬間、カメラ追従を切る
	isCameraLocked_ = true;
}
void SceneStage01::SpawnResultSets(int copyCount, float spacingZ) {
	if (resultSets_.empty()) return;

	const ResultSet base = resultSets_[0];
	resultSets_.clear();   // ← 毎回作り直す（重要）

	for (int c = 0; c < copyCount; ++c) { // ★ 0 から
		ResultSet set;

		for (size_t i = 0; i < base.parts.size(); ++i) {
			Object* src = base.parts[i];

			Object* obj = new Object;
			obj->IntObject(system_);
			obj->CreateDefaultData();
			obj->CreateModelData(src->modelHandle_);

			obj->objectParts_ = src->objectParts_; // マテリアル共有

			Vector3 pos = base.basePos[i];
			pos.z += spacingZ * c;   // ★ base もずらす

			obj->mainPosition.transform.translate = pos;
			obj->mainPosition.transform.rotate = src->mainPosition.transform.rotate;
			obj->mainPosition.transform.scale = src->mainPosition.transform.scale;

			obj->Update(usingCamera_);

			set.parts.push_back(obj);
			set.basePos.push_back(pos);
		}

		resultSets_.push_back(set);
	}
}
void SceneStage01::OpenShoji(ResultSet& set) {
	Object* left = set.parts[IDX_SYOUJI_L];
	Object* right = set.parts[IDX_SYOUJI_R];

	// 横にずらすだけ（即時）
	left->mainPosition.transform.translate.x -= 30.0f;
	right->mainPosition.transform.translate.x += 30.0f;
}
void SceneStage01::UpdateResultShoji() {

	Vector3 camPos = usingCamera_->GetPosition();

	constexpr float OPEN_DIST = 100.0f;
	constexpr float OFFSET = 10.0f;

	for (auto& set : resultSets_) {

		Vector3 base = set.basePos[0];
		float dist = std::abs(camPos.z - base.z);

		// ============================
		// ■ 開く判定（前進中）
		// ============================
		if (resultShojiPhase_ == ResultShojiPhase::CameraMove) {

			if (!set.shojiOpened && !set.shojiOpening) {
				if (dist < OPEN_DIST) {
					set.shojiOpening = true;
					set.shojiT = 0.0f;
				}
			}
		}

		// ============================
		// ■ 閉じ判定（戻り中）
		// ============================
		if (resultShojiPhase_ == ResultShojiPhase::CameraToPlayer) {

			if (set.shojiOpened && !set.shojiClosing) {

				if (dist < OPEN_DIST) {
					set.shojiCloseWait += system_->GetDeltaTime();

					if (set.shojiCloseWait >= CLOSE_WAIT_TIME) {
						set.shojiClosing = true;
						set.shojiCloseT = 0.0f;
					}
				} else {
					// 離れたらリセット
					set.shojiCloseWait = 0.0f;
				}
			}
		}

		// ============================
		// ■ 開くアニメ
		// ============================
		if (set.shojiOpening) {

			set.shojiT += system_->GetDeltaTime() / 0.6f;
			float t = std::clamp(set.shojiT, 0.0f, 1.0f);
			float e = 1.0f - (1.0f - t) * (1.0f - t);

			float lx = set.basePos[IDX_SYOUJI_L].x - OFFSET * e;
			float rx = set.basePos[IDX_SYOUJI_R].x + OFFSET * e;

			set.parts[IDX_SYOUJI_L]->mainPosition.transform.translate.x = lx;
			set.parts[IDX_PAPER_L]->mainPosition.transform.translate.x = lx;
			set.parts[IDX_SYOUJI_R]->mainPosition.transform.translate.x = rx;
			set.parts[IDX_PAPER_R]->mainPosition.transform.translate.x = rx;

			if (t >= 1.0f) {
				set.shojiOpening = false;
				set.shojiOpened = true;
			}
		}

		// ============================
		// ■ 閉じるアニメ
		// ============================
		if (set.shojiClosing) {

			set.shojiCloseT += system_->GetDeltaTime() / CLOSE_TIME;
			float t = std::clamp(set.shojiCloseT, 0.0f, 1.0f);
			float e = t * t; // EaseIn

			float lx = set.basePos[IDX_SYOUJI_L].x - OFFSET * (1.0f - e);
			float rx = set.basePos[IDX_SYOUJI_R].x + OFFSET * (1.0f - e);

			set.parts[IDX_SYOUJI_L]->mainPosition.transform.translate.x = lx;
			set.parts[IDX_PAPER_L]->mainPosition.transform.translate.x = lx;
			set.parts[IDX_SYOUJI_R]->mainPosition.transform.translate.x = rx;
			set.parts[IDX_PAPER_R]->mainPosition.transform.translate.x = rx;

			if (t >= 1.0f) {
				set.shojiClosing = false;
				set.shojiOpened = false;
				set.shojiCloseWait = 0.0f;
			}
		}
	}
}


//==============================
// 起動（1回だけ呼ぶ）
//==============================
//==============================
// 起動（1回だけ呼ぶ）
//==============================
void SceneStage01::StartResultShoji() {

	if (resultShojiPhase_ != ResultShojiPhase::None) return;
	isResultShojiActive_ = true;   // ★追加
	Camera* cam = usingCamera_;
	Vector3 camRot = cam->GetTransform().rotate;
	Vector3 camPos = cam->GetPosition();

	Vector3 base =
		SceneManager::CalcBaseFromCamera(
			camPos,
			camRot,
			Vector3(0.0f, 0.0f, 4.0f)
		);

	CreateResultShoji(camRot);
	SetResultShojiOpenPose(base, camRot);

	resultShojiT_ = 0.0f;
	resultShojiPhase_ = ResultShojiPhase::Closing;
}

//==============================
// 毎フレーム更新
//==============================
void SceneStage01::UpdateResultShojiAnim() {


	if (resultShojiPhase_ == ResultShojiPhase::None) return;

	float dt = system_->GetDeltaTime();
	if (dt > 0.1f) dt = 0.1f;

	// =============================
	// フェーズ制御
	// =============================
	if (resultShojiPhase_ == ResultShojiPhase::Closing) {

		resultShojiT_ += dt;
		if (resultShojiT_ >= 1.0f) {
			resultShojiT_ = 1.0f;
			resultShojiWaitTimer_ = 0.0f;
			resultShojiPhase_ = ResultShojiPhase::Wait;
		}
	} else if (resultShojiPhase_ == ResultShojiPhase::Wait) {

		resultShojiWaitTimer_ += dt;
		if (resultShojiWaitTimer_ >= 1.0f) {

			// カメラを演出開始位置へ
			Transform t = camera_->GetTransform();
			t.translate = Vector3(49.5f, 5.0f, -800.0f);
			t.rotate = Vector3(0.0f, 0.0f, 0.0f);
			camera_->SetCamera(t);
			debugCamera_->SetCamera(t);

			// ★★★ ここが決定打 ★★★
			Vector3 openRot(0.0f, 0.0f, 0.0f);

			resultShojiL_->mainPosition.transform.rotate = openRot;
			resultShojiR_->mainPosition.transform.rotate = openRot;
			resultShojiLPaper_->mainPosition.transform.rotate = openRot;
			resultShojiRPaper_->mainPosition.transform.rotate = openRot;

			resultShojiPhase_ = ResultShojiPhase::Opening;
		}
	}

	else if (resultShojiPhase_ == ResultShojiPhase::Opening) {

		resultShojiT_ -= dt;
		if (resultShojiT_ <= 0.0f) {
			resultShojiT_ = 0.0f;

			Transform camT = camera_->GetTransform();
			resultCameraStart_ = camT.translate;
			resultCameraEnd_ = Vector3(camT.translate.x, camT.translate.y, 27.0f);

			resultCameraT_ = 0.0f;
			resultShojiPhase_ = ResultShojiPhase::CameraMove;
		}
	} else if (resultShojiPhase_ == ResultShojiPhase::CameraMove) {


		resultCameraT_ += dt / 12.0f;   // 1秒

		float t = std::clamp(resultCameraT_, 0.0f, 1.0f);
		float ease = SceneManager::EaseOutSine(t);

		Vector3 pos;
		pos.x = resultCameraStart_.x + (resultCameraEnd_.x - resultCameraStart_.x) * ease;
		pos.y = resultCameraStart_.y + (resultCameraEnd_.y - resultCameraStart_.y) * ease;
		pos.z = resultCameraStart_.z + (resultCameraEnd_.z - resultCameraStart_.z) * ease;

		Transform camT = camera_->GetTransform();
		camT.translate = pos;
		camT.rotate = Vector3(0.0f, 0.0f, 0.0f);
		camera_->SetCamera(camT);
		debugCamera_->SetCamera(camT);

		if (t >= 1.0f) {

			// ★ 次のカメラ演出の準備
			Transform camT = camera_->GetTransform();
			resultCameraPlayerStart_ = camT.translate;

			// プレイヤーの Z - 20
			Vector3 playerPos = player_->mainPosition.transform.translate;
			resultCameraPlayerEnd_ = {
				playerPos.x,
				playerPos.y + 5,
				playerPos.z - 30.0f
			};

			resultCameraPlayerT_ = 0.0f;

			resultShojiPhase_ = ResultShojiPhase::CameraToPlayer;
			return;
		}

	} else if (resultShojiPhase_ == ResultShojiPhase::CameraToPlayer) {

		resultCameraPlayerT_ += dt / 1.2f;   // ← 好きな秒数
		float t = std::clamp(resultCameraPlayerT_, 0.0f, 1.0f);
		float ease = SceneManager::EaseOutSine(t);

		Vector3 pos;
		pos.x = resultCameraPlayerStart_.x +
			(resultCameraPlayerEnd_.x - resultCameraPlayerStart_.x) * ease;
		pos.y = resultCameraPlayerStart_.y +
			(resultCameraPlayerEnd_.y - resultCameraPlayerStart_.y) * ease;
		pos.z = resultCameraPlayerStart_.z +
			(resultCameraPlayerEnd_.z - resultCameraPlayerStart_.z) * ease;

		Transform camT = camera_->GetTransform();
		camT.translate = pos;
		camT.rotate = Vector3(0.0f, 0.0f, 0.0f);
		camera_->SetCamera(camT);
		debugCamera_->SetCamera(camT);
		if (t >= 1.0f) {

			DestroyResultShoji();

			// ★ ここでは「準備」だけ
			Vector3 camPos = camera_->GetTransform().translate;

			rankAnimStart_ = camPos + Vector3(0.0f, 0.0f, -3.0f);
			rankAnimEnd_ = camPos + Vector3(0.0f, 0.0f, 3.0f);

			rankAnimT_ = 0.0f;

			// ★ 待ち開始
			resultShojiWaitTimer_ = 0.0f;
			resultShojiPhase_ = ResultShojiPhase::RankAppear;
			return;
		}


	} else if (resultShojiPhase_ == ResultShojiPhase::RankAppear) {



		resultShojiWaitTimer_ += dt;
		if (resultShojiWaitTimer_ >= 0.4f) {

			// ★ ここが必須
			rankDebugPos_ = rankAnimStart_;
			rankDebugRot_ = Vector3(0.0f, 0.0f, 0.0f);

			rankAnimT_ = 0.0f;
			isRankAnimating_ = true;
			isFrameAnimating_ = true;
			frameAnimT_ = 0.0f;

			isRankAnimating_ = false;   // ★フレームが終わるまで文字は止める
			rankAnimT_ = 0.0f;          // 文字側は後で使う

			resultShojiPhase_ = ResultShojiPhase::None;
		}
	}




	// ★ 念のため：ここに来る直前に消えてても触らない（保険）
	if (!resultShojiL_ || !resultShojiR_
		|| !resultShojiLPaper_ || !resultShojiRPaper_) {
		return;
	}

	// =============================
	// 共通：位置計算
	// =============================
	Camera* cam = usingCamera_;
	Vector3 camRot = cam->GetTransform().rotate;
	Vector3 camPos = cam->GetPosition();

	Vector3 base = SceneManager::CalcBaseFromCamera(
		camPos, camRot, Vector3(0.0f, 0.0f, 4.0f)
	);

	Vector3 forward = SceneManager::CalcForward(camRot);
	Vector3 right = SceneManager::CalcRight(forward);

	float e = SceneManager::EaseOutSine(resultShojiT_);
	float off = (1.0f - e) * 10.2f;

	resultShojiL_->mainPosition.transform.translate.x = base.x - right.x * off;
	resultShojiL_->mainPosition.transform.translate.y = base.y;
	resultShojiL_->mainPosition.transform.translate.z = base.z - right.z * off;

	resultShojiR_->mainPosition.transform.translate.x = base.x + right.x * off;
	resultShojiR_->mainPosition.transform.translate.y = base.y;
	resultShojiR_->mainPosition.transform.translate.z = base.z + right.z * off;

	resultShojiLPaper_->mainPosition.transform.translate.x = base.x - right.x * off;
	resultShojiLPaper_->mainPosition.transform.translate.y = base.y;
	resultShojiLPaper_->mainPosition.transform.translate.z = base.z - right.z * off;

	resultShojiRPaper_->mainPosition.transform.translate.x = base.x + right.x * off;
	resultShojiRPaper_->mainPosition.transform.translate.y = base.y;
	resultShojiRPaper_->mainPosition.transform.translate.z = base.z + right.z * off;

	resultShojiL_->Update(usingCamera_);
	resultShojiR_->Update(usingCamera_);
	resultShojiLPaper_->Update(usingCamera_);
	resultShojiRPaper_->Update(usingCamera_);
}



Camera* SceneStage01::GetCamera() {
	return usingCamera_;   // ★ 絶対に camera_ じゃない
}

//==============================
// 初期配置（開いた状態）
//==============================
void SceneStage01::SetResultShojiOpenPose(
	const Vector3& base,
	const Vector3& camRot
) {
	Vector3 right =
		SceneManager::CalcRight(
			SceneManager::CalcForward(camRot)
		);

	float off = 10.2f;

	// 左（木）
	resultShojiL_->mainPosition.transform.translate.x = base.x - right.x * off;
	resultShojiL_->mainPosition.transform.translate.y = base.y;
	resultShojiL_->mainPosition.transform.translate.z = base.z - right.z * off;

	// 右（木）
	resultShojiR_->mainPosition.transform.translate.x = base.x + right.x * off;
	resultShojiR_->mainPosition.transform.translate.y = base.y;
	resultShojiR_->mainPosition.transform.translate.z = base.z + right.z * off;

	// 左（紙）
	resultShojiLPaper_->mainPosition.transform.translate.x = base.x - right.x * off;
	resultShojiLPaper_->mainPosition.transform.translate.y = base.y;
	resultShojiLPaper_->mainPosition.transform.translate.z = base.z - right.z * off;

	// 右（紙）
	resultShojiRPaper_->mainPosition.transform.translate.x = base.x + right.x * off;
	resultShojiRPaper_->mainPosition.transform.translate.y = base.y;
	resultShojiRPaper_->mainPosition.transform.translate.z = base.z + right.z * off;
}
//==============================
// リザルト障子生成
//==============================
void SceneStage01::CreateResultShoji(const Vector3& camRot) {
	resultShojiL_ = new Object;
	resultShojiR_ = new Object;
	resultShojiLPaper_ = new Object;
	resultShojiRPaper_ = new Object;

	resultShojiL_->IntObject(system_);
	resultShojiR_->IntObject(system_);
	resultShojiLPaper_->IntObject(system_);
	resultShojiRPaper_->IntObject(system_);

	resultShojiL_->CreateModelData(
		system_->SetModelObj("resources/gate/model/shoji_left.obj"));
	resultShojiR_->CreateModelData(
		system_->SetModelObj("resources/gate/model/shoji_right.obj"));
	resultShojiLPaper_->CreateModelData(
		system_->SetModelObj("resources/gate/model/shoji_left_paper.obj"));
	resultShojiRPaper_->CreateModelData(
		system_->SetModelObj("resources/gate/model/shoji_right_paper.obj"));

	// 木枠
	for (Object* o : { resultShojiL_, resultShojiR_ }) {
		for (auto& p : o->objectParts_) {
			if (!p.materialConfig) {
				p.materialConfig = std::make_shared<MaterialConfig>();
			}
			p.materialConfig->useModelTexture = false;
			p.materialConfig->textureHandle =
				system_->LoadTexture("resources/gate/model/wood_tate.png");
			p.materialConfig->enableLighting = true;
		}
		o->mainPosition.transform.scale = { 0.025f,0.025f,0.025f };
		o->mainPosition.transform.rotate = camRot;
	}

	// 紙
	for (Object* o : { resultShojiLPaper_, resultShojiRPaper_ }) {
		for (auto& p : o->objectParts_) {
			if (!p.materialConfig) {
				p.materialConfig = std::make_shared<MaterialConfig>();
			}
			p.materialConfig->useModelTexture = false;
			p.materialConfig->textureHandle =
				system_->LoadTexture("resources/gate/model/paper.png");
			p.materialConfig->enableLighting = false;
		}
		o->mainPosition.transform.scale = { 0.025f,0.025f,0.025f };
		o->mainPosition.transform.rotate = camRot;
	}
}
void SceneStage01::UpdateRankModels() {
	if (playerRank_ < 0 || playerRank_ >= kKanjiCount) return;

	Object* rankObj = KanjiObjs_[playerRank_];
	if (!rankObj) return;

	// ============================
	// ② フレームを「完全追従」(←見出しは維持)
	// ============================
	Object* frameOuter = KanjiObjs_[kFrameOuterIndex];
	Object* frameInner = KanjiObjs_[kFrameInnerIndex];
	if (!frameOuter || !frameInner) return;

	// ★ フレームが先に easing（開始・終了は文字と同じ）
	if (isFrameAnimating_) {
		frameAnimT_ += system_->GetDeltaTime() / 0.6f;
		float ft = std::clamp(frameAnimT_, 0.0f, 1.0f);
		float fe = SceneManager::EaseOutSine(ft);

		// 位置：あなたの既存ロジックと同じ「start→end」を使う
		Vector3 fpos = rankDebugPos_; // rankDebugPos_ は既に start ベースで使ってる前提
		fpos.z = rankAnimStart_.z + (rankAnimEnd_.z - rankAnimStart_.z) * fe;

		// 回転：あなたの既存ロジックと同じ（0→-15.6）をフレーム用に先行実行
		Vector3 frot = rankDebugRot_;
		frot.y = 0.0f + (-15.6f) * fe;

		frameOuter->mainPosition.transform.translate = fpos;
		frameOuter->mainPosition.transform.rotate = frot;
		frameOuter->mainPosition.transform.scale = rankDebugScale_;

		frameInner->mainPosition.transform.translate = fpos;
		frameInner->mainPosition.transform.rotate = frot;
		frameInner->mainPosition.transform.scale = rankDebugScale_;

		frameOuter->Update(usingCamera_);
		frameInner->Update(usingCamera_);

		// ★ フレームが終わったら文字 easing を解禁
		if (ft >= 1.0f) {
			isFrameAnimating_ = false;
			isRankAnimating_ = true;
			rankAnimT_ = 0.0f;

		}
	}

	// ============================
	// ① ランク本体（既存）  ※構造維持
	// ============================
	// ★フレーム中は文字を進めない
	if (!isFrameAnimating_ && isRankAnimating_) {

		rankAnimT_ += system_->GetDeltaTime() / 0.4f;
		float t = std::clamp(rankAnimT_, 0.0f, 1.0f);
		float ease = SceneManager::EaseOutSine(t);

		rankDebugPos_.z =
			rankAnimStart_.z +
			(rankAnimEnd_.z - rankAnimStart_.z) * ease;

		rankDebugRot_.y = 0.0f + (-15.6f) * ease;

		if (t >= 1.0f) {
			isRankAnimating_ = false;

			// ★ 完全追従フェーズへ
			isRankShake_ = true;
			rankShakeT_ = 0.0f;

			// スパーク（既存）
			StartSparkEffect2(
				rankObj->mainPosition.transform.translate - Vector3(0, -0.25f, -5),
				12,
				0.6f,
				0.05f
			);
		}

	}

	// ★文字は「フレーム完了後」だけ反映される
	if (!isFrameAnimating_) {
		rankObj->mainPosition.transform.translate = rankDebugPos_;
		rankObj->mainPosition.transform.rotate = Vector3(0, -15.6f, 0);
		rankObj->mainPosition.transform.scale = rankDebugScale_;
		rankObj->Update(usingCamera_);
	}

	// ============================
	// ② フレームを「完全追従」
	// ============================
	// ★追従は切り離し：フレームはフレームの easing で動かし、
	// それ以外の時間は「終点に固定」＝ここでは何も上書きしない
	// （Updateは必要なら残してOKだが、transformを触らないことが目的）
	if (!isFrameAnimating_) {
		frameOuter->Update(usingCamera_);
		frameInner->Update(usingCamera_);
	}


	// ============================
// ③ 追従＋減衰シェイク
// ============================
	if (isRankShake_) {

		rankShakeT_ += system_->GetDeltaTime();
		float t = rankShakeT_ / rankShakeDuration_;
		if (t > 1.0f) t = 1.0f;

		// ★ だんだん収束する係数（山形）
		float damp = (1.0f - t);
		damp = damp * damp; // なめらか

		// ★ sin ベースの微振動
		float shakeX = sinf(rankShakeT_ * 30.0f) * 0.15f * damp;
		float shakeY = cosf(rankShakeT_ * 24.0f) * 0.10f * damp;

		Vector3 shakeOffset = { shakeX, shakeY, 0.0f };

		// ★ 基準は「最終位置」
		Vector3 basePos = rankAnimEnd_;

		Vector3 finalPos = basePos + shakeOffset;

		// === 完全追従 ===
		rankObj->mainPosition.transform.translate = finalPos;
		rankObj->mainPosition.transform.rotate = rankDebugRot_;
		rankObj->mainPosition.transform.scale = rankDebugScale_;

		frameOuter->mainPosition.transform.translate = finalPos;
		frameOuter->mainPosition.transform.rotate = rankDebugRot_;
		frameOuter->mainPosition.transform.scale = rankDebugScale_;

		frameInner->mainPosition.transform.translate = finalPos;
		frameInner->mainPosition.transform.rotate = rankDebugRot_;
		frameInner->mainPosition.transform.scale = rankDebugScale_;

		if (t >= 1.0f) {
			isRankShake_ = false;

			// 最終固定
			rankObj->mainPosition.transform.translate = basePos;
			frameOuter->mainPosition.transform.translate = basePos;
			frameInner->mainPosition.transform.translate = basePos;
		}
	}

}
//==============================
// 破棄
//==============================
void SceneStage01::DestroyResultShoji() {

	delete resultShojiL_;        resultShojiL_ = nullptr;
	delete resultShojiR_;        resultShojiR_ = nullptr;
	delete resultShojiLPaper_;   resultShojiLPaper_ = nullptr;
	delete resultShojiRPaper_;   resultShojiRPaper_ = nullptr;
	isResultShojiActive_ = false; // ★追加
}

void SceneStage01::StartSparkEffect2(
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

void SceneStage01::UpdateSparkEffect2() {

	float dt = system_->GetDeltaTime();

	// sec / ms / us 吸収
	if (dt > 1000.0f) {
		dt *= 0.000001f;
	} else if (dt > 1.0f) {
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

		// ★ 楕円拡散係数
		const float ELLIPSE_X = 2.0f; // 横を広く
		const float ELLIPSE_Y = 0.6f; // 縦を狭く

		for (int i = 0; i < sparkCountPerBurst_; i++) {
			Spark s{};
			s.obj = new Object;
			s.obj->IntObject(system_);
			s.obj->CreateDefaultData();

			// ★ モデルを半分半分で切り替え
			const char* modelPath =
				(i % 2 == 0)
				? "resources/gate/model/spark.obj"
				: "resources/gate/model/spark2.obj";

			s.obj->CreateModelData(
				system_->SetModelObj(modelPath)
			);

			float angle = (rand() % 360) * (3.141592f / 180.0f);
			float radius = 0.5f;

			// ★ 楕円方向ベクトル
			Vector3 dir = {
				cosf(angle) * ELLIPSE_X,
				sinf(angle) * ELLIPSE_Y,
				0.0f
			};

			// 正規化（既存ロジック維持）
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

			s.startScale = { 0.25f, 0.25f, 0.25f };
			s.obj->mainPosition.transform.scale = s.startScale;

			float rotZ = atan2f(dir.y, dir.x) - 3.141592f * 0.5f;
			s.obj->mainPosition.transform.rotate = { 0.0f, 0.0f, rotZ };

			for (auto& part : s.obj->objectParts_) {
				if (!part.materialConfig) {
					part.materialConfig = std::make_shared<MaterialConfig>();
				}
				part.materialConfig->useModelTexture = false;
				part.materialConfig->textureHandle =
					system_->LoadTexture("resources/gate/model/punch.png");
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
		} else {
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

void SceneStage01::DrawSparkEffect2() {
	for (auto& s : sparks_) {
		if (s.obj) {
			s.obj->Draw();
		}
	}
}



void SceneStage01::UpdateOperationGuide() {
	// --- 1. キーボード・マウスの入力を検知 ---
	// 主要なキーやマウスボタンが押されたら「キーボードモード」にする
	// (全キー判定関数がない場合、よく使うキーを羅列するのが一般的です)
	if (system_->GetIsPush(DIK_W) || system_->GetIsPush(DIK_A) ||
		system_->GetIsPush(DIK_S) || system_->GetIsPush(DIK_D) ||
		system_->GetIsPush(DIK_T) ||
		system_->GetIsPush(DIK_SPACE) || system_->GetIsPush(DIK_RETURN) ||
		system_->GetMouseIsPush(0) || system_->GetMouseIsPush(1)) { // 左/右クリック

		isGamepadMode_ = false;
	}

	// --- 2. ゲームパッドの入力を検知 ---
	// スティックの傾きやボタン入力を検知したら「パッドモード」にする
	// ※スティックは微小な値が入ることがあるので閾値(デッドゾーン)を設ける
	const int kStickDeadZone = 3000; // 例: 最大32767に対して少し余裕を持つ

	bool isPadInput = false;

	// 左スティック入力判定
	if (abs(system_->GetGamepadLStickX()) > kStickDeadZone ||
		abs(system_->GetGamepadLStickY()) > kStickDeadZone) {
		isPadInput = true;
	}
	// ボタン入力判定 (0番～15番くらいまでチェック)
	// ※Engine側でAnyButtonがあればそれが楽ですが、なければループで確認
	for (int i = 0; i < 16; i++) {
		if (system_->GetGamepadIsPush(i)) {
			isPadInput = true;
			break;
		}
	}

	if (isPadInput) {
		isGamepadMode_ = true;
	}

	// ★重要: ここでテクスチャ差し替えは行わない！
	// 念のため両方のUpdateを呼んでおく（アニメーション等がある場合に備えて）
	guideKey_->Update(usingCamera_);
	guideGamepad_->Update(usingCamera_);
}

void SceneStage01::SetSceneTitle(SceneTitle* title) {
	sceneTitle_ = title;
}
