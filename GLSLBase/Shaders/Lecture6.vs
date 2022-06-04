#version 450

in vec3 a_vPosition;
in vec2 a_vTexCoord;

out vec2 v_vTexCoord;

void main()
{
	gl_Position = vec4(a_vPosition, 1.0f);
	v_vTexCoord = a_vTexCoord;
}
