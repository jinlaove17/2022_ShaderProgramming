#version 450

in vec3 a_vPosition;

uniform float u_fTime;

void main()
{
	gl_Position = vec4(u_fTime * a_vPosition, 1.0f);
}
