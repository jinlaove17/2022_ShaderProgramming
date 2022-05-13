#version 450

in vec3 a_vPosition;
in vec3 a_vVelocity;
in float a_fEmitTime;
in float a_fLifeTime;
in float a_fPeriod;
in float a_fAmp;
in float a_fRandomValue;
in vec4 a_vColor;

out vec4 v_vColor;
out vec2 v_vTexCoord;

uniform float u_fTime;
uniform vec3 u_vAccel;

bool g_bLoop = true; // ¼÷Á¦

const float g_fPI = 3.141592f;
const mat3 g_mRotate = mat3(0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
const vec3 g_vGravity = vec3(0.0f, -0.5f, 0.0f);

void main()
{	
	vec3 vNewPosition = vec3(0.0f);
	float fTime = u_fTime - a_fEmitTime;

	if (fTime > 0)
	{
		float fTemp = fTime / a_fLifeTime;
		float fFrac = fract(fTemp);
		vec3 vRotatedVelocity = normalize(a_vVelocity * g_mRotate);
		vec3 vNewAccel = u_vAccel + g_vGravity;
		
		fTime = fFrac * a_fLifeTime;

		vNewPosition.x = sin(2.0f * g_fPI * a_fRandomValue) + a_vPosition.x;
		vNewPosition.y = cos(2.0f * g_fPI * a_fRandomValue) + a_vPosition.y;
		vNewPosition += (fTime * a_vVelocity) + (0.5f * fTime * fTime * vNewAccel);
		vNewPosition += a_fAmp * fTime * sin(2.0f * g_fPI * a_fPeriod * fTime) * vRotatedVelocity;
		v_vColor = a_vColor * (1.0f - fFrac);
	}
	else
	{
		vNewPosition = vec3(99999.9f);
		v_vColor = vec4(0.0f);
	}

	gl_Position = vec4(vNewPosition, 1.0f);
	v_vTexCoord = a_vPosition.xy;
}
