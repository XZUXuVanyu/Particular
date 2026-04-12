#version 460 core
uniform mat4 mGlobal_VP;
uniform int iDraw_mode; // 0 = mesh, 1 = axis
uniform vec3 vCamera_pos;

out vec2 vUV;

const vec3 mesh_vertices[4] = vec3[4](
    vec3(-100.0, -100.0, 0.0),
    vec3( 100.0, -100.0, 0.0),
    vec3( 100.0,  100.0, 0.0),
    vec3(-100.0,  100.0, 0.0) 
);
const vec3 axis_vertices[2] = vec3[2](
    vec3(0.0, 0.0, -100.0),
    vec3(0.0, 0.0,  100.0) 
);
const int mesh_indices[6] = int[6](0, 1, 2, 2, 3, 0);
void main()
{
    vec3 local_pos;
    if (iDraw_mode == 0)
    {
        int idx = mesh_indices[gl_VertexID];
       
        local_pos = mesh_vertices[idx];
        local_pos.xy += vCamera_pos.xy;
        vUV = local_pos.xy;
    }
    else 
    {
        local_pos = axis_vertices[gl_VertexID];
        local_pos.z += vCamera_pos.z;
    }
    gl_Position = mGlobal_VP * vec4(local_pos, 1.0);
}