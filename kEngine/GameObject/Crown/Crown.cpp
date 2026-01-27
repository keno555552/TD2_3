#define NOMINMAX
#include "Crown.h"
#include <algorithm> // std::max用
#include <cmath>

Crown::Crown(kEngine* system) {
	IntObject(system);
	modelHandle_ = system_->SetModelObj("resources/TemplateResource/object/crown/crown.obj");
	Object::CreateModelData(modelHandle_);

	// 初期状態は非表示（スケール0）
	mainPosition.transform.scale = { 0.0f, 0.0f, 0.0f };
}

void Crown::Update(Camera* camera) {
	// ターゲットがいない場合は更新しない、または非表示
	if (currentTarget_ == nullptr) {
		mainPosition.transform.scale = { 0.0f, 0.0f, 0.0f };
		Object::Update(camera);
		return;
	}

	float rawDeltaTime = system_->GetDeltaTime();
	float dt = std::min(rawDeltaTime, 0.1f);

	// 1. 常時回転処理
	mainPosition.transform.rotate.y += rotationSpeed_ * dt;

	// 2. 出現アニメーション（スケール計算）
	float animScale = 1.0f;
	if (isAppearing_) {
		appearTimer_ += dt;
		float rate = std::min(appearTimer_ / kAppearDuration_, 1.0f);
		animScale = EaseOutBack(rate);

		if (rate >= 1.0f) {
			isAppearing_ = false;
		}
	}

	// 3. ターゲットのサイズに応じたスケーリング（矢印のロジックを流用）
	// ターゲット自体のグローバルスケールを取得 (ターゲットが0.5倍なら王冠も基準は0.5倍)
	float targetGlobalScale = currentTarget_->mainPosition.transform.scale.x;

	// 基本サイズ割合 + (腕倍率増加分 * 成長係数)
	float startRatio = 1.5f;  // 体の30%くらいの大きさから
	float growthRatio = 0.8f; // 腕が大きくなると王冠も少し大きくなる

	float finalScaleVal = (startRatio + (currentArmScale_ - 1.0f) * growthRatio) * targetGlobalScale;

	// アニメーション倍率を適用
	finalScaleVal *= animScale;

	mainPosition.transform.scale = { finalScaleVal, finalScaleVal, finalScaleVal };


	// 4. 位置調整（頭上に配置）
	// 腕のモデル上の半径 = 0.5 (直径1.0の半分)
	// 実際の腕の高さ(半径) = 現在の腕倍率 * 0.5f
	float armRadius = currentArmScale_ * 1.2f;

	// 体の頭頂部 = 0.5f (高さ1.0の半分)
	// 腕が小さい時は、最低でも頭の上(0.5f)にはあってほしい
	float heightBase = std::max(1.0f, armRadius);

	// 余白（マージン）。これがないとモデルにめり込むので少し浮かせます
	// 腕が大きくなるほど、少しマージンも広げると見栄えが良いです
	float margin = 1.0f + (currentArmScale_ * 0.1f);

	// 最終的な高さ
	float offsetY = heightBase + margin;

	mainPosition.transform.translate = { 0.0f, offsetY, 0.0f };

	// 5. 基底クラス更新
	Object::Update(camera);
}

void Crown::Draw() {
	// ターゲットがいる時だけ描画
	if (currentTarget_ != nullptr) {
		Object::Draw();
	}
}
void Crown::SetTarget(Object* targetObj, float maxArmScale) {
	// ターゲットが変更された場合
	if (currentTarget_ != targetObj) {
		currentTarget_ = targetObj;
		LinkToObject(targetObj);

		// 新しいターゲットになったので出現アニメーションを開始
		isAppearing_ = true;
		appearTimer_ = 0.0f;

		// 瞬間的にスケール0にする（次のUpdateで計算されるが念のため）
		mainPosition.transform.scale = { 0.0f, 0.0f, 0.0f };
	}

	// 腕サイズは毎フレーム更新
	currentArmScale_ = maxArmScale;
}

void Crown::LinkToObject(Object* targetObject) {
	if (targetObject != nullptr) {
		// 親の行列に追従させる
		followObject_ = &targetObject->mainPosition;
	} else {
		followObject_ = nullptr;
	}
}

// ポップな動き（少し大きくなってから戻る）
float Crown::EaseOutBack(float x) {
	const float c1 = 1.70158f;
	const float c3 = c1 + 1.0f;
	return 1.0f + c3 * std::pow(x - 1.0f, 3.0f) + c1 * std::pow(x - 1.0f, 2.0f);
}
