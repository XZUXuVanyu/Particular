#version 460 core
uniform int iDraw_mode;
out vec4 aFrag_color;

const vec3 mesh_color = vec3(1.0, 1.0, 1.0);
const vec3 axis_color = vec3(0.0, 1.0, 1.0);
void main() 
{
	if (iDraw_mode == 0)
	{
        aFrag_color = vec4(1.0,1.0,1.0,1.0);
	}
	else
	{
		aFrag_color = vec4(axis_color, 1.0f);
	}
}