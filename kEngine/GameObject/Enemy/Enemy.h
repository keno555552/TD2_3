#pragma once
#include "Object/Object.h"
#include "Vector3.h"
#include "kEngine.h"

static inline const float kEnemyWidth = 1.0f;

class Enemy : public Object {
public:
    Enemy(kEngine *system, const Vector3 &position);
    void Update(Camera *camera) override;

    void MoveEnemyByResult();

    // 外部から強制的に速度を与える（ノックバック用）
    void AddKnockback(const Vector3 &force);
    // 衝突判定のために腕の情報を公開
    Vector3 GetArmPositionL() const;
    Vector3 GetArmPositionR() const;
    float GetArmScaleL() const { return armScaleL_; }
    float GetArmScaleR() const { return armScaleR_; }

    // 見た目を更新する
    void UpdateAppearance();

    // ImGuI用
#ifdef USE_IMGUI
    float &GetRefArmScaleL() { return armScaleL_; }
    float &GetRefArmScaleR() { return armScaleR_; }
    float &GetRefDamageCD() { return kDamageCD_; }
    float &GetRefDamageTimer() { return kDamageParameter_; }
    float &GetRefKnockbackCD() { return knockbackCD_; }
    float &GetRefKnockbackTimer() { return knockbackTimer_; }
    bool &GetRefIsDamage() { return isDamage_; }
#endif

    // シーン側から「今攻撃中？」と聞くための関数
    bool IsAttackingR() const { return isAttackingR_; }
    bool IsAttackingL() const { return isAttackingL_; }

private:
    void Move(); // 将来AIを入れる場所

private:

    /////// CD

    /// DamageCD(無敵時間)
    float kDamageCD_ = 0.5f; ///(秒)
    float kDamageParameter_ = kDamageCD_;

    // ノックバック中の操作不能時間（秒）
    float knockbackCD_ = 0.5f;
    float knockbackTimer_ = knockbackCD_;

private:
    /// 移動速度
    Vector3 velocity_ = {};

    // プレイヤーと同じパラメータ構造
    float armScaleL_ = 1.0f;
    float armScaleR_ = 1.0f;
    // 体と腕の間の隙間（スケール1.0の時の距離）
    // 例: 0.1f なら 10cm 離す
    const float kArmBaseGap_ = 0.2f;
    // テスト用に片腕だけ最初から大きくしておくフラグなど
    bool isTestExpand_ = true;

private:
    // 計算用
    float deltaTime_{};

private:
    // ダメージフラグ
    bool isDamage_ = false;
    // 攻撃中フラグ
    bool isAttackingR_ = false;
    bool isAttackingL_ = false;
};
