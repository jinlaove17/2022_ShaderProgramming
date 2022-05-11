#version 450

layout(location = 0) out vec4 FragColor;

in vec4 v_vColor;

void main()
{
	FragColor = v_vColor;
}
