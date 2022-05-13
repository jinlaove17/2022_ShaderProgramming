#version 450

in vec3 a_vPosition;
in vec4 a_vColor;

out vec4 v_vColor;

void main()
{
	gl_Position = vec4(a_vPosition, 1.0f);
	v_vColor = vec4(a_vPosition.x + 0.5f, a_vPosition.y + 0.5f, 0.0f, 1.0f);
}
