#pragma once
#include "Scene.h"
#include "Object/Object.h"  
#include "Object\Sprite.h"
#include "DebugCamera.h"

class SceneTitle :public Scene
{
public:
	SceneTitle(kEngine* system);
	~SceneTitle();

	void Update() override;
	void Draw() override;

public:
	/// ============== カメラ関連 ==============///
	/// ステージカメラ(mainカメラ)
	Camera* camera_ = nullptr;


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
	// カメラ操作用
	float cameraMoveSpeed_ = 1.2f;
	void CameraControl();

	// ===== こぶし =====
	Object* punch_ = nullptr;
	int punchModelHandle_ = -1;

	uint32_t punchTexture_ = 0;

	// ===== Punch Debug =====
	Vector3 punchPos_ = { 0.0f, 0.0f, 2.0f };
	Vector3 punchRot_ = { 0.0f, 0.0f, 0.0f }; // degree
	Vector3 punchScale_ = { 0.1f, 0.1f, 0.1f };

	bool showPunchDebug_ = true;


	// Iris
	Object* irisMask_ = nullptr;
	uint32_t irisTex_ = 0;

	bool irisPlaying_ = false;
	float irisT_ = 0.0f;        // 0..1
	float irisSpeed_ = 1.6f;    // 開く速さ
	float irisMaxScale_ = 20.0f; // 画面覆う大きさ

};
