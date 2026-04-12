#version 460 core
uniform int iDraw_mode;
uniform vec3 vCamera_pos;
uniform float fCell_size;
in vec2 vUV;
out vec4 aFrag_color;

const vec3 mesh_color = vec3(0.2, 0.2, 0.2);
const vec3 axis_color = vec3(0.0, 1.0, 1.0);

vec4 grid_line_color(vec2 uv, float spacing, float thickness) 
{
	float dist = length(uv);
	if (dist >= 1000.0f) discard;

	vec2 grid = abs(fract(uv / spacing - 0.5f) - 0.5f) / fwidth(uv / spacing);
    float dist_to_girdline = min(grid.x, grid.y);
    float grid_alpha = exp(-dist );

	vec3 grid_color = mix(mesh_color, vec3(1.0f), dist_to_girdline * 0.9f);
	return vec4(grid_color, grid_alpha);
}

void main() 
{
	if (iDraw_mode == 0)
	{
		aFrag_color = grid_line_color(vUV, fCell_size, 1.0f);
	}
	else
	{
		vec3 color = axis_color;
		aFrag_color = vec4(axis_color, 1.0f);
	}
}