#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"

#include <windows.h>
#include <cstdlib>
#include <cassert>

Renderer::Renderer(int windowSizeX, int windowSizeY)
{
	// Default Setting
	glClearDepth(1.0f);
	Initialize(windowSizeX, windowSizeY);
}

Renderer::~Renderer()
{

}

void Renderer::Initialize(int windowSizeX, int windowSizeY)
{
	// Set window size
	m_WindowSizeX = windowSizeX;
	m_WindowSizeY = windowSizeY;

	// Load shaders
	m_SolidRectShader = CompileShaders("./Shaders/SolidRect.vs", "./Shaders/SolidRect.fs");

	// Create VBOs
	CreateVertexBufferObjects();

	// Initialize model transform matrix : used for rotating quad normal to parallel to camera direction
	m_m4Model = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	// Initialize camera settings
	m_v3CameraPosition = glm::vec3(0.0f, 0.0f, 1000.0f);
	m_v3CameraLookAt = glm::vec3(0.0f, 0.0f, 0.0f);
	m_v3CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
	m_m4View = glm::lookAt(m_v3CameraPosition, m_v3CameraLookAt, m_v3CameraUp);

	// Initialize projection matrix
	m_m4OrthoProj = glm::ortho(-(float)windowSizeX / 2.0f, (float)windowSizeX / 2.0f, -(float)windowSizeY / 2.0f, (float)windowSizeY / 2.0f, 0.0001f, 10000.0f);
	m_m4PersProj = glm::perspectiveRH(45.0f, 1.0f, 1.0f, 1000.0f);

	// Initialize projection-view matrix
	m_m4ProjView = m_m4OrthoProj * m_m4View; // Use ortho at this time
	//m_m4ProjView = m_m4PersProj * m_m4View;
}

bool Renderer::ReadFile(char* filename, std::string* target)
{
	std::ifstream file{ filename };

	if (file.fail())
	{
		std::cout << filename << " file loading failed..\n";
		file.close();

		return false;
	}

	std::string line{};

	while (getline(file, line))
	{
		target->append(line.c_str());
		target->append("\n");
	}

	return true;
}

unsigned char* Renderer::LoadBMPRaw(const char* imagepath, unsigned int& outWidth, unsigned int& outHeight)
{
	std::cout << "Loading bmp file " << imagepath << " ... " << std::endl;

	outWidth = outHeight = -1;

	// Data read from the header of the BMP file
	unsigned char header[54]{};
	unsigned int dataPos{};
	unsigned int imageSize{};

	// Actual RGB data
	unsigned char* data{};

	// Open the file
	FILE* file{};

	fopen_s(&file, imagepath, "rb");

	if (!file)
	{
		std::cout << "Image could not be opened, " << imagepath << " is missing. " << std::endl;

		return nullptr;
	}

	if (fread(header, 1, 54, file) != 54)
	{
		std::cout << imagepath << " is not a correct BMP file. " << std::endl;

		return nullptr;
	}

	if (header[0] != 'B' || header[1] != 'M')
	{
		std::cout << imagepath << " is not a correct BMP file." << std::endl;

		return nullptr;
	}

	if (*(int*)&(header[0x1E]) != 0)
	{
		std::cout << imagepath << " is not a correct BMP file." << std::endl;

		return nullptr;
	}

	if (*(int*)&(header[0x1C]) != 24)
	{
		std::cout << imagepath << " is not a correct BMP file." << std::endl;

		return nullptr;
	}

	dataPos = *(int*)&(header[0x0A]);
	imageSize = *(int*)&(header[0x22]);
	outWidth = *(int*)&(header[0x12]);
	outHeight = *(int*)&(header[0x16]);

	if (!imageSize)
	{
		imageSize = outWidth * outHeight * 3;
	}

	if (!dataPos)
	{
		dataPos = 54;
	}

	data = new unsigned char[imageSize];

	fread(data, 1, imageSize, file);
	fclose(file);

	std::cout << imagepath << " is succesfully loaded." << std::endl;

	return data;
}

void Renderer::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// 쉐이더 오브젝트 생성
	GLuint ShaderObj{ glCreateShader(ShaderType) };

	if (!ShaderObj)
	{
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[]{ pShaderText };
	GLint Lengths[]{ (GLint)strlen(pShaderText) };

	// 쉐이더 코드를 쉐이더 오브젝트에 할당
	glShaderSource(ShaderObj, 1, p, Lengths);

	// 할당된 쉐이더 코드를 컴파일
	glCompileShader(ShaderObj);

	GLint success{};

	// ShaderObj가 성공적으로 컴파일 되었는지 확인
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar InfoLog[1024]{};

		// OpenGL 의 shader log 데이터를 가져옴
		glGetShaderInfoLog(ShaderObj, 1024, nullptr, InfoLog);

		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s\n", pShaderText);
	}

	// ShaderProgram에 attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	// 빈 쉐이더 프로그램 생성
	GLuint ShaderProgram{ glCreateProgram() };

	// 쉐이더 프로그램이 만들어졌는지 확인
	if (!ShaderProgram)
	{
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs{};

	// shader.vs가 vs 안으로 로딩됨
	if (!ReadFile(filenameVS, &vs))
	{
		printf("Error compiling vertex shader\n");

		return -1;
	};

	std::string fs{};

	// shader.fs가 fs 안으로 로딩됨
	if (!ReadFile(filenameFS, &fs))
	{
		printf("Error compiling fragment shader\n");

		return -1;
	};

	// ShaderProgram 에 vs.c_str() 버텍스 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram 에 fs.c_str() 프레그먼트 쉐이더를 컴파일한 결과를 attach함
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success{};
	GLchar ErrorLog[1024]{};

	// Attach 완료된 shaderProgram을 링킹함
	glLinkProgram(ShaderProgram);

	// 링크가 성공했는지 확인
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (!Success)
	{
		// shader program 로그를 받아옴
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), nullptr, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << "Error linking shader program\n" << ErrorLog;

		return -1;
	}

	glValidateProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);

	if (!Success)
	{
		glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), nullptr, ErrorLog);
		std::cout << filenameVS << ", " << filenameFS << "Error validating shader program\n" << ErrorLog;

		return -1;
	}

	glUseProgram(ShaderProgram);
	std::cout << filenameVS << ", " << filenameFS << "Shader compiling is done.\n";

	return ShaderProgram;
}

void Renderer::CreateVertexBufferObjects()
{
	float rect[]
	{
		-0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f, 0.5f,  0.5f, 0.0f, // Triangle1
		-0.5f, -0.5f, 0.0f,  0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, // Triangle2
	};

	glGenBuffers(1, &m_VBORect);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glBufferData(GL_ARRAY_BUFFER, sizeof(rect), rect, GL_STATIC_DRAW);
}

GLuint Renderer::CreatePngTexture(char* filePath)
{
	// Load Pngs: Load file and decode image
	std::vector<unsigned char> image{};

	unsigned int width{};
	unsigned int height{};
	unsigned int error{ lodepng::decode(image, width, height, filePath) };

	if (error)
	{
		lodepng_error_text(error);
		assert(!error);

		return -1;
	}

	GLuint temp{};

	glGenTextures(1, &temp);
	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);

	return temp;
}

GLuint Renderer::CreateBmpTexture(char* filePath)
{
	// Load Bmp: Load file and decode image
	unsigned int width{};
	unsigned int height{};
	unsigned char* bmp{ LoadBMPRaw(filePath, width, height) };

	if (!bmp)
	{
		std::cout << "Error while loading bmp file : " << filePath << std::endl;
		assert(bmp);

		return -1;
	}

	GLuint temp{};

	glGenTextures(1, &temp);
	glBindTexture(GL_TEXTURE_2D, temp);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bmp);

	return temp;
}

void Renderer::Test()
{
	glUseProgram(m_SolidRectShader);

	int attribPosition{ glGetAttribLocation(m_SolidRectShader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
}
