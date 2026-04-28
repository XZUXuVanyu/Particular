//==============================================================================
#pragma once
#include "../Foundation.h"
#include <glm-master/glm/glm.hpp>
//==============================================================================
using namespace juce::gl;
//==============================================================================
namespace Crystal
{
	//==============================================================================
	struct Object_Handle
	{
		GLuint index;
		GLuint history;
	};
	//==============================================================================
	/* Parent class for independent objects to render */
	class Object : public Entity
	{
	public:
		//==============================================================================
		Object(juce::OpenGLContext& contex, const std::vector<juce::String>& shader_src);
		virtual ~Object();
	public:
		//==============================================================================
		/* Functions to be implemented by child */
		/*	Do child-specific resources initialisation, you can ignore it.
			However, you MUST complement Object::child_Initialise() */
		void			baseInitialise();
		/*	To be called in Renderer::renderOpenGL(), you can ignore it.
			However, you MUST complement Object::childRender() */
		void			baseRender(const glm::mat4& global_VP, const glm::vec3& camera_pos);
		/*	Do child-specific resources cleanup, you can ignore it.
			However, you MUST complement Object::childCleanup() */
		void			baseCleanup();
	public:
		//==============================================================================
		Object_Handle	getHandle()			const;
		GLuint			getRenderProgID()	const;
		GLuint			getComputeProgID()	const;
		GLuint			getVAOID()			const;
		GLuint			getVBOID()			const;
		size_t			getAllocatedSize()	const;
		juce::OpenGLContext& getGLContext() const;

		GLuint vao_id = 0;
		GLuint vbo_id = 0;
		GLuint ebo_id = 0;
	private:
		//==============================================================================
		void			loadShaderProg(const juce::String v_shader_name, const juce::String f_shader_name,
			const juce::String c_shader_name = "", bool with_compute_shader = false);
	private:
		//==============================================================================
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Object);
		Object_Handle object_handle;

		/* OpenGL identifiers */
		juce::String vert_shader_name, frag_shader_name, compute_shader_name;

		juce::OpenGLContext& gl_context;
		GLuint render_program_id = 0;
		GLuint compute_program_id = 0;

		std::unordered_map<juce::String, GLint> render_uniform_locations;
		std::unordered_map<juce::String, GLint> compute_uniform_locations;
	protected:
		//==============================================================================
		/* Child implementations that MUST to be completed */
		virtual void	childInitialise() = 0;
		virtual void	childRender(const glm::mat4& global_VP, const glm::vec3& camera_pos) = 0;
		virtual void	childCleanup() {};
	protected:
		/* OpenGL initialise auxilarities */
		/* TODO: this function should be implemented later after file output reconstructing */
		juce::File  getShaderFile(const juce::String& file_name) const;
		GLuint      genComputeProg(const juce::String src) const;
		GLuint      genComputeProgfromFile(const juce::String path) const;
		GLuint      genRenderProg(const juce::String vsrc, const juce::String fsrc) const;
		GLuint      genRenderProgfromFile(const juce::String vpath, const juce::String fpath) const;
		GLint		getUniformLoc(const juce::String& uniform_name, bool in_compute_shader = false);
		void		setVertexAttrib();
		void		fillInBufferData();
	};
	//==============================================================================
}