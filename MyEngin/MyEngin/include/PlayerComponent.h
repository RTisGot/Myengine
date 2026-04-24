#pragma once
#include "Core/Component.h"
#include "../imgui/imgui.h"

/**
 * @class PlayerComponent
 * @brief プレイヤー特有の振る舞いやカメラ視点を制御するコンポーネント
 * @details 実行中の一人称/三人称視点の切り替えや、プレイヤー固有のパラメータを管理します。
 */
class PlayerComponent : public Component {
public:
    /**
     * @brief カメラの表示モード定義
     */
    enum class ViewMode {
        FirstPerson = 0, // 一人称視点
        ThirdPerson = 1  // 三人称視点
    };

    // 現在選択されている表示モード（デフォルトは三人称）
    ViewMode viewMode = ViewMode::ThirdPerson;

    /**
     * @brief エディタのInspector上に表示する設定画面
     * @details ImGuiのコンボボックス等を用いて、視点モードを即座に切り替え可能にします。
     */
    void OnGui() override;
};