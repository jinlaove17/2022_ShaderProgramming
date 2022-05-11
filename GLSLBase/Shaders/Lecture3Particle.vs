#version 450

in vec3 a_vPosition;
in vec3 a_vVelocity;
in float a_fEmitTime;
in float a_fLifeTime;
in float a_fPeriod;
in float a_fAmp;
in float a_fRandomValue;

uniform float u_fTime;
uniform vec3 u_vAccel;

bool g_bLoop = true; // ¼÷Á¦

const float g_fPI = 3.141592f;
const mat3 g_mRotate = mat3(0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

void main()
{	
	vec3 vNewPosition = vec3(0.0f);
	float fTime = u_fTime - a_fEmitTime;

	if (fTime > 0)
	{
		float fTemp = fTime / a_fLifeTime;
		float fFrac = fract(fTemp);
		vec3 vRotatedVelocity = normalize(a_vVelocity * g_mRotate);
		
		fTime = fFrac * a_fLifeTime;

		vNewPosition.x = sin(2.0f * g_fPI * a_fRandomValue) + a_vPosition.x;
		vNewPosition.y = cos(2.0f * g_fPI * a_fRandomValue) + a_vPosition.y;
		vNewPosition += (fTime * a_vVelocity) + (0.5f * fTime * fTime * u_vAccel);
		vNewPosition += a_fAmp * fTime * sin(2.0f * g_fPI * a_fPeriod * fTime) * vRotatedVelocity;
	}
	else
	{
		vNewPosition = vec3(99999.9f);
	}

	gl_Position = vec4(vNewPosition, 1.0f);
}
