#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Enemy.h"
#include <numbers>

Enemy::Enemy(kEngine *system, const Vector3 &position) {
  Object::IntObject(system);
  Object::CreateDefaultData();
  mainPosition.transform = CreateDefaultTransform();
  mainPosition.transform.translate = position;
  objectParts_[0].materialConfig->lightModelType = LightModelType::HalfLambert;

  dashWindEffectL_ = new P_DashWind(system_);
  dashWindEffectR_ = new P_DashWind(system_);

  suctionEffect_ = new P_Suction(system_);

  planeReductionEffectL_ = new P_PlaneReduction(system_);
  planeReductionEffectL_->CreateModelData(system_->SetModelObj("resources/TemplateResource/object/plane/plane.obj"));
  planeReductionEffectR_ = new P_PlaneReduction(system_);
  planeReductionEffectR_->CreateModelData(system_->SetModelObj("resources/TemplateResource/object/plane/plane.obj"));
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
    velocity_ = {0, 0, 0};

    // ここに将来Move()などのAI移動が入ります
    // 移動・挙動更新
    Move();
  }

  forward_ = { -std::sin(mainPosition.transform.rotate.y), 0.0f,
                -std::cos(mainPosition.transform.rotate.y) };

  MoveEnemyByResult();

  // パーティクル
  dashWindEffectL_->SetGenerationPos(GetArmPositionL() + forward_/2.0f + Vector3(0.0f, GetArmScaleL() - 0.5f, 0.0f));
  dashWindEffectL_->SetTargetForward(forward_);
  dashWindEffectL_->SetArmScale(GetArmScaleL());
  dashWindEffectL_->Update(camera);
  dashWindEffectL_->SetEmitting(false);
  dashWindEffectL_->SetEmitting(true);

  if (IsAttackingL()) {
      dashWindEffectL_->SetEmitting(true);
  }

  dashWindEffectR_->SetGenerationPos(GetArmPositionR() + forward_/2.0f + Vector3(0.0f, GetArmScaleR() - 0.5f, 0.0f));
  dashWindEffectR_->SetTargetForward(forward_);
  dashWindEffectR_->SetArmScale(GetArmScaleR());
  dashWindEffectR_->Update(camera);
  dashWindEffectR_->SetEmitting(false);
  dashWindEffectR_->SetEmitting(true);

  if (IsAttackingR()) {
      dashWindEffectR_->SetEmitting(true);
  }

  suctionEffect_->SetArmScaleL(GetArmScaleL());
  suctionEffect_->SetTargetPosLeft(GetArmPositionL());
  suctionEffect_->SetTargetPosRight(GetArmPositionR());
  suctionEffect_->SetLeftEmitting(false);
  suctionEffect_->SetRightEmitting(false);

  suctionEffect_->SetLeftEmitting(true);
  suctionEffect_->SetRightEmitting(true);
  if (system_->GetIsPush(DIK_A)) {
      //suctionEffect_->SetLeftEmitting(true);
  }
  if (system_->GetIsPush(DIK_D)) {
      //suctionEffect_->SetRightEmitting(true);
  }

  suctionEffect_->Update(camera);


  planeReductionEffectL_->SetIsAction(system_->GetIsPush(DIK_A));
  planeReductionEffectL_->SetTargetPos(GetArmPositionL());
  planeReductionEffectL_->SetTargetScale(GetArmScaleL());
  planeReductionEffectL_->Update(camera);

  planeReductionEffectR_->SetIsAction(system_->GetIsPush(DIK_D));
  planeReductionEffectR_->SetTargetPos(GetArmPositionR());
  planeReductionEffectR_->SetTargetScale(GetArmScaleR());
  planeReductionEffectR_->Update(camera);

  // 親クラスの更新（描画など）
  Object::Update(camera);
}

void Enemy::Draw() {
    dashWindEffectL_->Draw();
    dashWindEffectR_->Draw();
    suctionEffect_->Draw();

    if (planeReductionEffectL_->GetIsActive())planeReductionEffectL_->Draw();
    if (planeReductionEffectR_->GetIsActive())planeReductionEffectR_->Draw();

    Object::Draw();
}

void Enemy::MoveEnemyByResult() {
  mainPosition.transform.translate.x += velocity_.x * deltaTime_;
  mainPosition.transform.translate.y += velocity_.y * deltaTime_;
  mainPosition.transform.translate.z += velocity_.z * deltaTime_;
}

void Enemy::Move() {
  // --- 仮の実装：AIはまだ無いので、停止状態で腕の状態だけ更新する ---

  // ダメージ無効化タイマー更新
  if (isDamage_) {
    // ダメージ無効化中はタイマーを減少させる
    kDamageParameter_ -= deltaTime_;
    if (kDamageParameter_ <= 0.0f) {
      isDamage_ = false;
      kDamageParameter_ = kDamageCD_;
    }
  }

  if (armScaleR_ > 1.0f) {
    isAttackingR_ = true;
  } else {
      isAttackingR_ = false;
  }
  if (armScaleL_ > 1.0f) {
    isAttackingL_ = true;
  } else {
      isAttackingL_ = false;
  }

  float currentTurnSpeed = 0.04f;

  // 旋回速度の計算：基本速度 + (膨張分 * 係数)
  // 腕が大きくなるほど、より鋭く旋回します
  float turnSpeed = currentTurnSpeed;
  mainPosition.transform.rotate.y += turnSpeed;

  // テスト: 常に右腕だけ膨らませておく（衝突確認用）
  // if (isTestExpand_) {
  //    armScaleR_ = 3.0f; // 大きくしておく
  //  isAttackingR_ = true; // 攻撃中フラグON
  //}

  mainPosition.transform.scale.y = mainPosition.transform.scale.x;
  mainPosition.transform.scale.z = mainPosition.transform.scale.x;

  // 見た目更新
  UpdateAppearance();
}

void Enemy::AddKnockback(const Vector3 &force) {
  if (isDamage_) {
    // ダメージ無効化中はノックバックを受け付けない
    return;
  }
  // 現在の速度に衝撃を加算
  velocity_.x += force.x;
  velocity_.z += force.z;

  // 0.5秒間ほど操作入力を受け付けず、慣性で飛ばされるようにする
  knockbackTimer_ = knockbackCD_;
  // ノックバック中は腕の攻撃も無効化
  isAttackingR_ = false;
  isAttackingL_ = false;
  // ノックバック中はダメージも受け付けない
  isDamage_ = true;
}

// 腕のワールド座標を返す（衝突判定用）
Vector3 Enemy::GetArmPositionL() const {
    // 本体座標と回転量を取得
    Vector3 centerPos = mainPosition.transform.translate;
    float rotY = mainPosition.transform.rotate.y;

    // 1. 親（全体）のスケール
    float totalScale = mainPosition.transform.scale.x;

    // 2. ワールド空間での距離を積み上げる
    // (a) 体の見た目の半径 = 0.5 * 0.5 = 0.25
    float worldBodyRadius = (kEnemyWidth / 2.0f) * totalScale;

    // (b) 腕の見た目の半径 = (1.0 * armScale) * 0.5 / 2.0 ... ではなく
    //     腕パーツ自体の大きさは armScale、それを親が totalScale倍する
    //     -> 半径 0.5 * armScale * totalScale
    float worldArmRadius = 0.5f * armScaleL_ * totalScale;

    // (c) 隙間 (ワールド空間では固定値にしたい！)
    float worldGap = kArmBaseGap_;

    // 3. 全て足す
    // 中心から、(体) + (隙間) + (腕半径) の距離にあるのが「腕の中心」
    float worldOffsetDist = worldBodyRadius + worldGap + worldArmRadius;

    // 4. 回転計算
    float offsetX = +worldOffsetDist; // 左腕はプラス方向（モデルによる）
    float offsetZ = 0.0f;

    // 回転行列の要領でオフセットを回転させる
    // (X, Z) を Y軸中心に回転
    float rotatedX = offsetX * std::cos(rotY) + offsetZ * std::sin(rotY);
    float rotatedZ = -offsetX * std::sin(rotY) + offsetZ * std::cos(rotY);

    // 本体座標に加算して返す
    return { centerPos.x + rotatedX,
            centerPos.y, // 高さは本体と同じと仮定（必要ならオフセットを加算）
            centerPos.z + rotatedZ };
}

Vector3 Enemy::GetArmPositionR() const {
    // 本体座標と回転量を取得
    Vector3 centerPos = mainPosition.transform.translate;
    float rotY = mainPosition.transform.rotate.y;

    // 1. 親（全体）のスケール
    float totalScale = mainPosition.transform.scale.x;

    // 2. ワールド空間での距離を積み上げる
    // (a) 体の見た目の半径 = 0.5 * 0.5 = 0.25
    float worldBodyRadius = (kEnemyWidth / 2.0f) * totalScale;

    // (b) 腕の見た目の半径 = (1.0 * armScale) * 0.5 / 2.0 ... ではなく
    //     腕パーツ自体の大きさは armScale、それを親が totalScale倍する
    //     -> 半径 0.5 * armScale * totalScale
    float worldArmRadius = 0.5f * armScaleR_ * totalScale;

    // (c) 隙間 (ワールド空間では固定値にしたい！)
    float worldGap = kArmBaseGap_;

    // 3. 全て足す
    // 中心から、(体) + (隙間) + (腕半径) の距離にあるのが「腕の中心」
    float worldOffsetDist = worldBodyRadius + worldGap + worldArmRadius;

    // 4. 回転計算
    float offsetX = -worldOffsetDist; // 右腕はマイナス方向（モデルによる）
    float offsetZ = 0.0f;

    // 回転行列の要領でオフセットを回転させる
    // (X, Z) を Y軸中心に回転
    float rotatedX = offsetX * std::cos(rotY) + offsetZ * std::sin(rotY);
    float rotatedZ = -offsetX * std::sin(rotY) + offsetZ * std::cos(rotY);

    // 本体座標に加算して返す
    return { centerPos.x + rotatedX,
            centerPos.y, // 高さは本体と同じと仮定（必要ならオフセットを加算）
            centerPos.z + rotatedZ };
}

void Enemy::UpdateAppearance() {
    // 1. 親（全体）のスケールを取得
    float totalScale = mainPosition.transform.scale.x;

    // 2. 「固定したい隙間」をローカル値に変換する
    // ワールドで kArmBaseGap(例:0.1) 離したいなら、ローカルでは 0.1 / 0.5 = 0.2 離す必要がある
    float localGap = kArmBaseGap_ / totalScale;

    // 3. ローカル座標でのオフセット計算
    // kPlayerWidth は元々ローカル定義(1.0)なので、そのまま使います。

    // ボディ半幅(0.5) + 補正済み隙間
    float localOffsetR = (kEnemyWidth / 2.0f) + localGap;
    float localOffsetL = (kEnemyWidth / 2.0f) + localGap;

    // 4. 適用
    // 右腕
    objectParts_[1].transform.scale = { armScaleR_, armScaleR_, armScaleR_ };
    objectParts_[1].transform.translate =
        objectParts_[0].transform.translate + Vector3{ -localOffsetR, 0.0f, 0.0f };
    // 左腕
    objectParts_[2].transform.scale = { armScaleL_, armScaleL_, armScaleL_ };
    objectParts_[2].transform.translate =
        objectParts_[0].transform.translate + Vector3{ +localOffsetL, 0.0f, 0.0f };
}