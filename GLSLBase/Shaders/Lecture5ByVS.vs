#version 450

in vec3 a_vPosition;

uniform float u_fTime;

const float g_fPI = 3.141592f;

void main()
{
	float fRadian = 2.0f * g_fPI * (a_vPosition.x + 1.0f) + u_fTime;
	vec3 vNewPosition = vec3(a_vPosition.x, 0.5f * sin(fRadian), 0.0f);

	gl_Position = vec4(vNewPosition, 1.0f);
}
