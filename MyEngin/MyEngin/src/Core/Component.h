#pragma once
#include <string>
#include <GLFW/glfw3.h>

class GameObject; // 前方宣言

class Component {
public:
	GameObject* owner = nullptr; //自分がどのオブジェクトについているか
	virtual ~Component(){}

	virtual void BeginPlay(){}           //初期化
	virtual void Update(float dt, GLFWwindow* window) {}//マイフレーム呼ばれる
	virtual void OnGui() {}         //エディタでの表示用
};
