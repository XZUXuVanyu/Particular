//==============================================================================
#include "Object.h"
#include "../Utilities.h"
//==============================================================================
constexpr uint8_t EMPTY_BIT		= 0b0000'0001;
constexpr uint8_t BINDING_BIT	= 0b0000'0010;
constexpr uint8_t ALLOCATE_BIT	= 0b0000'0100;
constexpr uint8_t DYNAMIC_BIT	= 0b0000'1000;
//==============================================================================
Object::Object(juce::OpenGLContext& context) 
	: gl_context(context), vbo_slots(4), access_table(4)
{
}
void Object::cleanup()
{
	auto& glfunc = gl_context.extensions;
	if (render_program_id)	glfunc.glDeleteProgram(render_program_id);
	if (compute_program_id) glfunc.glDeleteProgram(compute_program_id);
	if (vao_id) glfunc.glDeleteVertexArrays(1, &vao_id);
	if (ebo_id) glfunc.glDeleteBuffers(1, &ebo_id);
	for (GLuint i = 0; i < 4; i++)
		if (!access_table[i].is_empty)
			glfunc.glDeleteBuffers(1, &vbo_slots[i].vbo_id);
}
void Object::setHandle(const Object_Handle& handle)
{
	object_handle = handle;
}
Object_Handle Object::getHandle() const
{
	return object_handle;
}
GLuint Object::getRenderProgID() const
{
	return render_program_id;
}
GLuint Object::getComputeProgID() const
{
	return compute_program_id;
}
GLuint Object::getVAOID() const
{
	return vao_id;
}
size_t Object::getAllocatedSize() const
{
	size_t total_size = 0;
	for (size_t i = 0; i < vbo_slots.size(); i++)
		total_size += vbo_slots[i].allocated_size;

	return total_size;
}
juce::File Object::getShaderFile(const juce::String& file_name) const
{
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
void Object::loadShaderProg(const juce::String v_shader_name,
	const juce::String f_shader_name, const juce::String c_shader_name, bool with_compute_shader)
{
	if (with_compute_shader)
	{
		auto c_file = getShaderFile(c_shader_name);
		compute_program_id = genComputeProgfromFile(c_file.getFullPathName());
		if (compute_program_id == 0)
		{
			DBG("[ERROR] Failed to load program: " + c_shader_name);
			jassertfalse;
			return;
		}
	}

	auto v_file = getShaderFile(v_shader_name);
	auto f_file = getShaderFile(f_shader_name);
	render_program_id = genRenderProgfromFile(v_file.getFullPathName(), f_file.getFullPathName());
	if (render_program_id == 0)
	{
		DBG("[ERROR] Failed to load program: " + v_shader_name + " / " + f_shader_name);
		jassertfalse;
		return;
	}
}
void Object::genAndbindVAO()
{
	auto& glfunc = gl_context.extensions;
	glfunc.glGenVertexArrays(1, &vao_id);
	glfunc.glBindVertexArray(vao_id);
}
void Object::genAndbindVBO(GLuint target_slot, bool is_dynamic)
{
	CRYSTAL_CHECK(target_slot >= vbo_slots.size(), "Invalid slot index or empty layout");
	auto& glfunc = gl_context.extensions;
	auto& slot = vbo_slots[target_slot];

	if (slot.vbo_id != 0)
		glfunc.glDeleteBuffers(1, &slot.vbo_id);

	glfunc.glGenBuffers(1, &slot.vbo_id);
	glfunc.glBindBuffer(GL_ARRAY_BUFFER, slot.vbo_id);

	setVBOSlotState(target_slot, EMPTY_BIT, false);
	setVBOSlotState(target_slot, DYNAMIC_BIT, is_dynamic);
	setVBOSlotState(target_slot, BINDING_BIT, true);
}
/* TODO: optimize this */
void Object::setVBOSlotState(GLuint target_slot, const uint8_t mask, bool set)
{
	CRYSTAL_CHECK(target_slot >= vbo_slots.size(), "Invalid slot index");
	uint8_t& state = *reinterpret_cast<uint8_t*>(&access_table[target_slot]);
	if (set) state |= mask;
	else state &= ~mask;
}
void Object::allocateVBO(GLuint target_slot, size_t size)
{
	CRYSTAL_CHECK(target_slot >= vbo_slots.size(), "Invalid slot index");
	auto& slot = vbo_slots[target_slot];
	auto& glfunc = gl_context.extensions;
	uint8_t& state = *reinterpret_cast<uint8_t*>(&access_table[target_slot]);
	CRYSTAL_CHECK(state & EMPTY_BIT, "Specified VBO Slot is empty, call genAndbindVBO() first");
	if (!(state & BINDING_BIT))
	{
		glfunc.glBindBuffer(GL_ARRAY_BUFFER, slot.vbo_id);
		state |= BINDING_BIT;
	}
	glfunc.glBufferData(GL_ARRAY_BUFFER, size, nullptr, 
		(state & DYNAMIC_BIT) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
	slot.allocated_size = size;
	state |= ALLOCATE_BIT;
}
void Object::updateVBO(GLuint target_slot, const void* data_ptr, size_t size)
{
	CRYSTAL_CHECK(target_slot >= vbo_slots.size(), "Invalid slot index");
	auto& slot = vbo_slots[target_slot];
	auto& glfunc = gl_context.extensions;

	uint8_t& state = *reinterpret_cast<uint8_t*>(&access_table[target_slot]);
	CRYSTAL_CHECK(state & EMPTY_BIT, "Specified VBO Slot is empty, call genAndbindVBO() first");
	if (!(state & ALLOCATE_BIT) || size > slot.allocated_size)
	{
		allocateVBO(target_slot, size);
		state |= ALLOCATE_BIT;
	}
	glfunc.glBufferSubData(GL_ARRAY_BUFFER, 0, size, data_ptr);
}
void Object::setObjectVertAttrib(GLuint target_slot, const std::vector<GL_Vertex_Attrib> layout)
{
	if (target_slot >= vbo_slots.size() || layout.empty()) {
		DBG("[ERROR] Invalid slot index or empty layout");
		jassertfalse;
		return;
	}

	auto& slot = vbo_slots[target_slot];
	auto& glfunc = gl_context.extensions;
	
	uint8_t state = *reinterpret_cast<uint8_t*>(&access_table[target_slot]);
	if (state & EMPTY_BIT)
	{
		DBG("[ERROR] Specified VBO Slot is empty, call genAndbindVBO() first");
		jassertfalse;
		return;
	}

	glfunc.glBindVertexArray(vao_id);
	glfunc.glBindBuffer(GL_ARRAY_BUFFER, slot.vbo_id);
	for (const auto& attrib : layout)
	{
		glfunc.glEnableVertexAttribArray(attrib.location);
		glfunc.glVertexAttribPointer(
			attrib.location,
			attrib.size,
			attrib.type,
			attrib.normalized,
			attrib.stride,
			(const GLvoid*)(size_t)attrib.offset
		);
	}

	glfunc.glBindBuffer(GL_ARRAY_BUFFER, 0);
	setVBOSlotState(target_slot, BINDING_BIT, false);
}
//==============================================================================