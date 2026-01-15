#include "P_DashWind.h"
#include <numbers>

P_DashWind::P_DashWind(kEngine* system) :Particle(system){
	createTimer.Init0(0.1f, system_->GetTimeManager());
	defaultModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/suzanne/suzanne.obj");
	defaultTextureHandle_ = system_->LoadTextrue("resources/TemplateResource/texture/circle.png");
	commonMaterialConfig->useModelTexture = false;
	commonMaterialConfig->textureHandle = defaultTextureHandle_;
}

void P_DashWind::Update(Camera* camera) {
	CreateDashWind();
	UpdateDashWind();
	DeleteDashWind();
	Particle::Update(camera);
}

void P_DashWind::Draw() {
	Particle::Draw();
}

void P_DashWind::CreateDashWind() {
	if (!isEmitting_) return;
	createTimer.foreverUp();
	if (createTimer.parameter_ != 0) return;

	for (int i = 0; i < (int)createNumber_; i++) {
		if ((int)particleObjectList_.size() >= maxParticleNum_) return;

		Particle::AddObject();
		auto& p = particleObjectList_.back();

		p->part->CreateModelData(defaultModelHandle_);
		p->lifeTimeTimer.Init0(lifeTime_, system_->GetTimeManager());
		*p->part->objectParts_[0].materialConfig = *commonMaterialConfig;
		p->part->mainPosition.transform = CreateDefaultTransform();
		p->part->mainPosition.transform.translate = generationPos_;
		p->part->objectParts_[0].transform.scale = Vector3(0.05f, 0.05f, 0.05f);
		p->part->isBillboard_ = true;

		/*
		Vector3 up = { 0.0f,1.0f,0.0f };
		if (fabsf(Dot(targetForward_, up)) > 0.99f) {
			up = { 1.0f,0.0f,0.0f };
		}
		Vector3 u = Normalize(Cross(up, targetForward_));
		Vector3 v = Cross(targetForward_, u);

		std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
		float theta = distribution(randomEngine) * 2.0f * std::numbers::pi_v<float>;

		Vector3 diskDir = cosf(theta) * u + sinf(theta) * v;
		Vector3 offset = diskDir * radius_;

		Vector3 spawnPos = generationPos_ + targetForward_ * 0.5f + offset;
		p->part->mainPosition.transform.translate = spawnPos;

		Vector3 dir = Normalize((targetForward_)+Normalize(diskDir) * 0.3f);

		Vector3 velocity = dir * speed_;
		
		p->velocity.x = randomMaker_->randomFloat(0.2f, 1.0f);
		p->velocity = velocity;
		*/
		Vector3 f = Normalize(targetForward_);
		Vector3 up = { 0.0f, 1.0f, 0.0f };
		if (fabsf(Dot(f, up)) > 0.99f) {
			up = { 1.0f, 0.0f, 0.0f };
		}

		Vector3 u = Normalize(Cross(up, targetForward_));
		Vector3 v = Cross(f, u); // u,v,f が直交

		float theta = randomMaker_->randomFloat(0.0f, 1.0f) * 2.0f * std::numbers::pi_v<float>;

		Vector3 dir = u * cosf(theta) + v * sinf(theta);

		//p->velocity.x = randomMaker_->randomFloat(-1.0f, 1.0f);
		p->velocity = dir * speed_;
	}
}

void P_DashWind::UpdateDashWind() {
	targetForward_ = Normalize(targetForward_);
	for (auto& p : particleObjectList_) {
		float t = p->lifeTimeTimer.parameter_ / p->lifeTimeTimer.maxTime_;
		// フェードアウト
		p->part->objectParts_[0].materialConfig->textureColor.w = 1.0f - t;

		// ついでに縮めたいなら（Particle基底はscale更新してないのでここで）
		float s = 1.0f - t;
		//p->part->mainPosition.transform.scale = Vector3(s, s, s);

		p->part->mainPosition.transform.translate += p->velocity;
		Vector3 f = Vector3{ -targetForward_.x,-targetForward_.y,-targetForward_.z };
		p->part->mainPosition.transform.translate += Normalize(f) * speed_;
	}
}

void P_DashWind::DeleteDashWind() {
	for (auto& p : particleObjectList_) {
		if (p->lifeTimeTimer.parameter_ >= p->lifeTimeTimer.maxTime_) {
			p->isAlive = false;
			continue;
		}
	}
}
