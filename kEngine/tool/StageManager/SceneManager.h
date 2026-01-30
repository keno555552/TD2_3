#pragma once
#include "MaterialConfig.h"
#include "Scene.h"
#include "SceneTitle.h"
#include "Scene1.h"
#include "SceneStage01.h"
#include "SceneResult.h"
#include "SceneTestForGE.h"
#include "SceneTester.h"
#include "SceneTest.h"
#include "SceneTest2.h"
#include "AnimationSystem/AnimationEditor.h"
#include "DefaultMenu/DefaultMenu.h"
#include "CG4_HK_1/Effect2.h"
#include "Object/Object.h"  
class SceneManager {
public:
	SceneManager(kEngine* system);
	~SceneManager();

	void Update();
	void Render();

	bool GetIsEnd();


	// 障子（タイトルと同じ Object）
	Object* shojiL_ = nullptr;
	Object* shojiR_ = nullptr;

	// 障子（タイトルと同じ Object）
	Object* shojiL_paper_ = nullptr;
	Object* shojiR_paper_ = nullptr;

	bool shojiPlaying_ = false;
	float shojiT_ = 0.0f;

	// 退避
	SceneNum nextAfterShoji_ = SceneNum::S_NONE;
	Camera* shojiCam_ = nullptr;
	// SceneManager.h 内

// ★追加：障子フェーズ
	enum class ShojiPhase {
		None,
		Closing,
		WaitBeforeTransition,
		WaitNewCamera,   // ★追加：遷移先カメラ待ち
		Opening
	};


private:
	// ★追加：カメラ取得 & 生成を関数化
	Camera* ResolveCamera(Scene* s);
	void SpawnSceneByHandle(SceneNum handle);

private:
	// ★追加：障子の状態
	ShojiPhase shojiPhase_ = ShojiPhase::None;

	float shojiWaitT_ = 0.0f;
	float shojiWaitDuration_ = 1.0f; // ←「閉じたまま待つ秒数」ここだけ触ればいい

	// SceneManager.h（private: の中）
	Vector3 shojiBasePos_{ 0.0f, 0.0f, 0.0f };
	bool sceneJustSpawned_ = false;  // シーン生成直後の1フレームだけDrawを抑止


	// ===== 障子：関数化（変数は一切変更しない）=====
	void Shoji_CreateAll(const Vector3& camRot);
	void Shoji_SetInitialOpenPose(const Vector3& base, const Vector3& camRot);
	void Shoji_UpdateClosingOpening(float dt);
	void Shoji_UpdateWaitBeforeTransition(float dt);
	void Shoji_UpdateWaitNewCamera(float dt);
	void Shoji_DestroyAll();


public:
	SceneNum sceneUsingHandle_ = SceneNum::S_NONE;
	void SceneChanger();

	void StageCheckBoxUpdate();


private:
	Scene* sceneUsing_ = nullptr;
	Scene* sceneOld_ = nullptr;

private:
	kEngine* system_ = nullptr; // 借り

	DefaultMenu* defaultMenu_ = nullptr;

	/// ========= リソースハンドル ========= ///

	int helperTextureHandle_ = 0;

	/// ============ オブジェクト =========== ///

	SimpleSprite* helperSprite_ = nullptr;
	Vector2 helperSpriteScale_ = { 1.0f,1.0f };
	Vector2 helperSpritePos_ = { 50.0f,50.0f };

	/// =============== ステージ管理 ===============///

	bool stage[10]{};

	bool isFirst_ = true;

	bool isFromTitle_ = true;

	bool isReset_ = false;

	bool isEnd_ = false;





private:
	/// ============= ステージマネージメント ============///
	void ClearStage();

#ifdef USE_IMGUI
	void ImguiPart();
#endif
};

