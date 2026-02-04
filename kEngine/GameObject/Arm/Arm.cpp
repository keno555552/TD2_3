#include "Arm.h"
#include <numbers>

Arm::Arm(kEngine* system, Object* parent, bool isRight) {
    IntObject(system);

    // 新しい腕モデルの読み込み
    rightModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/arm/armL.obj"); // パスは適宜変更
    leftModelHandle_ = system_->SetModelObj("resources/TemplateResource/object/arm/armR.obj"); // パスは適宜変更
    
    isRight_ = isRight;
    if (isRight_) {
        Object::CreateModelData(rightModelHandle_);
    } else {
        Object::CreateModelData(leftModelHandle_);
    }

    // 親（プレイヤー）に追従設定
    LinkToObject(parent);

    // 初期回転など
}


void Arm::Update(Camera* camera, float gameScale) {

    // 1. スケールの適用
    // [最終スケール] = [モデル正規化] * [ゲーム倍率]
    float finalScale = kModelBaseScale_ * gameScale;
    mainPosition.transform.scale = { finalScale, finalScale, finalScale };


    // 2. オフセット（肩の位置）計算
    // BlenderでX:11.4mとのことだったので、それを基準にします。
    float modelOriginalWidth = 11.4f;

    // 現在のワールド空間での腕の「全幅」
    float currentWidth = modelOriginalWidth * finalScale;

    // 現在のワールド空間での腕の「半径（中心から端までの距離）」
    float currentHalfWidth = currentWidth / 2.0f;

    // ★ここが重要★
    // 「体の横（付け根）」に「腕の半径」を足した場所が、腕を置くべき中心点です。
    // これなら、いくら大きくなっても付け根の位置は不動になります。
    float offsetX = kBodyAttachmentX_ + currentHalfWidth;

    if (isRight_) {
        // 右腕はマイナス方向（Playerの実装に合わせる）
        mainPosition.transform.translate = { -offsetX, 0.0f, 0.0f };
    } else {
        // 左腕はプラス方向
        mainPosition.transform.translate = { +offsetX, 0.0f, 0.0f };
    }

    // 高さ(Y)や前後(Z)の微調整があればここで行う
    // mainPosition.transform.translate.y = 0.5f; 

    // 基底更新
    Object::Update(camera);
}

void Arm::Draw() {
    Object::Draw();
	//debugObject_->Draw();
}

void Arm::LinkToObject(Object* targetObject) {
    if (targetObject != nullptr) {
        followObject_ = &targetObject->mainPosition;
    }
}
