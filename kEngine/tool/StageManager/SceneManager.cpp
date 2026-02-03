#include "SceneManager.h"
#include <cmath>
#include <algorithm>

// ★ テクスチャは「一度だけロードして使い回す」
static int gShojiFrameTexHandle = -1;
static int gShojiPaperTexHandle = -1;

// ★ 障子遷移専用：旧シーンのカメラが消えても障子を描画し続けるための保持カメラ
static Camera* gShojiHoldCam = nullptr;

// =====================================================
// Camera Utility（Cross 不使用・安定版）
// =====================================================
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

float SceneManager::EaseOutSine(float t)
{
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return sinf(t * 1.57079632679f);
}

Vector3 SceneManager::CalcForward(Vector3 r)
{
    const float PI = 3.14159265359f;
    const float TWO_PI = 6.28318530718f;

    while (r.y > PI)  r.y -= TWO_PI;
    while (r.y < -PI) r.y += TWO_PI;
    while (r.x > PI)  r.x -= TWO_PI;
    while (r.x < -PI) r.x += TWO_PI;

    float cx = cosf(r.x);
    float sx = sinf(r.x);
    float cy = cosf(r.y);
    float sy = sinf(r.y);

    return NormalizeSafe(Vector3(
        sy * cx,
        -sx,
        cy * cx
    ));
}

Vector3 SceneManager::CalcRight(const Vector3& forward)
{
    Vector3 up(0.0f, 1.0f, 0.0f);
    return NormalizeSafe(Cross(up, forward));
}

Vector3 SceneManager::CalcBaseFromCamera(
    const Vector3& camPos,
    const Vector3& camRot,
    const Vector3& localOffset
)
{
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

// =====================================================
// ★ HoldCam utility
// =====================================================
static void EnsureShojiHoldCam(kEngine* system) {
    if (gShojiHoldCam) return;
    gShojiHoldCam = system->CreateCamera();
    gShojiHoldCam->SetFarClip(20000.0f); // とりあえず広め
}

static void CopyCameraToHoldCam(Camera* src, Camera* dst) {
    if (!src || !dst) return;

    // src の位置・回転を dst にコピー
    Vector3 p = src->GetPosition();
    Vector3 r = src->GetTransform().rotate;

    dst->Move(p);
    dst->GetTransform().rotate = r;
    dst->Update();
}

// =====================================================

SceneManager::SceneManager(kEngine* system) {
    system_ = system;
    sceneFactory_ = new SceneFactory(system_);
    sceneUsingNameHandle_ = "TITLE";

    if (gShojiFrameTexHandle < 0)
        gShojiFrameTexHandle = system_->LoadTexture("resources/gate/model/wood_tate.png");
    if (gShojiPaperTexHandle < 0)
        gShojiPaperTexHandle = system_->LoadTexture("resources/gate/model/paper.png");

    // ★ HoldCam を先に作っておく（遷移中に必ず描画できる）
    EnsureShojiHoldCam(system_);

    defaultMenu_ = new DefaultMenu(system_);
}

SceneManager::~SceneManager() {
    Shoji_DestroyAll();
    delete sceneUsing_;
    sceneUsing_ = nullptr;

    // gShojiHoldCam は static 共有なのでここでは delete しない（多重解放防止）
}

Camera* SceneManager::ResolveCamera(BaseScene* s) {
    if (!s) return nullptr;
    if (auto* t = dynamic_cast<SceneTitle*>(s)) return t->camera_;
    if (auto* tu = dynamic_cast<SceneTutorial*>(s)) return tu->GetCamera();
    if (auto* st = dynamic_cast<SceneStage01*>(s)) return st->GetCamera();
    if (auto* rt = dynamic_cast<SceneResult*>(s)) return rt->GetCamera();
    return nullptr;
}

void SceneManager::SceneChanger() {

    if (shojiPlaying_) return;

    // ★ ここが必須：シーンが無いなら生成する
    if (!sceneUsing_) {
        sceneUsing_ = sceneFactory_->CreateScene(sceneUsingNameHandle_);
        return;
    }

    // ===== ここから遷移判定 =====
    if (sceneUsing_->GetOutcome() == SceneOutcome::NEXT) {

        auto it = sceneFlow_.find(sceneUsingNameHandle_);
        if (it == sceneFlow_.end()) return;

        nextAfterShoji_ = it->second;

        Camera* currentCam = ResolveCamera(sceneUsing_);
        if (!currentCam) return;

        // ★ 旧シーンカメラの情報を HoldCam にコピーして、以降は HoldCam を使う
        EnsureShojiHoldCam(system_);
        CopyCameraToHoldCam(currentCam, gShojiHoldCam);
        shojiCam_ = gShojiHoldCam; // これで旧シーン削除しても障子カメラが死なない

        // 現在カメラから見て +Z に 4
        shojiBasePos_ = Vector3(0.0f, 0.0f, 4.0f);

        Vector3 camPos = shojiCam_->GetPosition();
        Vector3 camRot = shojiCam_->GetTransform().rotate;
        Vector3 base = CalcBaseFromCamera(camPos, camRot, shojiBasePos_);

        Shoji_CreateAll(camRot);
        Shoji_SetInitialOpenPose(base, camRot);

        shojiT_ = 0.0f;
        shojiWaitTimer_ = 0.0f;
        shojiPhase_ = ShojiPhase::Closing;
        shojiPlaying_ = true;
    }
}

void SceneManager::Update() {

    float dt = std::clamp(system_->GetDeltaTime(), 0.0f, 0.1f);

    if (shojiPlaying_) {
        if (shojiPhase_ == ShojiPhase::Closing || shojiPhase_ == ShojiPhase::Opening)
            Shoji_UpdateClosingOpening(dt);
        else if (shojiPhase_ == ShojiPhase::WaitBeforeTransition)
            Shoji_UpdateWaitBeforeTransition(dt);
        else if (shojiPhase_ == ShojiPhase::WaitNewCamera)
            Shoji_UpdateWaitNewCamera(dt);
        return;
    }

    SceneChanger();
    if (sceneUsing_) sceneUsing_->Update();
}

void SceneManager::Render() {

    if (sceneUsing_) {
        Camera* cam = ResolveCamera(sceneUsing_);
        if (cam) {
            system_->SetCamera(cam);
            sceneUsing_->Draw();
        }
    }

    // ★ shojiCam_ が常に有効（HoldCam or 新シーンCam）なので、ここで消えない
    if (shojiCam_) {
        system_->SetCamera(shojiCam_);
        if (shojiL_) shojiL_->Draw();
        if (shojiR_) shojiR_->Draw();
        if (shojiL_paper_) shojiL_paper_->Draw();
        if (shojiR_paper_) shojiR_paper_->Draw();
    }
}

// =====================================================
// Shoji
// =====================================================

void SceneManager::Shoji_CreateAll(const Vector3& camRot) {

    // 既にあるなら作り直さない（リーク防止）
    if (shojiL_ || shojiR_ || shojiL_paper_ || shojiR_paper_) return;

    shojiL_ = new Object;
    shojiR_ = new Object;
    shojiL_paper_ = new Object;
    shojiR_paper_ = new Object;

    shojiL_->IntObject(system_);
    shojiR_->IntObject(system_);
    shojiL_paper_->IntObject(system_);
    shojiR_paper_->IntObject(system_);

    shojiL_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_left.obj"));
    shojiR_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_right.obj"));
    shojiL_paper_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_left_paper.obj"));
    shojiR_paper_->CreateModelData(system_->SetModelObj("resources/gate/model/shoji_right_paper.obj"));

    // ★ テクスチャ設定（木枠）
    for (auto& part : shojiL_->objectParts_) {
        if (!part.materialConfig) part.materialConfig = std::make_shared<MaterialConfig>();
        part.materialConfig->useModelTexture = false;
        part.materialConfig->textureHandle = gShojiFrameTexHandle;
        part.materialConfig->enableLighting = true;
    }
    for (auto& part : shojiR_->objectParts_) {
        if (!part.materialConfig) part.materialConfig = std::make_shared<MaterialConfig>();
        part.materialConfig->useModelTexture = false;
        part.materialConfig->textureHandle = gShojiFrameTexHandle;
        part.materialConfig->enableLighting = true;
    }

    // ★ テクスチャ設定（和紙）
    for (auto& part : shojiL_paper_->objectParts_) {
        if (!part.materialConfig) part.materialConfig = std::make_shared<MaterialConfig>();
        part.materialConfig->useModelTexture = false;
        part.materialConfig->textureHandle = gShojiPaperTexHandle;
        part.materialConfig->enableLighting = false;
    }
    for (auto& part : shojiR_paper_->objectParts_) {
        if (!part.materialConfig) part.materialConfig = std::make_shared<MaterialConfig>();
        part.materialConfig->useModelTexture = false;
        part.materialConfig->textureHandle = gShojiPaperTexHandle;
        part.materialConfig->enableLighting = false;
    }

    // ★ スケール復活（勝手に消さない）
    shojiL_->mainPosition.transform.scale = { 0.025f, 0.025f, 0.025f };
    shojiR_->mainPosition.transform.scale = { 0.025f, 0.025f, 0.025f };
    shojiL_paper_->mainPosition.transform.scale = { 0.025f, 0.025f, 0.025f };
    shojiR_paper_->mainPosition.transform.scale = { 0.025f, 0.025f, 0.025f };

    shojiL_->mainPosition.transform.rotate = camRot;
    shojiR_->mainPosition.transform.rotate = camRot;
    shojiL_paper_->mainPosition.transform.rotate = camRot;
    shojiR_paper_->mainPosition.transform.rotate = camRot;
}

void SceneManager::Shoji_SetInitialOpenPose(const Vector3& base, const Vector3& camRot) {

    Vector3 forward = CalcForward(camRot);
    Vector3 right = CalcRight(forward);
    const float off = 10.2f;

    // 左（木枠）
    Vector3 leftPos = base;
    leftPos.x -= right.x * off;
    leftPos.y -= right.y * off;
    leftPos.z -= right.z * off;
    shojiL_->mainPosition.transform.translate = leftPos;

    // 右（木枠）
    Vector3 rightPos = base;
    rightPos.x += right.x * off;
    rightPos.y += right.y * off;
    rightPos.z += right.z * off;
    shojiR_->mainPosition.transform.translate = rightPos;

    // 左（紙）
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

    shojiL_->mainPosition.transform.rotate = camRot;
    shojiR_->mainPosition.transform.rotate = camRot;
    shojiL_paper_->mainPosition.transform.rotate = camRot;
    shojiR_paper_->mainPosition.transform.rotate = camRot;

    // ★ Update は必ず非nullカメラで
    if (shojiCam_) {
        shojiL_->Update(shojiCam_);
        shojiR_->Update(shojiCam_);
        shojiL_paper_->Update(shojiCam_);
        shojiR_paper_->Update(shojiCam_);
    }
}

void SceneManager::Shoji_UpdateClosingOpening(float dt) {

    if (!shojiCam_) return;

    if (shojiPhase_ == ShojiPhase::Closing) {
        shojiT_ += dt;
        if (shojiT_ > 1.0f) shojiT_ = 1.0f;
    }
    else {
        shojiT_ -= dt;
        if (shojiT_ < 0.0f) shojiT_ = 0.0f;
    }

    Vector3 camPos = shojiCam_->GetPosition();
    Vector3 camRot = shojiCam_->GetTransform().rotate;
    Vector3 base = CalcBaseFromCamera(camPos, camRot, shojiBasePos_);

    Vector3 forward = CalcForward(camRot);
    Vector3 right = CalcRight(forward);

    float off = (1.0f - EaseOutSine(shojiT_)) * 10.2f;

    Vector3 leftPos = base;
    leftPos.x -= right.x * off;
    leftPos.y -= right.y * off;
    leftPos.z -= right.z * off;

    Vector3 rightPos = base;
    rightPos.x += right.x * off;
    rightPos.y += right.y * off;
    rightPos.z += right.z * off;

    shojiL_->mainPosition.transform.rotate = camRot;
    shojiR_->mainPosition.transform.rotate = camRot;
    shojiL_paper_->mainPosition.transform.rotate = camRot;
    shojiR_paper_->mainPosition.transform.rotate = camRot;

    shojiL_->mainPosition.transform.translate = leftPos;
    shojiR_->mainPosition.transform.translate = rightPos;
    shojiL_paper_->mainPosition.transform.translate = leftPos;
    shojiR_paper_->mainPosition.transform.translate = rightPos;

    shojiL_->Update(shojiCam_);
    shojiR_->Update(shojiCam_);
    shojiL_paper_->Update(shojiCam_);
    shojiR_paper_->Update(shojiCam_);

    // ★ Closing完了 → 1秒待ってからシーン切替
    if (shojiPhase_ == ShojiPhase::Closing && shojiT_ >= 1.0f) {

        sceneUsingNameHandle_ = nextAfterShoji_;

        shojiWaitTimer_ = 0.0f;

        // ★ ここで shojiCam_ を切らない（HoldCam で描画継続するため）
        //    旧シーンはまだ残ってるが、shojiCam_ は HoldCam なので安全

        shojiPhase_ = ShojiPhase::WaitBeforeTransition;
        return;
    }

    if (shojiPhase_ == ShojiPhase::Opening && shojiT_ <= 0.0f) {
        Shoji_DestroyAll();
        return;
    }
}

void SceneManager::Shoji_UpdateWaitBeforeTransition(float dt) {

    shojiWaitTimer_ += dt;
    if (shojiWaitTimer_ < 1.0f) {
        // ★ 待ち中も HoldCam で描画し続ける。Update は不要（閉じたまま）
        return;
    }

    // ★ ここで初めてシーン切り替え
    delete sceneUsing_;
    sceneUsing_ = nullptr;

    sceneUsing_ = sceneFactory_->CreateScene(sceneUsingNameHandle_);

    // ★ 新シーンのカメラが来るまで、shojiCam_ は HoldCam のまま
    //    → だから遷移後も一瞬も消えない

    shojiPhase_ = ShojiPhase::WaitNewCamera;
}

void SceneManager::Shoji_UpdateWaitNewCamera(float dt) {

    if (!sceneUsing_) return;

    // ★ 新シーンを先に回す（カメラ生成待ち）
    sceneUsing_->Update();

    Camera* newCam = ResolveCamera(sceneUsing_);
    if (!newCam) {
        // ★ まだ新カメラが無い：HoldCam のまま描画継続
        return;
    }

    // ★ ここで初めて新カメラへ切替（同フレームで障子も配置更新するのでチラつかない）
    shojiCam_ = newCam;

    shojiBasePos_ = Vector3(0.0f, 0.0f, 4.0f);

    Vector3 camPos = shojiCam_->GetPosition();
    Vector3 camRot = shojiCam_->GetTransform().rotate;
    Vector3 base = CalcBaseFromCamera(camPos, camRot, shojiBasePos_);

    if (!shojiL_ || !shojiR_ || !shojiL_paper_ || !shojiR_paper_) {
        Shoji_CreateAll(camRot);
    }

    shojiL_->mainPosition.transform.rotate = camRot;
    shojiR_->mainPosition.transform.rotate = camRot;
    shojiL_paper_->mainPosition.transform.rotate = camRot;
    shojiR_paper_->mainPosition.transform.rotate = camRot;

    shojiL_->mainPosition.transform.translate = base;
    shojiR_->mainPosition.transform.translate = base;
    shojiL_paper_->mainPosition.transform.translate = base;
    shojiR_paper_->mainPosition.transform.translate = base;

    shojiL_->Update(shojiCam_);
    shojiR_->Update(shojiCam_);
    shojiL_paper_->Update(shojiCam_);
    shojiR_paper_->Update(shojiCam_);

    shojiT_ = 1.0f;
    shojiPhase_ = ShojiPhase::Opening;
}

void SceneManager::Shoji_DestroyAll() {
    delete shojiL_; shojiL_ = nullptr;
    delete shojiR_; shojiR_ = nullptr;
    delete shojiL_paper_; shojiL_paper_ = nullptr;
    delete shojiR_paper_; shojiR_paper_ = nullptr;

    shojiPlaying_ = false;
    shojiPhase_ = ShojiPhase::None;

    // ★ shojiCam_ は次の遷移で HoldCam をセットし直すのでここでは触らない
}
