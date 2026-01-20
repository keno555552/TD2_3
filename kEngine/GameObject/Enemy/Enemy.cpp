#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Enemy.h"
#include <numbers>

Enemy::Enemy(kEngine *system, const Vector3 &position) {
    Object::IntObject(system);
    Object::CreateDefaultData();
    mainPosition.transform = CreateDefaultTransform();
    mainPosition.transform.translate = position;
    objectParts_[0].materialConfig->lightModelType =
        LightModelType::HalfLambert;

    // 敵だとわかるように色を変える
    objectParts_[0].materialConfig->textureColor = {1.0f, 0.5f, 1.0f, 1.0f}; 
}

void Enemy::Update(Camera *camera) {
    // 1. エンジンから値を取得
    float rawDeltaTime = system_->GetDeltaTime();

    // 2. 異常な値をカットする（最大でも 0.1秒 = 10FPS相当 に制限）
    // これにより、起動直後やデバッグ中の巨大な跳ね上がりを無視できます
    deltaTime_ = std::min(rawDeltaTime, 0.1f);

    // ノックバック処理
    if (knockbackTimer_ > 0.0f) {
        knockbackTimer_ -= deltaTime_;
        // 減衰（摩擦）
        velocity_.x *= 0.90f;
        velocity_.z *= 0.90f;
    } else {
        // 通常時は速度をリセット（AI移動が入るまでは停止）
        velocity_ = { 0, 0, 0 };

        // ここに将来Move()などのAI移動が入ります
        // 移動・挙動更新
        Move();
    }
    
    MoveEnemyByResult();

    // 親クラスの更新（描画など）
    Object::Update(camera);
}

void Enemy::MoveEnemyByResult() {
    mainPosition.transform.translate.x += velocity_.x * deltaTime_;
    mainPosition.transform.translate.y += velocity_.y * deltaTime_;
    mainPosition.transform.translate.z += velocity_.z * deltaTime_;
}

void Enemy::Move() {
    // --- 仮の実装：AIはまだ無いので、停止状態で腕の状態だけ更新する ---

    float currentTurnSpeed = 0.04f;

    // Aボタン（左腕膨張 = 左の推進力UP）
    // 左が前に出るので、機体は「右」に回る (Y回転プラス)

    // 旋回速度の計算：基本速度 + (膨張分 * 係数)
    // 腕が大きくなるほど、より鋭く旋回します
    // 係数(0.05fなど)は調整してください
    float turnSpeed = currentTurnSpeed;
    mainPosition.transform.rotate.y += turnSpeed;

    // テスト: 常に右腕だけ膨らませておく（衝突確認用）
    //if (isTestExpand_) {
    //    armScaleR_ = 3.0f; // 大きくしておく
    //  isAttackingR_ = true; // 攻撃中フラグON
    //}

    // 見た目の更新（Player::Moveと同じロジックでパーツを同期）
    // 右腕 (Index 1)
    objectParts_[1].transform.scale = { armScaleR_, armScaleR_, armScaleR_ };
    objectParts_[1].transform.translate =
        objectParts_[0].transform.translate + Vector3{ -(kEnemyWidth / 1.2f), 0.0f, 0.0f };

    // 左腕 (Index 2)
    objectParts_[2].transform.scale = { armScaleL_, armScaleL_, armScaleL_ };
    objectParts_[2].transform.translate =
        objectParts_[0].transform.translate + Vector3{ +(kEnemyWidth / 1.2f), 0.0f, 0.0f };

}

void Enemy::AddKnockback(const Vector3 &force) {
    // 現在の速度に衝撃を加算
    velocity_.x += force.x;
    velocity_.z += force.z;

    // 0.5秒間ほど操作入力を受け付けず、慣性で飛ばされるようにする
    knockbackTimer_ = 0.5f;
}

// 腕のワールド座標を返す（衝突判定用）
Vector3 Enemy::GetArmPositionL() const {
    // 敵の本体座標と回転量を取得
    Vector3 centerPos = mainPosition.transform.translate;
    float rotY = mainPosition.transform.rotate.y;

    // ローカル空間での腕の位置（オフセット）
    // 左腕は X軸プラス方向
    float offsetX = +(kEnemyWidth / 1.2f);
    float offsetZ = 0.0f;

    // 回転行列の要領でオフセットを回転させる
    // (X, Z) を Y軸中心に回転
    float rotatedX = offsetX * std::cos(rotY) + offsetZ * std::sin(rotY);
    float rotatedZ = -offsetX * std::sin(rotY) + offsetZ * std::cos(rotY);

    // 本体座標に加算して返す
    return {
        centerPos.x + rotatedX,
        centerPos.y, // 高さは本体と同じと仮定（必要ならオフセットを加算）
        centerPos.z + rotatedZ
    };
}
Vector3 Enemy::GetArmPositionR() const {
    // 敵の本体座標と回転量を取得
    Vector3 centerPos = mainPosition.transform.translate;
    float rotY = mainPosition.transform.rotate.y;

    // ローカル空間での腕の位置（オフセット）
    // 右腕は X軸マイナス方向
    float offsetX = -(kEnemyWidth / 1.2f);
    float offsetZ = 0.0f; // 腕は真横にあるのでZはずれない

    // 回転行列の要領でオフセットを回転させる
    // (X, Z) を Y軸中心に回転
    float rotatedX = offsetX * std::cos(rotY) + offsetZ * std::sin(rotY);
    float rotatedZ = -offsetX * std::sin(rotY) + offsetZ * std::cos(rotY);

    // 本体座標に加算して返す
    return {
        centerPos.x + rotatedX,
        centerPos.y, // 高さは本体と同じと仮定（必要ならオフセットを加算）
        centerPos.z + rotatedZ
    };
}