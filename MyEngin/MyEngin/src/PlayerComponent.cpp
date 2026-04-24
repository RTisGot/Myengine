#include "PlayerComponent.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "Core/EditorUI.h"

/**
 * @brief インスペクター上でのプレイヤー設定UI
 * @details 視点切り替えの提供と、動作に必要なコンポーネントの依存関係チェックを行います。
 */
void PlayerComponent::OnGui() {
    // ヘッダーが閉じられている場合は描画をスキップ
    if (!ImGui::CollapsingHeader("Player Component", ImGuiTreeNodeFlags_DefaultOpen)) {
        return;
    }

    // 表示用ラベルの定義
    const char* viewItems[] = { "First Person", "Third Person" };
    int current = (viewMode == ViewMode::FirstPerson) ? 0 : 1;

    // --- 依存関係チェック ---
    // プレイヤーとして機能するためには移動制御（MoveComponent）が必須であるという設計意図
    const bool hasMoveComponent = owner && owner->HasComponent<MoveComponent>();

    if (!hasMoveComponent) {
        // エディタ上で視覚的に警告を表示
        ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.35f, 1.0f), "Warning: MoveComponent is required.");

        // システムログにエラーを通知（一度だけ出力されるようEditorUI側で制御されていることを想定）
        EditorUI::PushLog("PlayerComponent: MoveComponent is missing, functionality is restricted.", true);
    }

    // --- UI操作の有効/無効制御 ---
    // 依存コンポーネントがない場合は、設定変更による不具合を防ぐためにコンボボックスをグレーアウト
    ImGui::BeginDisabled(!hasMoveComponent);

    if (ImGui::Combo("View Mode", &current, viewItems, IM_ARRAYSIZE(viewItems))) {
        // 選択されたインデックスに基づいてViewMode列挙型を更新
        viewMode = (current == 0) ? ViewMode::FirstPerson : ViewMode::ThirdPerson;
    }

    ImGui::EndDisabled();
}
