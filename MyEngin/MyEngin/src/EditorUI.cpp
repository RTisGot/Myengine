#include "Core/EditorUI.h"
#include "MyEngine.h"
#include "GameObject.h"
#include "MoveComponent.h"

GameObject* EditorUI::focusedObject = nullptr;
void EditorUI::ShowMainEditor(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup) {
	ShowToolbar(isPlaying, worldObjects, backup);
	static int selected = -1; //選択内容のindexを初期化
	ShowOutliner(worldObjects, selected);
    ShowDetailsWindow();
}

void EditorUI::ShowToolbar(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup) {
	ImGui::Begin("Toolbar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	{
		if (isPlaying) {
			if (ImGui::Button("Stop")) {
				isPlaying = false;
				worldObjects = backup; // バックアップから元に戻す
			}
		}
		else {
			if (ImGui::Button("Play")) {
				isPlaying = true;
				backup = worldObjects; // 現在の状態をバックアップ
			}
		}
	}
	ImGui::End();
}

void EditorUI::ShowOutliner(std::vector<GameObject>& worldObjects, int& selectedIdx) {
    // 位置やサイズを main からこちらに移動
    ImGui::SetNextWindowPos(ImVec2(0, 50), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_Always);

    ImGui::Begin("Outliner", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    {
        ImGui::Text("World Hierarchy");
        ImGui::Separator();

        for (int i = 0; i < (int)worldObjects.size(); i++) {
            ImGui::PushID(i);
            bool is_selected = (selectedIdx == i);
            //一回Click
            if (ImGui::Selectable(worldObjects[i].name.c_str(), is_selected)) {
                selectedIdx = i;
            }

            //ダブルクリック判定
            if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
                focusedObject = &worldObjects[i]; // このオブジェクトを「詳細画面」の対象にする
            }

            // 右クリック削除メニュー
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Delete Object")) {
                    worldObjects.erase(worldObjects.begin() + i);
                    if (selectedIdx == i) selectedIdx = -1;
                    i--; // 要素を消したので index を調整
                }
                ImGui::EndPopup();
            }
            ImGui::PopID();
        }

        ImGui::Separator();
        if (ImGui::Button("Add Cube")) {
            worldObjects.push_back(GameObject("New Cube", glm::vec3(0, 0, 0)));
        }
    }
	ImGui::End();
}

// 選択されたオブジェクトの詳細を表示するウィンドウを作る
void EditorUI::ShowDetailsWindow() {
    if (!focusedObject) return; // 何もフォーカスされていなければ表示しない

    // 詳細ウィンドウ
    ImGui::Begin("Details Panel", nullptr);
    {
        ImGui::Text("Editing: %s", focusedObject->name.c_str());
        ImGui::Separator();

        // 名前変更
        char buf[64];
        strcpy_s(buf, focusedObject->name.c_str());
        if (ImGui::InputText("Object Name", buf, 64)) {
            focusedObject->name = buf;
        }

        // 座標編集
        ImGui::DragFloat3("Position", &focusedObject->position.x, 0.1f);

        ImGui::Separator();//表示分割
        ImGui::Text("Components");
        if (ImGui::Button("Add Component")) {
            ImGui::OpenPopup("ComponentMenu");
        }

        if (ImGui::BeginPopup("ComponentMenu")) {
            if (ImGui::MenuItem("Move Component")) {
				if (focusedObject != nullptr) {   //focusedObjectがnullptrでないことを確認
                    focusedObject->AddComponent(std::make_shared<MoveComponent>());
                }

            }
            if (ImGui::MenuItem("Physics Component")) {
               
            }
        }
        // コンポーネントのUIを表示
        for (auto comp : focusedObject->components) {
            comp->OnGui();
        }

        if (ImGui::Button("Close Details")) {
            focusedObject = nullptr; // 閉じるときの処理
        }
    }
    ImGui::End();
}