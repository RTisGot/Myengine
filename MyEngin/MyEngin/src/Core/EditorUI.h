#pragma once
#include <vector>
#include "GameObject.h"

struct GameObject;
// エディタのUIを管理するクラス
class EditorUI {
public:

	static void ShowMainEditor(bool& isPlayeing, std::vector<GameObject>& worldObject, std::vector<GameObject>& backup);

private:
	static void ShowToolbar(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup);
	static void ShowOutliner(std::vector<GameObject>& worldObjects, int& selectedIdx);
	static void ShowDetailsWindow();
	static GameObject* focusedObject; // 現在エディタで選択されているオブジェクトへのポインタ
};

