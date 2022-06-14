#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"

#include <windows.h>
#include <cstdlib>
#include <cassert>

float g_Time{};
float g_Points[]
{
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f,
	((float)rand() / (float)RAND_MAX), ((float)rand() / (float)RAND_MAX), 0.0f
};

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
	m_Lecture3Shader = CompileShaders("./Shaders/Lecture3.vs", "./Shaders/Lecture3.fs");
	m_Lecture3ParticleShader = CompileShaders("./Shaders/Lecture3Particle.vs", "./Shaders/Lecture3Particle.fs");
	m_Lecture4Shader = CompileShaders("./Shaders/Lecture4.vs", "./Shaders/Lecture4.fs");
	m_Lecture5ByVSShader = CompileShaders("./Shaders/Lecture5ByVS.vs", "./Shaders/Lecture5ByVS.fs");
	m_Lecture5ByFSShader = CompileShaders("./Shaders/Lecture5ByFS.vs", "./Shaders/Lecture5ByFS.fs");
	m_Lecture6Shader = CompileShaders("./Shaders/Lecture6.vs", "./Shaders/Lecture6.fs");
	m_Lecture9Shader = CompileShaders("./Shaders/Lecture9.vs", "./Shaders/Lecture9.fs");
	m_Lecture11Shader = CompileShaders("./Shaders/Lecture11.vs", "./Shaders/Lecture11.fs");

	// Create VBOs
	CreateVertexBufferObjects();

	// Create Particles
	CreateParticles(1000);

	// Create Lines
	CreateLine(100);

	// Create Textures
	CreateTextures();

	// Create Dummy Mesh
	CreateDummyMesh();

	// Create Frame Buffer Objects
	CreateFrameBufferObjects();

	// Load Texture
	m_RGBTexture = CreatePngTexture("Textures/RGB_Texture.png");

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

	float Lecture2Rect[]
	{
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f // Triangle1
	};

	glGenBuffers(1, &m_VBOLecture2);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture2Rect), Lecture2Rect, GL_STATIC_DRAW);

	float Lecture3Rect[]
	{
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f // Triangle1 (x, y, z, r, g, b, a)
	};

	glGenBuffers(1, &m_VBOLecture3);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture3Rect), Lecture3Rect, GL_STATIC_DRAW);

	const float PaticleSize{ 0.1f };
	float Lecture3ParticleRect[]
	{
		-PaticleSize, -PaticleSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		 PaticleSize,  PaticleSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-PaticleSize,  PaticleSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Triangle1 (x, y, z, r, g, b, a)
	    -PaticleSize, -PaticleSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		 PaticleSize, -PaticleSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		 PaticleSize,  PaticleSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f  // Triangle2 (x, y, z, r, g, b, a)
	};

	glGenBuffers(1, &m_VBOLecture3Particle);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3Particle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture3ParticleRect), Lecture3ParticleRect, GL_STATIC_DRAW);

	float rectSize{ 0.5f };
	float Lecture4Rect[]
	{
		-rectSize, -rectSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		 rectSize,  rectSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		-rectSize,  rectSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, // Triangle1 (x, y, z, r, g, b, a)
		-rectSize, -rectSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		 rectSize, -rectSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
		 rectSize,  rectSize, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f  // Triangle2 (x, y, z, r, g, b, a)
	};

	glGenBuffers(1, &m_VBOLecture4);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture4Rect), Lecture4Rect, GL_STATIC_DRAW);

	float Lecture4PositionRect[]
	{
		-rectSize, -rectSize, 0.0f,
		 rectSize,  rectSize, 0.0f,
		-rectSize,  rectSize, 0.0f, // Triangle1 (x, y, z)
		-rectSize, -rectSize, 0.0f,
		 rectSize, -rectSize, 0.0f,
		 rectSize,  rectSize, 0.0f  // Triangle2 (x, y, z)
	};

	float Lecture4ColorRect[]
	{
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f, // Triangle1 (r, g, b, a)
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 1.0f, 1.0f  // Triangle2 (r, g, b, a)
	};

	glGenBuffers(1, &m_VBOLecture4PositionPack);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4PositionPack);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture4PositionRect), Lecture4PositionRect, GL_STATIC_DRAW);

	glGenBuffers(1, &m_VBOLecture4ColorPack);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4ColorPack);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture4ColorRect), Lecture4ColorRect, GL_STATIC_DRAW);

	float Lecture4PositionToColorRect[]
	{
		-rectSize, -rectSize, 0.0f,
		 rectSize,  rectSize, 0.0f,
		-rectSize,  rectSize, 0.0f, // Triangle1 (x, y, z)
		-rectSize, -rectSize, 0.0f,
		 rectSize, -rectSize, 0.0f,
		 rectSize,  rectSize, 0.0f, // Triangle2 (x, y, z)
		 1.0f, 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f, 1.0f,    // Triangle1 (r, g, b, a)
		 1.0f, 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f, 1.0f,    // Triangle2 (r, g, b, a)
		 1.0f, 1.0f, 1.0f, 1.0f
	};

	glGenBuffers(1, &m_VBOLecture4);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture4PositionToColorRect), Lecture4PositionToColorRect, GL_STATIC_DRAW);

	rectSize = 1.0f;

	float Lecture5Rect[]
	{
		-rectSize, -rectSize, 0.0f,
		 rectSize,  rectSize, 0.0f,
		-rectSize,  rectSize, 0.0f,
		-rectSize, -rectSize, 0.0f,
		 rectSize, -rectSize, 0.0f,
		 rectSize,  rectSize, 0.0f,
	};

	glGenBuffers(1, &m_VBOLecture5ByFS);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture5ByFS);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture5Rect), Lecture5Rect, GL_STATIC_DRAW);

	rectSize = 0.5f;

	float Lecture6Rect[]
	{
		-rectSize, -rectSize, 0.0f, 0.0f, 0.0f, // x, y, z, tex_x, tex_y
		 rectSize,  rectSize, 0.0f, 1.0f, 1.0f,
		-rectSize,  rectSize, 0.0f, 0.0f, 1.0f,
		-rectSize, -rectSize, 0.0f, 0.0f, 0.0f,
		 rectSize, -rectSize, 0.0f, 1.0f, 0.0f,
		 rectSize,  rectSize, 0.0f, 1.0f, 1.0f
	};

	glGenBuffers(1, &m_VBOLecture6);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture6);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Lecture6Rect), Lecture6Rect, GL_STATIC_DRAW);
}

void Renderer::CreateParticles(int count)
{
	const int floatCount{ count * (3 + 3 + 1 + 1 + 1 + 1 + 1 + 4) * 3 * 2 }; // (x, y, z, vx, vy, vz, emit time, life time, period, amp, random value, r, g, b, a)
	const int vertexCount{ count * 3 * 2 };

	float* particleVertices{ new float[floatCount] {} };
	const float particleSize{ 0.05f };

	int index{};

	for (int i = 0; i < count; ++i)
	{
		float randomValueX{};// ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f }; // -1.0f ~ 1.0f
		float randomValueY{};// ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f }; // -1.0f ~ 1.0f
		float randomValueZ{};

		float randomValueVX{ ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f }; // -1.0f ~ 1.0f
		float randomValueVY{ ((float)rand() / (float)RAND_MAX - 0.5f) * 0.1f }; // -1.0f ~ 1.0f
		float randomValueVZ{};

		float randomEmitTime{ ((float)rand() / (float)RAND_MAX) * 5.0f };
		float randomLifeTime{ ((float)rand() / (float)RAND_MAX) * 2.0f };

		float randomPeriod{ ((float)rand() / (float)RAND_MAX) * 2.0f };
		float randomAmp{ ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f * 0.05f };

		float randomValue{ ((float)rand() / (float)RAND_MAX) };

		float randomR{ ((float)rand() / (float)RAND_MAX) };
		float randomG{ ((float)rand() / (float)RAND_MAX) };
		float randomB{ ((float)rand() / (float)RAND_MAX) };
		float randomA{ 1.0f };

		// v0
		// Position
		particleVertices[index++] = 0.5f * -particleSize + randomValueX;
		particleVertices[index++] = 0.5f * -particleSize + randomValueY;
		particleVertices[index++] = 0.0f;
		// Velocity
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.0f;
		// EmitTime
		particleVertices[index++] = randomEmitTime;
		// LifeTime
		particleVertices[index++] = randomLifeTime;
		// Period
		particleVertices[index++] = randomPeriod;
		// Amp
		particleVertices[index++] = randomAmp;
		// Random Value
		particleVertices[index++] = randomValue;
		// Random Color(r, g, b, a)
		particleVertices[index++] = randomR;
		particleVertices[index++] = randomG;
		particleVertices[index++] = randomB;
		particleVertices[index++] = randomA;

		// v1
		// Position
		particleVertices[index++] = 0.5f * particleSize + randomValueX;
		particleVertices[index++] = 0.5f * -particleSize + randomValueY;
		particleVertices[index++] = 0.0f;
		// Velocity
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.0f;
		// EmitTime
		particleVertices[index++] = randomEmitTime;
		// LifeTime
		particleVertices[index++] = randomLifeTime;
		// Period
		particleVertices[index++] = randomPeriod;
		// Amp
		particleVertices[index++] = randomAmp;
		// Random Value
		particleVertices[index++] = randomValue;
		// Random Color(r, g, b, a)
		particleVertices[index++] = randomR;
		particleVertices[index++] = randomG;
		particleVertices[index++] = randomB;
		particleVertices[index++] = randomA;

		// v2
		// Position
		particleVertices[index++] = 0.5f * particleSize + randomValueX;
		particleVertices[index++] = 0.5f * particleSize + randomValueY;
		particleVertices[index++] = 0.0f;
		// Velocity
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.0f;
		// EmitTime
		particleVertices[index++] = randomEmitTime;
		// LifeTime
		particleVertices[index++] = randomLifeTime;
		// Period
		particleVertices[index++] = randomPeriod;
		// Amp
		particleVertices[index++] = randomAmp;
		// Random Value
		particleVertices[index++] = randomValue;
		// Random Color(r, g, b, a)
		particleVertices[index++] = randomR;
		particleVertices[index++] = randomG;
		particleVertices[index++] = randomB;
		particleVertices[index++] = randomA;

		// v3
		// Position
		particleVertices[index++] = 0.5f * -particleSize + randomValueX;
		particleVertices[index++] = 0.5f * -particleSize + randomValueY;
		particleVertices[index++] = 0.0f;
		// Velocity
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.0f;
		// EmitTime
		particleVertices[index++] = randomEmitTime;
		// LifeTime
		particleVertices[index++] = randomLifeTime;
		// Period
		particleVertices[index++] = randomPeriod;
		// Amp
		particleVertices[index++] = randomAmp;
		// Random Value
		particleVertices[index++] = randomValue;
		// Random Color(r, g, b, a)
		particleVertices[index++] = randomR;
		particleVertices[index++] = randomG;
		particleVertices[index++] = randomB;
		particleVertices[index++] = randomA;

		// v4
		// Position
		particleVertices[index++] = 0.5f * particleSize + randomValueX;
		particleVertices[index++] = 0.5f * particleSize + randomValueY;
		particleVertices[index++] = 0.0f;
		// Velocity
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.0f;
		// EmitTime
		particleVertices[index++] = randomEmitTime;
		// LifeTime
		particleVertices[index++] = randomLifeTime;
		// Period
		particleVertices[index++] = randomPeriod;
		// Amp
		particleVertices[index++] = randomAmp;
		// Random Value
		particleVertices[index++] = randomValue;
		// Random Color(r, g, b, a)
		particleVertices[index++] = randomR;
		particleVertices[index++] = randomG;
		particleVertices[index++] = randomB;
		particleVertices[index++] = randomA;

		// v5
		// Position
		particleVertices[index++] = 0.5f * -particleSize + randomValueX;
		particleVertices[index++] = 0.5f * particleSize + randomValueY;
		particleVertices[index++] = 0.0f;
		// Velocity
		particleVertices[index++] = randomValueVX;
		particleVertices[index++] = randomValueVY;
		particleVertices[index++] = 0.0f;
		// EmitTime
		particleVertices[index++] = randomEmitTime;
		// LifeTime
		particleVertices[index++] = randomLifeTime;
		// Period
		particleVertices[index++] = randomPeriod;
		// Amp
		particleVertices[index++] = randomAmp;
		// Random Value
		particleVertices[index++] = randomValue;
		// Random Color(r, g, b, a)
		particleVertices[index++] = randomR;
		particleVertices[index++] = randomG;
		particleVertices[index++] = randomB;
		particleVertices[index++] = randomA;
	}

	glGenBuffers(1, &m_VBOLecture3Particles);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3Particles);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, particleVertices, GL_STATIC_DRAW);

	m_VBOLecture3ParticleVertexCount = vertexCount;

	delete[] particleVertices;
}

void Renderer::CreateLine(int count)
{
	const int floatCount{ 3 * count }; // x, y, z
	const int vertexCount{ count };

	float* lineVertices{ new float[floatCount] };
	int index{};

	for (int i = 0; i < count; ++i)
	{
		lineVertices[index++] = -1.0f + (2.0f * i / (count - 1));
		lineVertices[index++] = 0.0f;
		lineVertices[index++] = 0.0f;
	}

	glGenBuffers(1, &m_VBOLecture5ByVS);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture5ByVS);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, lineVertices, GL_STATIC_DRAW);

	m_VBOLecture5ByVertexCount = vertexCount;

	delete[] lineVertices;
}

void Renderer::CreateTextures()
{
	static const GLulong CheckBoard[]
	{
		0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
		0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
		0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
		0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
		0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
		0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF,
		0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000,
		0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF, 0x00000000, 0xFFFFFFFF
	};

	glGenTextures(1, &m_CheckerTexture);
	glBindTexture(GL_TEXTURE_2D, m_CheckerTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0, GL_RGBA, GL_UNSIGNED_BYTE, CheckBoard);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Renderer::CreateDummyMesh()
{
	float basePosX{ -0.5f };
	float basePosY{ -0.5f };
	float targetPosX{ 0.5f };
	float targetPosY{ 0.5f };

	int pointCountX{ 32 };
	int pointCountY{ 32 };

	float width{ targetPosX - basePosX };
	float height{ targetPosY - basePosY };

	float* point{ new float[pointCountX * pointCountY * 2] };
	float* vertices{ new float[(pointCountX - 1) * (pointCountY - 1) * 2 * 3 * 3] };
	
	m_VBOLecture9VertexCount = (pointCountX - 1) * (pointCountY - 1) * 2 * 3;

	// Prepare points
	for (int x = 0; x < pointCountX; ++x)
	{
		for (int y = 0; y < pointCountY; ++y)
		{
			point[(y * pointCountX + x) * 2 + 0] = basePosX + width * (x / (float)(pointCountX - 1));
			point[(y * pointCountX + x) * 2 + 1] = basePosY + height * (y / (float)(pointCountY - 1));
		}
	}

	// Make triangles
	int Index{};

	for (int x = 0; x < pointCountX - 1; ++x)
	{
		for (int y = 0; y < pointCountY - 1; ++y)
		{
			// Triangle 1
			vertices[Index++] = point[(y * pointCountX + x) * 2 + 0];
			vertices[Index++] = point[(y * pointCountX + x) * 2 + 1];
			vertices[Index++] = 0.0f;
			vertices[Index++] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 0];
			vertices[Index++] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 1];
			vertices[Index++] = 0.0f;
			vertices[Index++] = point[((y + 1) * pointCountX + x) * 2 + 0];
			vertices[Index++] = point[((y + 1) * pointCountX + x) * 2 + 1];
			vertices[Index++] = 0.0f;

			// Triangle 2
			vertices[Index++] = point[(y * pointCountX + x) * 2 + 0];
			vertices[Index++] = point[(y * pointCountX + x) * 2 + 1];
			vertices[Index++] = 0.0f;
			vertices[Index++] = point[(y * pointCountX + (x + 1)) * 2 + 0];
			vertices[Index++] = point[(y * pointCountX + (x + 1)) * 2 + 1];
			vertices[Index++] = 0.0f;
			vertices[Index++] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 0];
			vertices[Index++] = point[((y + 1) * pointCountX + (x + 1)) * 2 + 1];
			vertices[Index++] = 0.0f;
		}
	}

	glGenBuffers(1, &m_VBOLecture9);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture9);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (pointCountX - 1) * (pointCountY - 1) * 2 * 3 * 3, vertices, GL_STATIC_DRAW);

	delete[] point;
	delete[] vertices;
}

void Renderer::CreateFrameBufferObjects()
{
	for (UINT i = 0; i < 4; ++i)
	{
		glGenTextures(1, &m_FBOTextures[i]);
		glBindTexture(GL_TEXTURE_2D, m_FBOTextures[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 512, 512, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		glGenRenderbuffers(1, &m_DepthRenderBuffers[i]);
		glBindRenderbuffer(GL_RENDERBUFFER, m_DepthRenderBuffers[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 512, 512);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glGenFramebuffers(1, &m_FBOs[i]);
		glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_FBOTextures[i], 0);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_DepthRenderBuffers[i]);

		GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

		if (Status != GL_FRAMEBUFFER_COMPLETE)
		{
			std::cout << "Something goes wrong while gen FBO " << i << std::endl;
		}
	}

	// 다시 메인 프레임버퍼에 그린다.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
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
	glBindBuffer(GL_ARRAY_BUFFER, m_VBORect);

	int attribPosition{ glGetAttribLocation(m_SolidRectShader, "a_vPosition") };
	
	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::Lecture2Test()
{
	glUseProgram(m_SolidRectShader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture2);

	int attribPosition{ glGetAttribLocation(m_SolidRectShader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::Lecture3Test()
{
	glUseProgram(m_Lecture3Shader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3);

	int attribPosition{ glGetAttribLocation(m_Lecture3Shader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

	int attribColor{ glGetAttribLocation(m_Lecture3Shader, "a_vColor") };

	glEnableVertexAttribArray(attribColor);
	glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	int uniformTime{ glGetUniformLocation(m_Lecture3Shader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	int uniformColor{ glGetUniformLocation(m_Lecture3Shader, "u_vColor") };

	glUniform4f(uniformColor, 1.0f, 1.0f, 1.0f, 1.0f);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	g_Time -= 0.01f;

	if (g_Time < 0.0f)
	{
		g_Time = 1.0f;
	}

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribColor);
}

void Renderer::Lecture3ParticleTest()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	glUseProgram(m_Lecture3ParticleShader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3Particles);

	// Attribute
	int attribPosition{ glGetAttribLocation(m_Lecture3ParticleShader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), 0);

	int attribVelocity{ glGetAttribLocation(m_Lecture3ParticleShader, "a_vVelocity") };

	glEnableVertexAttribArray(attribVelocity);
	glVertexAttribPointer(attribVelocity, 3, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	int attribEmitTime{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fEmitTime") };

	glEnableVertexAttribArray(attribEmitTime);
	glVertexAttribPointer(attribEmitTime, 1, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (GLvoid*)(6 * sizeof(float)));

	int attribLifeTime{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fLifeTime") };

	glEnableVertexAttribArray(attribLifeTime);
	glVertexAttribPointer(attribLifeTime, 1, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (GLvoid*)(7 * sizeof(float)));

	int attribPeriod{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fPeriod") };

	glEnableVertexAttribArray(attribPeriod);
	glVertexAttribPointer(attribPeriod, 1, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (GLvoid*)(8 * sizeof(float)));

	int attribAmp{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fAmp") };

	glEnableVertexAttribArray(attribAmp);
	glVertexAttribPointer(attribAmp, 1, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (GLvoid*)(9 * sizeof(float)));

	int attribRandomValue{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fRandomValue") };

	glEnableVertexAttribArray(attribRandomValue);
	glVertexAttribPointer(attribRandomValue, 1, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (GLvoid*)(10 * sizeof(float)));

	int attribRandomColor{ glGetAttribLocation(m_Lecture3ParticleShader, "a_vColor") };

	glEnableVertexAttribArray(attribRandomColor);
	glVertexAttribPointer(attribRandomColor, 4, GL_FLOAT, GL_FALSE, 15 * sizeof(float), (GLvoid*)(11 * sizeof(float)));

	// Uniform
	int uniformTime{ glGetUniformLocation(m_Lecture3ParticleShader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	int uniformAccel{ glGetUniformLocation(m_Lecture3ParticleShader, "u_vAccel")};

	glUniform3f(uniformAccel, 0.0f, 0.0f, 0.0f);

	g_Time += 0.01f;

	glDrawArrays(GL_TRIANGLES, 0, m_VBOLecture3ParticleVertexCount);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribVelocity);
	glDisableVertexAttribArray(attribEmitTime);
	glDisableVertexAttribArray(attribLifeTime);
	glDisableVertexAttribArray(attribPeriod);
	glDisableVertexAttribArray(attribAmp);
	glDisableVertexAttribArray(attribRandomValue);
	glDisableVertexAttribArray(attribRandomColor);

	glDisable(GL_BLEND);
}

void Renderer::Lecture4Test()
{
	glUseProgram(m_Lecture4Shader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4);

	int attribPosition{ glGetAttribLocation(m_Lecture4Shader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

	int attribColor{ glGetAttribLocation(m_Lecture4Shader, "a_vColor") };

	glEnableVertexAttribArray(attribColor);
	glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribColor);
}

void Renderer::Lecture4RaindropTest()
{
	glUseProgram(m_Lecture4Shader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4);

	int attribPosition{ glGetAttribLocation(m_Lecture4Shader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

	int attribColor{ glGetAttribLocation(m_Lecture4Shader, "a_vColor") };

	glEnableVertexAttribArray(attribColor);
	glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	int uniformPoints{ glGetUniformLocation(m_Lecture4Shader, "u_vPoints") };

	glUniform3fv(uniformPoints, _countof(g_Points), g_Points);

	int uniformTime{ glGetUniformLocation(m_Lecture4Shader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	g_Time += 0.001f;

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribColor);
}

void Renderer::Lecture4RaderCirlceTest()
{
	glUseProgram(m_Lecture4Shader);

	// 1. 각 Array를 생성하여 Packing
	//int attribPosition{ glGetAttribLocation(m_Lecture4Shader, "a_vPosition") };

	//glEnableVertexAttribArray(attribPosition);
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4PositionPack);
	//glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	//int attribColor{ glGetAttribLocation(m_Lecture4Shader, "a_vColor") };

	//glEnableVertexAttribArray(attribColor);
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4ColorPack); // Color가 담긴 VBO를 Bind 해주어야 함!
	//glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// 2. Position과 Color 정보를 합친 Array를 생성하여 Packing(Position, Color를 번갈아가며)
	//	  => 지금까지 진행했던 실습들이 위와 같은 패킹방법을 사용한 것임.
	// 
	//glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4);

	//int attribPosition{ glGetAttribLocation(m_Lecture4Shader, "a_vPosition") };

	//glEnableVertexAttribArray(attribPosition);
	//glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);

	//int attribColor{ glGetAttribLocation(m_Lecture4Shader, "a_vColor") };

	//glEnableVertexAttribArray(attribColor);
	//glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	// 3. Position과 Color 정보를 합친 Array를 생성하여 Packing(Position이 전부 끝난 후 Color)
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture4);
	
	int attribPosition{ glGetAttribLocation(m_Lecture4Shader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int attribColor{ glGetAttribLocation(m_Lecture4Shader, "a_vColor") };

	glEnableVertexAttribArray(attribColor);
	glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(18 * sizeof(float)));

	int uniformPoints{ glGetUniformLocation(m_Lecture4Shader, "u_vPoints") };

	glUniform3fv(uniformPoints, _countof(g_Points), g_Points);

	int uniformTime{ glGetUniformLocation(m_Lecture4Shader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	g_Time += 0.001f;

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribColor);
}

void Renderer::Lecture5ByVSTest()
{
	glUseProgram(m_Lecture5ByVSShader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture5ByVS);

	int attribPosition{ glGetAttribLocation(m_Lecture5ByVSShader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int uniformTime{ glGetUniformLocation(m_Lecture5ByVSShader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	g_Time += 0.01f;

	glDrawArrays(GL_LINE_STRIP, 0, m_VBOLecture5ByVertexCount);

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::Lecture5ByFSTest()
{
	glUseProgram(m_Lecture5ByFSShader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture5ByFS);

	int attribPosition{ glGetAttribLocation(m_Lecture5ByFSShader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int uniformTime{ glGetUniformLocation(m_Lecture5ByFSShader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	g_Time += 0.01f;

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::Lecture6Test()
{
	glUseProgram(m_Lecture6Shader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture6);

	int attribPosition{ glGetAttribLocation(m_Lecture6Shader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);

	int attribTexCoord{ glGetAttribLocation(m_Lecture6Shader, "a_vTexCoord") };

	glEnableVertexAttribArray(attribTexCoord);
	glVertexAttribPointer(attribTexCoord, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	int uniformTexSampler{ glGetUniformLocation(m_Lecture6Shader, "u_sTexSampler") };

	glUniform1i(uniformTexSampler, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, m_RGBTexture);

	int uniformTexSampler1{ glGetUniformLocation(m_Lecture6Shader, "u_sTexSampler1") };

	glUniform1i(uniformTexSampler1, 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, m_CheckerTexture);

	int uniformTime{ glGetUniformLocation(m_Lecture6Shader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	g_Time += 0.01f;

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
	glDisableVertexAttribArray(attribTexCoord);
}

void Renderer::Lecture9Test()
{
	glUseProgram(m_Lecture9Shader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture9);

	int attribPosition{ glGetAttribLocation(m_Lecture9Shader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int uniformTime{ glGetUniformLocation(m_Lecture9Shader, "u_fTime") };

	glUniform1f(uniformTime, g_Time);

	g_Time += 0.03f;

	glDrawArrays(GL_LINE_STRIP, 0, m_VBOLecture9VertexCount);

	glDisableVertexAttribArray(attribPosition);
}

void Renderer::FBORender()
{
	// # FBO 0
	// 1. Bind Framebuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[0]);

	// 2. Set viewport
	glViewport(0, 0, 512, 512);

	// 3. Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 4. Rendering
	Lecture3ParticleTest();

	// # FBO 1
	// 1. Bind Framebuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[1]);

	// 2. Set viewport
	glViewport(0, 0, 512, 512);

	// 3. Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 4. Rendering
	Lecture4RaderCirlceTest();

	// # FBO 2
	// 1. Bind Framebuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[2]);

	// 2. Set viewport
	glViewport(0, 0, 512, 512);

	// 3. Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 4. Rendering
	Lecture6Test();

	// # FBO 3
	// 1. Bind Framebuffer object
	glBindFramebuffer(GL_FRAMEBUFFER, m_FBOs[3]);

	// 2. Set viewport
	glViewport(0, 0, 512, 512);

	// 3. Clear
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// 4. Rendering
	Lecture9Test();

	// Restore framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Draw Textures
	glViewport(0, 0, 250, 250);
	DrawFullScreenTexture(m_FBOTextures[0]);

	glViewport(250, 0, 250, 250);
	DrawFullScreenTexture(m_FBOTextures[1]);

	glViewport(0, 250, 250, 250);
	DrawFullScreenTexture(m_FBOTextures[2]);

	glViewport(250, 250, 250, 250);
	DrawFullScreenTexture(m_FBOTextures[3]);
}

void Renderer::DrawFullScreenTexture(GLuint textureID)
{
	glUseProgram(m_Lecture11Shader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture5ByFS);

	int attribPosition{ glGetAttribLocation(m_Lecture11Shader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	int uniformTexSampler{ glGetUniformLocation(m_Lecture11Shader, "u_sTexSampler") };

	glUniform1i(uniformTexSampler, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(attribPosition);
}
