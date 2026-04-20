#include "Mesh.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
void Mesh::initialise()
{
	auto& glfunc = gl_context.extensions;
	loadShaderProg("mesh.vert", "mesh.frag");
	glfunc.glGenVertexArrays(1, &vao_id);
	glfunc.glBindVertexArray(vao_id);

}
void Mesh::render(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	auto& glfunc = gl_context.extensions;
	glfunc.glUseProgram(render_program_id);
	glfunc.glUniformMatrix4fv(getUniformLoc("mGlobal_VP"), 1, GL_FALSE, glm::value_ptr(global_VP));

	/* Draw plain */
	glfunc.glUniform1i(getUniformLoc("iDraw_mode"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	/* Draw normal */
	glfunc.glUniform1i(getUniformLoc("iDraw_mode"), 1);
	glDrawArrays(GL_LINES, 0, 2);
	glfunc.glUseProgram(0);
}
void Mesh::cleanup()
{
	Object::cleanup();
}
