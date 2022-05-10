#version 450

in vec3 a_vPosition;
in vec3 a_vVelocity;

uniform float u_fTime;

void main()
{
	vec3 vNewPosition = a_vPosition + u_fTime * a_vVelocity;

	gl_Position = vec4(vNewPosition, 1.0f);
}
