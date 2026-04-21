//==============================================================================
#include "Sphere.h"
#include <glm-master/glm/glm.hpp>
//==============================================================================
UVSphere::UVSphere(juce::OpenGLContext& context, GLuint subdivision) 
    : Object(context), subdivision(subdivision)
{
    /* Here we have the size of verticies (S+1)*(S+1) to benefit interploation */
    const size_t vCount = (subdivision + 1) * (subdivision + 1);
    vertices.reserve(vCount);

    const size_t iCount = 6 * subdivision * subdivision;
    indices.reserve(iCount);
}
void UVSphere::initialise()
{
}
void UVSphere::render(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
}
void UVSphere::cleanup()
{
    Object::cleanup();
}
void UVSphere::genUVSphere()
{
    if (subdivision <= 1)
    {
        DBG("[ERROR] Invalid param");
        jassertfalse;
        return;
    }

    vertices.resize((subdivision + 1) * (subdivision + 1));
    indices.resize(6 * subdivision * subdivision);

    GLuint vert_idx = 0;
    GLdouble pi = juce::MathConstants<GLdouble>::pi;
    GLdouble delta_h = 2.0 * pi / (GLdouble)subdivision;
    GLdouble delta_v = pi / (GLdouble)subdivision;
    for (GLuint v = 0; v < subdivision + 1; ++v)
    {
        GLdouble phi = v * delta_v;
        GLdouble sin_phi = std::sin(phi);
        GLdouble cos_phi = std::cos(phi);

        for (GLuint h = 0; h < subdivision; ++h)
        {
            GLdouble theta = h * delta_h;

            GLfloat x = (GLfloat)(sin_phi * std::cos(theta));
            GLfloat y = (GLfloat)(sin_phi * std::sin(theta));
            GLfloat z = (GLfloat)(cos_phi);

            vert_idx = v * (subdivision + 1) + h;
            vertices[vert_idx] = glm::vec3(x, y, z);
        }
    }

    GLuint triangle_idx = 0;
    for (GLuint v = 0; v < subdivision; ++v)
    {
        for (GLuint h = 0; h < subdivision; ++h)
        {
            GLuint p0 = (v + 0) * (subdivision + 1) + (h + 0);
            GLuint p1 = (v + 1) * (subdivision + 1) + (h + 0);
            GLuint p2 = (v + 1) * (subdivision + 1) + (h + 1);
            GLuint p3 = (v + 0) * (subdivision + 1) + (h + 1);

            indices[triangle_idx++] = glm::uvec3(p0, p1, p3);
            indices[triangle_idx++] = glm::uvec3(p1, p2, p3);
        }
    }



}
//==============================================================================