#version 450

layout(location = 0) out vec4 FragColor;

in vec4 v_vColor;
in vec2 v_vTexCoord;

void main()
{
	float dist = distance(v_vTexCoord, vec2(0.0f, 0.0f));
	
	if (dist < 0.025f)
	{
		FragColor = v_vColor;
	}
	else
	{
		FragColor = vec4(0.0f);
	}
}
