#version 450

layout(location = 0) out vec4 FragColor;

in vec2 v_vTexCoord;

uniform float u_fTime;

const float g_fPI = 3.141592f;

void main()
{
	float fSineValue = 0.5f * sin(2.0f * 2.0f * g_fPI * v_vTexCoord.x + u_fTime);
	
	// y축의 값이 -1.0 ~ 1.0f 사이가 되도록 값을 다시 변환한다.
	float fHeight = 2.0f * v_vTexCoord.y - 1.0f;

	if (fSineValue - 0.01f < fHeight && fHeight < fSineValue)
	{
		FragColor = vec4(1.0f);
	}
	else
	{
		FragColor = vec4(0.0f);
	}
}
