#version 450

layout(location = 0) out vec4 FragColor;

in vec2 v_vTexCoord;

uniform sampler2D u_sTexSampler;

void main()
{
	FragColor = texture(u_sTexSampler, v_vTexCoord);
}
