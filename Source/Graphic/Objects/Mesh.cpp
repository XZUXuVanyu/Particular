//==============================================================================
#include "Mesh.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
void Mesh::initialise()
{
	loadShaderProg("mesh.vert", "mesh.frag");
	genAndbindVAO();
}
void Mesh::render(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	glUseProgram(getRenderProgID());
	glUniformMatrix4fv(getUniformLoc("mGlobal_VP"), 1, GL_FALSE, glm::value_ptr(global_VP));

	/* Draw plain */
	glUniform1i(getUniformLoc("iDraw_mode"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	/* Draw normal */
	glUniform1i(getUniformLoc("iDraw_mode"), 1);
	glDrawArrays(GL_LINES, 0, 2);
	glUseProgram(0);
}
void Mesh::cleanup()
{
	Object::cleanup();
}
//==============================================================================