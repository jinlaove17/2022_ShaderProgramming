#version 450

in vec3 a_vPosition;

out vec2 v_vTexCoord;

void main()
{
	gl_Position = vec4(a_vPosition, 1.0f);
	v_vTexCoord = vec2(0.5f * (a_vPosition.xy + 1.0f));
}
