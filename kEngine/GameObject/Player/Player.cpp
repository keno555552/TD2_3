#define NOMINMAX ///<<<必ずすべでのファイルの先頭に書くこと
#include "Player.h"
#include <algorithm>
#include <numbers>

Player::Player(kEngine* system, const Vector3& position) {
	Object::IntObject(system);
	Object::CreateDefaultData();
	mainPosition.transform = CreateDefaultTransform();
	mainPosition.transform.translate = position;
	objectParts_[0].materialConfig->lightModelType = LightModelType::HalfLambert;

	mainPosition.transform.rotate.y =
            std::numbers::pi_v<float>; // 初期向きをZマイナス方向に
}

void Player::Update(Camera* camera) {
	// 1. エンジンから値を取得
	float rawDeltaTime = system_->GetDeltaTime();

	// 2. 異常な値をカットする（最大でも 0.1秒 = 10FPS相当 に制限）
	// これにより、起動直後やデバッグ中の巨大な跳ね上がりを無視できます
	deltaTime_ = std::min(rawDeltaTime, 0.1f);
	BehaviorRootUpdate();

	Object::Update(camera);
}

void Player::AddKnockback(const Vector3 &force) {
	// 現在の速度に衝撃を加算
	velocity_.x += force.x;
	velocity_.z += force.z;

	// 接地フラグを折って、摩擦で即停止しないようにする
	onGround_ = false;

	// 0.5秒間ほど操作入力を受け付けず、慣性で飛ばされるようにする
	knockbackTimer_ = 0.5f;
}

void Player::BehaviorRootUpdate() {
	/// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	/// 移動量に速度の値をコピー
	collisionMapInfo.moveVector = velocity_;

	/// マップ衝突チェック
	Move();
	MapCollisionDecideDown(collisionMapInfo);
	MovePlayerByResult(collisionMapInfo);
	OnGroundChanger(collisionMapInfo);
}

Vector3 Player::CornerPosition(const Vector3& center, Corner4 corner) {
	Vector3 offsetTable[ (int)Corner4::kNumCorner] = {
		{+kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0}, // kRightBottom
		{-kPlayerWidth / 2.0f, -kPlayerHeight / 2.0f, 0}, // kLeftBottom
		{+kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0}, // kRightTop
		{-kPlayerWidth / 2.0f, +kPlayerHeight / 2.0f, 0}, // kLeftTop
	};

	Vector3 result;
	result.x = center.x + offsetTable[static_cast<uint32_t>(corner)].x;
	result.y = center.y + offsetTable[static_cast<uint32_t>(corner)].y;
	result.z = center.z + offsetTable[static_cast<uint32_t>(corner)].z;

	return result;
}

void Player::Move() {
	// 入力状態の取得
	bool pushA = system_->GetIsPush(DIK_A);
	bool pushD = system_->GetIsPush(DIK_D);

	// 腕の膨張・収縮処理
	// Aキーで左腕、Dキーで右腕を膨らませる
	UpdateArmScale(armScaleL_, pushA);
	UpdateArmScale(armScaleR_, pushD);

	// ノックバック中かどうかで処理を分ける
	if (knockbackTimer_ > 0.0f) {
		// --- 吹っ飛び中 ---
		knockbackTimer_ -= deltaTime_;

		// 空気抵抗（減衰）をかけて、徐々に遅くする
		velocity_.x *= 0.95f;
		velocity_.z *= 0.95f;
	} else {
		// 旋回と速度の計算
		if (pushA && pushD) {
			// 【停止】両方膨らませると急ブレーキ
			currentSpeed_ *= 0.8f;
		} else if (!pushA && !pushD) {
			// 【加速】ボタンを離した際、腕が縮みながら加速
			// 現在の「最も膨らんでいる腕」の余剰分（1.0を超えている分）を計算
			float maxExtraScale = std::max(armScaleL_, armScaleR_) - 1.0f;
			if (maxExtraScale > 0.0f) {
				// 腕が縮んでいる最中は、膨らみ具合に比例してブーストをかける
				// 1.0(最大) ～ 0.0(縮みきり) の割合を算出
				float boostRatio = maxExtraScale / (kMaxArmScale - 1.0f);
				currentSpeed_ = kBaseSpeed + (kBoostPower * boostRatio);
			} else {
				currentSpeed_ = kBaseSpeed;
			}
		} else {
			// 【旋回】片方のボタンだけ押されている状態

			// 旋回力を固定値、または腕のサイズを加味して計算
			// (scaleを足すことで、腕が大きいほど急旋回できるようになります)
			float currentTurnSpeed = kTurnSpeed;

			if (pushA) {
				// Aボタン（左腕膨張 = 左の推進力UP）
				// 左が前に出るので、機体は「右」に回る (Y回転プラス)

				// 旋回速度の計算：基本速度 + (膨張分 * 係数)
				// 腕が大きくなるほど、より鋭く旋回します
				// 係数(0.05fなど)は調整してください
				float turnSpeed = kTurnSpeed + (armScaleL_ - 1.0f) * 0.02f;
				mainPosition.transform.rotate.y += turnSpeed;
			} else if (pushD) {
				// Dボタン（右腕膨張 = 右の推進力UP）
				// 右が前に出るので、機体は「左」に回る (Y回転マイナス)

				float turnSpeed = kTurnSpeed + (armScaleR_ - 1.0f) * 0.02f;
				mainPosition.transform.rotate.y -= turnSpeed;
			}
			currentSpeed_ = kBaseSpeed;
		}

		// 進行方向への移動計算
		// Y軸回転から進行方向ベクトルを計算 (3Dの場合)
		forward_ = {
			-std::sin(mainPosition.transform.rotate.y),
			0.0f,
			-std::cos(mainPosition.transform.rotate.y)
		};

		velocity_.x = forward_.x * currentSpeed_;
		velocity_.z = forward_.z * currentSpeed_;

		// 見た目の更新（どの状態でも毎フレーム実行する）
		// 右腕 (Index 1)
		objectParts_[1].transform.scale = { armScaleR_, armScaleR_, armScaleR_ };
		objectParts_[1].transform.translate =
			objectParts_[0].transform.translate + Vector3{ -(kPlayerWidth / 1.2f), 0.0f, 0.0f };

		// 左腕 (Index 2)
		objectParts_[2].transform.scale = { armScaleL_, armScaleL_, armScaleL_ };
		objectParts_[2].transform.translate =
			objectParts_[0].transform.translate + Vector3{ +(kPlayerWidth / 1.2f), 0.0f, 0.0f };
	}
}

void Player::OnGroundChanger(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			//MapChipType mapChipType;
			//MapChipField::IndexSet indexSet;
			std::vector<Vector3> positionsNew((int)Corner4::kNumCorner);
			for (uint32_t i = 0; i < positionsNew.size(); i++) {
				Vector3 translation_ = {};
				translation_.x = mainPosition.transform.translate.x + info.moveVector.x* deltaTime_;
				translation_.y = mainPosition.transform.translate.y + info.moveVector.y* deltaTime_;
				translation_.z = mainPosition.transform.translate.z + info.moveVector.z* deltaTime_;
				positionsNew[i] = CornerPosition(translation_, static_cast<Corner4>(i));
			}
			bool hit = false;
			// 左下点の判定
			//indexSet = mapChipField_->GetMapChipIndexByPosition(positionsNew[kLeftBottom]);
			//mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			//if (mapChipType == MapChipType::kBlock) {
			//	hit = true;
			//}
			if (positionsNew[(int)Corner4::kLeftBottom].y <= 0) {
				hit = true;
			}
			// 右下点の判定
			//indexSet = mapChipField_->GetMapChipIndexByPosition(positionsNew[kRightBottom]);
			//mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			//if (mapChipType == MapChipType::kBlock) {
			//	hit = true;
			//}
			if (positionsNew[(int)Corner4::kRightBottom].y <= 0) {
				hit = true;
			}

			if (!hit) {
				onGround_ = false;
			}
		}
	} else {
		if (info.floorHit) {
			onGround_ = true;
			velocity_.x *= (1.0f - kAttenuationLanding);
			velocity_.y = 0.0f;
		}
	}
}

void Player::MapCollisionDecideDown(CollisionMapInfo& info) {
	/// 移動後の4つの角の座標
	std::vector<Vector3> positionsNew((int)Corner4::kNumCorner);

	for (uint32_t i = 0; i < positionsNew.size(); i++) {
		Vector3 translation_ = {};
		translation_.x = mainPosition.transform.translate.x + info.moveVector.x* deltaTime_;
		translation_.y = mainPosition.transform.translate.y + info.moveVector.y* deltaTime_;
		translation_.z = mainPosition.transform.translate.z + info.moveVector.z* deltaTime_;
		positionsNew[i] = CornerPosition(translation_, static_cast<Corner4>(i));
	}
	if (info.moveVector.y > 0)
		return;

	// 当たり判定を行う
	bool hit = false;

	//MapChipType mapChipType;
	//MapChipType mapChipTypeNext;
	//MapChipField::IndexSet indexSet;
	//// 左下点の判定
	//indexSet = mapChipField_->GetMapChipIndexByPosition(positionsNew[kLeftBottom]);
	//mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	//if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
	//	hit = true;
	//}
	if (positionsNew[(int)Corner4::kLeftBottom].y <= 0) {
		hit = true;
	}
	//// 右下点の判定
	//indexSet = mapChipField_->GetMapChipIndexByPosition(positionsNew[kRightBottom]);
	//mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	//mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	//if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
	//	hit = true;
	//}
	if (positionsNew[(int)Corner4::kRightBottom].y <= 0) {
		hit = true;
	}
	// ブロックにヒット?
	if (hit) {
		//// めり込みを排除する方向に移動量を設定する
		//indexSet = mapChipField_->GetMapChipIndexByPosition(positionsNew[kLeftBottom]);
		//MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexByPosition(worldTransform_.translation_);
		//if (indexSetNow.yIndex != indexSet.yIndex) {
		//	// めり込み先ブロックの矩形取得
		//	MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		//	info.moveVector.y = std::max(0.0f, rect.top - (kPlayerHeight / 2.0f) - worldTransform_.translation_.y);
		//	// 床判定であることを記録する
		//	info.floorHit = true;
		//}
		info.moveVector.y = std::max(0.0f, 0.0f - (kPlayerHeight / 2.0f) - mainPosition.transform.translate.y);
		info.floorHit = true;
	}
}



void Player::MovePlayerByResult(const CollisionMapInfo& info) {
	mainPosition.transform.translate.x += info.moveVector.x * deltaTime_;
	mainPosition.transform.translate.y += info.moveVector.y * deltaTime_;
	mainPosition.transform.translate.z += info.moveVector.z * deltaTime_;
}

// 腕のスケールを更新する補助関数
void Player::UpdateArmScale(float &scale, bool isPush) {
	if (isPush) {
		scale += kExpandSpeed * deltaTime_;

		isAttackingR_ = false;
		isAttackingL_ = false;
	} else {
		scale -= kShrinkSpeed * deltaTime_;


		if (scale <= 1.0f) {
			scale = 1.0f;
		}
		if (armScaleL_ > 1.0f) {
			isAttackingL_ = true;
		}
		if (armScaleR_ > 1.0f) {
			isAttackingR_ = true;
		}
	}
	scale = std::clamp(scale, 1.0f, kMaxArmScale);
}

// 腕のワールド座標を返す（衝突判定用）
Vector3 Player::GetArmPositionL() const {
	// 敵の本体座標と回転量を取得
	Vector3 centerPos = mainPosition.transform.translate;
	float rotY = mainPosition.transform.rotate.y;

	// ローカル空間での腕の位置（オフセット）
	// 左腕は X軸プラス方向
	float offsetX = +(kPlayerWidth / 1.2f);
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
Vector3 Player::GetArmPositionR() const {
	// 敵の本体座標と回転量を取得
	Vector3 centerPos = mainPosition.transform.translate;
	float rotY = mainPosition.transform.rotate.y;

	// ローカル空間での腕の位置（オフセット）
	// 右腕は X軸マイナス方向
	float offsetX = -(kPlayerWidth / 1.2f);
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