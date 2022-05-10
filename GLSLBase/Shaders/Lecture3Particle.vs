#version 450

in vec3 a_vPosition;

void main()
{
	gl_Position = vec4(a_vPosition, 1.0f);
}
