#define GLEW_STATIC
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"
#include "Mesh.h"
#include "GameObject.h"
#include "Tagsystem.h"
#include "Core/EditorUI.h"
#include "PlayerComponent.h"
#include "MoveComponent.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/type_ptr.hpp>
#include <vector>
#include <iostream>

// --- グローバル設定・リソース定義 ---

glm::mat4 modelMatrix = glm::mat4(1.0f); // モデル行列の初期化（単位行列）
const GLint WIDTH = 800, HEIGHT = 600;   // デフォルトのウィンドウサイズ

// 頂点データ(Vertex Data): 立方体を構成するユニークな8頂点を定義
std::vector<Vertex> vertices = {
	// 前面 (Z+)
	{{-0.5f, -0.5f,  0.5f}}, {{ 0.5f, -0.5f,  0.5f}}, {{ 0.5f,  0.5f,  0.5f}}, {{-0.5f,  0.5f,  0.5f}},
	// 背面 (Z-)
	{{-0.5f, -0.5f, -0.5f}}, {{ 0.5f, -0.5f, -0.5f}}, {{ 0.5f,  0.5f, -0.5f}}, {{-0.5f,  0.5f, -0.5f}}
};

// インデックスデータ(Index Data / EBO): 頂点配列を参照して三角形ポリゴンを構築
// メモリ使用量を削減し、GPUのキャッシュ効率を最適化する
std::vector<unsigned int> indices = {
	0, 1, 2, 2, 3, 0, // 前面
	1, 5, 6, 6, 2, 1, // 右面
	7, 6, 5, 5, 4, 7, // 背面
	4, 0, 3, 3, 7, 4, // 左面
	3, 2, 6, 6, 7, 3, // 上面
	4, 5, 1, 1, 0, 4  // 下面
};

/**
 * @brief ユーザー入力の監視と反映
 * @param window GLFWウィンドウへのポインタ
 * @details ESCキー押下時に終了フラグを立てる等のリアルタイム入力を処理する
 */
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

// --- シーン管理・永続データ ---

std::vector<GameObject> worldObjects; // アクティブな全ゲームオブジェクト
std::vector<GameObject> editorBackup; // PIE(Play In Editor)停止時の復元用バックアップ

int main()
{
	// エディタの実行状態管理フラグ
	bool isPlaying = false;

	// トランスフォーム操作用変数
	static float position[3] = { 0.0f, 0.0f, 0.0f };
	static float rotation = 0.0f;
	static float scale = 1.0f;

	// --- GLFWの初期化 ---
	if (!glfwInit())
	{
		std::cerr << "Fatal Error: GLFWの初期化に失敗しました。" << std::endl;
		glfwTerminate();
		return 1;
	}

	// OpenGLコンテキストプロパティの設定 (Version 3.3 Core Profile)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // macOS互換性確保

	// プライマリモニターの解像度に基づいたフルスクリーンウィンドウの生成
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);

	GLFWwindow* window = glfwCreateWindow(
		mode->width,
		mode->height,
		"My Engine",
		NULL, // ウィンドウモードの場合はNULL
		NULL
	);

	if (window) glfwSetWindowPos(window, 100, 100);

	if (!window)
	{
		std::cerr << "Fatal Error: GLFWウィンドウの生成に失敗しました。" << std::endl;
		glfwTerminate();
		return 1;
	}

	// フレームバッファ（実際の描画ピクセルサイズ）の取得
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(window, &bufferWidth, &bufferHeight);

	glfwMakeContextCurrent(window); // 現在のコンテキストを対象に設定
	glewExperimental = GL_TRUE;     // モダンな機能の有効化

	// --- GLEWの初期化 (拡張機能のロード) ---
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Fatal Error: GLEWの初期化に失敗しました。" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	glEnable(GL_DEPTH_TEST); // 深度テストを有効化（3D空間の前後関係を正しく描画）

	// --- ImGui初期設定 (ミドルウェアのセットアップ) ---
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
	ImGui::StyleColorsDark(); // エディタらしいダークテーマを適用

	// --- アセット・リソースのロード ---
	Shader ourShader("shader.vert", "shader.frag"); // カスタムシェーダーのコンパイル
	Mesh myMesh(vertices, indices);                 // プリミティブメッシュの生成

	// --- エディタ用カメラ・制御変数 ---
	static float yaw = -90.0f;   // 水平回転
	static float pitch = 0.0f;   // 垂直回転
	float radius = 5.0f;         // 注視点からの距離
	static double lastX = 400, lastY = 300;
	static bool firstMouse = true;

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	bool isDragging = false;       // オブジェクトドラッグ状態フラグ
	bool lastLeftState = false;    // 前フレームのマウス左ボタン状態
	float dragDistance = 0.0f;
	glm::vec3 dragOffset;
	static int selected = -1;      // 現在選択中のオブジェクトID
	static Vector3 spawnPos;
	worldObjects.reserve(100);     // メモリ再確保を抑制するための予約
	double lastFrameTime = glfwGetTime();

	// ------------------------------------------------------------------
	// メインゲームループ
	// ------------------------------------------------------------------
	while (!glfwWindowShouldClose(window))
	{
		// デルタタイム(dt)の算出
		const double currentTime = glfwGetTime();
		const float dt = static_cast<float>(currentTime - lastFrameTime);
		lastFrameTime = currentTime;

		// ビューポート計算 (左右パネルを除いた中央領域を算出)
		int display_w, display_h;
		glfwGetFramebufferSize(window, &display_w, &display_h);

		const int leftPanelWidth = (int)EditorUI::kLeftPanelWidth;
		const int rightPanelWidth = (int)EditorUI::kRightPanelWidth;
		const int topBarHeight = (int)EditorUI::kTopBarHeight;
		const int viewportWidth = display_w - leftPanelWidth - rightPanelWidth;
		const int viewportHeight = display_h - topBarHeight;

		glViewport(leftPanelWidth, 0, viewportWidth, viewportHeight);

		// 射影行列の更新 (アスペクト比追従)
		float aspect = (float)viewportWidth / (float)viewportHeight;
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspect, 0.1f, 100.0f);

		glfwPollEvents();

		// ImGuiフレーム開始シーケンス
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		processInput(window);

		// PIE実行中のロジック更新
		if (isPlaying) {
			for (auto& obj : worldObjects) {
				obj.Update(dt, window);
			}
			Tagsystem::Update(worldObjects);
		}

		// エディタUIの描画
		EditorUI::ShowMainEditor(isPlaying, worldObjects, editorBackup);

		// --- カメラ座標計算 (Arcball Camera) ---
		static float camPos[3] = { 0.0f, 0.0f, 3.0f };
		Matrix4 model = Matrix4(1.0f);

		// オイラー角からカメラの前方ベクトルを算出
		Vector3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		Vector3 cameraFront = glm::normalize(front);

		Vector3 camVec = Vector3(camPos[0], camPos[1], camPos[2]);
		glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 upVec = glm::vec3(0.0f, 1.0f, 0.0f);

		// カメラ制御入力の処理 (UIへの入力を遮断)
		if (!io.WantCaptureMouse) {
			if (io.MouseWheel != 0.0f) {
				radius -= io.MouseWheel * 1.0f;
				if (radius < 0.1f) radius = 0.1f;
			}
			if (ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
				float sensitivity = 0.2f;
				yaw += io.MouseDelta.x * sensitivity;
				pitch -= io.MouseDelta.y * sensitivity;
				if (pitch > 89.0f)  pitch = 89.0f;
				if (pitch < -89.0f) pitch = -89.0f;
			}
		}

		// ビューモード（一人称/三人称）に応じたカメラ位置確定
		std::shared_ptr<PlayerComponent> activePlayerComp = nullptr;
		GameObject* activePlayerObject = nullptr;
		for (auto& obj : worldObjects) {
			auto playerComp = obj.GetComponent<PlayerComponent>();
			if (playerComp) {
				activePlayerComp = playerComp;
				activePlayerObject = &obj;
				break;
			}
		}

		if (isPlaying && activePlayerComp && activePlayerObject) {
			const glm::vec3 forward = glm::normalize(glm::vec3(front.x, front.y, front.z));
			if (activePlayerComp->viewMode == PlayerComponent::ViewMode::FirstPerson) {
				camVec = activePlayerObject->position + glm::vec3(0.0f, 1.6f, 0.0f);
				targetPos = camVec + forward;
			}
			else {
				targetPos = activePlayerObject->position;
				camVec.x = targetPos.x + radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
				camVec.y = targetPos.y + radius * sin(glm::radians(pitch));
				camVec.z = targetPos.z + radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
			}
		}
		else {
			camVec.x = targetPos.x + radius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
			camVec.y = targetPos.y + radius * sin(glm::radians(pitch));
			camVec.z = targetPos.z + radius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
		}
		glm::mat4 view = glm::lookAt(camVec, targetPos, glm::vec3(0, 1, 0));

		// --- コンテキストメニュー (右クリック操作) ---
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !io.WantCaptureMouse) {
			ImGui::OpenPopup("ObjectMenu");

			double x, y;
			glfwGetCursorPos(window, &x, &y);
			Vector3 rayDir = calculateRayFromPixel(x, y, projection, view);

			float dist = 5.0f;
			spawnPos = camVec + (rayDir * dist);
			float snap = 0.5f; // グリッドスナップ設定
			spawnPos = glm::round(spawnPos / snap) * snap;
		}

		if (ImGui::BeginPopup("ObjectMenu")) {
			if (selected != -1) {
				ImGui::Text("Selected: %s", worldObjects[selected].name.c_str());
				if (ImGui::MenuItem("Delete", "Delete Key")) {
					worldObjects.erase(worldObjects.begin() + selected);
					selected = -1;
					isDragging = false;
				}
				ImGui::Separator();
			}
			ImGui::Text("--- Add Object ---");
			if (ImGui::MenuItem("Cube")) { worldObjects.push_back(GameObject("Cube", Vector3(0, 0, 0))); }
			ImGui::EndPopup();
		}

		// --- 3Dピッキング・ドラッグ操作ロジック ---
		bool currentLeftState = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

		if (currentLeftState && !lastLeftState && !io.WantCaptureMouse) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			glm::vec3 rayDir = calculateRayFromPixel(xpos, ypos, projection, view);

			int hitIndex = -1;
			float closestDist = 100000.0f;

			for (int i = 0; i < (int)worldObjects.size(); i++) {
				float distToRay = glm::length(glm::cross(rayDir, worldObjects[i].position - camVec));
				if (distToRay < 0.8f) { // 当たり判定の閾値
					float distToCam = glm::distance(camVec, worldObjects[i].position);
					if (distToCam < closestDist) {
						closestDist = distToCam;
						hitIndex = i;
					}
				}
			}

			if (hitIndex != -1) {
				selected = hitIndex;
				isDragging = true;
				dragDistance = glm::distance(camVec, worldObjects[selected].position);
			}
			else {
				selected = -1;
				isDragging = false;
			}
		}

		// ドラッグ中のオブジェクト位置更新
		if (isDragging && currentLeftState && selected != -1 && !io.WantCaptureMouse) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			float currentDist = glm::distance(camVec, worldObjects[selected].position);
			Vector3 rayDir = calculateRayFromPixel(xpos, ypos, projection, view);
			worldObjects[selected].position = camVec + (rayDir * currentDist);

			float snapValue = 0.5f; // 移動スナップ
			worldObjects[selected].position = glm::round(worldObjects[selected].position / snapValue) * snapValue;
		}

		lastLeftState = currentLeftState;

		// タグシステムとUI更新
		Tagsystem::Update(worldObjects);
		Tagsystem::ShowImGuiWindow();

		// --- レンダーパス: 背景とバッファクリア ---
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		// ユニフォーム変数のロケーション取得
		unsigned int modelLoc = glGetUniformLocation(ourShader.ID, "model");
		unsigned int viewLoc = glGetUniformLocation(ourShader.ID, "view");
		unsigned int projLoc = glGetUniformLocation(ourShader.ID, "projection");
		int vertexColorLocation = glGetUniformLocation(ourShader.ID, "ourColor");

		// カメラ・射影行列の適用
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

		// グリッド描画
		glm::mat4 identity = glm::mat4(1.0f);
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(identity));
		glUniform4f(vertexColorLocation, 0.4f, 0.4f, 0.4f, 1.0f);
		DrawGrid(ourShader, myMesh);

		// オブジェクト描画ループ
		for (int i = 0; i < worldObjects.size(); i++) {
			glm::mat4 objModel = worldObjects[i].getModelMatrix();

			// 選択中オブジェクトのアウトライン描画 (深度テスト無効化による強調)
			if (i == selected) {
				glDisable(GL_DEPTH_TEST);
				glm::mat4 outlineModel = glm::scale(objModel, glm::vec3(1.02f));
				glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(outlineModel));
				glUniform4f(vertexColorLocation, 0.0f, 0.5f, 1.0f, 1.0f);
				myMesh.Draw(ourShader);
				glEnable(GL_DEPTH_TEST);
			}

			// 本体描画
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(objModel));
			if (i == selected) {
				glUniform4f(vertexColorLocation, 1.0f, 1.0f, 0.0f, 1.0f); // ハイライト色
			}
			else {
				glUniform4f(vertexColorLocation, 1.0f, 0.5f, 0.2f, 1.0f); // 通常色
			}
			myMesh.Draw(ourShader);
		}

		// ImGui描画実行
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window); // ダブルバッファの入れ替え
	}

	// --- 終了シーケンス・リソース解放 ---
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

/**
 * @brief シェーダープログラムの生成とリンク
 * @param vsrc バーテックスシェーダーのソースコード
 * @param fsrc フラグメントシェーダーのソースコード
 * @return リンク済みシェーダープログラムID
 */
GLuint createProgram(const char* vsrc, const char* fsrc)
{
	const GLuint program(glCreateProgram());

	if (vsrc != NULL)
	{
		const GLuint vobj(glCreateShader(GL_VERTEX_SHADER));
		glShaderSource(vobj, 1, &vsrc, NULL);
		glCompileShader(vobj);
		glAttachShader(program, vobj);
		glDeleteShader(vobj);
	}

	if (fsrc != NULL)
	{
		const GLuint fobj(glCreateShader(GL_FRAGMENT_SHADER));
		glShaderSource(fobj, 1, &fsrc, NULL);
		glCompileShader(fobj);
		glAttachShader(program, fobj);
		glDeleteShader(fobj);
	}

	// 属性位置の固定バインド
	glBindAttribLocation(program, 0, "position");
	glBindFragDataLocation(program, 0, "fragment");
	glLinkProgram(program);

	return program;
}