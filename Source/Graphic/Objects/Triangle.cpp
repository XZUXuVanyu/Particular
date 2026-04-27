//==============================================================================
#include "Triangle.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
void Triangle::childInitialise()
{

	auto& glfunc = getGLContext().extensions;
	glfunc.glGenVertexArrays(1, &vao_id);
	glfunc.glBindVertexArray(vao_id);

	glfunc.glGenBuffers(1, &vbo_id);
	glfunc.glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

	glfunc.glGenBuffers(1, &ebo_id);
	glfunc.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);

	glfunc.glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * sizeof(GLuint)), indices.data(), GL_STATIC_DRAW);
	glfunc.glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(GLfloat)), vertices.data(), GL_STATIC_DRAW);
	glfunc.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 12, 0);

    glfunc.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
    glfunc.glEnableVertexAttribArray(0);

	glfunc.glBindVertexArray(0);
    glfunc.glBindBuffer(GL_ARRAY_BUFFER, 0);
	glfunc.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void Triangle::childRender(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	auto& glfunc = getGLContext().extensions;
	glfunc.glUniformMatrix4fv(getUniformLoc("mGlobal_VP"), 1, GL_FALSE, glm::value_ptr(global_VP));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
}
void Triangle::childCleanup()
{
}
