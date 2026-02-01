#include "Tutorial.h"

Tutorial::Tutorial(kEngine* system) {
	system_ = system;

	// タイマーの初期化（DefaultMenuを参考）
	animationTimer_ = new Timer();
	animationTimer_->Init0(kAnimTime_, system_->GetTimeManager());
}

Tutorial::~Tutorial() {
	// メモリ解放
	for (Object* obj : pages_) {
		delete obj;
	}
	pages_.clear();
	delete animationTimer_;
}

void Tutorial::AddPage(const std::string& modelPath) {
	// ■■■ リストへの追加処理 ■■■
	// 1. 新しいオブジェクトを生成
	Object* newPage = new Object();
	newPage->IntObject(system_);

	// 2. モデルをロードして紐付け (モデルの場合)
	// ※画像(スプライト)の場合は LoadTextrue と SimpleSprite を使ってください
	int handle = system_->SetModelObj(modelPath);
	newPage->CreateModelData(handle);

	// 3. 初期座標などの設定
	// 画面中央や、カメラの目の前に配置するなど調整
	newPage->mainPosition.transform.translate = { 0.0f, 0.0f, 0.0f };
	newPage->mainPosition.transform.scale = { 0.0f, 0.0f, 0.0f }; // 最初は非表示
	newPage->isBillboard_ = true;

	// 4. リストに追加
	pages_.push_back(newPage);
}

void Tutorial::Open(int index) {
	// 範囲外チェック
	if (index < 0 || index >= pages_.size()) return;

	// 指定されたページをセット
	currentPageIndex_ = index;

	// 開く処理
	state_ = TutorialState::Opening;
	animationTimer_->Reset0(); // タイマーリセット
}

void Tutorial::Close() {
	if (state_ == TutorialState::Hidden) return;
	state_ = TutorialState::Closing;
	animationTimer_->Reset0();
}

void Tutorial::Update(Camera* camera) {
	// 非表示なら何もしない
	if (state_ == TutorialState::Hidden) return;

	// タイマー更新
	// (Timerクラスの仕様に合わせて呼び出し。ToMix等はDefaultMenuを参考に)
	animationTimer_->ToMix();
	float t = animationTimer_->linearity(); // 0.0 -> 1.0

	switch (state_) {
	case TutorialState::Opening:
		// 出現アニメーション（スケール 0 -> 1）
		UpdateCurrentPageAnim(animationTimer_->easyOut(0.0f, 1.0f, t), camera);
		//UpdateCurrentPageAnim(1.0f, camera);
		if (animationTimer_->parameter_ >= animationTimer_->maxTime_) {
			state_ = TutorialState::Active;
		}
		break;

	case TutorialState::Active:
		// 表示中（1.0で維持）
		UpdateCurrentPageAnim(1.0f, camera);

		// 入力待ち (決定キーやクリックなど)
		if (system_->GetTriggerOn(DIK_RETURN) || system_->GetTriggerOn(DIK_SPACE) || system_->GetGamepadTriggerOn(VK_PAD_A)) {
			NextPage(camera);
		}
		// スキップ/閉じる (Bボタンなど)
		if (system_->GetTriggerOn(DIK_ESCAPE) || system_->GetGamepadTriggerOn(VK_PAD_B)) {
			Close();
		}
		break;

	case TutorialState::Closing:
		// 終了アニメーション（スケール 1 -> 0）
		UpdateCurrentPageAnim(animationTimer_->easyIn(1, 0, t), camera);
		if (animationTimer_->parameter_ >= animationTimer_->maxTime_) {
			state_ = TutorialState::Hidden;
			// 完全に閉じたらスケール0にしておく
			UpdateCurrentPageAnim(0.0f, camera);
		}
		break;
	}

	// 表示中のオブジェクトだけUpdateを呼ぶ（カメラ回転に対応させるため等）
	if (currentPageIndex_ < pages_.size()) {
		// カメラ情報が必要なら引数で受け取るか、nullptrで更新
		// 説明テキストがビルボードならカメラが必要
		pages_[currentPageIndex_]->Update(camera);
	}
}

void Tutorial::Draw() {
	if (state_ == TutorialState::Hidden) return;

	// 現在のページだけ描画
	if (currentPageIndex_ < pages_.size()) {
		pages_[currentPageIndex_]->Draw();
	}
}

void Tutorial::NextPage(Camera* camera) {
	// ページを進める
	currentPageIndex_++;

	// リストの最後を超えたらチュートリアル終了
	if (currentPageIndex_ >= pages_.size()) {
		// 最後のページだったので閉じる
		Close();
		// インデックスを安全な値に戻しておく
		currentPageIndex_ = (int)pages_.size() - 1;
	} else {
		// 次のページへ切り替え（アニメーションなど必要ならここでリセット）
		// 今回は瞬時に切り替える実装ですが、Closing -> Openingとし直してもリッチになります

		// 前のページのスケールをリセットし、今のページのスケールをセット
		if (currentPageIndex_ - 1 >= 0) {
			pages_[currentPageIndex_ - 1]->mainPosition.transform.scale = { 0,0,0 };
		}
		UpdateCurrentPageAnim(1.0f, camera);
	}
}

void Tutorial::UpdateCurrentPageAnim(float scale, Camera* camera) {
	if (currentPageIndex_ < pages_.size()) {

		// 1. モデルのスケール設定
		float baseScale = 1.0f; // 大きすぎた場合はここを小さく(0.1fなど)調整
		float finalScale = baseScale * scale;

		Object* obj = pages_[currentPageIndex_];
		obj->mainPosition.transform.scale = { finalScale, finalScale, finalScale };


		// ■■■■■■ 修正箇所: カメラの目線の先に配置する計算 ■■■■■■

		// カメラの情報取得
		Vector3 camPos = camera->GetPosition();
		Vector3 camRot = camera->GetTransform().rotate; // ラジアン(Radian)前提

		// カメラからの距離（近すぎると見切れるので調整）
		float distance = 5.0f;

		// カメラの回転から「正面ベクトル(Forward)」を計算
		// 一般的な3D座標系（Y-Up）での計算式
		// X回転(Pitch): 上下の首振り
		// Y回転(Yaw)  : 左右の首振り
		Vector3 forward;
		forward.x = std::sin(camRot.y) * std::cos(camRot.x);
		forward.y = -std::sin(camRot.x); // ※注: エンジンによって符号(+/-)が逆の場合があります
		forward.z = std::cos(camRot.y) * std::cos(camRot.x);

		// カメラの位置から、正面方向に distance 分だけ進んだ場所を計算
		obj->mainPosition.transform.translate = {
			camPos.x + forward.x * distance,
			camPos.y + forward.y * distance,
			camPos.z + forward.z * distance
		};

		// 3. 回転の同期
		// Planeなどの2D板ポリゴンは、カメラと同じ向きにしないと裏が見えたり薄くなったりします
		// isBillboard_ = true にしていても、基本の回転は合わせておくと安全です
		obj->mainPosition.transform.rotate = camRot;
	}
}