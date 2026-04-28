//==============================================================================
#include <atomic>
#include "Object.h"
#include "../Utilities.h"
//==============================================================================
using namespace Crystal;
//==============================================================================
Object::Object(juce::OpenGLContext& context, const std::vector<juce::String>& shader_src)
	: gl_context(context), vert_shader_name(shader_src[0]), frag_shader_name(shader_src[1]), compute_shader_name(shader_src[2])
{
	CRYSTAL_CHECK(shader_src.size() != 3, "shader_src MUST contain 3 names");
	setState(Entity_Constructed);
}
Crystal::Object::~Object()
{
	baseCleanup();
}
void Object::baseInitialise()
{
	CRYSTAL_CHECK(getState() != Entity_Constructed, "Bad construction or wrong initialising logic");
	setState(Entity_Initialising);
	if (compute_shader_name.isNotEmpty())
	{
		auto c_file = getShaderFile(compute_shader_name);
		compute_program_id = genComputeProgfromFile(c_file.getFullPathName());
		CRYSTAL_CHECK(compute_program_id == 0, "Failed to load program: " + compute_shader_name);
	}

	auto v_file = getShaderFile(vert_shader_name);
	auto f_file = getShaderFile(frag_shader_name);
	render_program_id = genRenderProgfromFile(v_file.getFullPathName(), f_file.getFullPathName());
	CRYSTAL_CHECK(render_program_id == 0, "Failed to load program: " + vert_shader_name + " / " + frag_shader_name);

	childInitialise();
	setState(Entity_Initialised);
}
/* TODO: add splited state change */
void Object::baseRender(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	CRYSTAL_CHECK(getState() != Entity_Initialised, "Object::baseRender() can ONLY be called when Entity_Initialised");
	setState(Entity_OnProcessing);
	auto& glfunc = gl_context.extensions;
	glfunc.glUseProgram(render_program_id);
	glfunc.glBindVertexArray(vao_id);

	childRender(global_VP, camera_pos);

	glfunc.glBindVertexArray(0);
	glfunc.glUseProgram(0);
	setState(Entity_Initialised);
}
void Object::baseCleanup()
{
	if (getState() < Entity_Constructed) return;
	CRYSTAL_CHECK(!(getState() >= Entity_Constructed), "Object is in an invalid state for cleanup", );
	setState(Entity_Deconstructing);

	childCleanup();

	auto& glfunc = gl_context.extensions;
	if (render_program_id)	glfunc.glDeleteProgram(render_program_id);
	if (compute_program_id) glfunc.glDeleteProgram(compute_program_id);
	if (vao_id) glfunc.glDeleteVertexArrays(1, &vao_id);
	if (ebo_id) glfunc.glDeleteBuffers(1, &ebo_id);
	if (vbo_id) glfunc.glDeleteBuffers(1, &vbo_id);

	setState(Entity_Deconstructed);
}
Object_Handle Object::getHandle() const
{
	CRYSTAL_CHECK(!(getState() >= Entity_Initialised),
		"Object::getHandle() can ONLY be called  after Entity_Initialised", Object_Handle());
	return object_handle;
}
GLuint Object::getRenderProgID() const
{
	CRYSTAL_CHECK(!(getState() >= Entity_Initialised),
		"Object::getRenderProgID() can ONLY be called after Entity_Initialised", 0);
	return render_program_id;
}
GLuint Object::getComputeProgID() const
{
	CRYSTAL_CHECK(!(getState() >= Entity_Initialised),
		"Object::getComputeProgID() can ONLY be called after Entity_Initialised", 0);
	return compute_program_id;
}
GLuint Object::getVAOID() const
{
	CRYSTAL_CHECK(!(getState() >= Entity_Initialised),
		"Object::getVAOID() can ONLY be called after Entity_Initialised", 0);
	return vao_id;
}
GLuint Object::getVBOID() const
{
	CRYSTAL_CHECK(!(getState() >= Entity_Initialised),
		"Object::getVBOID() can ONLY be called after Entity_Initialised", 0);
	return vbo_id;
}
/* TODO: add this implementaion */
size_t Object::getAllocatedSize() const
{
	CRYSTAL_CHECK(!(getState() >= Entity_Initialised),
		"Object::getAllocatedSize() can ONLY be called after Entity_Initialised", 0);
	size_t total_size = 0;
	return total_size;
}
juce::OpenGLContext& Object::getGLContext() const
{
	return gl_context;
}
/* TODO: weak file system adaptive */
juce::File Object::getShaderFile(const juce::String& file_name) const
{
	CRYSTAL_CHECK(getState() != Entity_Initialising, 
		"Object::getShaderFile() can ONLY be called when Entity_Initialising", juce::File());
	auto file = juce::File::getSpecialLocation(juce::File::currentExecutableFile);
	for (int i = 0; i < 6; ++i)
		file = file.getParentDirectory();

	auto result = file.getChildFile("Source").getChildFile("Graphic").getChildFile("Shaders")
		.getChildFile(file_name);
	CRYSTAL_CHECK(!result.existsAsFile(), "Not a valid file name provided", juce::File());
	DBG("[CRYSTAL-INFO]: Project Root detected at: " + file.getFullPathName());
	return result;
}
GLint Object::getUniformLoc(const juce::String& uniform_name, bool in_compute_shader)
{
	CRYSTAL_CHECK(!(getState() >= Entity_Initialised),
		"Object::getUniformLoc() can ONLY be called after Entity_Initialised", -1);
	auto& glfunc = gl_context.extensions;
	GLint	location = 0;
	GLuint	target_prog = in_compute_shader ? compute_program_id : render_program_id;
	auto& target_cache = in_compute_shader ? compute_uniform_locations : render_uniform_locations;

	CRYSTAL_CHECK(target_prog == 0, "Unable to locate uniform \"" + uniform_name + "\"", -1);

	auto it = target_cache.find(uniform_name);
	if (it != target_cache.end()) return it->second;

	GLint loc = glfunc.glGetUniformLocation(target_prog, uniform_name.toRawUTF8());
	target_cache[uniform_name] = loc;

	return loc;
}
GLuint Object::genComputeProg(const juce::String src) const
{
	CRYSTAL_CHECK(getState() != Entity_Initialising,
		"Object::genComputeProg() can ONLY be called when Entity_Initialising", 0);
	CRYSTAL_CHECK(src.isEmpty(), "Invalid src path", 0);

	auto& glfunc = gl_context.extensions;

	GLint success = 0;
	char msg[1024];

	GLuint shader = glfunc.glCreateShader(GL_COMPUTE_SHADER);
	const char* shader_ptr = src.toRawUTF8();
	glfunc.glShaderSource(shader, 1, &shader_ptr, nullptr);
	glfunc.glCompileShader(shader);

	glfunc.glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glfunc.glGetShaderInfoLog(shader, 512, NULL, msg);
		juce::Logger::writeToLog("Compute shader compile error:" + juce::String(msg));
		return 0;
	}

	GLuint prog = glCreateProgram();
	glfunc.glAttachShader(prog, shader);
	glfunc.glLinkProgram(prog);

	glfunc.glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success)
	{
		glfunc.glGetProgramInfoLog(prog, 1024, NULL, msg);
		juce::Logger::writeToLog("[ERROR] Compute shader program link error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}
	glfunc.glDeleteShader(shader);
	return prog;
}
GLuint Object::genComputeProgfromFile(const juce::String path) const
{
	CRYSTAL_CHECK(getState() != Entity_Initialising,
		"Object::genComputeProgfromFile() can ONLY be called when Entity_Initialising", 0);
	juce::File shaderFile = path;
	CRYSTAL_CHECK(!shaderFile.existsAsFile(), "Compute shader source file path don't exist", 0);
	return genComputeProg(shaderFile.loadFileAsString());
}
GLuint Object::genRenderProg(const juce::String vsrc, const juce::String fsrc) const
{
	CRYSTAL_CHECK(getState() != Entity_Initialising,
		"Object::genRenderProg() can ONLY be called when Entity_Initialising", 0);
	CRYSTAL_CHECK(vsrc.isEmpty() || fsrc.isEmpty(), "Source code could not be empty", 0);
	auto& glfunc = gl_context.extensions;

	GLint success = 0;
	char msg[1024];

	GLuint vshader = glfunc.glCreateShader(GL_VERTEX_SHADER);
	const char* shader_ptr = vsrc.toRawUTF8();
	glfunc.glShaderSource(vshader, 1, &shader_ptr, nullptr);
	glfunc.glCompileShader(vshader);

	glfunc.glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glfunc.glGetShaderInfoLog(vshader, 1024, NULL, msg);
		juce::Logger::writeToLog("[ERROR] Vertex shader compile error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}

	GLuint fshader = glfunc.glCreateShader(GL_FRAGMENT_SHADER);
	const char* fhader_ptr = fsrc.toRawUTF8();
	glfunc.glShaderSource(fshader, 1, &fhader_ptr, nullptr);
	glfunc.glCompileShader(fshader);

	glfunc.glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glfunc.glGetShaderInfoLog(fshader, 1024, NULL, msg);
		DBG("[ERROR] Fragment shader compile error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}

	GLuint prog = glfunc.glCreateProgram();
	glfunc.glAttachShader(prog, vshader);
	glfunc.glAttachShader(prog, fshader);
	glfunc.glLinkProgram(prog);

	glfunc.glGetProgramiv(prog, GL_LINK_STATUS, &success);
	if (!success)
	{
		glfunc.glGetProgramInfoLog(prog, 512, NULL, msg);
		juce::Logger::writeToLog("[ERROR] Render shader program link error:" + juce::String(msg));
		jassertfalse;
		return 0;
	}

	glfunc.glDeleteShader(vshader);
	glfunc.glDeleteShader(fshader);
	return prog;
}
GLuint Object::genRenderProgfromFile(const juce::String vpath, const juce::String fpath) const
{
	CRYSTAL_CHECK(getState() != Entity_Initialising,
		"Object::genRenderProgfromFile() can ONLY be called when Entity_Initialising", 0);
	juce::File vshaderFile = vpath;
	juce::File fshaderFile = fpath;
	CRYSTAL_CHECK(!vshaderFile.existsAsFile() || !fshaderFile.existsAsFile(), "Shader source file path don't exist", 0);
	return genRenderProg(vshaderFile.loadFileAsString(), fshaderFile.loadFileAsString());
}
//==============================================================================