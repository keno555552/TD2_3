#pragma once
#include "Object/Object.h"
#include <vector>
#include <functional>
#include "Vector3.h"
#include "kEngine.h"
#include "Scene.h"
#include "GameObject/Object/Corner.h"

// 最大膨張率
static inline const float kMaxArmScale = 30.0f;
// 腕が縮む速さ
static inline const float kShrinkSpeed = 0.4f;
// 腕が膨らむ速さ
static inline const float kExpandSpeed = 0.3f;
// 旋回性能
static inline const float kTurnSpeed = 0.01f;
// 自動直進の基本速度
static inline const float kBaseSpeed = 3.0f;
// 離した時の加速力
static inline const float kBoostPower = 4.0f;

/// 移動速度
static inline const float kAcceleration = 3.0f;
/// 移動減衰
static inline const float kAttenuation = 5.0f;
/// 回転速度
static inline const float kTimeTurn = 0.15f;
/// 移動限界速度
static inline const float kLimitRunSpeed = 2.0f;
/// 重力加速度
static inline const float kGravityAcceleration = 10.0f;
/// 最大落下速度
static inline const float kLimitFallSpeed = 10.0f;
/// ジャンプ初速
static inline const float kJumpAcceleration = 5.0f;
/// キャラクターの当たり判定サイズ
static inline const float kPlayerWidth = 1.0f;
static inline const float kPlayerHeight = 1.0f;
/// 着地時の速度減衰率
static inline const float kAttenuationLanding = 0.00000001f;


class Player :public Object
{
public:
	Player(kEngine* system, const Vector3& position = Vector3{ 0,0,0 });

	void Update(Camera* camera) override;

	// 外部から強制的に速度を与える（ノックバック用）
	void AddKnockback(const Vector3 &force);

	// 衝突判定のために腕の情報を公開
	Vector3 GetArmPositionL() const;
	Vector3 GetArmPositionR() const;
	// 左腕のスケールを取得
	float GetArmScaleL() const { return armScaleL_; }
    // 右腕のスケールを取得
    float GetArmScaleR() const { return armScaleR_; }
    
    Vector3 GetForward() const { return forward_; }

	// ImGuI用
#ifdef USE_IMGUI
    float &GetRefArmScaleL() { return armScaleL_; }
    float &GetRefArmScaleR() { return armScaleR_; }
    float &GetRefDamageCD() { return kDamageCD_; }
    float &GetRefDamageTimer() { return kDamageParameter_; }
    float &GetRefKnockbackCD() { return knockbackCD_; }
    float &GetRefKnockbackTimer() { return knockbackTimer_; }
#endif

    bool IsDamage() { return isDamage_; }
	// シーン側から「今攻撃中？」と聞くための関数(片方で両方連動するかどうか確認する)
	bool IsAttackingR() const { return isAttackingR_; }
	bool IsAttackingL() const { return isAttackingL_; }

private:

	enum class LRDirection { kRight, kLeft, None };

private:

	std::vector<std::function<void()>> function;

private:

	/// マップとの当たり判定情報
	struct CollisionMapInfo {
		bool ceilingHit = false; // 天井に当の当たり判定
		bool floorHit = false;   // 床に当の当たり判定
		bool wallHit = false;    // 壁に当の当たり判定
		Vector3 moveVector = {}; // 移動量
	};

	

private:

	void BehaviorRootUpdate();
	Vector3 CornerPosition(const Vector3& center, Corner4 corner);
	void Move();
	void OnGroundChanger(const CollisionMapInfo& info);
	void MapCollisionDecideDown(CollisionMapInfo& info);
	void MovePlayerByResult(const CollisionMapInfo& info);
	// 腕のスケールを更新する補助関数
	void UpdateArmScale(float &scale, bool isPush);
	// 見た目を更新する
	void UpdateAppearance();

private:
	/////// State
	/// HP
	float HP_ = 3.0f;

	/////// CD
	/// dushCD
	float dushCD_ = 1.0f; ///(秒)
	float dushCDParameter_ = dushCD_ * 60.0f;

	/// Bullet
	float kBulletCD_ = 0.25; ///(秒)
	float kBulletParameter_ = kBulletCD_ * 60.0f;

	/// DamageCD(無敵時間)
	float kDamageCD_ = 1.0f; ///(秒)
	float kDamageParameter_ = kDamageCD_;

	Vector3 forward_;

	// ノックバック中の操作不能時間（秒）
    float knockbackCD_ = 0.5f;
	float knockbackTimer_ = knockbackCD_;

private:

	/// 経過時間

	//////// プレイヤーデータ
	// 左腕のスケール（1.0が通常）
	float armScaleL_ = 1.0f;
	// 右腕のスケール
	float armScaleR_ = 1.0f;
	// 体と腕の間の隙間（スケール1.0の時の距離）
    // 例: 0.1f なら 10cm 離す
	const float kArmBaseGap_ = 0.2f;
	// 現在の移動速度
	float currentSpeed_ = 0.0f;

	/// 移動速度
	Vector3 velocity_ = {};
	/// 方向/開始時の方向
	LRDirection lrDirection_ = LRDirection::kRight;
	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_{};
	// 接地状態フラグ
	bool onGround_ = true;

	float kBlank = 0.0f; // ジャンプ中のブランク時間

private:
	// 計算用
	float deltaTime_{};

private:
	// ダメージフラグ
	bool isDamage_ = false;
	// デスフラグ
	bool isDead_ = false;

	// 攻撃中フラグ
	bool isAttackingR_ = false;
	bool isAttackingL_ = false;
};

