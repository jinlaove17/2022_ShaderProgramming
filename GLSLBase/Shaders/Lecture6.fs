#version 450

layout(location = 0) out vec4 FragColor;

in vec2 v_vTexCoord;

uniform sampler2D u_sTexSampler;

vec4 Flip()
{
	vec2 vNewTexCoord = vec2(v_vTexCoord.x, 1.0f - v_vTexCoord.y);
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord);
	
	return vNewColor;
}

void main()
{
	FragColor = Flip();
}
