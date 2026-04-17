#include "Core/EditorUI.h"
#include "MyEngine.h"
#include "GameObject.h"
#include "MoveComponent.h"
#include "PlayerComponent.h"

int EditorUI::selectedIndex = -1;
int EditorUI::focusedIndex = -1;
std::vector<std::string> EditorUI::outputLogs;

void EditorUI::PushLog(const std::string& message, bool isError) {
    const std::string prefix = isError ? "[Error] " : "[Info] ";
    const std::string line = prefix + message;
    if (!outputLogs.empty() && outputLogs.back() == line) {
        return;
    }
    outputLogs.push_back(line);
    if (outputLogs.size() > 120) {
        outputLogs.erase(outputLogs.begin());
    }
}
void EditorUI::ShowMainEditor(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup) {
	ShowToolbar(isPlaying, worldObjects, backup);
	ShowOutliner(worldObjects);
    ShowDetailsWindow(worldObjects);
    ShowOutputLog();
}

void EditorUI::ShowToolbar(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup) {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(display.x, kTopBarHeight), ImGuiCond_Always);
	ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	{
		if (isPlaying) {
			if (ImGui::Button("Stop")) {
				isPlaying = false;
				worldObjects = backup; // ?o?b?N?A?b?v????????
			}
		}
		else {
			if (ImGui::Button("Play")) {
				isPlaying = true;
				backup = worldObjects; // ?????????o?b?N?A?b?v
			}
		}
        ImGui::SameLine();
        ImGui::TextUnformatted("|");
        ImGui::SameLine();
        ImGui::Text("Objects: %d", (int)worldObjects.size());
	}
	ImGui::End();
}

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
            //???Click
            if (ImGui::Selectable(worldObjects[i].name.c_str(), is_selected)) {
                selectedIndex = i;
                focusedIndex = i;
            }

            //?_?u???N???b?N????
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                focusedIndex = i; // ????I?u?W?F?N?g???u?????v????????
            }

            // ?E?N???b?N?????j???[
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Object")) {
                    worldObjects.erase(worldObjects.begin() + i);
                    if (selectedIndex == i) selectedIndex = -1;
                    if (focusedIndex == i) focusedIndex = -1;
                    if (selectedIndex > i) selectedIndex--;
                    if (focusedIndex > i) focusedIndex--;
                    i--; // ?v?f??????????? index ???
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
        }
    }
	ImGui::End();
}

// ?I???????I?u?W?F?N?g?????\??????E?B???h?E?????
void EditorUI::ShowDetailsWindow(std::vector<GameObject>& worldObjects) {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    const float panelHeight = display.y - kTopBarHeight;
    ImGui::SetNextWindowPos(ImVec2(display.x - kRightPanelWidth, kTopBarHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kRightPanelWidth, panelHeight), ImGuiCond_Always);
    ImGui::Begin("Details", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    if (focusedIndex < 0 || focusedIndex >= (int)worldObjects.size()) {
        ImGui::TextUnformatted("Select an object from Outliner.");
        ImGui::End();
        return;
    }

    GameObject& focusedObject = worldObjects[focusedIndex];
    {
        ImGui::Text("Editing: %s", focusedObject.name.c_str());
        ImGui::Separator();

        // ???O??X
        char buf[64];
        strcpy_s(buf, focusedObject.name.c_str());
        if (ImGui::InputText("Object Name", buf, 64)) {
            focusedObject.name = buf;
        }

        // ???W??W
        ImGui::DragFloat3("Position", &focusedObject.position.x, 0.1f);
        ImGui::DragFloat3("Rotation", &focusedObject.rotation.x, 1.0f);
        ImGui::DragFloat3("Scale", &focusedObject.scale.x, 0.1f);

        ImGui::Separator();//?\??????
        ImGui::Text("Components");
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("ComponentMenu");
        }

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
               
            }
            ImGui::EndPopup();
        }
        // ?R???|?[?l???g??UI??\??
        for (const auto& comp : focusedObject.components) {
            comp->OnGui();
        }

        if (ImGui::Button("Clear Selection")) {
            focusedIndex = -1;
            selectedIndex = -1;
        }
    }
    ImGui::End();
}

void EditorUI::ShowOutputLog() {
    ImVec2 display = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2(0.0f, display.y - kBottomPanelHeight), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(kLeftPanelWidth, kBottomPanelHeight), ImGuiCond_Always);
    ImGui::Begin("Output Log", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    if (ImGui::Button("Clear")) {
        outputLogs.clear();
    }
    ImGui::Separator();
    if (outputLogs.empty()) {
        ImGui::TextUnformatted("[Info] Ready.");
    }
    else {
        for (const std::string& line : outputLogs) {
            ImVec4 color = (line.find("[Error]") == 0) ? ImVec4(1.0f, 0.35f, 0.35f, 1.0f) : ImVec4(0.8f, 0.9f, 1.0f, 1.0f);
            ImGui::TextColored(color, "%s", line.c_str());
        }
    }
    ImGui::End();
}