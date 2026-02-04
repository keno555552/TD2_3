#pragma once
#include "kEngine.h"
#include "Object/Object.h"
#include "DebugTool/DebugObject.h"

class Arm : public Object {
public:
    // isRight: 右腕か左腕かのフラグ（配置や回転の反転に使用）
    Arm(kEngine* system, Object* parent, bool isRight);

    // gameScale: プレイヤーのロジック上の腕倍率（1.0 ~ 5.0など）
    void Update(Camera* camera, float gameScale);
    void Draw() override;

private:
    // モデルハンドル
    int leftModelHandle_ = 0;
    int rightModelHandle_ = 0;

    // 右腕かどうか
    bool isRight_ = false;

    // ■■■ モデル正規化用定数 ■■■
    // モデルが約18mと巨大なので、ゲーム内の1mサイズに合わせるための係数
    const float kModelBaseScale_ = 0.09f;

    // ■■■ 位置調整用定数 ■■■
    // 「体の中心から、腕が生え始める位置（肩の付け根）」を定義します。
    // プレイヤーの体の幅が1.0mなら、中心から0.5mくらいが付け根です。
    const float kBodyAttachmentX_ = 0.8f;

    void LinkToObject(Object* targetObject);

};