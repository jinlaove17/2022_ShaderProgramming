#include "stdafx.h"
#include "Renderer.h"
#include "LoadPng.h"

#include <windows.h>
#include <cstdlib>
#include <cassert>

float g_Time{};

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

	// Create VBOs
	CreateVertexBufferObjects();
	CreateParticles(1000);

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
	// ���̴� ������Ʈ ����
	GLuint ShaderObj{ glCreateShader(ShaderType) };

	if (!ShaderObj)
	{
		fprintf(stderr, "Error creating shader type %d\n", ShaderType);
	}

	const GLchar* p[]{ pShaderText };
	GLint Lengths[]{ (GLint)strlen(pShaderText) };

	// ���̴� �ڵ带 ���̴� ������Ʈ�� �Ҵ�
	glShaderSource(ShaderObj, 1, p, Lengths);

	// �Ҵ�� ���̴� �ڵ带 ������
	glCompileShader(ShaderObj);

	GLint success{};

	// ShaderObj�� ���������� ������ �Ǿ����� Ȯ��
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar InfoLog[1024]{};

		// OpenGL �� shader log �����͸� ������
		glGetShaderInfoLog(ShaderObj, 1024, nullptr, InfoLog);

		fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		printf("%s\n", pShaderText);
	}

	// ShaderProgram�� attach!!
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint Renderer::CompileShaders(char* filenameVS, char* filenameFS)
{
	// �� ���̴� ���α׷� ����
	GLuint ShaderProgram{ glCreateProgram() };

	// ���̴� ���α׷��� ����������� Ȯ��
	if (!ShaderProgram)
	{
		fprintf(stderr, "Error creating shader program\n");
	}

	std::string vs{};

	// shader.vs�� vs ������ �ε���
	if (!ReadFile(filenameVS, &vs))
	{
		printf("Error compiling vertex shader\n");

		return -1;
	};

	std::string fs{};

	// shader.fs�� fs ������ �ε���
	if (!ReadFile(filenameFS, &fs))
	{
		printf("Error compiling fragment shader\n");

		return -1;
	};

	// ShaderProgram �� vs.c_str() ���ؽ� ���̴��� �������� ����� attach��
	AddShader(ShaderProgram, vs.c_str(), GL_VERTEX_SHADER);

	// ShaderProgram �� fs.c_str() �����׸�Ʈ ���̴��� �������� ����� attach��
	AddShader(ShaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);

	GLint Success{};
	GLchar ErrorLog[1024]{};

	// Attach �Ϸ�� shaderProgram�� ��ŷ��
	glLinkProgram(ShaderProgram);

	// ��ũ�� �����ߴ��� Ȯ��
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);

	if (!Success)
	{
		// shader program �α׸� �޾ƿ�
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
}

void Renderer::CreateParticles(int count)
{
	const int floatCount{ count * (3 + 3 + 1 + 1 + 1 + 1) * 3 * 2 }; // (x, y, z, vx, vy, vz, period, amp)
	const int vertexCount{ count * 3 * 2 };

	float* particleVertices{ new float[floatCount] {} };
	const float particleSize{ 0.01f };

	int index{};

	for (int i = 0; i < count; ++i)
	{
		float randomValueX{};// ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f }; // -1.0f ~ 1.0f
		float randomValueY{};// ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f }; // -1.0f ~ 1.0f
		float randomValueZ{};

		float randomValueVX{ 1.0f };//((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f }; // -1.0f ~ 1.0f
		float randomValueVY{};// ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f }; // -1.0f ~ 1.0f
		float randomValueVZ{};

		float randomEmitTime{ ((float)rand() / (float)RAND_MAX) * 5.0f };
		float randomLifeTime{ 1.0f };//((float)rand() / (float)RAND_MAX) };

		float randomPeriod{ ((float)rand() / (float)RAND_MAX) * 2.0f };
		float randomAmp{ ((float)rand() / (float)RAND_MAX - 0.5f) * 2.0f * 0.2f };

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
	}

	glGenBuffers(1, &m_VBOLecture3Particles);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3Particles);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * floatCount, particleVertices, GL_STATIC_DRAW);

	m_VBOLecture3ParticleVertexCount = vertexCount;

	delete[] particleVertices;
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
	glUseProgram(m_Lecture3ParticleShader);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBOLecture3Particles);

	// Attribute
	int attribPosition{ glGetAttribLocation(m_Lecture3ParticleShader, "a_vPosition") };

	glEnableVertexAttribArray(attribPosition);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), 0);

	int attribVelocity{ glGetAttribLocation(m_Lecture3ParticleShader, "a_vVelocity") };

	glEnableVertexAttribArray(attribVelocity);
	glVertexAttribPointer(attribVelocity, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)(3 * sizeof(float)));

	int attribEmitTime{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fEmitTime") };

	glEnableVertexAttribArray(attribEmitTime);
	glVertexAttribPointer(attribEmitTime, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)(6 * sizeof(float)));

	int attribLifeTime{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fLifeTime") };

	glEnableVertexAttribArray(attribLifeTime);
	glVertexAttribPointer(attribLifeTime, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)(7 * sizeof(float)));

	int attribPeriod{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fPeriod") };

	glEnableVertexAttribArray(attribPeriod);
	glVertexAttribPointer(attribPeriod, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)(8 * sizeof(float)));

	int attribAmp{ glGetAttribLocation(m_Lecture3ParticleShader, "a_fAmp") };

	glEnableVertexAttribArray(attribAmp);
	glVertexAttribPointer(attribAmp, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float), (GLvoid*)(9 * sizeof(float)));

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
}
