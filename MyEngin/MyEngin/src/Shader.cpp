#include <GL/glew.h>
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
	std::string vertexCode;   //バーテックスシェーダーのコード
	std::string fragmentCode; //フラグメントシェーダーのコード
	std::ifstream vShaderFile; //バーテックス(頂点)シェーダーファイル
	std::ifstream fShaderFile; //フラグメント(色)シェーダーファイル
	//例外処理
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);//論理的または致命的なエラーが発生したときに例外をアラートするように設定
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try {
		//ファイルを開く
		vShaderFile.open(vertexPath);
		fShaderFile.open(fragmentPath);
		std::stringstream vShaderStream, fShaderStream;
		//ファイルの内容をストリームに読み込む
		vShaderStream << vShaderFile.rdbuf();
		fShaderStream << fShaderFile.rdbuf();
		//ファイルを閉じる
		vShaderFile.close();
		fShaderFile.close();
		//ストリームの内容を文字列に変換するして格納
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
	}
	//エラー時
	catch (std::ifstream::failure& e) {//読み込み失敗時に変数eで受け取り
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;//エラー内容を出力
	}
	const char* vShaderCode = vertexCode.c_str();     //バーテックスシェーダーコードのC文字列への変換
	const char* fShaderCode = fragmentCode.c_str();   //フラグメントシェーダーコードのC文字列への変換
	unsigned int vertex, fragment;  //シェーダーオブジェクトのID
	//バーテックスシェーダーのコンパイル
	vertex = glCreateShader(GL_VERTEX_SHADER);    //バーテックスシェーダーオブジェクトの作成
	glShaderSource(vertex, 1, &vShaderCode, NULL); //シェーダーコードをオブジェクトにセットする
	glCompileShader(vertex);                      //シェーダーをコンパイルする
	CheckCompileErrors(vertex, "VERTEX");         //コンパイルエラーをチェックする

	fragment = glCreateShader(GL_FRAGMENT_SHADER);    //フラグメントシェーダーオブジェクトの作成
	glShaderSource(fragment, 1, &fShaderCode, NULL); //シェーダーコードをオブジェクトにセットする
	glCompileShader(fragment);                      //シェーダーをコンパイルする
	CheckCompileErrors(fragment, "FRAGMENT");       //コンパイルエラーをチェックする

	ID = glCreateProgram();                      //プログラムオブジェクトの作成
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	CheckCompileErrors(ID, "PROGRAM");

	glDeleteShader(vertex);
	glDeleteShader(fragment);
}

void Shader::use() const {
	glUseProgram(ID);      //ID番のプログラムを有効に(切り替え)
}

//--------------------変数関数内容-----------------------//
void Shader::setInt(const std::string& name, int value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);//glGetUniformLocationで取得した位置にvalue(値)を書き込む
}

void Shader::setBool(const std::string& name, bool value) const {
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);//glGetUniformLocationで取得した位置にvalue(値)を書き込む
}

void Shader::setFloat(const std::string& name, float value) const {
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);//glGetUniformLocationで取得した位置にvalue(値)を書き込む
}

//--------------------関数内容-----------------------//
void Shader::setVec3(const std::string& name, const Vector3& value) const {
	glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);//glGetUniformLocationで取得した位置に順番に書き込む
}

void Shader::setMat4(const std::string& name, const Matrix4& mat) const {
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);//glGetUniformLocationで取得した位置に行列データを書き込む
}

void Shader::CheckCompileErrors(unsigned int shader, std::string type) {
	int success; //コンパイル成功したかどうかのフラグ
	char infoLog[1024];//エラー内容を格納する文字列
	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);//コンパイルの情報をsuccessに格納する
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}