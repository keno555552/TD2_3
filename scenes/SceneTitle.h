#pragma once
#include "BaseScene.h"
#include "Object/Object.h"  
#include "Object\Sprite.h"
#include "DebugCamera.h"
#include <array>

class SceneTitle :public BaseScene
{
public:
	SceneTitle(kEngine* system);
	~SceneTitle();

	void Update() override;
	void Draw() override;
	// タイトル側専用。Stageと共有しない
	// タイトル側専用。Stageと共有しない
	static constexpr int kPreloadDoujouCount = 15;
public:

	float skydomeRotateY_ = 0.0f;
	const float kSkydomeRotateSpeed_ = 0.05f; // rad / sec（好みで）

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

	// パッド入力の連続防止
	bool padSelectNeutral_ = true;

	// スティック用デッドゾーン
	static constexpr int kStickDeadZone = 3000;


	int GetSelectedIndex() const { return selectIndex_; }
	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	Camera* camera_ = nullptr;

	// カメラ歩行移動
	bool cameraWalkPlaying_ = false;
	float cameraWalkT_ = 0.0f;
	float cameraWalkDuration_ = 1.8f;

	Vector3 cameraWalkStart_ = {};
	Vector3 cameraWalkEnd_ = { -8.7f, 7.5f, 37.5f };

	float cameraWalkBobTime_ = 0.0f;
	float cameraWalkBobAmp_ = 0.15f;   // 歩行の上下量
	float cameraWalkBobFreq_ = 10.0f;  // 歩行テンポ
	bool gateOpenTriggered_ = false;

	/// ========= テキスチャーハンドル =========///
	int testTextTitle{};

	/// ============ オブジェクト ============///
	SimpleSprite* sprite_ = new SimpleSprite;

	Camera* usingCamera_ = nullptr;

	// ===== 門パーツ =====
	static constexpr int kGatePartCount = 10;

	Object* gates_[kGatePartCount] = {};
	uint32_t  gateModels_[kGatePartCount] = {};
	int       gateTextures_[kGatePartCount] = {};

	// ===== 門パーツ =====
	static constexpr int kSelectPartCount = 14;

	Object* selects_[kSelectPartCount] = {};
	uint32_t  selectModels_[kSelectPartCount] = {};
	int       selectTextures_[kSelectPartCount] = {};


	// ===== Gate Debug =====
	Vector3 selects_Pos_[kSelectPartCount];

	// ===== Gate Debug =====
	Vector3 gatePos_[kGatePartCount];
	Vector3 gateRot_[kGatePartCount];
	Vector3 gateScale_[kGatePartCount];

	// ===== Gate Open Animation =====
	bool gateOpenPlaying_ = false;
	float gateOpenT_ = 0.0f;
	float gateOpenDuration_ = 5.0f; // 秒

	float gateStartRotY_[2];
	float gateTargetRotY_[2] = { -1.53f, 1.53f };


	// ===== Camera Shake (Gate Open) =====
	float gateShakeTime_ = 0.0f;
	float gateShakeStrength_ = 0.16f; // 揺れの強さ（小さめ）
	float gateShakeFreq_ = 202.0f;     // 揺れ速度


	/// ========= Lighting関連 =========/// <<<<<<<<<<<<<<<<<<<<<<<
	DirectionalLight eveningLight_{};


	// ===== 文字パーツ =====
	static constexpr int kMoziPartCount = 3;

	Object* mozis_[kMoziPartCount] = {};
	uint32_t  moziModels_[kMoziPartCount] = {};
	int       moziTextures_[kMoziPartCount] = {};


	struct MoziOrbit {
		Vector3 center;
		float angle;
		float radius;
	};

	std::array<MoziOrbit, kMoziPartCount> moziOrbit_;

	float moziOrbitTime_ = 0.0f;
	float moziOrbitDuration_ = 6.0f;   // 収束までの時間
	float moziRotateSpeed_ = 4.0f;     // 回転速度（rad/sec）
	float moziStartRadius_ = 20.0f;
	float moziEndRadius_ = 0.0f;


	// ===== Mozi Final Easing =====
	bool moziSettlePlaying_ = false;
	int  moziSettleIndex_ = 0;        // 今動かしている文字（順番配列のindex）
	float moziSettleT_ = 0.0f;
	float moziSettleDuration_ = 0.6f;

	// 実行順（1,3,2）
	int moziSettleOrder_[kMoziPartCount] = { 0, 2, 1 };

	// 開始Transform（収束終了時）
	Vector3 moziStartPos_[kMoziPartCount];
	Vector3 moziStartRot_[kMoziPartCount];
	Vector3 moziStartScale_[kMoziPartCount];

	// 最終Transform（指定値）
	Vector3 moziFinalPos_[kMoziPartCount] = {
		{ -2.85f, 17.65f, 5.9f },   // 文字1
		{  -0.5f, 17.65f, 5.9f },   // 文字2
		{  1.85f, 17.65f, 5.9f },   // 文字3
	};
	Vector3 moziFinalRot_[kMoziPartCount] = {
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
	};
	Vector3 moziFinalScale_[kMoziPartCount] = {
		{ 0.14f, 0.14f, 0.14f },
		{ 0.14f, 0.14f, 0.14f },
		{ 0.14f, 0.14f, 0.14f },
	};

	bool moziOrbitFinished_ = false;

	// カメラ操作用
	float cameraMoveSpeed_ = 0.8f;
	void CameraControl();

	// ===== こぶし =====
	Object* punch_ = nullptr;
	int punchModelHandle_ = -1;

	uint32_t punchTexture_ = 0;

	// ===== Punch Debug =====
	Vector3 punchPos_ = { -0.45f, 17.4f, 7.3f };
	Vector3 punchRot_ = { 0.0f, 0.0f, 0.0f }; // degree
	Vector3 punchScale_ = { 3.34f, 3.34f, 0.1f };

	bool showPunchDebug_ = true;

	// ===== パンチ演出 =====
	bool  punchAnimPlaying_ = false;
	bool  punchAnimReturning_ = false;
	float punchAnimT_ = 0.0f;

	Vector3 punchScaleStart_ = { 0.0f, 0.0f, 0.0f };
	Vector3 punchScalePeak_ = { 1.34f, 1.34f, 0.1f };

	float punchAnimDuration_ = 0.15f; // 速めが気持ちいい

	// Iris

	bool irisPlaying_ = false;
	float irisT_ = 0.0f;        // 0..1
	float irisSpeed_ = 2.0f;    // 開く速さ
	float irisMaxScale_ = 1000.0f; // 画面覆う大きさ
	Object* irisHole_ = nullptr;
	int irisHoleModelHandle_ = -1;
	uint32_t irisHoleTex_ = 0;


	// Camera easing
	bool cameraMovePlaying_ = false;
	float cameraMoveT_ = 0.0f;

	Vector3 cameraStartPos_;
	Vector3 cameraEndPos_;
	Vector3 cameraPrevPos_;

	Object* skydome_ = nullptr;

	int skydomeModelHandle_ = 0;
	/// テキスチャーハンドル
	int skydomeTextureHandle_ = 0;

	bool cameraShakePlaying_ = false;
	float cameraShakeTime_ = 0.0f;
	Vector3 cameraShakeOffset_ = { 0.0f, 0.0f, 0.0f };


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
	void StartSparkEffect(
		const Vector3& basePos,
		int countPerBurst,
		float duration,
		float interval
	);

	// 更新（★これが本体）
	void UpdateSparkEffect();
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

	Vector3 moziPos_[kMoziPartCount];
	Vector3 moziRot_[kMoziPartCount];
	Vector3 moziScale_[kMoziPartCount];


	// セレクト関連
	int selectIndex_ = 0;        // 0〜2
	int selectCount_ = 3;        // 選択肢数
	Vector3 selectBasePos_;      // 選択肢0の基準位置
	float selectSpacing_ = 4; // 横間隔（調整用）

	// カーソル拡縮
	float cursorScaleTime_ = 0.0f;
	float cursorScaleSpeed_ = 2.5f;   // 速さ（調整用）
	float cursorScaleMin_ = 0.1f;
	float cursorScaleMax_ = 0.105f;




	//Camera* GetUsingCamera() override;
	// SceneTitle / SceneStage01

};
