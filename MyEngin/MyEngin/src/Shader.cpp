#include <GL/glew.h>
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <sstream>
#include <iostream>

/**
 * @brief シェーダープログラムのコンストラクタ
 * @param vertexPath 頂点シェーダー（.vert等）のファイルパス
 * @param fragmentPath フラグメントシェーダー（.frag等）のファイルパス
 * @details ソースファイルの読み込み、コンパイル、リンクを一括して行い、GPUプログラムを生成します。
 */
Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    // ファイル読み込み時の例外フラグ設定（論理エラー、致命的エラー）
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        // --- 1. ファイルストリームからシェーダーソースを読み込む ---
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;

        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();

        vShaderFile.close();
        fShaderFile.close();

        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
    }

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    // --- 2. シェーダーのコンパイル ---
    unsigned int vertex, fragment;

    // 頂点シェーダー (Vertex Shader)
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    CheckCompileErrors(vertex, "VERTEX");

    // フラグメントシェーダー (Fragment Shader)
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    CheckCompileErrors(fragment, "FRAGMENT");

    // --- 3. シェーダープログラムのリンク ---
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    CheckCompileErrors(ID, "PROGRAM");

    // リンク後は個別のシェーダーオブジェクトは不要なため削除
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

/**
 * @brief このシェーダープログラムを現在のレンダリングパイプラインで使用する
 */
void Shader::use() const {
    glUseProgram(ID);
}

// ----------------------------------------------------------------------------
// Uniform変数設定関数群（CPUからGPUへのデータ転送）
// ----------------------------------------------------------------------------

void Shader::setInt(const std::string& name, int value) const {
    // ユニフォーム変数のロケーションを取得し、整数値を転送
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setBool(const std::string& name, bool value) const {
    // 真偽値は整数にキャストして転送（GLSL内ではboolとして扱われる）
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setFloat(const std::string& name, float value) const {
    // 浮動小数点数を転送
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const Vector3& value) const {
    // 3成分ベクトル（RGBやXYZ）を転送
    glUniform3f(glGetUniformLocation(ID, name.c_str()), value.x, value.y, value.z);
}

void Shader::setMat4(const std::string& name, const Matrix4& mat) const {
    // 4x4行列（MVP行列等）を転送。列優先(GL_FALSE)を指定
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

/**
 * @brief コンパイルおよびリンクのエラーチェック
 * @param shader チェック対象のID (ShaderまたはProgram)
 * @param type チェック対象の種類 ("VERTEX", "FRAGMENT", "PROGRAM")
 */
void Shader::CheckCompileErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];

    if (type != "PROGRAM") {
        // シェーダーのコンパイル状況を取得
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else {
        // プログラムのリンク状況を取得
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}