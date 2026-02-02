#pragma once
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


class SceneStage01 :public BaseScene {
public:
	SceneStage01(kEngine *system);
	~SceneStage01();
	void Update() override;
	void Draw() override;


	Camera* GetCamera() override { return camera_; }
	// ★ これを追加
	void ArrangeByArmSize();

	struct ResultSet {
		std::vector<Object*> parts;
		std::vector<Vector3> basePos;

		bool shojiOpening = false;
		bool shojiOpened = false;
		float shojiT = 0.0f; // 0 → 1
	};

	// リザルト一式を塊のまま複製してZ方向に並べる
	void SpawnResultSets(int copyCount, float spacingZ);
	void UpdateResultShoji();
	void OpenShoji(ResultSet& set);
	// ResultModelPaths の並びに対応
	enum ResultPartIndex {
		IDX_RESULT_WALL = 0,
		IDX_RESULT_WOOD = 1,
		IDX_SYOUJI_L = 2,
		IDX_SYOUJI_R = 3,
		IDX_PAPER_L = 4,
		IDX_PAPER_R = 5,
	};


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

	std::vector<Enemy*> enemies_{}; // enemyのリスト

	Object* guideArrow_ = nullptr; // ガイド矢印
	Crown* crown_ = nullptr; // 1位用王冠

	Object* wall_[4] = { nullptr };

	P_DashWind* dashWindEffect_ = nullptr;

	P_Suction* suctionEffect_ = nullptr;

	P_PlaneReduction* planeReductionEffectL_ = nullptr;
	P_PlaneReduction* planeReductionEffectR_ = nullptr;

	Explosion* explosion_ = nullptr;

	const Vector3 kBaseActorScale_ = Vector3(0.5f, 0.5f, 0.5f);

	// チュートリアルお試し用
	Tutorial* tutorial_ = nullptr;

	/// =============== フラグ ================///
	bool useDebugCamera = false;

	// ヒットストップ（スロー）管理用
	float hitStopTimer_ = 0.0f;





	// ===== 門パーツ =====
	static constexpr int kDoujouPartCount = 15;

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


	

		std::vector<ResultSet> resultSets_;

		bool isResultGenerated_ = false;

		// ===== 門パーツ =====
		static constexpr int kResultPartCount = 6;

		Object* Results_[kResultPartCount] = {};
		uint32_t  ResultModels_[kResultPartCount] = {};
		int  ResultTextures_[kResultPartCount] = {};


		// ===== Gate Debug =====
		Vector3 ResultPos_[kResultPartCount];
		Vector3  ResultRot_[kResultPartCount];
		Vector3 ResultScale_[kResultPartCount];



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
};

