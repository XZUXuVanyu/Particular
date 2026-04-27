// triangle.vert
#version 430 core
uniform mat4 mGlobal_VP;
layout (location = 0) in vec3 aPos;
void main() 
{
    gl_Position = mGlobal_VP * vec4(aPos, 1.0);
}