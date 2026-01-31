#include "SceneManager.h"
#include <cmath>

// ★ テクスチャは「一度だけロードして使い回す」(2回目以降の未適用対策)
static int gShojiFrameTexHandle = -1;
static int gShojiPaperTexHandle = -1;

// ===== 手動方向計算 =====
static Vector3 NormalizeSafe(const Vector3& v) {
	float len = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	if (len < 1e-6f) return Vector3(0.0f, 0.0f, 1.0f);
	return Vector3(v.x / len, v.y / len, v.z / len);
}

static Vector3 Cross(const Vector3& a, const Vector3& b) {
	return Vector3(
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x
	);
}

static Vector3 CalcForward(Vector3 r) {
	const float PI = 3.14159265359f;
	const float TWO_PI = 6.28318530718f;

	// 笘・菫ｮ豁｣・壼ｺｦ謨ｰ竊偵Λ繧ｸ繧｢繝ｳ螟画鋤縺ｮ隱､辷・ｒ髦ｲ縺・
	// 縲・.28繧医ｊ螟ｧ縺阪＞・晏ｺｦ縲阪→豎ｺ繧∵遠縺｡縺吶ｋ縺ｨ縲√Λ繧ｸ繧｢繝ｳ闢・ｩ阪〒蠢・★螢翫ｌ繧九・
	// 蠎ｦ謨ｰ縺ｯ譎ｮ騾・0・・60・医○縺・●縺・ｱ360・峨↑縺ｮ縺ｧ縲√◎縺ｮ遽・峇縺縺大ｺｦ縺ｨ縺励※謇ｱ縺・・
	if ((std::fabs(r.x) > TWO_PI && std::fabs(r.x) <= 360.0f) ||
		(std::fabs(r.y) > TWO_PI && std::fabs(r.y) <= 360.0f) ||
		(std::fabs(r.z) > TWO_PI && std::fabs(r.z) <= 360.0f)) {
		const float DEG2RAD = PI / 180.0f;
		r.x *= DEG2RAD;
		r.y *= DEG2RAD;
		r.z *= DEG2RAD;
	}

	// ★ 角度は正規化して安定化（A旋回で負方向に積んでも破綻しない）
	while (r.y > PI) r.y -= TWO_PI;
	while (r.y < -PI) r.y += TWO_PI;

	while (r.x > PI) r.x -= TWO_PI;
	while (r.x < -PI) r.x += TWO_PI;

	float cx = std::cos(r.x);
	float sx = std::sin(r.x);
	float cy = std::cos(r.y);
	float sy = std::sin(r.y);

	return NormalizeSafe(Vector3(
		sy * cx,
		-sx,
		cy * cx
	));
}

static Vector3 CalcRight(const Vector3& forward) {
	Vector3 up(0.0f, 1.0f, 0.0f);
	return NormalizeSafe(Cross(up, forward));
}

// offset.x = Right, offset.y = Up, offset.z = Forward
static Vector3 CalcBaseFromCamera(
	const Vector3& camPos,
	const Vector3& camRot,
	const Vector3& localOffset
) {
	Vector3 forward = CalcForward(camRot);
	Vector3 right = CalcRight(forward);
	Vector3 up(0.0f, 1.0f, 0.0f);

	Vector3 base = camPos;

	base.x += right.x * localOffset.x;
	base.y += right.y * localOffset.x;
	base.z += right.z * localOffset.x;

	base.x += up.x * localOffset.y;
	base.y += up.y * localOffset.y;
	base.z += up.z * localOffset.y;

	base.x += forward.x * localOffset.z;
	base.y += forward.y * localOffset.z;
	base.z += forward.z * localOffset.z;

	return base;
}

// ===== Easing：だんだん遅くなる =====
static float EaseOutSine(float t) {
	if (t < 0.0f) t = 0.0f;
	if (t > 1.0f) t = 1.0f;
	return std::sin(t * 1.57079632679f); // PI/2
}


// =====================================================

SceneManager::SceneManager(kEngine* system) {
	system_ = system;
	sceneFactory_ = new SceneFactory(system_);

	sceneUsingNameHandle_ = "TITLE";

	helperTextureHandle_ = system_->LoadTexture("resources/texture/helper.png");
	helperSprite_ = new SimpleSprite;
	helperSprite_->IntObject(system_);
	helperSprite_->CreateDefaultData();
	helperSprite_->mainPosition.transform.scale = { 0.5f,0.5f,1.0f };
	helperSprite_->mainPosition.transform.translate = { 0.0f,550.0f,0.0f };
	helperSprite_->objectParts_[0].materialConfig->textureHandle = helperTextureHandle_;

	// ★ 障子テクスチャも「ここで1回だけ」ロード（2回目以降の未適用対策）
	if (gShojiFrameTexHandle < 0) {
		gShojiFrameTexHandle = system_->LoadTexture("resources/gate/model/wood_tate.png");
	}
	if (gShojiPaperTexHandle < 0) {
		gShojiPaperTexHandle = system_->LoadTexture("resources/gate/model/paper.png");
	}

	defaultMenu_ = new DefaultMenu(system_);
}

SceneManager::~SceneManager() {
	if (shojiL_) { delete shojiL_; shojiL_ = nullptr; }
	if (shojiR_) { delete shojiR_; shojiR_ = nullptr; }
	if (shojiL_paper_) { delete shojiL_paper_; shojiL_paper_ = nullptr; }
	if (shojiR_paper_) { delete shojiR_paper_; shojiR_paper_ = nullptr; }

	if (sceneUsing_) { delete sceneUsing_; sceneUsing_ = nullptr; }
	if (sceneOld_) { delete sceneOld_; sceneOld_ = nullptr; }
}

Camera* SceneManager::ResolveCamera(BaseScene* s) {
	if (!s) return nullptr;
	if (auto* t = dynamic_cast<SceneTitle*>(s)) return t->camera_;
	if (auto* st = dynamic_cast<SceneStage01*>(s)) return st->GetCamera();
	if (auto* rt = dynamic_cast<SceneResult*>(s)) return rt->GetCamera();
	return nullptr;
}

//void SceneManager::SpawnSceneByHandle(SceneNum handle) {
//	switch (handle) {
//	case SceneNum::S_TITLE:    sceneUsing_ = new SceneTitle(system_); break;
//	case SceneNum::S_STAGE_01: sceneUsing_ = new SceneStage01(system_); break;
//	case SceneNum::S_TESTER:   sceneUsing_ = new SceneTester(system_); break;
//	case SceneNum::S_Result:   sceneUsing_ = new SceneResult(system_); break;
//	default: sceneUsing_ = nullptr; break;
//	}
//	sceneJustSpawned_ = (sceneUsing_ != nullptr);
//}

void SceneManager::SceneChanger() {

	if (sceneUsing_ && !shojiPlaying_) {

		bool isSceneChange = false;

		switch (sceneUsing_->GetOutcome()) {

		case SceneOutcome::NEXT:
		{
			auto targetScene = sceneFlow_.find(sceneUsingNameHandle_);
			if (targetScene != sceneFlow_.end()) {

				nextAfterShoji_ = targetScene->second;
				Camera* cam = ResolveCamera(sceneUsing_);
				if (!cam) {
					sceneUsingNameHandle_ = nextAfterShoji_;
					isSceneChange = true;
					return;
				}

				shojiCam_ = cam;

				// カメラローカルオフセット（保持）
				shojiBasePos_ = Vector3(0.0f, 0.0f, 4.0f);

				Vector3 camPos = shojiCam_->GetPosition();
				Vector3 camRot = shojiCam_->GetTransform().rotate;
				Vector3 base = CalcBaseFromCamera(camPos, camRot, shojiBasePos_);

				// ここから「元の障子生成ブロック」を関数に移しただけ
				Shoji_CreateAll(camRot);

				// 初期位置（開いた状態）
				Shoji_SetInitialOpenPose(base, camRot);

				shojiT_ = 0.0f;
				shojiWaitT_ = 0.0f;
				shojiPhase_ = ShojiPhase::Closing;
				shojiPlaying_ = true;

			} else {
				Logger::Log("[kError] SM :: SceneChanger: Scene not found in sceneFlow_: " + sceneUsingNameHandle_);
			}
		}
		break;

		case SceneOutcome::RETRY:
			isSceneChange = true;
			break;

		case SceneOutcome::RETURN:
			isSceneChange = true;
			sceneUsingNameHandle_ = "TITLE";
			break;
		case SceneOutcome::EXIT:
			kEngine::EndGame();
			break;

		}

		if (defaultMenu_->IsBack()) {
			isSceneChange = true;
			sceneUsingNameHandle_ = "TITLE";
		}

		if (defaultMenu_->IsRetry()) {
			isSceneChange = true;
		}

		if (!isSceneChange)return;

		delete sceneUsing_, sceneUsing_ = nullptr;
	}

	sceneUsing_ = sceneFactory_->CreateScene(sceneUsingNameHandle_);
}

//void SceneManager::SceneChanger() {
//
//	if (sceneUsing_ && !shojiPlaying_) {
//		if (sceneUsing_->GetScenePhase() == ScenePhase::EXIT) {
//
//			nextAfterShoji_ = sceneUsing_->GetNextStage();
//			Camera* cam = ResolveCamera(sceneUsing_);
//			if (!cam) {
//				sceneUsingHandle_ = nextAfterShoji_;
//				delete sceneUsing_;
//				sceneUsing_ = nullptr;
//				return;
//			}
//
//			shojiCam_ = cam;
//
//			// カメラローカルオフセット（保持）
//			shojiBasePos_ = Vector3(0.0f, 0.0f, 4.0f);
//
//			Vector3 camPos = shojiCam_->GetPosition();
//			Vector3 camRot = shojiCam_->GetTransform().rotate;
//			Vector3 base = CalcBaseFromCamera(camPos, camRot, shojiBasePos_);
//
//			// ここから「元の障子生成ブロック」を関数に移しただけ
//			Shoji_CreateAll(camRot);
//
//			// 初期位置（開いた状態）
//			Shoji_SetInitialOpenPose(base, camRot);
//
//			shojiT_ = 0.0f;
//			shojiWaitT_ = 0.0f;
//			shojiPhase_ = ShojiPhase::Closing;
//			shojiPlaying_ = true;
//			return;
//		}
//	}
//
//	if (sceneUsing_) {
//		if (sceneUsing_->GetScenePhase() == ScenePhase::EXIT) {
//			sceneUsingHandle_ = sceneUsing_->GetNextStage();
//			delete sceneUsing_;
//			sceneUsing_ = nullptr;
//		}
//	}
//
//	if (!sceneUsing_) {
//		SpawnSceneByHandle(sceneUsingHandle_);
//	}
//}

void SceneManager::Update() {

	if (shojiPlaying_) {

		float dt = system_->GetDeltaTime();
		if (dt > 0.1f) dt = 0.1f;
		if (dt < 0.0f) dt = 0.0f;

		// ★木枠だけでなく紙も必須
		if (!shojiL_ || !shojiR_ || !shojiL_paper_ || !shojiR_paper_) return;

		// Closing / Opening
		if (shojiPhase_ == ShojiPhase::Closing || shojiPhase_ == ShojiPhase::Opening) {
			Shoji_UpdateClosingOpening(dt);
			return;
		}

		// 閉じたまま待つ（旧シーン）
		if (shojiPhase_ == ShojiPhase::WaitBeforeTransition) {
			Shoji_UpdateWaitBeforeTransition(dt);
			// 元コードと同じ return 条件（Wait が足りない時は中で return してる）
			if (shojiPhase_ == ShojiPhase::WaitNewCamera) return;
			return;
		}

		// 遷移先カメラ待ち（新シーン）
		if (shojiPhase_ == ShojiPhase::WaitNewCamera) {
			Shoji_UpdateWaitNewCamera(dt);
			return;
		}
	}

	SceneChanger();
	if (sceneUsing_) {
		sceneUsing_->Update();
		sceneJustSpawned_ = false;
	}
}

void SceneManager::Render() {

	if (sceneUsing_ && !sceneJustSpawned_) {
		sceneUsing_->Draw();
	}

	if (shojiL_) shojiL_->Draw();
	if (shojiR_) shojiR_->Draw();
	if (shojiL_paper_) shojiL_paper_->Draw();
	if (shojiR_paper_) shojiR_paper_->Draw();
}



// =====================================================
// ★ 障子：関数化（ロジック・変数名・順序は元のまま）
// =====================================================

void SceneManager::Shoji_CreateAll(const Vector3& camRot) {

	// =========================
	// 障子：木枠
	// =========================
	shojiL_ = new Object;
	shojiR_ = new Object;
	shojiL_->IntObject(system_);
	shojiR_->IntObject(system_);

	// ★ CreateDefaultData は呼ばない（CreateModelData が objectParts_ を作り直すため）
	shojiL_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_left.obj"));
	shojiR_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_right.obj"));

	// ★ テクスチャ設定（木枠）※CreateModelData の後に「一度だけ」設定
	for (auto& part : shojiL_->objectParts_) {
		if (!part.materialConfig)
			part.materialConfig = std::make_shared<MaterialConfig>();
		part.materialConfig->useModelTexture = false;
		part.materialConfig->textureHandle = gShojiFrameTexHandle;
		part.materialConfig->enableLighting = true;
	}
	for (auto& part : shojiR_->objectParts_) {
		if (!part.materialConfig)
			part.materialConfig = std::make_shared<MaterialConfig>();
		part.materialConfig->useModelTexture = false;
		part.materialConfig->textureHandle = gShojiFrameTexHandle;
		part.materialConfig->enableLighting = true;
	}

	shojiL_->mainPosition.transform.scale = { 0.025f,0.025f,0.025f };
	shojiR_->mainPosition.transform.scale = { 0.025f,0.025f,0.025f };
	shojiL_->mainPosition.transform.rotate = camRot;
	shojiR_->mainPosition.transform.rotate = camRot;

	// =========================
	// 障子：紙
	// =========================
	shojiL_paper_ = new Object;
	shojiR_paper_ = new Object;
	shojiL_paper_->IntObject(system_);
	shojiR_paper_->IntObject(system_);

	// ★ CreateDefaultData は呼ばない（CreateModelData が objectParts_ を作り直すため）
	shojiL_paper_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_left_paper.obj"));
	shojiR_paper_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_right_paper.obj"));

	// ★ テクスチャ設定（和紙）※CreateModelData の後に「一度だけ」設定
	for (auto& part : shojiL_paper_->objectParts_) {
		if (!part.materialConfig)
			part.materialConfig = std::make_shared<MaterialConfig>();
		part.materialConfig->useModelTexture = false;
		part.materialConfig->textureHandle = gShojiPaperTexHandle;
		part.materialConfig->enableLighting = false;
	}
	for (auto& part : shojiR_paper_->objectParts_) {
		if (!part.materialConfig)
			part.materialConfig = std::make_shared<MaterialConfig>();
		part.materialConfig->useModelTexture = false;
		part.materialConfig->textureHandle = gShojiPaperTexHandle;
		part.materialConfig->enableLighting = false;
	}

	shojiL_paper_->mainPosition.transform.scale = { 0.025f,0.025f,0.025f };
	shojiR_paper_->mainPosition.transform.scale = { 0.025f,0.025f,0.025f };
	shojiL_paper_->mainPosition.transform.rotate = camRot;
	shojiR_paper_->mainPosition.transform.rotate = camRot;
}
void SceneManager::Shoji_SetInitialOpenPose(const Vector3& base, const Vector3& camRot) {
	Vector3 forward = CalcForward(camRot);
	Vector3 right = CalcRight(forward);

	const float off = 10.2f;

	// 蟾ｦ・域惠譫・・
	Vector3 leftPos = base;
	leftPos.x -= right.x * off;
	leftPos.y -= right.y * off;
	leftPos.z -= right.z * off;
	shojiL_->mainPosition.transform.translate = leftPos;

	// 蜿ｳ・域惠譫・・
	Vector3 rightPos = base;
	rightPos.x += right.x * off;
	rightPos.y += right.y * off;
	rightPos.z += right.z * off;
	shojiR_->mainPosition.transform.translate = rightPos;

	// 蟾ｦ・育ｴ呻ｼ・
	Vector3 leftPaperPos = base;
	leftPaperPos.x -= right.x * off;
	leftPaperPos.y -= right.y * off;
	leftPaperPos.z -= right.z * off;
	shojiL_paper_->mainPosition.transform.translate = leftPaperPos;

	// 右（紙）
	Vector3 rightPaperPos = base;
	rightPaperPos.x += right.x * off;
	rightPaperPos.y += right.y * off;
	rightPaperPos.z += right.z * off;
	shojiR_paper_->mainPosition.transform.translate = rightPaperPos;
}


void SceneManager::Shoji_DestroyAll() {
	delete shojiL_; shojiL_ = nullptr;
	delete shojiR_; shojiR_ = nullptr;
	delete shojiL_paper_; shojiL_paper_ = nullptr;
	delete shojiR_paper_; shojiR_paper_ = nullptr;
	shojiPlaying_ = false;
	shojiPhase_ = ShojiPhase::None;
}

void SceneManager::Shoji_UpdateClosingOpening(float dt) {

	// ★★★ 追加：Opening 中でもシーンを更新する ★★★
	if (shojiPhase_ == ShojiPhase::Opening && sceneUsing_) {
		sceneUsing_->Update();
	}
	if (!shojiCam_) return;

	if (shojiPhase_ == ShojiPhase::Closing) {
		shojiT_ += dt / 1.0f;
		if (shojiT_ > 1.0f) shojiT_ = 1.0f;
	}
	else {
		shojiT_ -= dt / 1.0f;
		if (shojiT_ < 0.0f) shojiT_ = 0.0f;
	}

	Vector3 camPos = shojiCam_->GetPosition();
	Vector3 camRot = shojiCam_->GetTransform().rotate;

	Vector3 forward = CalcForward(camRot);
	Vector3 right = CalcRight(forward);

	Vector3 base = CalcBaseFromCamera(camPos, camRot, shojiBasePos_);

	float easedT = EaseOutSine(shojiT_);
	float off = (1.0f - easedT) * 10.2f;

	shojiL_->mainPosition.transform.rotate = camRot;
	shojiR_->mainPosition.transform.rotate = camRot;
	shojiL_->mainPosition.transform.translate = base - right * off;
	shojiR_->mainPosition.transform.translate = base + right * off;
	shojiL_->Update(shojiCam_);
	shojiR_->Update(shojiCam_);

	shojiL_paper_->mainPosition.transform.rotate = camRot;
	shojiR_paper_->mainPosition.transform.rotate = camRot;
	shojiL_paper_->mainPosition.transform.translate = base - right * off;
	shojiR_paper_->mainPosition.transform.translate = base + right * off;
	shojiL_paper_->Update(shojiCam_);
	shojiR_paper_->Update(shojiCam_);

	if (shojiPhase_ == ShojiPhase::Closing && shojiT_ >= 1.0f) {
		shojiPhase_ = ShojiPhase::WaitBeforeTransition;
		shojiWaitT_ = 0.0f;
	}
	if (shojiPhase_ == ShojiPhase::Opening && shojiT_ <= 0.0f) {
		// 元コードと同じ delete 順・代入
		delete shojiL_; shojiL_ = nullptr;
		delete shojiR_; shojiR_ = nullptr;
		delete shojiL_paper_; shojiL_paper_ = nullptr;
		delete shojiR_paper_; shojiR_paper_ = nullptr;
		shojiPlaying_ = false;
		shojiPhase_ = ShojiPhase::None;
	}
}

void SceneManager::Shoji_UpdateWaitBeforeTransition(float dt) {

	if (shojiCam_) {
		Vector3 camPos = shojiCam_->GetPosition();
		Vector3 camRot = shojiCam_->GetTransform().rotate;

		Vector3 base = CalcBaseFromCamera(camPos, camRot, shojiBasePos_);

		shojiL_->mainPosition.transform.rotate = camRot;
		shojiR_->mainPosition.transform.rotate = camRot;
		shojiL_->mainPosition.transform.translate = base;
		shojiR_->mainPosition.transform.translate = base;
		shojiL_->Update(shojiCam_);
		shojiR_->Update(shojiCam_);

		shojiL_paper_->mainPosition.transform.rotate = camRot;
		shojiR_paper_->mainPosition.transform.rotate = camRot;
		shojiL_paper_->mainPosition.transform.translate = base;
		shojiR_paper_->mainPosition.transform.translate = base;
		shojiL_paper_->Update(shojiCam_);
		shojiR_paper_->Update(shojiCam_);
	}

	/// 待機時間計測
	shojiWaitT_ += dt;
	/// 待機時間終了まで戻る
	if (shojiWaitT_ < shojiWaitDuration_) return;

	/// 待機時間終了したらシーン切り替え
	auto targetScene = sceneFlow_.find(sceneUsingNameHandle_);
	if (targetScene != sceneFlow_.end()) {

			sceneUsingNameHandle_ = nextAfterShoji_;
			return;
	} else {
		Logger::Log("[kError] SM :: SceneChanger: Scene not found in sceneFlow_: " + sceneUsingNameHandle_);
	}

	/// 元コードと同じ delete 順・代入
	delete sceneUsing_, sceneUsing_ = nullptr;
	sceneUsing_ = sceneFactory_->CreateScene(sceneUsingNameHandle_);
	sceneUsing_->Update();

	shojiCam_ = nullptr;
	shojiPhase_ = ShojiPhase::WaitNewCamera;
}

void SceneManager::Shoji_UpdateWaitNewCamera(float dt) {
	(void)dt;

	if (sceneUsing_) sceneUsing_->Update();

	sceneJustSpawned_ = false;

	Camera* cam = ResolveCamera(sceneUsing_);
	if (!cam) return;

	shojiCam_ = cam;
	shojiPhase_ = ShojiPhase::Opening;
	shojiT_ = 1.0f;
}

