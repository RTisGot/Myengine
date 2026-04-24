#include "Core/EditorUI.h"
#include "MyEngine.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "PlayerComponent.h"

// 静的メンバ変数の実体定義と初期化
int EditorUI::selectedIndex = -1;
int EditorUI::focusedIndex = -1;
std::vector<std::string> EditorUI::outputLogs;

/**
 * @brief エディタコンソールにメッセージを出力
 * @param message ログ内容
 * @param isError エラー表示（赤文字）フラグ
 */
void EditorUI::PushLog(const std::string& message, bool isError) {
    const std::string prefix = isError ? "[Error] " : "[Info] ";
    const std::string line = prefix + message;

    // 同一メッセージの連続投稿を抑制
    if (!outputLogs.empty() && outputLogs.back() == line) {
        return;
    }

    outputLogs.push_back(line);

    // ログの最大保持数を120行に制限（メモリリーク防止）
    if (outputLogs.size() > 120) {
        outputLogs.erase(outputLogs.begin());
    }
}

/**
 * @brief エディタ全体の描画関数
 * @details 各ウィンドウの描画関数を順次呼び出し、レイアウトを構成します。
 */
void EditorUI::ShowMainEditor(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup) {
    ShowToolbar(isPlaying, worldObjects, backup);
    ShowOutliner(worldObjects);
    ShowDetailsWindow(worldObjects);
    ShowOutputLog();
}

/**
 * @brief ツールバーウィンドウ（再生/停止制御）
 */
void EditorUI::ShowToolbar(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup) {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(display.x, kTopBarHeight), ImGuiCond_Always);

    ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    {
        if (isPlaying) {
            if (ImGui::Button("Stop")) {
                isPlaying = false;
                worldObjects = backup; // 実行前の状態に復元（PIEの終了）
                PushLog("Play-In-Editor stopped. State restored.");
            }
        }
        else {
            if (ImGui::Button("Play")) {
                isPlaying = true;
                backup = worldObjects; // 現在の状態を保存（PIEの開始）
                PushLog("Play-In-Editor started.");
            }
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("|");
        ImGui::SameLine();
        ImGui::Text("Active Objects: %d", (int)worldObjects.size());
    }
    ImGui::End();
}

/**
 * @brief アウトライナーウィンドウ（シーンの階層構造表示）
 */
void EditorUI::ShowOutliner(std::vector<GameObject>& worldObjects) {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    const float outlinerHeight = display.y - kTopBarHeight - kBottomPanelHeight;
    ImGui::SetNextWindowPos(ImVec2(0.0f, kTopBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kLeftPanelWidth, outlinerHeight), ImGuiCond_Always);

    ImGui::Begin("Outliner", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    {
        ImGui::Text("World Hierarchy");
        ImGui::Separator();

        for (int i = 0; i < (int)worldObjects.size(); i++) {
            ImGui::PushID(i);
            bool is_selected = (selectedIndex == i);

            // リストからのオブジェクト選択
            if (ImGui::Selectable(worldObjects[i].name.c_str(), is_selected)) {
                selectedIndex = i;
                focusedIndex = i;
            }

            // ダブルクリック判定（将来的なフォーカス移動用）
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                focusedIndex = i;
            }

            // 右クリックによるコンテキストメニュー
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Object")) {
                    worldObjects.erase(worldObjects.begin() + i);
                    if (selectedIndex == i) selectedIndex = -1;
                    if (focusedIndex == i) focusedIndex = -1;
                    if (selectedIndex > i) selectedIndex--;
                    if (focusedIndex > i) focusedIndex--;
                    i--; // 要素削除に伴うインデックス調整
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }

        ImGui::Separator();
        if (ImGui::Button("Add Cube")) {
            worldObjects.push_back(GameObject("New Cube", glm::vec3(0, 0, 0)));
            selectedIndex = (int)worldObjects.size() - 1;
            focusedIndex = selectedIndex;
            PushLog("New Cube created at origin.");
        }
    }
    ImGui::End();
}

/**
 * @brief 詳細ウィンドウ（オブジェクトのトランスフォーム・コンポーネント編集）
 */
void EditorUI::ShowDetailsWindow(std::vector<GameObject>& worldObjects) {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    const float panelHeight = display.y - kTopBarHeight;
    ImGui::SetNextWindowPos(ImVec2(display.x - kRightPanelWidth, kTopBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kRightPanelWidth, panelHeight), ImGuiCond_Always);
    ImGui::Begin("Details", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    // オブジェクト未選択時のプレースホルダー表示
    if (focusedIndex < 0 || focusedIndex >= (int)worldObjects.size()) {
        ImGui::TextUnformatted("Select an object from Outliner.");
        ImGui::End();
        return;
    }

    GameObject& focusedObject = worldObjects[focusedIndex];
    {
        ImGui::Text("Editing: %s", focusedObject.name.c_str());
        ImGui::Separator();

        // 共通プロパティ：オブジェクト名
        char buf[64];
        strcpy_s(buf, focusedObject.name.c_str());
        if (ImGui::InputText("Object Name", buf, 64)) {
            focusedObject.name = buf;
        }

        // 共通プロパティ：トランスフォーム
        ImGui::DragFloat3("Position", &focusedObject.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &focusedObject.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &focusedObject.scale.x, 0.1f);

        ImGui::Separator();
        ImGui::Text("Components");
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("ComponentMenu");
        }

        // コンポーネント追加用メニュー
        if (ImGui::BeginPopup("ComponentMenu")) {
            if (ImGui::MenuItem("Move Component")) {
                focusedObject.AddComponent(std::make_shared<MoveComponent>());
                PushLog("MoveComponent added to " + focusedObject.name + ".");
            }
            if (ImGui::MenuItem("Player Component")) {
                focusedObject.AddComponent(std::make_shared<PlayerComponent>());
                PushLog("PlayerComponent added to " + focusedObject.name + ".");
            }
            if (ImGui::MenuItem("Physics Component")) {
                // TODO: 物理エンジンの統合時に実装
            }
            ImGui::EndPopup();
        }

        // 各コンポーネント固有のUIを描画（ポリモーフィズム）
        for (const auto& comp : focusedObject.components) {
            comp->OnGui();
        }

        ImGui::Separator();
        if (ImGui::Button("Clear Selection")) {
            focusedIndex = -1;
            selectedIndex = -1;
        }
    }
    ImGui::End();
}

/**
 * @brief 出力ログウィンドウ（コンソール）
 */
void EditorUI::ShowOutputLog() {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, display.y - kBottomPanelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kLeftPanelWidth, kBottomPanelHeight), ImGuiCond_Always);
    ImGui::Begin("Output Log", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (ImGui::Button("Clear Log")) {
        outputLogs.clear();
    }
    ImGui::Separator();

    if (outputLogs.empty()) {
        ImGui::TextUnformatted("[Info] System ready.");
    }
    else {
        // 各ログ行の描画（エラーは赤色で表示）
        for (const std::string& line : outputLogs) {
            ImVec4 color = (line.find("[Error]") == 0) ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f) : ImVec4(0.8f, 0.9f, 1.0f, 1.0f);
            ImGui::TextColored(color, "%s", line.c_str());
        }
    }

    // 自動スクロール（最新ログを常に表示）
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::End();
}