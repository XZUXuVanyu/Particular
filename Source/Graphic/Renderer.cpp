//==============================================================================
#include "Renderer.h"
#include "Camera.h"
#include <glm-master/glm/glm.hpp>
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
struct GL_Vertex_Attrib
{
	GLuint		location;
	GLint		size;
	GLenum		type;
	GLboolean	normalized;
	GLsizei		stride;
	GLuint		offset;
};
void RenderObject::cleanup()
{
	if (render_program_id)	glDeleteProgram(render_program_id);
	if (compute_program_id) glDeleteProgram(compute_program_id);
	if (vao_id) glDeleteVertexArrays(1, &vao_id);
	if (!vbo_id.empty())
	{
		glDeleteBuffers((GLsizei)vbo_id.size(), vbo_id.data());
		vbo_id.clear();
	}
}
//==============================================================================
/* class RenderObject */
/* TODO: this function should be implemented later after file output reconstructing */
juce::File RenderObject::getShaderFile(const juce::String& file_name) const
{
	auto file = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
	for (int i = 0; i < 6; ++i)
		file = file.getParentDirectory();

	auto result = file.getChildFile("Source").getChildFile("Graphic").getChildFile("Shaders")
		.getChildFile(file_name);
	if (!result.existsAsFile())
	{
		DBG("[ERROR] Not a valid file name provided");
		jassertfalse;
	}

	DBG("[INFO] Project Root detected at: " + file.getFullPathName());
	return result;
	
}
GLint RenderObject::getUniformLoc(const juce::String& uniform_name, bool in_compute_shader)
{
	GLint	location = 0;
	GLuint	target_prog	= in_compute_shader ? compute_program_id : render_program_id;
	auto&	target_cache = in_compute_shader ? compute_uniform_locations : render_uniform_locations;
	
	if (target_prog == 0)
	{
		DBG("[Warning] Unable to locate uniform \"" + uniform_name + "\"");
		return -1;
	}

	auto it = target_cache.find(uniform_name);
	if (it != target_cache.end()) return it->second;

	GLint loc = glGetUniformLocation(target_prog, uniform_name.toRawUTF8());
	target_cache[uniform_name] = loc;

	return loc;
}
GLuint RenderObject::genComputeProg(const juce::String src) const
{
	if (src.isEmpty())
	{
		juce::Logger::writeToLog("[ERROR] Source code could not be empty");
		jassertfalse;
		return 0;
	}
	GLint success = 0;
	char msg[1024];

	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	const char* shader_ptr = src.toRawUTF8();
	glShaderSource(shader, 1, &shader_ptr, nullptr);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(shader, 512, NULL, msg);
		juce::Logger::writeToLog("Compute shader compile error:" + juce::String(msg));
		return 0;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, shader);
	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success)
	{
		// FIX: use 'prog' not the shader id when getting the program info log
		glGetProgramInfoLog(prog, 1024, NULL, msg);
		juce::Logger::writeToLog("[ERROR] Compute shader program link error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}
	glDeleteShader(shader);
	return prog;
}
GLuint RenderObject::genComputeProgfromFile(const juce::String path) const
{
	juce::File shaderFile = path;
	if (!shaderFile.existsAsFile())
	{
		juce::Logger::writeToLog("[ERROR] Compute shader source file path don't exist");
		jassertfalse;
		return 0;
	}
	return genComputeProg(shaderFile.loadFileAsString());
}
GLuint RenderObject::genRenderProg(const juce::String vsrc, const juce::String fsrc) const
{
	if (vsrc.isEmpty() || fsrc.isEmpty())
	{
		juce::Logger::writeToLog("Source code could not be empty!");
		return 0;
	}
	GLint success = 0;
	char msg[1024];

	GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
	const char* shader_ptr = vsrc.toRawUTF8();
	glShaderSource(vshader, 1, &shader_ptr, nullptr);
	glCompileShader(vshader);

	glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vshader, 1024, NULL, msg);
		juce::Logger::writeToLog("[ERROR] Vertex shader compile error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}

	GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
	const char* fhader_ptr = fsrc.toRawUTF8();
	glShaderSource(fshader, 1, &fhader_ptr, nullptr);
	glCompileShader(fshader);

	glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fshader, 1024, NULL, msg);
		DBG("[ERROR] Fragment shader compile error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}

	GLuint prog = glCreateProgram();
	glAttachShader(prog, vshader);
	glAttachShader(prog, fshader);
	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(prog, 512, NULL, msg);
		juce::Logger::writeToLog("[ERROR] Render shader program link error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}

	glDeleteShader(vshader);
	glDeleteShader(fshader);
	return prog;
}
GLuint RenderObject::genRenderProgfromFile(const juce::String vpath, const juce::String fpath) const
{
	juce::File vshaderFile = vpath;
	juce::File fshaderFile = fpath;
	if (!vshaderFile.existsAsFile() || !fshaderFile.existsAsFile())
	{
		juce::Logger::writeToLog("Shader source file path don't exist");
		jassertfalse;
		return 0;
	}
	return genRenderProg(vshaderFile.loadFileAsString(), fshaderFile.loadFileAsString());
}
void RenderObject::loadShaderProg(const juce::String v_shader_name, const juce::String f_shader_name,
	const juce::String c_shader_name, bool with_compute_shader)
{
	if (with_compute_shader)
	{
		auto c_file = getShaderFile(c_shader_name);
		compute_program_id = genComputeProgfromFile(c_file.getFullPathName());
		if (compute_program_id == 0)
		{
			DBG("[ERROR] Failed to load program: " + c_shader_name);
			jassertfalse;
		}
	}

	auto v_file = getShaderFile(v_shader_name);
	auto f_file = getShaderFile(f_shader_name);
	render_program_id = genRenderProgfromFile(v_file.getFullPathName(), f_file.getFullPathName());
	if (render_program_id == 0)
	{
		DBG("[ERROR] Failed to load program: " + v_shader_name + " / " + f_shader_name);
		jassertfalse;
	}
}
//==============================================================================
/* class MainRenderer */
Renderer::Renderer(juce::OpenGLContext& context) : gl_context(context)
{
	main_camera = std::make_unique<Camera>();
	mesh = std::make_unique<GlobalMesh>(10, 0);
	setWantsKeyboardFocus(true);

	addAndMakeVisible(debug_info);
	debug_info.setMultiLine(true);
	debug_info.setReadOnly(true);
	debug_info.setAlpha(0.6);

	DBG("[INFO] Renderer constructed");
}
Renderer::~Renderer()
{
}
void Renderer::newOpenGLContextCreated()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mesh.get()->initialise();
}
void Renderer::renderOpenGL()
{
	GLdouble current_time = juce::Time::getMillisecondCounter() * 0.001;
	GLdouble dt = (current_time - timer);
	timer = current_time;

	if (dt > 0.1) dt = 0.1;
	if (main_camera.get() != nullptr) main_camera.get()->update(dt);

	glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (mesh != nullptr) mesh.get()->render(
		main_camera.get()->getGlobalVP(), main_camera.get()->getCameraPos());
}
void Renderer::openGLContextClosing()
{
}
void Renderer::paint(juce::Graphics& g)
{
}
void Renderer::resized()
{
	if (main_camera == nullptr)
	{
		DBG("[ERROR] Bad initialization for main_camera");
		jassertfalse;
	}
	main_camera->setLastMousePos(localPointToGlobal(getLocalBounds().getCentre()));
	main_camera->onWindowResize(getLocalBounds());
	debug_info.setBoundsRelative(0.0, 0.0, 0.4, 0.3);
}
void Renderer::mouseDown(const juce::MouseEvent& event)
{
	grabKeyboardFocus();
}
//==============================================================================
/* class GlobalMesh */
GlobalMesh::GlobalMesh(GLuint division, GLuint sub_division)
{
}
void GlobalMesh::initialise()
{
	/* load mesh shader */
	loadShaderProg("mesh.vert", "mesh.frag");
	glUseProgram(render_program_id);
	glUniform1f(getUniformLoc("fCell_size"), cell_size);
}
void GlobalMesh::render(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	if (!render_program_id) initialise();
	glUseProgram(render_program_id);
	glBindVertexArray(vao_id);

	glUniformMatrix4fv(getUniformLoc("mGlobal_VP"), 1, GL_FALSE, glm::value_ptr(global_VP));
	glUniform3fv(getUniformLoc("vCamera_pos"), 1, glm::value_ptr(camera_pos));

	/* draw mode: 0 = mesh, 1 = axis */
	glUniform1i(getUniformLoc("iDraw_mode"), 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glUniform1i(getUniformLoc("iDraw_mode"), 1);
	glDrawArrays(GL_LINES, 0, 2);
}
void GlobalMesh::cleanup()
{
	RenderObject::cleanup();
}
GLuint GlobalMesh::getProgramID()
{
	return render_program_id;
}
//==============================================================================