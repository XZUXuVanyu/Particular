//==============================================================================
#include "Object.h"
#include "../Utilities.h"
//==============================================================================
Object::Object(juce::OpenGLContext& context, const std::vector<juce::String>& shader_src)
	: gl_context(context), vert_shader_name(shader_src[0]), frag_shader_name(shader_src[1]), compute_shader_name(shader_src[2])
{
	CRYSTAL_CHECK(shader_src.size() != 3, "shader_src MUST contain 3 names");
	current_state.store(Object_State::Constructed);
}
void Object::baseInitialise()
{
	CRYSTAL_CHECK(current_state.load() != Object_State::Constructed, "Failed to construct Object");
	current_state.store(Object_State::Initialising);
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
	current_state.store(Object_State::Ready);
}
void Object::baseRender(const glm::mat4& global_VP, const glm::vec3& camera_pos)
{
	CRYSTAL_CHECK(current_state.load() != Object_State::Ready,
		"Object::baseRender() can ONLY be called with Object_State::Ready");
	current_state.store(Object_State::Rendering);
	auto& glfunc = gl_context.extensions;
	glfunc.glUseProgram(render_program_id);
	glfunc.glBindVertexArray(vao_id);

	childRender(global_VP, camera_pos);

	glfunc.glBindVertexArray(0);
	glfunc.glUseProgram(0);
	current_state.store(Object_State::Ready);
}
void Object::baseCleanup()
{
	CRYSTAL_CHECK(current_state.load() != Object_State::Ready,
		"Object::baseCleanup() can ONLY be called with Object_State::Ready");
	current_state.store(Object_State::Deconstructing);
	childCleanup();

	auto& glfunc = gl_context.extensions;
	if (render_program_id)	glfunc.glDeleteProgram(render_program_id);
	if (compute_program_id) glfunc.glDeleteProgram(compute_program_id);
	if (vao_id) glfunc.glDeleteVertexArrays(1, &vao_id);
	if (ebo_id) glfunc.glDeleteBuffers(1, &ebo_id);
	if (vbo_id) glfunc.glDeleteBuffers(1, &vbo_id);
	current_state.store(Object_State::Null);
}
Object_State Object::getCurrentState() const
{
	return current_state.load();
}
Object_Handle Object::getHandle() const
{
	Object_State state = current_state.load();
	CRYSTAL_CHECK(state != Object_State::Ready && state != Object_State::Rendering,
		"Object::getHandle() can ONLY be called with Object_State::Ready", Object_Handle());
	return object_handle;
}
GLuint Object::getRenderProgID() const
{
	Object_State state = current_state.load();
	CRYSTAL_CHECK(state != Object_State::Ready && state != Object_State::Rendering,
		"Object::getRenderProgID() can ONLY be called in Ready or Rendering states", 0);
	return render_program_id;
}
GLuint Object::getComputeProgID() const
{
	Object_State state = current_state.load();
	CRYSTAL_CHECK(state != Object_State::Ready && state != Object_State::Rendering,
		"Object::getComputeProgID() can ONLY be called in Ready or Rendering states", 0);
	return compute_program_id;
}
GLuint Object::getVAOID() const
{
	Object_State state = current_state.load();
	CRYSTAL_CHECK(state != Object_State::Ready && state != Object_State::Rendering,
		"Object::getVAOID() can ONLY be called in Ready or Rendering states", 0);
	return vao_id;
}
GLuint Object::getVBOID() const
{
	Object_State state = current_state.load();
	CRYSTAL_CHECK(state != Object_State::Ready && state != Object_State::Rendering,
		"Object::getVAOID() can ONLY be called in Ready or Rendering states", 0);
	return vbo_id;
}
/* TODO: add this implementaion */
size_t Object::getAllocatedSize() const
{
	Object_State state = current_state.load();
	CRYSTAL_CHECK(state != Object_State::Ready && state != Object_State::Rendering,
		"Object::getAllocatedSize() can ONLY be called in Ready or Rendering states", 0);
	size_t total_size = 0;
	return total_size;
}
juce::OpenGLContext& Object::getGLContext() const
{
	return gl_context;
}
juce::File Object::getShaderFile(const juce::String& file_name) const
{
	CRYSTAL_CHECK(current_state.load() != Object_State::Initialising,
		"Object::getShaderFile() can ONLY be called during Initialising state", juce::File());

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
	Object_State state = current_state.load();
	CRYSTAL_CHECK(state != Object_State::Initialising && state != Object_State::Ready && state != Object_State::Rendering,
		"Object::getUniformLoc() can ONLY be called during Initialising, Ready, or Rendering states", -1);
	auto& glfunc = gl_context.extensions;
	GLint	location = 0;
	GLuint	target_prog = in_compute_shader ? compute_program_id : render_program_id;
	auto& target_cache = in_compute_shader ? compute_uniform_locations : render_uniform_locations;

	CRYSTAL_CHECK(target_prog == 0, "[Warning] Unable to locate uniform \"" + uniform_name + "\"", -1);

	auto it = target_cache.find(uniform_name);
	if (it != target_cache.end()) return it->second;

	GLint loc = glfunc.glGetUniformLocation(target_prog, uniform_name.toRawUTF8());
	target_cache[uniform_name] = loc;

	return loc;
}
GLuint Object::genComputeProg(const juce::String src) const
{
	CRYSTAL_CHECK(current_state.load() != Object_State::Initialising,
		"Object::genComputeProg() can ONLY be called during Initialising state", 0);
	if (src.isEmpty())
	{
		juce::Logger::writeToLog("[ERROR] Source code could not be empty");
		jassertfalse;
		return 0;
	}

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
	CRYSTAL_CHECK(current_state.load() != Object_State::Initialising,
		"Object::genComputeProgfromFile() can ONLY be called during Initialising state", 0);
	juce::File shaderFile = path;
	if (!shaderFile.existsAsFile())
	{
		DBG("[ERROR] Compute shader source file path don't exist");
		jassertfalse;
		return 0;
	}
	return genComputeProg(shaderFile.loadFileAsString());
}
GLuint Object::genRenderProg(const juce::String vsrc, const juce::String fsrc) const
{
	CRYSTAL_CHECK(current_state.load() != Object_State::Initialising,
		"Object::genRenderProg() can ONLY be called during Initialising state", 0);
	if (vsrc.isEmpty() || fsrc.isEmpty())
	{
		DBG("[ERROR] Source code could not be empty");
		jassertfalse;
		return 0;
	}

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
	CRYSTAL_CHECK(current_state.load() != Object_State::Initialising,
		"Object::genRenderProgfromFile() can ONLY be called during Initialising state", 0);
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
//==============================================================================