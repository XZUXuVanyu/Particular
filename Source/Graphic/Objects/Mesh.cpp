//==============================================================================
#include "Mesh.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
void Mesh::childInitialise()
{
}
void Mesh::childRender(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	auto& glfunc = getGLContext().extensions;
	glfunc.glUniformMatrix4fv(getUniformLoc("mGlobal_VP"), 1, GL_FALSE, glm::value_ptr(global_VP));

	/* Draw plain */
	glfunc.glUniform1i(getUniformLoc("iDraw_mode"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	/* Draw normal */
	glfunc.glUniform1i(getUniformLoc("iDraw_mode"), 1);
	glDrawArrays(GL_LINES, 0, 2);
}
void Mesh::childCleanup()
{

}
