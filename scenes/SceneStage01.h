#pragma once
#include "Scene.h"
#include "Object\Object.h"
#include "Object\Sprite.h"
#include "DebugCamera.h"
#include "Player\Player.h"
#include "Enemy\Enemy.h"
#include "tool\MapChip\MapChipField.h"
#include "kEngine/GameObject/Particle/DashWind/P_DashWind.h"
#include "kEngine/GameObject/Particle/Suction/P_Suction.h"

class SceneStage01 :public Scene {
public:
	SceneStage01(kEngine *system);
	~SceneStage01();
	void Update() override;
	void Draw() override;


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
	float cameraDistanceMultiply_ = 15.0f; // 倍率値
	/// カメラ高さ(実数値)
	float cameraHeight_ = 9.0f; // 実数値
	float cameraHeightMultiply_ = 8.0f; // 倍率値

	/// =========== リソースハンドル ============///
	/// モデルハンドル
	int skydomeModelHandle_ = 0;
	int playerModelHandle_ = 0;
	int wallModelHandle_ = 0;

	/// テキスチャーハンドル
	int boxTextureHandle_ = 0;
	int tryTextureHandle_ = 0;
	int uvTextureHandle_ = 0;

	/// =========== ゲームオブジェクト ==========///
	Object *skydome_ = nullptr;
	Player *player_ = nullptr;
	SimpleSprite *sprite_ = nullptr;
	SimpleSprite *sprite2_ = nullptr;

	Enemy *enemy_ = nullptr;

	Object* wall_[4] = { nullptr };

	P_DashWind* dashWindEffect_ = nullptr;

	P_Suction* suctionEffect_ = nullptr;

	/// =============== フラグ ================///
	bool useDebugCamera = false;

private:
	/// ============= シーン内命令 =============///
	void CameraPart();
	void ImguiPart();

	/// ============= 衝突判定関数 ============/// 
	// 攻撃による当たり判定
	void CheckAttackCollision();
};

