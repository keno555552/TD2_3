#include "P_DashWind.h"
#include <numbers>

P_DashWind::P_DashWind(kEngine* system) :Particle(system){
	createTimer.Init0(0.1f, system_->GetTimeManager());
	system_;
	defaultModelHandle_ = system_->SetModelObj("resources/game/model/Particle_Dash.obj");
	defaultTextureHandle_ = system_->LoadTexture("resources/game/model/Particle_Dash.png");
	commonMaterialConfig->useModelTexture = false;
	commonMaterialConfig->textureHandle = defaultTextureHandle_;
}

void P_DashWind::Update(Camera* camera) {
	CreateDashWind(); // 生成、スポーン
	UpdateDashWind(); // 更新
	DeleteDashWind(); //
	Particle::Update(camera);
}

void P_DashWind::Draw() {
	Particle::Draw();
}

void P_DashWind::CreateDashWind() {
	if (!isEmitting_) return; // パーティクルしなければ早期リターン
	createTimer.foreverUp(); // タイマー
	if (createTimer.parameter_ != 0) return;

	for (int i = 0; i < (int)createNumber_; i++) {
		if ((int)particleObjectList_.size() >= maxParticleNum_) return;

		Particle::AddObject();
		auto& p = particleObjectList_.back();

		//lifeTime_ = armScale_;

		p->part->CreateModelData(defaultModelHandle_);
		p->lifeTimeTimer.Init0(lifeTime_ * armScale_, system_->GetTimeManager());
		*p->part->objectParts_[0].materialConfig = *commonMaterialConfig;


		p->part->mainPosition.transform = CreateDefaultTransform();
		p->part->mainPosition.transform.translate = generationPos_;
		p->part->objectParts_[0].transform.scale = Vector3(0.05f, 0.05f, 0.05f);
		p->part->objectParts_[0].transform.scale = Vector3(0.2f, 0.2f, 0.2f);
		p->part->isBillboard_ = false;

		
		Vector3 forward = Normalize(targetForward_); // 正面
		Vector3 up = { 0.0f, 1.0f, 0.0f }; // 
		if (fabsf(Dot(forward, up)) > 0.99f) {
			up = { 1.0f, 0.0f, 0.0f };
		}

		Vector3 u = Normalize(Cross(up, targetForward_));
		Vector3 v = Cross(forward, u); // u,v,f が直交

		float theta = randomMaker_->randomFloat(0.0f, 1.0f) * 2.0f * std::numbers::pi_v<float>;

		Vector3 direction = u * cosf(theta) + v * sinf(theta);

		p->velocity = direction * (speed_ * armScale_);
		p->velocity = direction * speed_;
	}
}

void P_DashWind::UpdateDashWind() {
	targetForward_ = Normalize(targetForward_);
	for (auto& p : particleObjectList_) {
		float t = p->lifeTimeTimer.parameter_ / p->lifeTimeTimer.maxTime_;
		// フェードアウト
		p->part->objectParts_[0].materialConfig->textureColor.w = 1.0f - t;

		// ついでに縮めたいなら（Particle基底はscale更新してないのでここで）
		//float s = 1.0f - t;
		float s = t * t;
		p->part->mainPosition.transform.scale = Vector3(s, s, s);

		p->part->mainPosition.transform.translate += p->velocity;
		Vector3 f = Vector3{ -targetForward_.x,-targetForward_.y,-targetForward_.z };
		//p->part->mainPosition.transform.translate += Normalize(f) * speed_;
	}
}

void P_DashWind::DeleteDashWind() {
	/*
	for (auto& p : particleObjectList_) {
		if (p->lifeTimeTimer.parameter_ >= p->lifeTimeTimer.maxTime_) {
			p->isAlive = false;
			// 中に new した Object を先に delete
			delete p->part;
			p->part = nullptr;

			// ParticleData 自体を delete
			delete p;
			
			continue;
		}
	}
	particleObjectList_.clear();
	*/
	particleObjectList_.erase(
		std::remove_if(particleObjectList_.begin(), particleObjectList_.end(),
			[](ParticleData* p) {
				if (!p) return true;

				if (p->lifeTimeTimer.parameter_ >= p->lifeTimeTimer.maxTime_) {
					delete p->part;
					p->part = nullptr;
					delete p;
					return true; // ← vectorからも消す
				}
				return false;
			}),
		particleObjectList_.end());
}
