#include "StartTimer.h"

StartTimer::StartTimer(kEngine* kEngine) {
	SimpleSprite::IntObject(kEngine);
	timer_ = new Timer();
	timer_->Init0(1.0f, kEngine->GetTimeManager());
	timer_->SetInfluenceByTimeScale(true);

	/// ========= テキスチャーハンドル =========///

	std::string basePath = "resources/texture/StartTimer/";

	numberTextureHandles_[2] = kEngine->LoadTexture(basePath + "3.png");
	numberTextureHandles_[1] = kEngine->LoadTexture(basePath + "2.png");
	numberTextureHandles_[0] = kEngine->LoadTexture(basePath + "1.png");
	startTextureHandle_ = kEngine->LoadTexture(basePath + "Start.png");

	/// ============ オブジェクト =============///

	for (int i = 0; i < 3; i++) {
		numberSprites_[i] = new SimpleSprite();
		numberSprites_[i]->IntObject(kEngine);
		numberSprites_[i]->CreateDefaultData();
		numberSprites_[i]->mainPosition.transform.scale = { startScale_,startScale_,1.0f };
		numberSprites_[i]->mainPosition.transform.translate = { 640.0f,360.0f,0.0f };
		numberSprites_[i]->objectParts_[0].materialConfig->textureHandle = numberTextureHandles_[i];
		numberSprites_[i]->objectParts_[0].materialConfig->textureColor = usingColor;
		numberSprites_[i]->objectParts_[0].anchorPoint = { 422.0f,178.0f };
	}
	startSprite_ = new SimpleSprite();
	startSprite_->IntObject(kEngine);
	startSprite_->CreateDefaultData();
	startSprite_->mainPosition.transform.scale = { startScale_,startScale_,1.0f };
	startSprite_->mainPosition.transform.translate = { 640.0f,360.0f,0.0f };
	startSprite_->objectParts_[0].materialConfig->textureHandle = startTextureHandle_;
	startSprite_->objectParts_[0].materialConfig->textureColor = usingColor;
	startSprite_->objectParts_[0].anchorPoint = { 422.0f,178.0f };


	
	CountSE_ = system_->SoundLoadSE("resources/gate/SE/Game/Count.wav");
	StartSE_ = system_->SoundLoadSE("resources/gate/SE/Game/Start.wav");
}

void StartTimer::Update(Camera* camera) {

	if (!isStart_) return;

	/// ========= タイマー更新 ========= ///
	if (numberCount_ >= 0) {
		if (timer_->parameter_ != timer_->maxTime_) {
			timer_->ToMix();
		}
		else {

			// ここで「次の表示」に切り替わるので、先に減らす
			numberCount_--;

			// ★ 減らした後の値（= 次に表示される状態）に合わせて鳴らす
			// 3/2/1 表示開始のタイミングで鳴る
			if (CountSE_ >= 0 && numberCount_ >= 1) { // 3,2,1 に相当（numberCount_ が 3,2,1 のとき）
				system_->SoundPlaySE(CountSE_, 0.8f);
			}

			// START 表示開始のタイミングで鳴る（numberCount_ が 0 になった瞬間）
			if (numberCount_ == 0) {
				if (StartSE_ >= 0 && !startSEPlayed_) {
					system_->SoundPlaySE(StartSE_, 0.8f);
					startSEPlayed_ = true;
				}
			}

			timer_->Reset0();
		}
	}

	/// ========= オブジェクト更新 ========= ///

	int movingObjectIndex = numberCount_ - 1;

	if (movingObjectIndex >= 0) {
		numberSprites_[movingObjectIndex]->objectParts_[0].materialConfig->textureColor.w = timer_->linearity(1.0f, 0.0f);
		float scaleValue = timer_->linearity(startScale_, 1.0f);
		numberSprites_[movingObjectIndex]->mainPosition.transform.scale = { scaleValue,scaleValue,1.0f };

	}
	else {
		startSprite_->objectParts_[0].materialConfig->textureColor.w = timer_->linearity(1.0f, 0.0f);
		float scaleValue = timer_->linearity(startScale_, 1.0f);

		startSprite_->mainPosition.transform.scale = { scaleValue,scaleValue,1.0f };
	}

	/// ========== スプライト配置 ========== ///

}


void StartTimer::Draw() {

	//ImGuiPart();
	if (numberCount_ < 0 && timer_->parameter_ == timer_->maxTime_)return;

	int movingObjectIndex = numberCount_ - 1;

	if (movingObjectIndex >= 0) {
		numberSprites_[movingObjectIndex]->Draw();
	} else if(movingObjectIndex == -1){
		startSprite_->Draw();
	}

}

void StartTimer::Start() {
	// ★ すでに開始済みなら何もしない
	if (hasStarted_) return;
	hasStarted_ = true;          // ← ★ これが抜けてた
	isStart_ = true;
	startSEPlayed_ = false;
	// ★ 最初の「3」が表示される瞬間のSE
	if (CountSE_ >= 0) {
		system_->SoundPlaySE(CountSE_, 0.8f);
	}
}

void StartTimer::Stop() {
	isStart_ = false;
}

void StartTimer::Reset() {
	for (int i = 0; i < 3; i++) {
		numberSprites_[i]->mainPosition.transform.scale = { startScale_, startScale_, 1.0f };
		numberSprites_[i]->objectParts_[0].materialConfig->textureColor = usingColor;
	}
	startSprite_->mainPosition.transform.scale = { startScale_, startScale_, 1.0f };
	startSprite_->objectParts_[0].materialConfig->textureColor = usingColor;


	timer_->Reset0();
	numberCount_ = 3;
	isStart_ = false;

	hasStarted_ = false;   // ★ これを忘れると再スタートできない
}

bool StartTimer::IsTimeUp() {
	if (numberCount_ > -1)return false;
	if (timer_->parameter_ != 0)return false;
	return true;
}

Timer& StartTimer::GetTime() {
	return *timer_;
}

void StartTimer::ImGuiPart() {
#ifdef USE_IMGUI

	float scale = numberSprites_[0]->mainPosition.transform.scale.x;

	ImGui::Begin("Scale");
	ImGui::SliderFloat2("position", &numberSprites_[0]->mainPosition.transform.translate.x, 0.0f, 1280.0f);
	ImGui::SliderFloat("scale", &scale,0.1f,1.0f);
	ImGui::End();

	numberSprites_[0]->mainPosition.transform.scale = { scale ,scale ,1.0f };
#endif // DEBUG
}
