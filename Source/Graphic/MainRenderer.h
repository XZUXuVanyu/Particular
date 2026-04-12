//==============================================================================
/* This program is dedicated in rendering particles. */
//==============================================================================
#pragma once
#include <vector>
#include <unordered_map>
#include <JuceHeader.h>
#include <glm-master/glm/glm.hpp>
#include "../Math/Quaternion.h"
using namespace juce::gl;
//==============================================================================
/* Simple struct for fill in OpenGL vertex attributes */
struct GL_Vertex_Attrib;
//==============================================================================
/* Parent class for independent objects to render */
class RenderObject
{
public:
	//==============================================================================
	RenderObject() = default;
	virtual ~RenderObject() = default;

	/* Functions to be implemented by child */
	/* Initializes derived-class resources. Always use loadShaderProg() to handle shader compilation and linking. */
	virtual void initialise() = 0;
	virtual void render(const glm::mat4& global_VP, const glm::vec3& camera_pos) = 0;
	/* Do child-specific resources cleanup and MUST call RenderObject::cleanup() at end to release base GL handles. */
	virtual void cleanup() = 0;
protected:
	/* OpenGL identifiers */
	GLuint render_program_id = 0; 
	GLuint compute_program_id = 0;

	GLuint vao_id = 0; 
	std::vector<GLuint> vbo_id;

	std::unordered_map<juce::String, GLint> render_uniform_locations;
	std::unordered_map<juce::String, GLint> compute_uniform_locations;

	/* OpenGL auxiliraties */
	juce::File  getShaderFile(const juce::String& file_name) const;
	GLint		getUniformLoc(const juce::String& uniform_name, bool in_compute_shader = false);
	GLuint      genComputeProg(const juce::String src) const;
	GLuint      genComputeProgfromFile(const juce::String path) const;
	GLuint      genRenderProg(const juce::String vsrc, const juce::String fsrc) const;
	GLuint      genRenderProgfromFile(const juce::String vpath, const juce::String fpath) const;
	
	/* Always call this function to load shader for child */
	void        loadShaderProg(const juce::String v_shader_name, const juce::String f_shader_name,
		const juce::String c_shader_name = "", bool with_compute_shader = false);

private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RenderObject);
};
/* A mesh that act as global ground ref */
class GlobalMesh : public RenderObject
{
public:
	//==============================================================================
	GlobalMesh(GLuint division, GLuint sub_division);

	void initialise() override;
	void render(const glm::mat4& global_VP, const glm::vec3& camera_pos) override;
	void cleanup() override;
	GLuint getProgramID();

private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GlobalMesh);
	GLfloat cell_size = 1.0f;
};
/* A renderer that organizes different RenderObjects */
class MainRenderer : public juce::OpenGLRenderer, public juce::Component
{
public:
	//==============================================================================
	MainRenderer(juce::OpenGLContext& context);
	~MainRenderer() override;

	/* JUCE OpenGL */
	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	/* JUCE Component */
	void paint(juce::Graphics& g) override;
	void resized() override;
	bool keyPressed(const juce::KeyPress& key) override;
	void mouseDown(const juce::MouseEvent& event) override;
	void mouseDrag(const juce::MouseEvent& event) override;
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainRenderer);
	juce::OpenGLContext& gl_context;
	juce::TextEditor debug_info;

	/* Camera properties */
	GLfloat fov = juce::degreesToRadians(45.0);
	GLfloat dnear = 0.01, dfar = 1000.0;
	GLfloat aspect = 16.0 / 9.0;

	/* Global coordinate params */
	glm::vec3 camera_pos{ 0.0, -5.0, 2.0 };
	Quaternion camera_r{ 0.0, 1.0,0.0,0.0 };
	Quaternion camera_f{ 0.0, 0.0,1.0,0.0 };
	Quaternion camera_u{ 0.0, 0.0,0.0,1.0 };
	Quaternion camera_rotation{ 1.0, 0.0,0.0,0.0 };

	/* Global transform matrices */
	glm::mat4 view_mat;
	glm::mat4 projection_mat;
	glm::mat4 global_VP;
	void update_Vmat();
	void update_Pmat();

	/* Mouse control  */
	GLfloat sensitivity_x;
	GLfloat sensitivity_y;
	juce::Point<GLint> last_mouse_pos{ 0,0 };

	/* Independent objects to render */
	std::unique_ptr<GlobalMesh> mesh;
};
//==============================================================================