//==============================================================================
/* This program is dedicated in rendering particles. */
//==============================================================================
#pragma once
#include <vector>
#include <unordered_map>
#include <JuceHeader.h>
#include <glm-master/glm/glm.hpp>
using namespace juce::gl;
//==============================================================================
/* Forward declarations */
class Camera;
//==============================================================================
/* Simple struct for fill in OpenGL vertex attributes */
struct GL_Vertex_Attrib;
//==============================================================================
/* Handle for lookup objects */
struct Object_Handle
{
	GLuint index;
	GLuint history;
};
/* Parent class for independent objects to render */
class Object
{
public:
	//==============================================================================
	Object(juce::OpenGLContext& context);
	virtual ~Object() = default;

	/* Functions to be implemented by child */
	/* Initializes derived-class resources. Always use loadShaderProg() to handle shader compilation and linking. */
	virtual void initialise() = 0;
	virtual void render(const glm::mat4& global_VP, const glm::vec3& camera_pos) = 0;
	/* Do child-specific resources cleanup and MUST call Object::cleanup() at end to release base GL handles. */
	virtual void cleanup() = 0;

	void setHandle(const Object_Handle& handle);
	Object_Handle getHandle() const;
protected:
	/* OpenGL identifiers */
	juce::OpenGLContext& gl_context;
	GLuint render_program_id = 0;
	GLuint compute_program_id = 0;

	GLuint vao_id = 0; 
	std::vector<GLuint> vbo_id;

	std::unordered_map<juce::String, GLint> render_uniform_locations;
	std::unordered_map<juce::String, GLint> compute_uniform_locations;

	/* OpenGL initialise auxilarities */
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
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Object);
	Object_Handle object_handle;
};
//==============================================================================
/* Render slot */
struct Render_Slot
{
	std::unique_ptr<Object>	object;
	GLuint history = 0;
};
struct Slot_Flags {
	uint8_t isempty		: 1 = true;
	uint8_t isready		: 1 = false;
	uint8_t isvisible	: 1 = false;
	uint8_t reserved	: 5 = false;
};
/* Renderer that manage all objects */
class Renderer : public juce::OpenGLRenderer, public juce::Component, public juce::Timer
{
public:
	//==============================================================================
	Renderer(juce::OpenGLContext& context);
	~Renderer() override;

	/* JUCE OpenGL */
	juce::OpenGLContext& getglContext();
	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	/* JUCE Component */
	void paint(juce::Graphics& g) override;
	void resized() override;

	/* JUCE Timer */
	void timerCallback() override;

	/* Render Objects */
	Object_Handle registerObject(std::unique_ptr<Object> object, GLuint target_slot);
	void makeObjectVisible(const Object_Handle& handle);
	void makeObjectHidden(const Object_Handle& handle);
	void removeObject(const Object_Handle& handle);
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Renderer);
	juce::OpenGLContext& gl_context;
	juce::TextEditor debug_info;
	GLdouble timer;
	GLdouble dt;
	void updateTime();

	/* Main camera */
	std::unique_ptr<Camera> main_camera;

	/* Independent objects to render */
	std::vector<Render_Slot> active_slots;
	std::vector<Slot_Flags> access_table;

	std::queue<std::pair<GLuint, Slot_Flags>> set_state_queue;
	std::queue<std::pair<GLuint, std::unique_ptr<Object>>> register_queue;
	std::queue<GLuint> remove_queue;
	juce::CriticalSection request_lock;
	GLboolean isHandleValid(const Object_Handle& handle);
	void processRequests();
};
//==============================================================================