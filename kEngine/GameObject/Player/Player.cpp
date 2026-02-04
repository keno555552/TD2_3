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

	forward_ = { -std::sin(mainPosition.transform.rotate.y), 0.0f,
				-std::cos(mainPosition.transform.rotate.y) };

  // ■■■ 新しい腕の生成 ■■■
    // 第2引数に「自分(this)」を渡して追従させる
  armL_ = new Arm(system, this, false); // 左腕
  armR_ = new Arm(system, this, true);  // 右腕

  armL_->objectParts_[0].materialConfig->lightModelType = LightModelType::FlameNeonGlow;
  armL_->objectParts_[0].materialConfig->textureColor = {
      255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f
  };
  armL_->objectParts_[0].materialConfig->textureColor2 = {
       36.0f / 255.0f, 195.0f / 255.0f, 255.0f / 255.0f, 174.0f / 255.0f
  };
  armL_->objectParts_[0].materialConfig->textureColor3 = {
      255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f
  };
  armL_->objectParts_[0].materialConfig->intensity = 0.776f;
  armL_->objectParts_[0].materialConfig->heightScale = 2.922f;


  armR_->objectParts_[0].materialConfig->lightModelType = LightModelType::FlameNeonGlow;
  armR_->objectParts_[0].materialConfig->textureColor = {
      255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f
  };
  armR_->objectParts_[0].materialConfig->textureColor2 = {
       36.0f / 255.0f, 195.0f / 255.0f, 255.0f / 255.0f, 174.0f / 255.0f
  };
  armR_->objectParts_[0].materialConfig->textureColor3 = {
      255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f
  };
  armR_->objectParts_[0].materialConfig->intensity = 0.776f;
  armR_->objectParts_[0].materialConfig->heightScale = 2.922f;


}

Player::~Player() {
	delete armL_;
	delete armR_;
	delete leftHandDebugObject_;
	delete rightHandDebugObject_;
}

void Player::Update(Camera* camera) {
	// 1. エンジンから値を取得
	float rawDeltaTime = system_->GetScaledDeltaTime();

	// 2. 異常な値をカットする（最大でも 0.1秒 = 10FPS相当 に制限）
	// これにより、起動直後やデバッグ中の巨大な跳ね上がりを無視できます
	deltaTime_ = std::min(rawDeltaTime, 0.1f);
	BehaviorRootUpdate();

	Object::Update(camera);

	// ■■■ 腕の更新 ■■■
	  // ここで現在の「倍率(armScaleL_/R_)」を渡すだけ！
	if (armL_) {
		armL_->objectParts_[0].materialConfig->textureColor = Color1;
		armL_->objectParts_[0].materialConfig->textureColor2 = Color2;
		armL_->objectParts_[0].materialConfig->textureColor3 = Color3;
		armL_->objectParts_[0].materialConfig->intensity = colorIntensity;
		armL_->objectParts_[0].materialConfig->heightScale = colorHeightScale;
		armL_->Update(camera, armScaleL_);
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

void Player::Draw() {
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
	ImGuiPart();
}

void Player::AddKnockback(const Vector3& force) {
	if (isDamage_) {
		// ダメージ無効化中はノックバックを受け付けない
		return;
	}

	// 現在の速度に衝撃を加算
	velocity_.x += force.x;
	velocity_.z += force.z;

	// 接地フラグを折って、摩擦で即停止しないようにする
	onGround_ = false;

	// 0.5秒間ほど操作入力を受け付けず、慣性で飛ばされるようにする
	knockbackTimer_ = knockbackCD_;
	// ノックバック中は腕の攻撃も無効化
	isAttackingR_ = false;
	isAttackingL_ = false;
	// ノックバック中はダメージも受け付けない
	isDamage_ = true;
}

void Player::ResetArm(bool isRight) {
	//if (isDamage_) {
	//    // ダメージ無効化中はノックバックを受け付けない
	//    return;
	//}
	// 初期サイズに戻す（1.0f または ゲーム開始時のサイズ）
	if (isRight) {
		armScaleR_ = 1.0f;
	} else {
		armScaleL_ = 1.0f;
	}
}

void Player::BehaviorRootUpdate() {
    if (isTutorial_) {
        velocity_ = { 0.0f, 0.0f, 0.0f };
    }
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
	Vector3 offsetTable[(int)Corner4::kNumCorner] = {
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
	bool pushA =
		system_->GetIsPush(DIK_A)
		|| system_->GetGamepadIsPush(VK_PAD_LSHOULDER);
	bool pushD =
		system_->GetIsPush(DIK_D)
		|| system_->GetGamepadIsPush(VK_PAD_RSHOULDER);

	// 腕の膨張・収縮処理
	// Aキーで左腕、Dキーで右腕を膨らませる
	UpdateArmScale(armScaleL_, pushA);
	UpdateArmScale(armScaleR_, pushD);

	// ダメージ無効化タイマー更新
	if (isDamage_) {
		// ダメージ無効化中はタイマーを減少させる
		kDamageParameter_ -= deltaTime_;
		if (kDamageParameter_ <= 0.0f) {
			isDamage_ = false;
			kDamageParameter_ = kDamageCD_;
		}
	}

	/// --- 移動処理 ---
	// ノックバック中かどうかで処理を分ける
	if (knockbackTimer_ > 0.0f) {
		// --- 吹っ飛び中 ---
		knockbackTimer_ -= deltaTime_;

		// 空気抵抗（減衰）をかけて、徐々に遅くする
		velocity_.x *= 0.95f;
		velocity_.z *= 0.95f;
	} else {

		// --- 通常移動処理 ---
		isCharging_ = false;
		// 旋回と速度の計算
		if (pushA && pushD) {
			// 【停止】両方膨らませると急ブレーキ
			currentSpeed_ *= 0.8f;
			isCharging_ = true;
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

			if (armScaleL_ > 1.0f) {
				isAttackingL_ = true;
			} else {
				isAttackingL_ = false;
			}
			if (armScaleR_ > 1.0f) {
				isAttackingR_ = true;
			} else {
				isAttackingR_ = false;
			}
		} else {
			// 【旋回】片方のボタンだけ押されている状態

			if (pushA) {
				// Aボタン（左腕膨張 = 左の推進力UP）
				// 左が前に出るので、機体は「右」に回る (Y回転プラス)

				// 旋回速度の計算：基本速度 + (膨張分 * 係数)
				// 腕が大きくなるほど、より鋭く旋回します
				// 係数は調整してください
				float turnSpeed = kTurnSpeed + (armScaleL_ - 1.0f) * 0.02f;
				turnSpeed = std::min(turnSpeed, kTurnSpeed * 10.0f);
				mainPosition.transform.rotate.y += turnSpeed;

				// 攻撃判定解除
				isAttackingL_ = false;
			} else if (pushD) {
				// Dボタン（右腕膨張 = 右の推進力UP）
				// 右が前に出るので、機体は「左」に回る (Y回転マイナス)

				float turnSpeed = kTurnSpeed + (armScaleR_ - 1.0f) * 0.02f;
				turnSpeed = std::min(turnSpeed, kTurnSpeed * 10.0f);
				mainPosition.transform.rotate.y -= turnSpeed;

				// 攻撃判定解除
				isAttackingR_ = false;
			}
			currentSpeed_ = kBaseSpeed;
		}

		// 進行方向への移動計算
		// Y軸回転から進行方向ベクトルを計算 (3Dの場合)
		forward_ = { -std::sin(mainPosition.transform.rotate.y), 0.0f,
					-std::cos(mainPosition.transform.rotate.y) };

		velocity_.x = forward_.x * currentSpeed_;
		velocity_.z = forward_.z * currentSpeed_;

		mainPosition.transform.scale.y = mainPosition.transform.scale.x;
		mainPosition.transform.scale.z = mainPosition.transform.scale.x;

	}
	// プレイヤーの見た目を更新(仮の体部分のため)
	UpdateAppearance();
}

void Player::OnGroundChanger(const CollisionMapInfo& info) {
	if (onGround_) {
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			// MapChipType mapChipType;
			// MapChipField::IndexSet indexSet;
			std::vector<Vector3> positionsNew((int)Corner4::kNumCorner);
			for (uint32_t i = 0; i < positionsNew.size(); i++) {
				Vector3 translation_ = {};
				translation_.x =
					mainPosition.transform.translate.x + info.moveVector.x * deltaTime_;
				translation_.y =
					mainPosition.transform.translate.y + info.moveVector.y * deltaTime_;
				translation_.z =
					mainPosition.transform.translate.z + info.moveVector.z * deltaTime_;
				positionsNew[i] = CornerPosition(translation_, static_cast<Corner4>(i));
			}
			bool hit = false;
			// 左下点の判定
			// indexSet =
			// mapChipField_->GetMapChipIndexByPosition(positionsNew[kLeftBottom]);
			// mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex,
			// indexSet.yIndex); if (mapChipType == MapChipType::kBlock) { 	hit = true;
			//}
			if (positionsNew[(int)Corner4::kLeftBottom].y <= 0) {
				hit = true;
			}
			// 右下点の判定
			// indexSet =
			// mapChipField_->GetMapChipIndexByPosition(positionsNew[kRightBottom]);
			// mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex,
			// indexSet.yIndex); if (mapChipType == MapChipType::kBlock) { 	hit = true;
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
		translation_.x =
			mainPosition.transform.translate.x + info.moveVector.x * deltaTime_;
		translation_.y =
			mainPosition.transform.translate.y + info.moveVector.y * deltaTime_;
		translation_.z =
			mainPosition.transform.translate.z + info.moveVector.z * deltaTime_;
		positionsNew[i] = CornerPosition(translation_, static_cast<Corner4>(i));
	}
	if (info.moveVector.y > 0)
		return;

	// 当たり判定を行う
	bool hit = false;

	// MapChipType mapChipType;
	// MapChipType mapChipTypeNext;
	// MapChipField::IndexSet indexSet;
	//// 左下点の判定
	// indexSet =
	// mapChipField_->GetMapChipIndexByPosition(positionsNew[kLeftBottom]);
	// mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex,
	// indexSet.yIndex); mapChipTypeNext =
	// mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// if (mapChipType == MapChipType::kBlock && mapChipTypeNext !=
	// MapChipType::kBlock) { 	hit = true;
	// }
	if (positionsNew[(int)Corner4::kLeftBottom].y <= 0) {
		hit = true;
	}
	//// 右下点の判定
	// indexSet =
	// mapChipField_->GetMapChipIndexByPosition(positionsNew[kRightBottom]);
	// mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex,
	// indexSet.yIndex); mapChipTypeNext =
	// mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	// if (mapChipType == MapChipType::kBlock && mapChipTypeNext !=
	// MapChipType::kBlock) { 	hit = true;
	// }
	if (positionsNew[(int)Corner4::kRightBottom].y <= 0) {
		hit = true;
	}
	// ブロックにヒット?
	if (hit) {
		//// めり込みを排除する方向に移動量を設定する
		// indexSet =
		// mapChipField_->GetMapChipIndexByPosition(positionsNew[kLeftBottom]);
		// MapChipField::IndexSet indexSetNow =
		// mapChipField_->GetMapChipIndexByPosition(worldTransform_.translation_);
		// if (indexSetNow.yIndex != indexSet.yIndex) {
		//	// めり込み先ブロックの矩形取得
		//	MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex,
		//indexSet.yIndex); 	info.moveVector.y = std::max(0.0f, rect.top -
		//(kPlayerHeight / 2.0f) - worldTransform_.translation_.y);
		//	// 床判定であることを記録する
		//	info.floorHit = true;
		// }
		info.moveVector.y = std::max(0.0f, 0.0f - (kPlayerHeight / 2.0f) -
			mainPosition.transform.translate.y);
		info.floorHit = true;
	}
}

void Player::MovePlayerByResult(const CollisionMapInfo& info) {
	mainPosition.transform.translate.x += info.moveVector.x * deltaTime_;
	mainPosition.transform.translate.y += info.moveVector.y * deltaTime_;
	mainPosition.transform.translate.z += info.moveVector.z * deltaTime_;
}

// 腕のスケールを更新する補助関数
void Player::UpdateArmScale(float& scale, bool isPush) {
	if (isPush) {
		scale += kExpandSpeed * deltaTime_;

	} else {
		scale -= kShrinkSpeed * deltaTime_;

		if (scale <= 1.0f) {
			scale = 1.0f;
		}
	}
	scale = std::clamp(scale, 1.0f, kMaxArmScale);
}

void Player::UpdateAppearance() {
	// 1. 親（全体）のスケールを取得
	float totalScale = mainPosition.transform.scale.x;

	// 2. 「固定したい隙間」をローカル値に変換する
	// ワールドで kArmBaseGap(例:0.1) 離したいなら、ローカルでは 0.1 / 0.5 = 0.2 離す必要がある
	float localGap = kArmBaseGap_ / totalScale;

	// 3. ローカル座標でのオフセット計算
	// kPlayerWidth は元々ローカル定義(1.0)なので、そのまま使います。

	// ボディ半幅(0.5) + 補正済み隙間
	float localOffsetR = (kPlayerWidth / 2.0f) + localGap;
	float localOffsetL = (kPlayerWidth / 2.0f) + localGap;

	// 4. 適用
	// 右腕
	objectParts_[1].transform.scale = { 0, 0, 0 };
	objectParts_[1].transform.translate =
		objectParts_[0].transform.translate + Vector3{ (kPlayerWidth / 2.0f), 0.0f, 0.0f };
	// 左腕
	objectParts_[2].transform.scale = { 0, 0, 0 };
	objectParts_[2].transform.translate =
		objectParts_[0].transform.translate + Vector3{ -(kPlayerWidth / 2.0f), 0.0f, 0.0f };
}

void Player::ImGuiPart() {
#ifdef USE_IMGUI
	ImGui::Text("=== Player ===");
	ImGui::ColorEdit4("Arm Color1", &Color1.x);
	ImGui::ColorEdit4("Arm Color2", &Color2.x);
	ImGui::ColorEdit4("Arm Color3", &Color3.x);
	ImGui::SliderFloat("Color Intensity", &colorIntensity, 0.0f, 2.0f);
	ImGui::SliderFloat("Color HeightScale", &colorHeightScale, 0.0f, 5.0f);
	ImGui::Text("================");
#endif
}

void Player::OnWallHit(const Vector3& normal) {
	// 壁の押し返し方向成分の速度をゼロにする（あるいは少し跳ね返す）

	// X方向の壁（左右）に当たった場合
	if (normal.x != 0.0f) {
		// 壁に向かって進んでいる場合のみ止める（壁から離れようとしている時は止めない）
		if ((normal.x > 0 && velocity_.x < 0) || (normal.x < 0 && velocity_.x > 0)) {
			velocity_.x = 0.0f;
			// 少しバウンドさせたい場合はこちらを使う
			// velocity_.x *= -0.5f; 
		}
	}

	// Z方向の壁（奥・手前）に当たった場合
	if (normal.z != 0.0f) {
		if ((normal.z > 0 && velocity_.z < 0) || (normal.z < 0 && velocity_.z > 0)) {
			velocity_.z = 0.0f;
			// velocity_.z *= -0.5f;
		}
	}
}

// 腕のワールド座標を返す（衝突判定用）
Vector3 Player::GetArmPositionL() const {
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

Vector3 Player::GetArmPositionR() const {
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