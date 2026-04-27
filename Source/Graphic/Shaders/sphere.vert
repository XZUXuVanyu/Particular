#version 460 core
uniform mat4 mGlobal_VP;
uniform int iDraw_mode; // 0 = UVsphere, 1 = analaticalsphere

layout (location = 0) in vec3 aPos;
void main()
{
	if (iDraw_mode == 0)
	{
		gl_Position = mGlobal_VP * vec4(aPos, 1.0);
	}
}