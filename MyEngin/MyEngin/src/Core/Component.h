#pragma once
#include <string>

class GameObject; // 前方宣言

class Component {
public:
	GameObject* owner; //自分がどのオブジェクトについているか
	virtual ~Component(){}

	virtual void BeginPlay(){}           //初期化
	virtual void Update(float dt) {}//マイフレーム呼ばれる
	virtual void OnGui() {}         //エディタでの表示用
};
