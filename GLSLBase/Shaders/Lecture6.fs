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

vec4 Practice_1()
{
	vec2 vNewTexCoord = vec2(v_vTexCoord.x, abs(2.0f * (v_vTexCoord.y - 0.5f)));
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord);
	
	return vNewColor;
}

vec4 Practice_2()
{
	vec2 vNewTexCoord = vec2(3.0f * v_vTexCoord.x, v_vTexCoord.y / 3.0f + floor(3.0f * v_vTexCoord.x) / 3.0f);
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord);
	
	return vNewColor;
}

vec4 Practice_3()
{
	vec2 vNewTexCoord = vec2(3.0f * v_vTexCoord.x, v_vTexCoord.y / 3.0f + floor(3.0f * (1.0f - v_vTexCoord.x)) / 3.0f);
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord);

	return vNewColor;
}

vec4 Practice_4()
{
	vec2 vNewTexCoord = vec2(v_vTexCoord.x, fract(3.0f * v_vTexCoord.y) / 3.0f + (2.0f - floor(3.0f * v_vTexCoord.y)) / 3.0f);
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord);

return vNewColor;
}

vec4 Practice_5()
{
	vec2 vNewTexCoord = vec2(fract(2.0f * v_vTexCoord.x) + 0.5f * floor(2.0f * v_vTexCoord.y), fract(2.0f * v_vTexCoord.y));
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord);
	
	return vNewColor;
}

vec4 Practice_6()
{
	vec2 vNewTexCoord = vec2(fract(2.0f * v_vTexCoord.x), fract(2.0f * v_vTexCoord.y) + 0.5f * floor(2.0f * v_vTexCoord.x));
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord);
	
	return vNewColor;
}

void main()
{
	FragColor = Practice_6();
}
