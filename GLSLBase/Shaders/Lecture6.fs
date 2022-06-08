#version 450

layout(location = 0) out vec4 FragColor;

in vec2 v_vTexCoord;

uniform sampler2D u_sTexSampler;
uniform sampler2D u_sTexSampler1;
uniform float u_fTime;

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

vec4 MultiTexture()
{
	vec2 vNewTexCoord = vec2(fract(2.0f * v_vTexCoord.x), v_vTexCoord.y);
	vec4 vNewColor = vec4(0.0f);
	
	if (v_vTexCoord.x < 0.5f)
	{
		vNewColor = texture(u_sTexSampler, vNewTexCoord);
	}
	else
	{
		vNewColor = texture(u_sTexSampler1, vNewTexCoord);
	}

	return vNewColor;
}

vec4 AddTextures()
{
	vec2 vNewTexCoord = vec2(v_vTexCoord);

	// glTexParameteri()의 세번째 인자가 CLAMP이기 때문임, WRAP_R로 변경하거나 fract() 함수로 0.0f ~ 1.0f 범위로 보정
	vec2 vMovingTexCoord = vec2(fract(v_vTexCoord.x + u_fTime), v_vTexCoord.y);
	vec4 vNewColor = texture(u_sTexSampler, vNewTexCoord) * texture(u_sTexSampler1, vMovingTexCoord);

	return vNewColor;
}

void main()
{
	FragColor = AddTextures();
}
