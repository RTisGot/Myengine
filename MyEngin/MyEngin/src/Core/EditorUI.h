#pragma once
#include <vector>
#include <string>
#include "GameObject.h"

struct GameObject;
//EditorUIクラス
class EditorUI {
public:
	static constexpr float kLeftPanelWidth = 320.0f;    //左側のパネルの幅
	static constexpr float kRightPanelWidth = 340.0f;   //右側のパネルの幅
	static constexpr float kTopBarHeight = 46.0f;       //上部のバーの高さ
	static constexpr float kBottomPanelHeight = 170.0f; //下部のパネルの高さ

	static void ShowMainEditor(bool& isPlayeing, std::vector<GameObject>& worldObject, std::vector<GameObject>& backup);
	static void PushLog(const std::string& message, bool isError = false);

private:
	static void ShowToolbar(bool& isPlaying, std::vector<GameObject>& worldObjects, std::vector<GameObject>& backup);
	static void ShowOutliner(std::vector<GameObject>& worldObjects);
	static void ShowDetailsWindow(std::vector<GameObject>& worldObjects);
	static void ShowOutputLog();
	static int selectedIndex;
	static int focusedIndex; // 
	static std::vector<std::string> outputLogs;
};

