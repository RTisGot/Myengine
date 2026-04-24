#pragma once
#include <vector>
#include <string>

// GameObject.h への依存を最小限にするための前方宣言
struct GameObject;

/**
 * @class EditorUI
 * @brief ゲームエンジンのエディタインターフェース（GUI）を管理するクラス
 * @details ImGuiを用いて、シーン構成、オブジェクト詳細、ログ出力などのウィンドウを構築します。
 */
class EditorUI {
public:
    // --- エディタレイアウトの定数定義（ピクセル単位） ---
    static constexpr float kLeftPanelWidth = 320.0f;    // シーンアウトライナー等の幅
    static constexpr float kRightPanelWidth = 340.0f;   // インスペクター（詳細）パネルの幅
    static constexpr float kTopBarHeight = 46.0f;       // ツールバーの高さ
    static constexpr float kBottomPanelHeight = 170.0f; // ログ表示等の下部パネルの高さ

    /**
     * @brief エディタ全体のメインUI描画を実行する
     * @param isPlaying 現在のPIE（実行中）フラグ
     * @param worldObjects 現在のシーンに存在する全オブジェクト
     * @param backup PIE開始・停止時に状態を保存するためのバックアップコンテナ
     */
    static void ShowMainEditor(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup);

    /**
     * @brief エディタのログウィンドウにメッセージを送信する
     * @param message 表示するテキスト内容
     * @param isError エラー（赤文字）として表示するかどうか
     */
    static void PushLog(const std::string& message, bool isError = false);

private:
    /**
     * @brief 上部ツールバー（再生・停止ボタン、設定など）を描画
     */
    static void ShowToolbar(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup);

    /**
     * @brief シーンアウトライナー（オブジェクトの一覧表示・選択）を描画
     */
    static void ShowOutliner(std::vector<GameObject>& worldObjects);

    /**
     * @brief 詳細ウィンドウ（選択中オブジェクトのTransformやComponentの編集）を描画
     */
    static void ShowDetailsWindow(std::vector<GameObject>& worldObjects);

    /**
     * @brief コンソールログウィンドウの描画
     */
    static void ShowOutputLog();

    // 内部状態管理変数
    static int selectedIndex;               // 現在選択されているオブジェクトのリスト内インデックス
    static int focusedIndex;                // フォーカスされているUI項目のインデックス
    static std::vector<std::string> outputLogs; // 蓄積されたログメッセージの履歴
};