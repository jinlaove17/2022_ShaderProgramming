#version 450

in vec3 a_vPosition;
in vec4 a_vColor;

out vec4 v_vColor;

uniform float u_fTime;

void main()
{
	gl_Position = vec4(u_fTime * a_vPosition, 1.0f);
	v_vColor = a_vColor;
}
