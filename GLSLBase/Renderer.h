#pragma once

#include "Dependencies/glew.h"
#include "Dependencies/wglew.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtx/euler_angles.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

class Renderer
{
private:
	bool         m_Initialized{};

	unsigned int m_WindowSizeX{};
	unsigned int m_WindowSizeY{};

	glm::vec3    m_v3CameraPosition{}; // Camera Position
	glm::vec3    m_v3CameraLookAt{};   // Camera Look
	glm::vec3    m_v3CameraUp{};       // Camera Up
	
	glm::mat4    m_m4Model{};
	glm::mat4    m_m4View{};
	glm::mat4    m_m4OrthoProj{};
	glm::mat4    m_m4PersProj{};
	glm::mat4    m_m4ProjView{};

	GLuint       m_VBORect{};
	GLuint       m_SolidRectShader{};

	GLuint		 m_VBOLecture2{};

	GLuint		 m_Lecture3Shader{};

public:
	Renderer(int windowSizeX, int windowSizeY);
	~Renderer();

	GLuint CreatePngTexture(char* filePath);
	GLuint CreateBmpTexture(char* filePath);

	void Test();
	void Lecture2Test();
	void Lecture3Test();

private:
	void Initialize(int windowSizeX, int windowSizeY);

	bool ReadFile(char* filename, std::string* target);
	unsigned char* Renderer::LoadBMPRaw(const char* imagepath, unsigned int& outWidth, unsigned int& outHeight);

	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	GLuint CompileShaders(char* filenameVS, char* filenameFS);

	void CreateVertexBufferObjects();
};
