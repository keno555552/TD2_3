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

    // シーン側から「今攻撃中？」と聞くための関数
    bool IsAttackingR() const { return isAttackingR_; }
    bool IsAttackingL() const { return isAttackingL_; }

private:
    void Move(); // 将来AIを入れる場所

private:
    // ノックバック中の操作不能時間（秒）
    float knockbackTimer_ = 0.0f;

    /// 移動速度
    Vector3 velocity_ = {};

    // プレイヤーと同じパラメータ構造
    float armScaleL_ = 1.0f;
    float armScaleR_ = 1.0f;
    // テスト用に片腕だけ最初から大きくしておくフラグなど
    bool isTestExpand_ = true;

private:
    // 計算用
    float deltaTime_{};

private:
    // 攻撃中フラグ
    bool isAttackingR_ = false;
    bool isAttackingL_ = false;
};
