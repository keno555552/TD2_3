#pragma once
#include "Scene.h"
#include "Object\Sprite.h"

class SceneResult :public Scene
{
public:
	SceneResult(kEngine* system);
	~SceneResult();

	void Update() override;
	void Draw() override;
	/// ============== カメラ関連 ==============///
/// ステージカメラ(mainカメラ)
	Camera* camera_ = nullptr;
	Camera* GetCamera() override { return camera_; }

public:

	/// ========= テキスチャーハンドル =========///
	int testTextResult{};

	/// ============ オブジェクト ============///
	SimpleSprite* sprite_ = new SimpleSprite;
};

