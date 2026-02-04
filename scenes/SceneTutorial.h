#pragma once
#include "BaseScene.h"
#include "Object/Object.h"
#include "Object/Sprite.h"
#include "Player/Player.h" // プレイヤーヘッダ（既存のものを使用）
#include "Enemy/Enemy.h"  // 敵ヘッダ
#include "DebugCamera.h"
#include <vector>
#include <algorithm> // clamp用
#include "kEngine/GameObject/Particle/DashWind/P_DashWind.h"
#include "kEngine/GameObject/Particle/Suction/P_Suction.h"
#include "GameObject/Particle/P_PlaneReduction/P_PlaneReduction.h"
#include "GameObject/Particle/explosion/Explosion.h"

// チュートリアルの進行状況
enum class TutorialStep {
    Move,       // 1. 基本移動（旋回）
    ArmExpand,  // 2. 腕の膨張
    Dash,       // 3. 加速（ダッシュ）
    Battle,     // 4. 無敵と無防備（敵を殴る）
    Caution,    // 5. 注意事項（テキストのみ）
    Finished    // 完了
};

class SceneTutorial : public BaseScene {
public:
    SceneTutorial(kEngine* system);
    ~SceneTutorial() override;

    void Update() override;
    void Draw() override;

    Camera* GetCamera() override;

private:
    // --- 内部ヘルパー関数 ---
    void UpdateStepMove();      // 旋回判定
    void UpdateStepExpand();    // 膨張判定
    void UpdateStepDash();      // ダッシュ判定
    void UpdateStepBattle();    // 戦闘判定
    void UpdateStepCaution();   // 注意事項確認
    // 現在のステップの進捗率(0.0f ~ 1.0f)を計算する関数
    float GetProgressRatio();

    // テキストモデルの切り替え
    void ChangeTextModel(const std::string& modelPath);

    /// ============= シーン内命令 =============///
    void CameraPart();

    /// ============= 衝突判定関数 ============/// 
    // 攻撃による当たり判定
    void CheckAttackCollision();

private:
    // --- 基本コンポーネント ---
    Camera* camera_ = nullptr;
    /// 今使っているカメラ
    Camera* usingCamera_ = nullptr;			// 借り用
    /// カメラ距離
    float cameraDistance_ = 15.0f; // 実数値
    float cameraDistanceMultiply_ = 7.0f; // 倍率値
    /// カメラ高さ(実数値)
    float cameraHeight_ = 9.0f; // 実数値
    float cameraHeightMultiply_ = 4.0f; // 倍率値
    // ▼▼▼ 手動ズーム制御用 ▼▼▼
    float manualZoomOffset_ = 15.0f;        // 現在の手動ズーム量（0が基準）
    const float kManualZoomSpeed_ = 10.0f; // ズームの変化速度
    const float kMinManualZoom_ = -5.0f;   // 最大ズームイン（近づける限界）
    const float kMaxManualZoom_ = 15.0f;   // 最大ズームアウト（離れる限界）
    // ズームアウトした時に、少し視点も高くするかどうかの係数
    // (1.0なら距離と同じだけ高さも増える。0.0なら高さは変わらない)
    const float kManualZoomHeightRatio_ = 0.5f;
    // ▼▼▼ カメラ演出用変数 ▼▼▼
    bool isCameraTransiting_ = false;   // 移動演出中フラグ
    float cameraTransitionTimer_ = 0.0f; // 経過時間
    const float kCameraTransitionDuration_ = 2.0f; // 移動にかける時間
    // 補間用：開始地点と終了地点
    Vector3 camAnimStartPos_{};
    Vector3 camAnimEndPos_{};
    Vector3 camAnimStartRot_{};
    Vector3 camAnimEndRot_{};

    Object* skydome_ = nullptr;

    const Vector3 kBaseActorScale_ = Vector3(0.5f, 0.5f, 0.5f);
    Player* player_ = nullptr;
    int playerModelHandle_ = 0;

    // 左右の情報をまとめる
    struct ArmInfo {
        bool isRight;
        Vector3 pos;
        float scale;
        bool isAtk;
    };
    // チュートリアル用テキスト表示モデル
    Object* textObj_ = nullptr;

    Object* tatami_ = nullptr;

    // --- 進行管理 ---
    TutorialStep currentStep_ = TutorialStep::Move;

    // ★追加: 進捗バー用スプライト
    SimpleSprite* barFrame_ = nullptr; // 背景（枠）
    SimpleSprite* barBrank_ = nullptr; // 空（伸びた状態）
    SimpleSprite* barFill_ = nullptr;  // 中身（伸びる方）

    // バーの最大幅（スケール1.0の時のサイズ、または基準スケール）
    const Vector2 kBarPos_ = { 360.0f, 20.0f }; // 画面上中央あたり
    const Vector2 kBarScale_ = { 120.0f, 4.0f }; // バーのサイズ

    // ステップ遷移時のウェイト用
    float stepWaitTimer_ = 0.0f;
    bool isStepCompleted_ = false;

    // 1. 移動用変数
    float rotateTimer_ = 0.0f;       // 旋回している時間
    const float kRotateQuota_ = 2.0f;// 2秒間旋回したらクリア

    // 3. ダッシュ用変数
    int dashSuccessCount_ = 0;       // 成功回数
    const int kDashQuota_ = 2;       // 2回成功でクリア
    bool wasCharging_ = false;       // 前フレームでチャージしていたか
    bool wasDashing_ = false;        // 前フレームでダッシュしていたか

    // 4. バトル用変数
    // 敵管理用の構造体
    struct TutorialEnemyData {
        Enemy* enemy = nullptr; // 敵本体
        bool isDefeated = false; // 倒されたかどうかのフラグ
        // ▼▼▼ 出現アニメーション用 ▼▼▼
        bool isSpawning = true;     // 出現演出中フラグ
        float spawnTimer = 0.0f;    // 経過時間
        Vector3 spawnStartPos;      // 開始位置（上空）
        Vector3 spawnEndPos;        // 着地点（地面）
    };
    std::vector<TutorialEnemyData> enemies_;
    int killCount_ = 0;
    const int kKillQuota_ = 2;       // 2体倒したらクリア
    float spawnTimer_ = 0.0f;        // 敵が死んだ後の再スポーン用

    // パーティクル用変数
    P_DashWind* dashWindEffectL_ = nullptr;
    P_DashWind* dashWindEffectR_ = nullptr;

    P_Suction* suctionEffect_ = nullptr;

    P_PlaneReduction* planeReductionEffectL_ = nullptr;
    P_PlaneReduction* planeReductionEffectR_ = nullptr;

    Explosion* explosion_ = nullptr;
};