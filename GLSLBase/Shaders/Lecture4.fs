#version 450

layout(location = 0) out vec4 FragColor;

in vec4 v_vColor;

uniform vec3 u_vPoints[10];
uniform float u_fTime;

const float g_fPI = 3.141592f;

vec4 DrawHalfRect()
{
	// 사각형의 절반 중 윗부분만 그려보기
	vec4 vNewColor = vec4(0.0f);

	if (v_vColor.y > 0.5f)
	{
		vNewColor = vec4(1.0f);
	}

	return vNewColor;
}

vec4 DrawCircle()
{
	// 내부가 채워진 원 그려보기
	vec4 vNewColor = vec4(0.0f);
	float fDist = distance(v_vColor.xy, vec2(0.5f, 0.5f));

	if (fDist < 0.5f)
	{
		vNewColor = vec4(1.0f);
	}
	
	return vNewColor;
}

vec4 DrawLineCircle()
{
	// 내부가 빈 원 그려보기
	vec4 vNewColor = vec4(0.0f);
	float fDist = distance(v_vColor.xy, vec2(0.5f, 0.5f));

	if (0.48f < fDist && fDist < 0.5f)
	{
		vNewColor = vec4(1.0f);
	}
	
	return vNewColor;
}

vec4 DrawCrossPattern()
{
	float fAxisX = sin((10.0f * 2.0f * g_fPI * v_vColor.x) + 0.5f * g_fPI);
	float fAxisY = sin((10.0f * 2.0f * g_fPI * v_vColor.y) + 0.5f * g_fPI);
	float fAxisMax = max(fAxisX, fAxisY);
	vec4 vNewColor = vec4(fAxisMax);

	return vNewColor;
}

vec4 DrawMultipleCircles()
{
	// 여러 개의 동심원 그려보기
	float fDist = distance(v_vColor.xy, vec2(0.5f, 0.5f)); // 0.0f ~ 0.5f
	vec4 vNewColor = vec4(sin(10.0f * 2.0f * g_fPI * fDist));

	return vNewColor;
}

vec4 DrawCircles()
{
	vec4 vNewColor = vec4(0.0f);

	for (int i = 0; i < 10; ++i)
	{
		float fDist = distance(v_vColor.xy, u_vPoints[i].xy);

		if (fDist < u_fTime)
		{
			vNewColor += vec4(sin(10.0f * 2.0f * g_fPI * fDist - 100.0f * u_fTime));
		}
	}

	return vNewColor;
}

vec4 DrawRaderCircle()
{
	float fDist = distance(v_vColor.xy, vec2(0.5f, 0.0f));

	// sin의 값은 -1.0f ~ 1.0f 사이의 값이므로, 제곱(pow)하게 되면 양수부분은 작아지고 음수부분은 값이 커진다.
	// 이때, clamp 함수를 이용하여, sin의 결과 값중 음수값을 제거한다.
	float fSineValue = clamp(pow(sin(2.0f * g_fPI * fDist - 100.0f * u_fTime), 4), 0.0f, 1.0f); 
	vec4 vNewColor = vec4(0.5f * fSineValue);        
	
	for (int i = 0; i < 10; ++i)
	{
		float fToPointDist = distance(v_vColor.xy, u_vPoints[i].xy);

		if (fToPointDist < 0.1f) // fToPointDist가 0.0f ~ 0.1f 일때만, 포인트 주위에 원을 형성한다. 즉, 포인트를 중심으로 하고 반지름이 0.1f인 원을 생성
		{
			vNewColor += vec4(20.0f * fSineValue * (0.1f - fToPointDist), 0.0f, 0.0f, 1.0f);
		}
	}

	return vNewColor;
}

void main()
{
	FragColor = DrawRaderCircle();
}
