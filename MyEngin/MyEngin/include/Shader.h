#pragma once
#include <string>
#include "Core/Math.h"

/**
* @brief シェーダーのコンパイルと使用を管理するクラス。
*/
class Shader {
public:
	unsigned int ID;                                           //GPU識別描画用番号
	Shader(const char* vertexPath, const char* fragmentPath);
	void use() const;

	//変数の値をシェーダーに送るための関数
	void setBool(const std::string& name, bool value) const;   //bool用関数
	void setInt(const std::string& name, int value) const;     //Int用関数
	void setFloat(const std::string& name, float value) const; //float用変数

	/**
 * @brief GPU側のUniform変数(vec3)に数値を転送。
 * @param name シェーダー内の変数名 (例: "lightColor")
 * @param value 転送する3次元ベクトル (x, y, z)
 */
	void setVec3(const std::string& name, const Vector3& value) const;

	/**
 * @brief 4x4行列(mat4)をシェーダーに転送。
 * @note 主にモデル・ビュー・プロジェクション行列（座標変換）に使用
 * @param name シェーダー内の変数名
 * @param mat 転送するGLM行列データ
 */
	void setMat4(const std::string& name, const Matrix4& mat) const;

private:
	//コンパイルエラーcheck用関数
	void CheckCompileErrors(unsigned int shader, std::string type);
};