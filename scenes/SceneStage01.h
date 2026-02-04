#pragma once
#include <vector>
#include "BaseScene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "DebugCamera.h"
#include "Player\Player.h"
#include "Enemy\Enemy.h"
#include "Crown\Crown.h"
#include "tool\MapChip\MapChipField.h"
#include "kEngine/GameObject/Particle/DashWind/P_DashWind.h"
#include "kEngine/GameObject/Particle/Suction/P_Suction.h"
#include "GameObject/Particle/P_PlaneReduction/P_PlaneReduction.h"
#include "GameObject/Particle/explosion/Explosion.h"
#include "Tutorial/Tutorial.h"
#include "CountdownTimer/CountdownTimer.h"
#include "StartTimer/StartTimer.h"
#include "SceneTitle.h"

class SceneStage01 :public BaseScene {
public:
	SceneStage01(kEngine *system);
	~SceneStage01();
	void Update() override;
	void Draw() override;

	Camera* GetCamera() override;
	// ★ これを追加
	void ArrangeByArmSize();

	struct ResultSet {
		std::vector<Object*> parts;
		std::vector<Vector3> basePos;

		bool shojiOpening = false;
		bool shojiOpened = false;
		float shojiT = 0.0f; // 0 → 1
		// ★ 追加
		bool shojiClosing = false;
		float shojiCloseWait = 0.0f; // 近づいてからの待ち
		float shojiCloseT = 0.0f;    // 閉じアニメ用
	};
	 float CLOSE_WAIT_TIME = 0.05f; // 近づいてから待つ時間
	 float CLOSE_TIME = 0.5f; // 閉じる時間

	bool isResultShojiActive_ = false;
	float resultShojiWaitTimer_ = 0.0f;

	// リザルト一式を塊のまま複製してZ方向に並べる
	void SpawnResultSets(int copyCount, float spacingZ);
	void UpdateResultShoji();
	void OpenShoji(ResultSet& set);
	void UpdateRankModels();
	int playerRank_ = -1; // 0 = 1位, 1 = 2位 ...

	// ResultModelPaths の並びに対応
	enum ResultPartIndex {
		IDX_RESULT_WALL = 0,
		IDX_RESULT_WOOD = 1,
		IDX_SYOUJI_L = 2,
		IDX_SYOUJI_R = 3,
		IDX_PAPER_L = 4,
		IDX_PAPER_R = 5,
	};
	// ===== Result Shoji =====
	enum class ResultShojiPhase {
		None,
		Closing,
		Wait,      // ★追加
		Opening,
		CameraMove,   // ★追加
		CameraToPlayer,   // ★追加
		RankAppear   // ★追加
	};

	// ===== 順位モデル演出 =====
	bool isRankAnimating_ = false;
	bool isRankAppear_ = false;
	float rankAnimT_ = 0.0f;

	Vector3 rankAnimStart_;
	Vector3 rankAnimEnd_;


	float resultCameraT_ = 0.0f;
	Vector3 resultCameraStart_;
	Vector3 resultCameraEnd_;

	Vector3 resultCameraPlayerStart_;
	Vector3 resultCameraPlayerEnd_;
	float   resultCameraPlayerT_ = 0.0f;


	ResultShojiPhase resultShojiPhase_ = ResultShojiPhase::None;
	float resultShojiT_ = 0.0f;

	// 障子オブジェクト
	Object* resultShojiL_ = nullptr;
	Object* resultShojiR_ = nullptr;
	Object* resultShojiLPaper_ = nullptr;
	Object* resultShojiRPaper_ = nullptr;

	// API
	void StartResultShoji();
	void UpdateResultShojiAnim();
	void CreateResultShoji(const Vector3& camRot);
	void SetResultShojiOpenPose(const Vector3& base, const Vector3& camRot);
	void DestroyResultShoji();
	
	void SetSceneTitle(SceneTitle* title);
	static void SetParticleDrawBlocked(bool block);
	static bool IsParticleDrawBlocked();

	static bool particleDrawBlocked_;
private:
	kEngine *system_ = nullptr;				// 借り

	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	Camera *camera_ = nullptr;
	/// デバックカメラ
	DebugCamera *debugCamera_ = nullptr;
	/// 今使っているカメラ
	Camera *usingCamera_ = nullptr;			// 借り用

	/// カメラ距離
	float cameraDistance_ = 15.0f; // 実数値
	float cameraDistanceMultiply_ = 7.0f; // 倍率値
	/// カメラ高さ(実数値)
	float cameraHeight_ = 9.0f; // 実数値
	float cameraHeightMultiply_ = 4.0f; // 倍率値
	// ▼▼▼ 手動ズーム制御用 ▼▼▼
	float manualZoomOffset_ = 0.0f;        // 現在の手動ズーム量（0が基準）
	const float kManualZoomSpeed_ = 10.0f; // ズームの変化速度
	const float kMinManualZoom_ = -5.0f;   // 最大ズームイン（近づける限界）
	const float kMaxManualZoom_ = 15.0f;   // 最大ズームアウト（離れる限界）
	// ズームアウトした時に、少し視点も高くするかどうかの係数
	// (1.0なら距離と同じだけ高さも増える。0.0なら高さは変わらない)
	const float kManualZoomHeightRatio_ = 0.5f;

	/// =========== リソースハンドル ============///
	/// モデルハンドル
	int skydomeModelHandle_ = 0;
	int playerModelHandle_ = 0;
	int guideArrowModelHandle_ = 0;
	int wallModelHandle_ = 0;

	/// テキスチャーハンドル
	int boxTextureHandle_ = 0;
	int tryTextureHandle_ = 0;
	int uvTextureHandle_ = 0;
	int guideKeyHandle_ = 0; // キーボード用の画像
	int guideGamepadHandle_ = 0;  // ゲームパッド用の画像

	/// =========== ゲームオブジェクト ==========///
	// 左右の情報をまとめる
	struct ArmInfo {
		bool isRight;
		Vector3 pos;
		float scale;
		bool isAtk;
	};
	Object *skydome_ = nullptr;
	Player *player_ = nullptr;
	SimpleSprite *sprite_ = nullptr;
	SimpleSprite *sprite2_ = nullptr;

	// 操作ガイド用スプライト
	// スプライトを2つ用意する
	SimpleSprite* guideKey_ = nullptr; // キーボード用
	SimpleSprite* guideGamepad_ = nullptr; // パッド用

	std::vector<Enemy*> enemies_{}; // enemyのリスト

	Object* guideArrow_ = nullptr; // ガイド矢印
	Crown* crown_ = nullptr; // 1位用王冠

	Object* wall_[4] = { nullptr };

	P_DashWind* dashWindEffectL_ = nullptr;
	P_DashWind* dashWindEffectR_ = nullptr;

	P_Suction* suctionEffect_ = nullptr;

	P_PlaneReduction* planeReductionEffectL_ = nullptr;
	P_PlaneReduction* planeReductionEffectR_ = nullptr;

	Explosion* explosion_ = nullptr;

	const Vector3 kBaseActorScale_ = Vector3(0.5f, 0.5f, 0.5f);

	// タイマー
	Timer* startUpTimer;
	CountdownTimer* countdownTimer_ = nullptr;
	StartTimer* startTimer_ = nullptr;

	// チュートリアルお試し用
	Tutorial* tutorial_ = nullptr;

	/// =============== フラグ ================///
	bool useDebugCamera = false;

	// ヒットストップ（スロー）管理用
	float hitStopTimer_ = 0.0f;

	// 現在ゲームパッドモードかどうか
	bool isGamepadMode_ = false;

	// ===== Rank Debug (ImGui用) =====
	Vector3 rankDebugPos_ = { 49.0f, 5.0f, 0.0f };
	Vector3 rankDebugRot_ = { 0.0f, 0.0f, 0.0f };
	Vector3 rankDebugScale_ = { 0.2f, 0.2f, 0.2f };

	bool useRankDebug_ = true; // ImGui操作を使うか


	// ===== 門パーツ =====
	static constexpr int kDoujouPartCount = 16;

	Object* Doujous_[kDoujouPartCount] = {};
	uint32_t  DoujouModels_[kDoujouPartCount] = {};
	int      DoujouTextures_[kDoujouPartCount] = {};


	// ===== Gate Debug =====
	Vector3 DoujouPos_[kDoujouPartCount];
	Vector3 DoujouRot_[kDoujouPartCount];
	Vector3 DoujouScale_[kDoujouPartCount];

	// タイル畳（畳 + フレーム）の実体置き場
	std::vector<Object*> tatamiTiles_;   // 畳
	std::vector<Object*> tatamiFrames_;  // フレーム
	Vector3 arrangeOrigin_ = { 50.0f, 0.0f, 50.0f }; // 好きな場所
		bool isCameraLocked_ = false; // ★ 追従停止フラグ


		int rankDelayFrame_ = 0;   // ランク開始待ち用
		int rankAnimFrame_ = 0;   // ランクアニメ用


		std::vector<ResultSet> resultSets_;

		bool isResultGenerated_ = false;
		bool NotDraw_ = false;


		bool  isRankEndWait_ = false;
		float rankEndWaitT_ = 0.0f;
		const float rankEndWaitDuration_ = 2.0f; // ← 待ち時間（秒）

		// ===== 門パーツ =====
		static constexpr int kResultPartCount = 6;

		Object* Results_[kResultPartCount] = {};
		uint32_t  ResultModels_[kResultPartCount] = {};
		int  ResultTextures_[kResultPartCount] = {};


		// ===== Gate Debug =====
		Vector3 ResultPos_[kResultPartCount];
		Vector3  ResultRot_[kResultPartCount];
		Vector3 ResultScale_[kResultPartCount];

		static constexpr int kKanjiCount = 13;
		 int kFrameOuterIndex = 11; // number_flame_flame.obj
		 int kFrameInnerIndex = 12; // number_flame.obj
		 bool isFrameAnimating_ = false;
		 float frameAnimT_ = 0.0f;

		Object* KanjiObjs_[kKanjiCount] = {};
		uint32_t  KanjiModels_[kKanjiCount] = {};
		int  KanjiTextures_[kKanjiCount] = {};
		// ===== Gate Debug =====
		Vector3 KanjiPos_[kKanjiCount];
		Vector3  KanjiRot_[kKanjiCount];
		Vector3 KanjiScale_[kKanjiCount];
		
		bool  isRankShake_ = false;
		float rankShakeT_ = 0.0f;
		float rankShakeDuration_ = 0.6f; // シェイク全体時間

		static constexpr int kIllustrationPartCount = 10;


		int IllustrationModels[kIllustrationPartCount] = {};
		int IllustrationTextures[kIllustrationPartCount] = {};
		Object* Illustrations[kIllustrationPartCount] = {};

		Vector3 IllustrationPos[kIllustrationPartCount] = {};
		Vector3 IllustrationRot[kIllustrationPartCount] = {};
		Vector3 IllustrationScale[kIllustrationPartCount] = {};

		// =====================
// Spark Effect
// =====================

		struct Spark {
			Object* obj = nullptr;
			Vector3 velocity;
			float life = 0.0f;
			float lifeMax = 0.0f;
			Vector3 startScale;   // ★ 追加
		};


		std::vector<Spark> sparks_;

		bool sparkPlaying_ = false;
		float sparkTimer_ = 0.0f;
		float sparkIntervalTimer_ = 0.0f;

		float sparkDuration_ = 0.0f;
		float sparkInterval_ = 0.0f;
		int sparkCountPerBurst_ = 0;
		Vector3 sparkBasePos_;

		// 開始
		void StartSparkEffect2(
			const Vector3& basePos,
			int countPerBurst,
			float duration,
			float interval
		);

		// 更新（★これが本体）
		void UpdateSparkEffect2();

		void DrawSparkEffect2();

		// セレクトSE
  // =====================
		int  selectMoveSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool selectSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int Title_PunchSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool Title_PunchSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int  Gate_OpenSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool Gate_OpenLoaded_ = false; // ★ 一度だけロードするためのフラグ

		int  DecideSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool DecideSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int AppearSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool AppearSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int WalkSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool WalkSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int BGM_ = -1;     // SEハンドル（-1 = 未ロード）
		bool BGM_oaded_ = false; // ★ 一度だけロードするためのフラグ

		int BGMR_ = -1;     // SEハンドル（-1 = 未ロード）
		bool BGMR_oaded_ = false; // ★ 一度だけロードするためのフラグ
		SceneTitle* sceneTitle_ = nullptr;


		int CloseSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool CloseSELoaded_ = false; // ★ 一度だけロードするための


		int OpenSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool OpenSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int FlameSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool FlameSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int RankSE_ = -1;
		bool RankSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int StartSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool StartSELoaded_ = false; // ★ 一度だけロードするためのフラグ

		int CountSE_ = -1;     // SEハンドル（-1 = 未ロード）
		bool CountSELoaded_ = false; // ★ 一度だけロードするためのフラグ
private:
	/// ============= シーン内命令 =============///
	void CameraPart();
	void ImguiPart();

	/// ============= 衝突判定関数 ============/// 
	// 攻撃による当たり判定
	void CheckAttackCollision();

	// スローを開始する関数
	// scale: 時間の速度 (0.1f なら 10分の1 の速度)
	// duration: スローを維持する実時間 (秒)
	void StartHitStop(float scale, float duration);

	/// ============ ガイド矢印関連 ============///
	// ガイド矢印の更新
	void UpdateGuideArrow();

	/// ============ 王冠関連 ============///
	// 王冠の更新
	void UpdateCrown();

	/// ============ 操作説明関連 ============///
	// 操作ガイドの表示切り替え更新用関数
	void UpdateOperationGuide();
	
};

