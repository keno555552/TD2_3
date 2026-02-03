#include "Explosion.h"

Explosion::Explosion(kEngine* system) :Particle(system) {
	createTimer.Init0(0.1f, system_->GetTimeManager()); // 生きる秒数
	defaultModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/suzanne/suzanne.obj");
	defaultTextureHandle_ = system_->LoadTexture("resources/TemplateResource/texture/circle.png");
	commonMaterialConfig->useModelTexture = false;
	commonMaterialConfig->textureHandle = defaultTextureHandle_;
}

void Explosion::Update(Camera* camera) {
	CreateExplosion();
	CreateExplosion();
	CreateExplosion();
	UpdateExplosion();
	DeleteExplosion();
	Particle::Update(camera);
}

void Explosion::Draw() {
	Particle::Draw();
}

void Explosion::CreateExplosion() {
	// 同時に数か所で発生させないようにする
	coolTimer_ += 1.0f / 60.0f;
	if (!isEmitting_ || coolTimer_ <= 0.5f) {
		return;
	}
	coolTimer_ = 0.0f;

	createTimer.ToZero(); // タイマー
	if (createTimer.parameter_ != 0) return;

	for (int i = 0; i < (int)createNumber_; i++) {
		// 最大数を越えていれば早期リターン
		if ((int)particleObjectList_.size() >= maxParticleNum_) return;

		Particle::AddObject();
		auto& p = particleObjectList_.back();

		p->part->CreateModelData(defaultModelHandle_);
		*p->part->objectParts_[0].materialConfig = *commonMaterialConfig;
		p->part->isBillboard_ = false;
		p->part->mainPosition.transform = CreateDefaultTransform();
		lifeTime_ = randomMaker_->randomFloat(0.5f, 1.0f); // 生存時間
		p->lifeTimeTimer.Init0(lifeTime_, system_->GetTimeManager());

		// ランダムな進む方向
		Vector3 direction = {
			randomMaker_->randomFloat(-0.1f, 0.1f),
			randomMaker_->randomFloat(-0.1f, 0.1f),
			randomMaker_->randomFloat(-0.1f, 0.1f)
		};
		direction = Normalize(direction);

		// ランダムな速さ
		float minSpeed = 0.01f;
		float maxSpeed = 0.05f;
		float speed = randomMaker_->randomFloat(minSpeed, maxSpeed);

		p->velocity = direction * speed;
		// ランダムなスケール
		float scale = randomMaker_->randomFloat(0.05f, 0.1f);
		p->part->objectParts_[0].transform.scale = Vector3(scale, scale, scale);
		p->size = scale;
		p->part->mainPosition.transform.translate = pos_; // 初期位置
	}
}

void Explosion::UpdateExplosion() {
	for (auto& p : particleObjectList_) {
		float t = p->lifeTimeTimer.parameter_ / p->lifeTimeTimer.maxTime_;
		//float easedT = t * t * t; // easeInCubic
		float easedT = t * t * t * t * t; // easeInQuint
		float scale = p->size * (1.0f - easedT) + 0.0f * easedT;
		p->part->objectParts_[0].transform.scale = Vector3(scale, scale, scale);
		p->part->objectParts_[0].materialConfig->textureColor.w = 1.0f; // アルファ値(念のため1.0fに設定)
		p->part->mainPosition.transform.translate += p->velocity;
	}
}

void Explosion::DeleteExplosion() {
	isEmitting_ = false;
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
