#pragma once
#include "Object/Object.h"
#include "Object/Sprite.h"

class StartTimer : public SimpleSprite
{
public:
	StartTimer(kEngine* kEngine);


	void Update(Camera* camera) override;
	void Draw() override;

	void Start();
	void Stop();
	void Reset();
	bool IsTimeUp();

	Timer& GetTime();
	int GetCount() const { return numberCount_; }
	bool isRunning() const { return isStart_; }


private:
	bool startSEPlayed_ = false;
	bool hasStarted_ = false;

	int StartSE_ = -1;     // SEハンドル（-1 = 未ロード）
	bool StartSELoaded_ = false; // ★ 一度だけロードするためのフラグ

	int CountSE_ = -1;     // SEハンドル（-1 = 未ロード）
	bool CountSELoaded_ = false; // ★ 一度だけロードするためのフラグ
	/// ========= テキスチャーハンドル =========///
	/// 
	/// 0 =1,1=2,2=3
	int numberTextureHandles_[3];
	int startTextureHandle_;


	/// ============ オブジェクト =============///

	SimpleSprite* numberSprites_[3]{};
	SimpleSprite* startSprite_ = nullptr;



	/// =========== フラク、内部変数 ===========///
	Vector4 usingColor = { 1.0f,1.0f,1.0f,0.0f };
	float startScale_ = 0.5f;
	Timer* timer_ = nullptr;
	int numberCount_ = 3;
	bool isStart_ = false;

private:

	void ImGuiPart();

};

