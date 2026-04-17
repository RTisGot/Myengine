#pragma once
#include "Core/Component.h"
#include "../imgui/imgui.h"

class PlayerComponent : public Component {
public:
    enum class ViewMode {
        FirstPerson = 0,
        ThirdPerson = 1
    };

    ViewMode viewMode = ViewMode::ThirdPerson;

    void OnGui() override;
};
