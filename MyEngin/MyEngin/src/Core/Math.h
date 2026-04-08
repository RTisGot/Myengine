#pragma once

/**
* 関数や変数を簡単に定義するためのヘッダーファイル
* */
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>

using Vector3 = glm::vec3;  //3次元ベクトルの型定義
using Vector4 = glm::vec4;  //4次元ベクトルの型定義
using Matrix4 = glm::mat4;	//4x4行列の型定義

const Vector3 Vector3Up(0.0f, 1.0f, 0.0f);
const Vector3 Vector3Zero(0.0f, 0.0f, 0.0f);