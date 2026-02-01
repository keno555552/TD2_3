#pragma once
#include "kEngine.h"
#include "Object/Object.h"
#include <vector>
#include <string>
#include <algorithm>

// チュートリアルの状態
enum class TutorialState {
	Hidden,     // 非表示
	Opening,    // 出現アニメーション
	Active,     // 表示中（操作待ち）
	Closing     // 終了アニメーション
};

class Tutorial {
public:
	Tutorial(kEngine* system);
	~Tutorial();

	void Update(Camera* camera);
	void Draw();

	// ■■■ 要望機能: 指定した番号のテキストを表示して開始 ■■■
	// index: 表示したいページの番号 (デフォルトは0)
	void Open(int index = 0);

	// 強制終了
	void Close();

	// ■■■ 要望機能: パスを渡すだけでページを追加 ■■■
	// modelPath: "resources/..." などのファイルパス
	void AddPage(const std::string& modelPath);

	// 表示中かどうか（プレイヤーの操作停止用などに）
	bool IsActive() const { return state_ != TutorialState::Hidden; }

private:
	kEngine* system_ = nullptr;

	// ■ ページ管理用リスト
	// Handleを個別に変数で持つのではなく、リストで一括管理します
	std::vector<Object*> pages_;

	// 現在のページ番号
	int currentPageIndex_ = 0;

	// 状態管理
	TutorialState state_ = TutorialState::Hidden;

	// アニメーション用タイマー（DefaultMenuと同じTimerクラスを使用）
	Timer* animationTimer_ = nullptr;
	const float kAnimTime_ = 0.3f;

	// 入力制御（押しっぱなし防止）
	bool isPushDecide_ = false;

	// 内部処理: ページの切り替え
	void NextPage(Camera* camera);
	// 内部処理: 現在のページの座標やスケール更新
	void UpdateCurrentPageAnim(float scale, Camera* camera);
};