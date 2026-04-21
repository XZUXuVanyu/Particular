//==============================================================================
#pragma once
#include <JuceHeader.h>
#include <glm-master/glm/glm.hpp>
//==============================================================================
using namespace juce::gl;
//==============================================================================
struct GL_Vertex_Attrib
{
	GLuint		location;
	GLint		size;
	GLenum		type;
	GLboolean	normalized;
	GLsizei		stride;
	size_t		offset;
};
struct VBO_slot
{
	GLuint vbo_id = 0;
	GLuint allocated_size = 0;
};
struct VBO_Slot_Flags 
{
	uint8_t is_empty		: 1 = true;
	uint8_t is_binded		: 1 = false;
	uint8_t is_allocated	: 1 = false;
	uint8_t is_dynamic		: 1 = false;
	uint8_t reserved		: 4 = 0;
};
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
	virtual ~Object() {};

	/* Functions to be implemented by child */
	/* Initializes derived-class resources. Always use loadShaderProg() to handle shader compilation and linking. */
	virtual void	initialise() = 0;
	virtual void	render(const glm::mat4& global_VP, const glm::vec3& camera_pos) = 0;
	/* Do child-specific resources cleanup and MUST call Object::cleanup() at end to release base GL handles. */
	virtual void	cleanup() = 0;

	void			setHandle(const Object_Handle& handle);
	Object_Handle	getHandle() const;

	GLuint			getRenderProgID() const;
	GLuint			getComputeProgID() const;
	GLuint			getVAOID() const;
	size_t			getAllocatedSize() const;
protected:
	/* OpenGL initialise auxilarities */
	/* TODO: this function should be implemented later after file output reconstructing */
	juce::File  getShaderFile(const juce::String& file_name) const;
	GLuint      genComputeProg(const juce::String src) const;
	GLuint      genComputeProgfromFile(const juce::String path) const;
	GLuint      genRenderProg(const juce::String vsrc, const juce::String fsrc) const;
	GLuint      genRenderProgfromFile(const juce::String vpath, const juce::String fpath) const;
	/* Always call this function to load shader for child */
	void        loadShaderProg(const juce::String v_shader_name, const juce::String f_shader_name,
		const juce::String c_shader_name = "", bool with_compute_shader = false);

	void		genAndbindVAO();
	void		genAndbindVBO(GLuint target_slot, bool is_dynamic);
	void		setVBOSlotState(GLuint target_slot, const uint8_t mask, bool set);
	void		allocateVBO(GLuint target_slot, size_t size);
	void		updateVBO(GLuint target_slot, const void* data_ptr, size_t size);
	void		genAndbindEBO();
	void		setObjectVertAttrib(GLuint target_slot, const std::vector<GL_Vertex_Attrib> layout);
	GLint		getUniformLoc(const juce::String& uniform_name, bool in_compute_shader = false);
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Object);
	Object_Handle object_handle;

	/* OpenGL identifiers */
	juce::OpenGLContext& gl_context;
	GLuint render_program_id = 0;
	GLuint compute_program_id = 0;

	GLuint vao_id = 0;
	GLuint ebo_id = 0;

	std::vector<VBO_slot> vbo_slots;
	std::vector<VBO_Slot_Flags> access_table;

	std::unordered_map<juce::String, GLint> render_uniform_locations;
	std::unordered_map<juce::String, GLint> compute_uniform_locations;
};
//==============================================================================