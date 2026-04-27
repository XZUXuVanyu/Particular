//==============================================================================
#pragma once
#include "../Object.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
/* UVSphere vertex attribs */
inline constexpr GL_Vertex_Attrib pos = {
		.location = 0,
		.size = 3,
		.type = GL_FLOAT,
		.normalized = GL_FALSE,
		.stride = sizeof(glm::vec3),
		.offset = 0 };
/* A sphere */
class UVSphere : public Object
{
public:
	using	Object::Object;
	void	childInitialise() override;
	void	childRender(const glm::mat4& global_VP, const glm::vec3& camera_pos) override;
	void	childCleanup() override;

	void genUVSphere();

private:
	GLuint subdivision = 5;

	std::vector<glm::vec3>	vertices;
	std::vector<glm::uvec3>	indices;
};
//==============================================================================
