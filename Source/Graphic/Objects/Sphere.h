//==============================================================================
#pragma once
#include "../Object.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
GL_Vertex_Attrib UVSphere_Vert_Attrib
{
	.location = 0, 
	.size = 3*sizeof(GLfloat), 
	.type = GL_STATIC_DRAW, 
	.normalized = GL_FALSE, 
	.offset = 0
};
//==============================================================================
/* A sphere */
class UVSphere : public Object
{
public:
	/* Generate a UV sphere with specified subdivision */
	UVSphere(juce::OpenGLContext& context, GLuint subdivision);
	void initialise() override;
	void render(const glm::mat4& global_VP, const glm::vec3& camera_pos) override;
	void cleanup() override;

	void genUVSphere();
private:
	GLuint subdivision = 0;

	std::vector<glm::vec3>	vertices;
	std::vector<glm::uvec3>	indices;
};
//==============================================================================