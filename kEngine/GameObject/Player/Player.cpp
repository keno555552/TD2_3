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

	mainPosition.transform.rotate.y = 0.0f;
}

void Player::Update(Camera* camera) {
	deltaTime_ = system_->GetDeltaTime();
	BehaviorRootUpdate();

	Object::Update(camera);
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

	// 旋回と速度の計算
	if (pushA && pushD) {
		// 【停止】両方膨らませると急ブレーキ
		currentSpeed_ *= 0.8f;
	} else if (!pushA && !pushD) {
		// 【加速】ボタンを離した際、腕が縮みながら加速（元に戻るまでブースト）
		if (armScaleL_ > 1.01f || armScaleR_ > 1.01f) {
			currentSpeed_ = kBaseSpeed + kBoostPower;
		} else {
			currentSpeed_ = kBaseSpeed;
		}
	} else {
		// 【旋回】膨らんでいる方に旋回
		// 原点が付け根にあれば、スケールを変えるだけで外側に伸びる
		objectParts_[1].transform.scale.x = armScaleR_;
		objectParts_[1].transform.scale.y = armScaleR_;
		objectParts_[1].transform.scale.z = armScaleR_;
		objectParts_[2].transform.scale.x = armScaleL_;
		objectParts_[2].transform.scale.y = armScaleL_;
		objectParts_[2].transform.scale.z = armScaleL_;
		float turnAmount = (armScaleR_ - armScaleL_) * kTurnSpeed;
		mainPosition.transform.rotate.y += turnAmount;
		currentSpeed_ = kBaseSpeed;
	}

	// 4. 進行方向への移動計算
	// Y軸回転から進行方向ベクトルを計算 (3Dの場合)
	Vector3 forward = {
		std::sin(mainPosition.transform.rotate.y),
		0.0f,
		std::cos(mainPosition.transform.rotate.y)
	};

	/*velocity_.x = forward.x * currentSpeed_;
	velocity_.z = forward.z * currentSpeed_;*/
	velocity_.x = 0.0f;
	velocity_.z = 0.0f;

	// 5. 腕の座標調整（重ならないようにずらす
	// 左腕
	objectParts_[1].transform.translate =
		objectParts_[0].transform.translate +
		Vector3{ -(kPlayerWidth / 2.0f + 0.25f), 0.0f, 0.0f };
	// 右腕
	objectParts_[2].transform.translate =
		objectParts_[0].transform.translate +
		Vector3{ +(kPlayerWidth / 2.0f + 0.25f), 0.0f, 0.0f };
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
	} else {
		scale -= kShrinkSpeed * deltaTime_;
	}
	scale = std::clamp(scale, 1.0f, kMaxArmScale);
}