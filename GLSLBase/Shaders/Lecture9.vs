#version 450

in vec3 a_vPosition;

out vec4 v_vColor;

uniform float u_fTime;

const float g_fPI = 3.141592f;

void Flag()
{
	vec3 vNewPosition = a_vPosition;
	float fToOriginDistX = a_vPosition.x - -0.5f;
	float fToOriginDistXY = distance(vec2(-0.5f, 0.0f), a_vPosition.xy);

	//float fAxisX = 0.2f * fToOriginDistX * sin(2.0f * g_fPI * (a_vPosition.x + 0.5f) - u_fTime);
	//float fAxisY = 0.5f * fToOriginDistX * sin(2.0f * g_fPI * (a_vPosition.x + 0.5f) - u_fTime);
	float fAxisX = 0.2f * fToOriginDistX * sin(2.0f * g_fPI * fToOriginDistXY - u_fTime);
	float fAxisY = 0.5f * fToOriginDistX * sin(2.0f * g_fPI * fToOriginDistXY - u_fTime);

	vNewPosition.x += fAxisX;
	vNewPosition.y += fAxisY;

	gl_Position = vec4(vNewPosition, 1.0f);
	v_vColor = vec4((sin(2.0f * g_fPI * (a_vPosition.x + 0.5f)) + 1.0f) / 2.0f);
}

void main()
{
	Flag();
}
