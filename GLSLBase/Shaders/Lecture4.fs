#version 450

layout(location = 0) out vec4 FragColor;

in vec4 v_vColor;

void main()
{
	vec4 newColor = vec4(0.0f);

	// ���ΰ� ä���� �� �׷�����
	//float dist = distance(v_vColor.xy, vec2(0.5f, 0.5f));

	//if (dist < 0.5f)
	//{
	//	newColor = vec4(1.0f);
	//}

	// �簢���� ���� �� ���κи� �׷�����
	//if (v_vColor.y > 0.5f)
	//{
	//	newColor = vec4(1.0f);
	//}

	// ���ΰ� �� �� �׷�����
	float dist = distance(v_vColor.xy, vec2(0.5f, 0.5f));

	if (0.48f < dist && dist < 0.5f)
	{
		newColor = vec4(1.0f);
	}

	FragColor = newColor;
}
