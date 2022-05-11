#version 450

in vec3 a_vPosition;
in vec3 a_vVelocity;
in float a_fEmitTime;
in float a_fLifeTime;

uniform float u_fTime;
uniform vec3 u_vAccel;

void main()
{	
	vec3 vNewPosition;
	float fTime = u_fTime - a_fEmitTime;

	if (fTime > 0)
	{
		vNewPosition = a_vPosition + (fTime * a_vVelocity) + (0.5f * fTime * fTime * u_vAccel);
	}
	else
	{
		vNewPosition = vec3(99999.9f);
	}

	gl_Position = vec4(vNewPosition, 1.0f);
}
