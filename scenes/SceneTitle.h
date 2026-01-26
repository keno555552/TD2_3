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
	static constexpr int kGatePartCount = 2;

	Object* gates_[kGatePartCount] = {};
	uint32_t  gateModels_[kGatePartCount] = {};
	int       gateTextures_[kGatePartCount] = {};
	// カメラ操作用
	float cameraMoveSpeed_ = 0.2f;
	void CameraControl();

};
