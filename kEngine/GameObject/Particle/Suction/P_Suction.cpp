#include "P_Suction.h"

P_Suction::P_Suction(kEngine* system) :Particle(system) {
	createTimer.Init0(0.1f, system_->GetTimeManager());
	defaultModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/suzanne/suzanne.obj");
	defaultTextureHandle_ = system_->LoadTextrue("resources/TemplateResource/texture/circle.png");
	commonMaterialConfig->useModelTexture = false;
	commonMaterialConfig->textureHandle = defaultTextureHandle_;
}

void P_Suction::Update(Camera* camera) {
	CreateSuction();
	UpdateSuction();
	DeleteSuction();
	Particle::Update(camera);
}

void P_Suction::Draw() {
	Particle::Draw();
}

void P_Suction::CreateSuction() {
	if (!isLeftEmitting_ && !isRightEmitting_) return; // パーティクルを出さない場合早期リターン
	createTimer.foreverUp();
	if (createTimer.parameter_ != 0) return;

	EmitterDesc emitters[2] = {
		{&isLeftEmitting_,&targetPosLeft_,&armScaleL_},
		{&isRightEmitting_,&targetPosRight_,&armScaleR_}
	};
	for (auto& e : emitters) { // 左右回すfor文
		if (!(*e.isEmitting)) continue;

		const uint32_t count = createNumber_;

		for (uint32_t i = 0; i < count; i++) {
			if ((int)particleObjectList_.size() >= maxParticleNum_) return;

			Particle::AddObject();

			auto& p = particleObjectList_.back();

			p->part->CreateModelData(defaultModelHandle_);
			p->lifeTimeTimer.Init0(lifeTime_, system_->GetTimeManager());
			*p->part->objectParts_[0].materialConfig = *commonMaterialConfig;
			p->part->mainPosition.transform = CreateDefaultTransform();
			p->part->objectParts_[0].transform.scale = Vector3(0.1f, 0.1f, 0.1f);
			p->part->objectParts_[0].transform.scale = Vector3(0.05f, 0.05f, 0.05f);
			p->part->isBillboard_ = true;

			Vector3& tp = *e.targetPos;
			const float s = *e.armScale;

			p->part->mainPosition.transform.translate.x = tp.x + randomMaker_->randomFloat(-s, s);
			p->part->mainPosition.transform.translate.y = tp.y + randomMaker_->randomFloat(0.0f, s);
			p->part->mainPosition.transform.translate.z = tp.z + randomMaker_->randomFloat(-s, s);

			Vector3 direction = tp - p->part->mainPosition.transform.translate;
			direction = Normalize(direction);

			p->velocity = direction * speed_;
		}
	}
}

void P_Suction::UpdateSuction() {
	for (auto& p : particleObjectList_) {
		float t = p->lifeTimeTimer.parameter_ / p->lifeTimeTimer.maxTime_;
		// ついでに縮めたいなら
		float s = 1.0f - t;
		//p->part->mainPosition.transform.scale = Vector3(s, s, s);
		p->part->mainPosition.transform.translate += p->velocity;
	}
}

void P_Suction::DeleteSuction() {
	for (auto& p : particleObjectList_) {
		if (p->lifeTimeTimer.parameter_ >= p->lifeTimeTimer.maxTime_) {
			p->isAlive = false;
			continue;
		}
	}
}
