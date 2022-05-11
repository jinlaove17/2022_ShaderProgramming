#version 450

in vec3 a_vPosition;
in vec3 a_vVelocity;
in float a_fEmitTime;
in float a_fLifeTime;
in float a_fPeriod;
in float a_fAmp;

uniform float u_fTime;
uniform vec3 u_vAccel;

bool g_bLoop = true; // ¼÷Á¦

const float g_fPI = 3.141592f;

void main()
{	
	vec3 vNewPosition = vec3(0.0f);
	float fTime = u_fTime - a_fEmitTime;

	if (fTime > 0)
	{
		float fTemp = fTime / a_fLifeTime;
		float fFrac = fract(fTemp);
		
		fTime = fFrac * a_fLifeTime;

		vNewPosition.x = a_vPosition.x + (fTime * a_vVelocity.x) + (0.5f * fTime * fTime * u_vAccel.x);
		vNewPosition.y = a_vPosition.y + a_fAmp * sin(2.0f * g_fPI * a_fPeriod * fTime);
	}
	else
	{
		vNewPosition = vec3(99999.9f);
	}

	gl_Position = vec4(vNewPosition, 1.0f);
}
