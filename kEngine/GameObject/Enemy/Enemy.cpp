#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Enemy.h"
#include <numbers>

Enemy::Enemy(kEngine* system, const Vector3& position) {
	Object::IntObject(system);
	Object::CreateDefaultData();
	mainPosition.transform = CreateDefaultTransform();
	mainPosition.transform.translate = position;
	objectParts_[0].materialConfig->lightModelType = LightModelType::HalfLambert;

	currentSpeed_ = 2.0f;

	// ■■■ 新しい腕の生成 ■■■
	  // 第2引数に「自分(this)」を渡して追従させる
	armL_ = new Arm(system, this, false); // 左腕
	armR_ = new Arm(system, this, true);  // 右腕

	// デバッグ用オブジェクトの生成
	leftHandDebugObject_ = new DebugObject(system_);
	leftHandDebugObject_->SetShowNumber(true);

	rightHandDebugObject_ = new DebugObject(system_);
	rightHandDebugObject_->SetShowNumber(true);

#pragma region パーティクル
	dashWindEffectL_ = new P_DashWind(system_);
	dashWindEffectR_ = new P_DashWind(system_);

	suctionEffect_ = new P_Suction(system_);

	planeReductionEffectL_ = new P_PlaneReduction(system_);
	planeReductionEffectL_->CreateModelData(system_->SetModelObj("resources/game/model/circle.obj"));
	planeReductionEffectR_ = new P_PlaneReduction(system_);
	planeReductionEffectR_->CreateModelData(system_->SetModelObj("resources/game/model/circle.obj"));
#pragma endregion
}

Enemy::~Enemy() {
	delete armL_;
	delete armR_;
	delete leftHandDebugObject_;
	delete rightHandDebugObject_;
}

void Enemy::Update(Camera *camera) {
  // 1. エンジンから値を取得
  float rawDeltaTime = system_->GetScaledDeltaTime();

	// 2. 異常な値をカットする（最大でも 0.1秒 = 10FPS相当 に制限）
	// これにより、起動直後やデバッグ中の巨大な跳ね上がりを無視できます
	deltaTime_ = std::min(rawDeltaTime, 0.1f);

	forward_ = { -std::sin(mainPosition.transform.rotate.y), 0.0f,
				  -std::cos(mainPosition.transform.rotate.y) };

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

	if (isTutorial_ && !isDamage_) {
		velocity_ = { 0, 0, 0 };
	}

	MoveEnemyByResult();

#pragma region パーティクル

	if(distanceWithPlayer_ <= 25.0f) // プレイヤーからの距離が遠ければ処理しない
	{
		/* 攻撃時のダッシュエフェクト */
		dashWindEffectL_->SetGenerationPos(GetArmPositionL() + forward_ / 2.0f + Vector3(0.0f, GetArmScaleL() - 0.5f, 0.0f));
		dashWindEffectL_->SetGenerationPos(GetArmPositionL() + forward_ * GetArmScaleL());
		dashWindEffectL_->SetTargetForward(forward_);
		dashWindEffectL_->SetArmScale(GetArmScaleL());
		dashWindEffectL_->Update(camera);
		dashWindEffectL_->SetEmitting(false);

		if (IsAttackingL()) { // アタックしているときにダッシュエフェクト
			dashWindEffectL_->SetEmitting(true);
		}

		dashWindEffectR_->SetGenerationPos(GetArmPositionR() + forward_ / 2.0f + Vector3(0.0f, GetArmScaleR() - 0.5f, 0.0f));
		dashWindEffectR_->SetGenerationPos(GetArmPositionR() + forward_ * GetArmScaleR());
		dashWindEffectR_->SetTargetForward(forward_);
		dashWindEffectR_->SetArmScale(GetArmScaleR());
		dashWindEffectR_->Update(camera);
		dashWindEffectR_->SetEmitting(false);

		if (IsAttackingR()) { // アタックしているときにダッシュエフェクト
			dashWindEffectR_->SetEmitting(true);
		}

		/* チャージ時の吸い込みエフェクト */
		suctionEffect_->SetArmScaleL(GetArmScaleL());
		suctionEffect_->SetTargetPosLeft(GetArmPositionL());
		suctionEffect_->SetTargetPosRight(GetArmPositionR());
		suctionEffect_->SetLeftEmitting(false);
		suctionEffect_->SetRightEmitting(false);
		if (!isAttackingL_ && armScaleL_ > 1.0f) { // 攻撃状態ではない&&スケールが1.0f以上(つまりチャージ中のとき)
			suctionEffect_->SetLeftEmitting(true);
		}
		if (!isAttackingR_ && armScaleR_ > 1.0f) { // 攻撃状態ではない&&スケールが1.0f以上(つまりチャージ中のとき)
			suctionEffect_->SetRightEmitting(true);
		}
		suctionEffect_->Update(camera);

		/* チャージ時の地面での円エフェクト */
		if (!isAttackingL_ && armScaleL_ > 1.0f) {
			planeReductionEffectL_->SetIsAction(true);
		}
		planeReductionEffectL_->SetTargetPos(GetArmPositionL());
		planeReductionEffectL_->SetTargetScale(GetArmScaleL());
		planeReductionEffectL_->Update(camera);
		if (!isAttackingR_ && armScaleR_ > 1.0f) {
			planeReductionEffectR_->SetIsAction(true);
		}
		planeReductionEffectR_->SetTargetPos(GetArmPositionR());
		planeReductionEffectR_->SetTargetScale(GetArmScaleR());
		planeReductionEffectR_->Update(camera);
	}
#pragma endregion

	// 親クラスの更新（描画など）
	Object::Update(camera);

	// ■■■ 腕の更新 ■■■
	 // ここで現在の「倍率(armScaleL_/R_)」を渡すだけ！
	if (armL_) {
		leftHandDebugObject_->mainPosition.transform.translate = GetArmPositionL();
		leftHandDebugObject_->UpdateShowNumber(static_cast<int>(armScaleL_ * 10.0f));
		leftHandDebugObject_->Update(camera);
	}
	if (armR_) {
		armR_->Update(camera, armScaleR_);
		rightHandDebugObject_->mainPosition.transform.translate = GetArmPositionR();
		rightHandDebugObject_->UpdateShowNumber(static_cast<int>(armScaleR_ * 10.0f));
		rightHandDebugObject_->Update(camera);
	}
}

void Enemy::Draw() {
	if (distanceWithPlayer_ <= 25.0f)
	{
		dashWindEffectL_->Draw();
		dashWindEffectR_->Draw();
		suctionEffect_->Draw();

		if (planeReductionEffectL_->GetIsActive())planeReductionEffectL_->Draw();
		if (planeReductionEffectR_->GetIsActive())planeReductionEffectR_->Draw();
	}

	Object::Draw();

	// 腕の描画
	if (armL_) {
		armL_->Draw();
		leftHandDebugObject_->Draw();
	}
	if (armR_) {
		armR_->Draw();
		rightHandDebugObject_->Draw();
	}
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


	// 現在の「最も膨らんでいる腕」の余剰分（1.0を超えている分）を計算
	float maxExtraScale = std::max(armScaleL_, armScaleR_) - 1.0f;
	if (maxExtraScale > 0.0f) {
		// 腕が縮んでいる最中は、膨らみ具合に比例してブーストをかける
		// 1.0(最大) ～ 0.0(縮みきり) の割合を算出
		float boostRatio = maxExtraScale / (30.0f - 1.0f);
		currentSpeed_ = kBaseSpeed + (4.0f * boostRatio);
	} else {
		currentSpeed_ = kBaseSpeed;
	}

	// 攻撃フラグ管理
	if (armScaleR_ > 1.01f) {
		isAttackingR_ = true;
	} else {
		isAttackingR_ = false;
	}
	if (armScaleL_ > 1.01f) {
		isAttackingL_ = true;
	} else {
		isAttackingL_ = false;
	}

	isCharging_ = false;

	if (isTargetCharging_) {
		// 自身から見て右か左か
		Vector3 toTarget = targetPos_ - mainPosition.transform.translate;
		toTarget.y = 0.0f;
		toTarget = Normalize(toTarget);
		forward_.y = 0.0f;
		forward_ = Normalize(forward_);
		float crossY = Cross(forward_, toTarget).y;

		constexpr float kTurnEpsilon = 0.2f; // 許容範囲

		if (crossY > kTurnEpsilon) { // 左

			armScaleL_ += 0.3f * deltaTime_;

			float turnSpeed = 0.01f + (armScaleL_ - 1.0f) * 0.02f;
			turnSpeed = std::min(turnSpeed, 0.01f * 10.0f);
			mainPosition.transform.rotate.y += turnSpeed;

			// 攻撃判定解除
			isAttackingL_ = false;
		} else {
			armScaleL_ -= 0.4f * deltaTime_;
			if (armScaleL_ <= 1.0f) {
				armScaleL_ = 1.0f;
			}
		}
		if (crossY < -kTurnEpsilon) { // 右

			armScaleR_ += 0.3f * deltaTime_;

			float turnSpeed = 0.01f + (armScaleR_ - 1.0f) * 0.02f;
			turnSpeed = std::min(turnSpeed, 0.01f * 10.0f);
			mainPosition.transform.rotate.y -= turnSpeed;

			// 攻撃判定解除
			isAttackingR_ = false;
		} else {
			armScaleR_ -= 0.4f * deltaTime_;
			if (armScaleR_ <= 1.0f) {
				armScaleR_ = 1.0f;
			}
		}
	} else {
		armScaleL_ += 0.3f * deltaTime_;
		isAttackingL_ = false;
		armScaleR_ += 0.3f * deltaTime_;
		isAttackingR_ = false;
		isCharging_ = true;
	}
	armScaleL_ = std::clamp(armScaleL_, 1.0f, 30.0f);
	armScaleR_ = std::clamp(armScaleR_, 1.0f, 30.0f);


	mainPosition.transform.scale.y = mainPosition.transform.scale.x;
	mainPosition.transform.scale.z = mainPosition.transform.scale.x;

	velocity_ = forward_ * currentSpeed_; // 速度を正面に設定

	// 見た目更新(仮の体部分のため)
	UpdateAppearance();
}

void Enemy::AddKnockback(const Vector3& force) {
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

void Enemy::ResetArm(bool isRight) {
	//if (isDamage_) {
	//    // ダメージ無効化中はノックバックを受け付けない
	//    return;
	//}
	if (isRight) {
		armScaleR_ = 1.0f;
	} else {
		armScaleL_ = 1.0f;
	}
}

// 腕のワールド座標を返す（衝突判定用）
Vector3 Enemy::GetArmPositionL() const {
	// 定数は同じ
	const float kModelBaseScale = 0.09f;
	const float kBodyAttachmentX = 0.8f;
	const float kArmOriginalWidth = 11.4f;

	// 左腕のスケールを使用
	float finalScale = kModelBaseScale * armScaleL_;
	float currentHalfWidth = (kArmOriginalWidth * finalScale) / 2.0f;
	float dist = kBodyAttachmentX + currentHalfWidth;

	// プレイヤー(親)のスケールを取得して、距離に掛ける
	// これで「見た目」の配置距離と一致します
	float parentScale = mainPosition.transform.scale.x;
	dist *= parentScale;

	// 左腕はローカル座標で「プラスX方向」
	float localX = +dist;
	float localZ = 0.0f;

	float rotY = mainPosition.transform.rotate.y;
	Vector3 centerPos = mainPosition.transform.translate;

	float worldX = centerPos.x + (localX * std::cos(rotY) + localZ * std::sin(rotY));
	float worldZ = centerPos.z + (-localX * std::sin(rotY) + localZ * std::cos(rotY));
	float worldY = centerPos.y;

	return Vector3{ worldX, worldY, worldZ };
}

void Enemy::OnWallHit(const Vector3& normal) {
    // X方向の壁（左右）
    if (normal.x != 0.0f) {
        if ((normal.x > 0 && velocity_.x < 0) || (normal.x < 0 && velocity_.x > 0)) {
            velocity_.x = 0.0f;
        }
    }
    // Z方向の壁（奥・手前）
    if (normal.z != 0.0f) {
        if ((normal.z > 0 && velocity_.z < 0) || (normal.z < 0 && velocity_.z > 0)) {
            velocity_.z = 0.0f;
        }
    }
}

Vector3 Enemy::GetArmPositionR() const {
	// 1. Arm.h と同じ定数を用意する（★ここがズレると判定もズレます！）
	   // 本当は共通ヘッダー(Const.hなど)に置くのがベストですが、今はここに書きます
	const float kModelBaseScale = 0.09f;    // モデルの縮小率
	const float kBodyAttachmentX = 0.8f;   // 肩の付け根の位置
	const float kArmOriginalWidth = 11.4f;  // モデルの元々の幅

	// 2. Arm::Update と全く同じ計算で「腕の中心までの距離」を出す

	// 現在のゲーム倍率を掛けて、最終的なスケールを出す
	float finalScale = kModelBaseScale * armScaleR_;

	// 現在の腕の半径（長さの半分）を計算
	float currentHalfWidth = (kArmOriginalWidth * finalScale) / 2.0f;

	// 「付け根」+「半径」=「中心までの距離」
	float dist = kBodyAttachmentX + currentHalfWidth;

	// プレイヤー(親)のスケールを取得して、距離に掛ける
	// これで「見た目」の配置距離と一致します
	float parentScale = mainPosition.transform.scale.x;
	dist *= parentScale;

	// 3. ワールド座標への回転変換
	// 右腕はローカル座標で「マイナスX方向」にあるとします
	float localX = -dist;
	float localZ = 0.0f;

	// プレイヤーの回転(Y軸)に合わせて回す
	float rotY = mainPosition.transform.rotate.y;
	Vector3 centerPos = mainPosition.transform.translate;

	float worldX = centerPos.x + (localX * std::cos(rotY) + localZ * std::sin(rotY));
	float worldZ = centerPos.z + (-localX * std::sin(rotY) + localZ * std::cos(rotY));

	// 高さはとりあえず体の中心と同じにしておく（必要ならオフセットを加算）
	float worldY = centerPos.y;

	return Vector3{ worldX, worldY, worldZ };
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
	objectParts_[1].transform.scale = { 0, 0, 0 };
	objectParts_[1].transform.translate =
		objectParts_[0].transform.translate + Vector3{ -localOffsetR, 0.0f, 0.0f };
	// 左腕
	objectParts_[2].transform.scale = { 0, 0, 0 };
	objectParts_[2].transform.translate =
		objectParts_[0].transform.translate + Vector3{ +localOffsetL, 0.0f, 0.0f };
}