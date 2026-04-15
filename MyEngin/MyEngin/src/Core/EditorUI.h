#pragma once
#include <vector>
#include "GameObject.h"

// エディタのUIを管理するクラス
class EditorUI{
public:

	static void ShowMainEditor(bool& isPlayeing, std::vector<GameObject>& worldObject, std::vector<GameObject>& backup);

private:
	static void ShoeToolbar(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup);

};