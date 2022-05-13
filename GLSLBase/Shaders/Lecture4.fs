#version 450

layout(location = 0) out vec4 FragColor;

in vec4 v_vColor;

const float g_fPI = 3.141592f;

vec4 DrawHalfRect()
{
	// �簢���� ���� �� ���κи� �׷�����
	vec4 vNewColor = vec4(0.0f);

	if (v_vColor.y > 0.5f)
	{
		vNewColor = vec4(1.0f);
	}

	return vNewColor;
}

vec4 DrawCircle()
{
	// ���ΰ� ä���� �� �׷�����
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
	// ���ΰ� �� �� �׷�����
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
	// ���� ���� ���ɿ� �׷�����
	float fDist = distance(v_vColor.xy, vec2(0.5f, 0.5f)); // 0.0f ~ 0.5f
	vec4 vNewColor = vec4(sin(10.0f * 2.0f * g_fPI * fDist));

	return vNewColor;
}

void main()
{
	FragColor = DrawMultipleCircles();
}
