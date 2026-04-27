//==============================================================================
#include "Sphere.h"
#include <glm-master/glm/glm.hpp>
//==============================================================================
void UVSphere::genUVSphere()
{
	if (subdivision <= 1)
	{
		DBG("[ERROR] Invalid param");
		jassertfalse;
		return;
	}

	vertices.resize((subdivision * subdivision) + 1);
	indices.resize(2 * (subdivision * subdivision - subdivision));

	GLuint vert_idx_offset = 1;
	GLdouble pi = juce::MathConstants<GLdouble>::pi;
	GLdouble delta_h = 2.0 * pi / (GLdouble)subdivision;
	GLdouble delta_v = pi / (GLdouble)subdivision;
	
	/* North Pole & South Pole */
	vertices[0] = glm::vec3(0, 0, 10);
	vertices.back() = glm::vec3(0, 0, -10);

	/* Generate vertcies */
	for (GLuint v = 1; v <= subdivision - 1; v++)
	{
		GLdouble phi = (GLdouble)v * delta_v;
		GLdouble cos_phi = std::cos(phi);
		GLdouble sin_phi = std::sin(phi);

		for (GLuint h = 0; h <= subdivision; h++)
		{
			GLdouble theta = (GLdouble)h * delta_h;
			GLdouble cos_theta = std::cos(theta);
			GLdouble sin_theta = std::sin(theta);

			GLfloat x = (GLfloat)(sin_phi * cos_theta);
			GLfloat y = (GLfloat)(sin_phi * sin_theta);
			GLfloat z = (GLfloat)(cos_phi);

			GLuint vert_idx = vert_idx_offset + (v - 1) * (subdivision + 1) + h;
			vertices[vert_idx] = 10.0f * glm::vec3{ x,y,z };
		}
	}

	/* Generate indices */
	GLuint triangle_count = 0;
	/* North Cap */
	for (GLuint i = 0; i < subdivision; i++) 
	{
		GLuint p0 = 0, p1 = i + 1, p2 = i + 2;
		indices[triangle_count++] = glm::uvec3{ p0, p2, p1 };
	}
	/* Middle Band */
	for (GLuint v = 1; v <= subdivision - 2; v++)
	{
		GLuint ring_start = vert_idx_offset + (v - 1) * (subdivision + 1);
		GLuint ring_delta_idx = (subdivision + 1);
		for (GLuint h = 0; h <= subdivision - 1; h++)
		{
			GLuint p0 = ring_start + h;
			GLuint p1 = p0 + ring_delta_idx;
			GLuint p2 = p1 + 1;
			GLuint p3 = p0 + 1;

			indices[triangle_count++] = glm::uvec3{ p0, p1, p2 };
			indices[triangle_count++] = glm::uvec3{ p2, p3, p0 };
		}
	}
	/* South Cap */
	for (GLuint i = 0; i < subdivision; i++)
	{
		GLuint p0 = vertices.size() - 1, p1 = p0 - (subdivision + 1) + i, p2 = p1 + 1;
		indices[triangle_count++] = glm::uvec3{ p0, p2, p1 };
	}
	DBG("[INFO] size = " + juce::String(vertices.size()) + " " + juce::String(indices.size()));
}
void UVSphere::childInitialise()
{
	const size_t vCount = (subdivision * subdivision) + 1;
	vertices.resize(vCount);

	const size_t iCount = 2 * (subdivision * subdivision - subdivision);
	indices.resize(iCount);

	genUVSphere();

	auto& glfunc = getGLContext().extensions;
	glfunc.glGenVertexArrays(1, &vao_id);
	glfunc.glBindVertexArray(vao_id);

	glfunc.glGenBuffers(1, &vbo_id);
	glfunc.glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

	glfunc.glGenBuffers(1, &ebo_id);
	glfunc.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_id);

	glfunc.glBufferData(GL_ELEMENT_ARRAY_BUFFER, (indices.size() * sizeof(glm::uvec3)), indices.data(), GL_STATIC_DRAW);
	glfunc.glBufferData(GL_ARRAY_BUFFER, (vertices.size() * sizeof(glm::vec3)), vertices.data(), GL_STATIC_DRAW);
	glfunc.glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), nullptr);
	glfunc.glEnableVertexAttribArray(0);

	glfunc.glBindVertexArray(0);
	glfunc.glBindBuffer(GL_ARRAY_BUFFER, 0);
	glfunc.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void UVSphere::childRender(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	auto& glfunc = getGLContext().extensions;
	glfunc.glUniformMatrix4fv(getUniformLoc("mGlobal_VP"), 1, GL_FALSE, glm::value_ptr(global_VP));

	glfunc.glUniform1i(getUniformLoc("iDraw_mode"), 0);
	glDrawElements(GL_TRIANGLES, 3 * indices.size(), GL_UNSIGNED_INT, nullptr);
}
void UVSphere::childCleanup()
{

}
//==============================================================================

