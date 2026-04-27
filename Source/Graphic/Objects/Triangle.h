//==============================================================================
#pragma once
#include "../Object.h"
//==============================================================================
inline constexpr GL_Vertex_Attrib triangle_pos = 
{
	.location = 0,
	.size = 3,
	.type = GL_FLOAT,
	.normalized = GL_FALSE,
	.stride = 3 * sizeof(GLfloat),
	.offset = 0
};

class Triangle : public Object
{
public:
	using Object::Object;
	void	childInitialise() override;
	void	childRender(const glm::mat4& global_VP, const glm::vec3& camera_pos) override;
	void	childCleanup() override;

private:
	std::vector<GLfloat> vertices = {
		// x,      y,      z
		-0.5f, -0.5f,  0.0f,  // Index 0: Bottom-Left
		 0.5f, -0.5f,  0.0f,  // Index 1: Bottom-Right
		 0.5f,  0.5f,  0.0f,  // Index 2: Top-Right
		-0.5f,  0.5f,  0.0f   // Index 3: Top-Left
	};

	std::vector<GLuint> indices = {
		0, 1, 3, // Triangle 1
		1, 2, 3  // Triangle 2
	};
};
//==============================================================================