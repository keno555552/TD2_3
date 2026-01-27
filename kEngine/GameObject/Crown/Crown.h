#pragma once
#include "kEngine.h"
#include "Object/Object.h"

class Crown : public Object {
public:
	Crown(kEngine* system);

	void Update(Camera* camera) override;
	void Draw() override;

public:
	/// <summary>
	/// ターゲットを設定
	/// </summary>
	/// <param name="targetObj">ターゲットオブジェクト</param>
	/// <param name="maxArmScale">その対象の現在の最大腕サイズ（高さ・大きさ計算用）</param>
	void SetTarget(Object* targetObj, float maxArmScale);

private:

	/// =========== リソースハンドル ============///
	int modelHandle_ = 0;

	/// =============== 制御パラメータ ==============///
	Object* currentTarget_ = nullptr; // 現在のターゲット
	float currentArmScale_ = 1.0f;    // 現在のターゲットの腕サイズ

	// アニメーション用
	float appearTimer_ = 0.0f;
	const float kAppearDuration_ = 0.5f; // 出現にかかる時間（秒）
	bool isAppearing_ = false;

	// 回転用
	float rotationSpeed_ = 0.6f; // 回転速度

private:
	/// =============== 内部関数 ==============///
	// ターゲットへのリンク（HPBar同様）
	void LinkToObject(Object* targetObject);

	// イージング関数 (EaseOutBack: 少し飛び出して戻る動きなどが王冠っぽいですが、今回はシンプルなEaseOutCubic等で)
	float EaseOutBack(float x);
};