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
Object::Object(juce::OpenGLContext& context) : gl_context(context)
{
}
void Object::cleanup()
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
void Object::setHandle(const Object_Handle& handle)
{
	object_handle = handle;
}
Object_Handle Object::getHandle() const
{
	return object_handle;
}
//==============================================================================
/* class Object */
/* TODO: this function should be implemented later after file output reconstructing */
juce::File Object::getShaderFile(const juce::String& file_name) const
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
GLint Object::getUniformLoc(const juce::String& uniform_name, bool in_compute_shader)
{
	auto& glfunc = gl_context.extensions;

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
		juce::Logger::writeToLog("[ERROR] Compute shader source file path don't exist");
		jassertfalse;
		return 0;
	}
	return genComputeProg(shaderFile.loadFileAsString());
}
GLuint Object::genRenderProg(const juce::String vsrc, const juce::String fsrc) const
{
	if (vsrc.isEmpty() || fsrc.isEmpty())
	{
		juce::Logger::writeToLog("Source code could not be empty!");
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
void Object::loadShaderProg(const juce::String v_shader_name, const juce::String f_shader_name,
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
Renderer::Renderer(juce::OpenGLContext& context) : gl_context(context), active_slots(16), access_table(16)
{
	startTimer(100);
	
	addAndMakeVisible(debug_info);
	debug_info.setMultiLine(true); debug_info.setReadOnly(true); debug_info.setAlpha(0.3);

	main_camera = std::make_unique<Camera>();
	DBG("[INFO] Renderer constructed");
}
Renderer::~Renderer()
{
}
juce::OpenGLContext& Renderer::getglContext()
{
	return gl_context;
}
void Renderer::newOpenGLContextCreated()
{
}
void Renderer::renderOpenGL()
{
	processRequests();
	updateTime();

	if (main_camera == nullptr) return;
	main_camera->update(dt, getMouseXYRelative());

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 global_VP = main_camera->getGlobalVP();
	glm::vec3 camera_pos = main_camera->getCameraPos();
	for (GLuint idx = 0; idx < access_table.size(); idx++)
		if (!access_table[idx].isempty)
			if (access_table[idx].isready && access_table[idx].isvisible)
				active_slots[idx].object->render(global_VP, camera_pos);
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
	main_camera->onWindowResize(getScreenBounds());
	debug_info.setBoundsRelative(0.0, 0.0, 0.5, 0.4);
}
void Renderer::timerCallback()
{
	juce::String wall_time_text = juce::Time::getCurrentTime().toString(false, true, true, true);
	juce::String mouse_pos_text = juce::String(main_camera->getMouseNDCPos().x,2) 
		+ "," + juce::String(main_camera->getMouseNDCPos().y, 2);
	juce::String window_size_text = juce::String(main_camera->getWindowSize().getX())
		+ "," + juce::String(main_camera->getWindowSize().getY());
	juce::String camera_pos_text = juce::String(main_camera->getCameraPos().x,2)
		+ "," + juce::String(main_camera->getCameraPos().y, 2)
		+ "," + juce::String(main_camera->getCameraPos().z, 2);

	juce::String slots_state;
	slots_state << "--- Slot(0-3) States ---\n";
	{
		for (int i = 0; i < 4; ++i)
		{
			auto history = active_slots[i].history;
			auto flags = access_table[i];
			slots_state << "Slot " << i << ": History = " << (int)history << " [";
			slots_state << (flags.isempty	? "E" : "U");
			slots_state << (flags.isready	? "R" : "N");
			slots_state << (flags.isvisible ? "V" : "H");
			slots_state << "] ";
			slots_state << "\n";
		}
	}

	juce::String debug_text;
	debug_text << "--- System Info ---" << "\n"
		<< "Wall Time: " << wall_time_text << "\n"
		<< "Delta Time: " << juce::String(dt, 3) << "s" << "\n"
		<< "Mouse Position: " << "(" << mouse_pos_text << ")" << "\n"
		<< "Window Size : " << "(" << window_size_text << ")" << "\n"
		<< "Camera Position: " << "(" << camera_pos_text << ")" << "\n"
		<< slots_state;

	debug_info.setText(debug_text, juce::dontSendNotification);
}
/* TODO: here exist an overwrap problem, though it isn't an urgent, better finds out a way to fix this */
Object_Handle Renderer::registerObject(std::unique_ptr<Object> object, GLuint target_slot)
{
	const juce::ScopedLock open(request_lock);

	active_slots[target_slot].history++;
	GLuint current_version = active_slots[target_slot].history;

	object->setHandle({ target_slot, current_version });
	register_queue.push({ target_slot, std::move(object) });
	return { target_slot, current_version };
}
/* TODO: complete the logic here */
void Renderer::makeObjectVisible(const Object_Handle& handle)
{
	if (!isHandleValid(handle)) 
	{
		DBG("[ERROR] Invalid handle");
		jassertfalse;
		return;
	}
	Slot_Flags target_flag;
	target_flag.isempty = false;
	target_flag.isready = true;
	target_flag.isvisible = true;
	{
		const juce::ScopedLock open(request_lock);
		set_state_queue.push({ handle.index, target_flag });
	}
}
void Renderer::makeObjectHidden(const Object_Handle& handle)
{
	if (!isHandleValid(handle))
	{
		DBG("[ERROR] Invalid handle");
		jassertfalse;
		return;
	}
	Slot_Flags target_flag;
	target_flag.isempty = false;
	target_flag.isready = true;
	target_flag.isvisible = false;
	{
		const juce::ScopedLock open(request_lock);
		set_state_queue.push({ handle.index, target_flag });
	}
}
void Renderer::removeObject(const Object_Handle& handle)
{
	if (!isHandleValid(handle))
	{
		DBG("[ERROR] Invalid handle");
		jassertfalse;
		return;
	}
	{
		const juce::ScopedLock open(request_lock);
		active_slots[handle.index].history++;
		remove_queue.push(handle.index);
	}
}
void Renderer::updateTime()
{
	GLdouble current_time = juce::Time::getMillisecondCounter() * 0.001;
	dt = (current_time - timer);
	timer = current_time;

	if (dt > 0.1) dt = 0.1;
}
GLboolean Renderer::isHandleValid(const Object_Handle& handle)
{
	const juce::ScopedLock open(request_lock);
	if (handle.index >= active_slots.size()) 
	{
		DBG("[ERROR] Invalid index, array out of bound");
		jassertfalse;
		return GL_FALSE;
	}
	return (handle.history == active_slots[handle.index].history) ? GL_TRUE : GL_FALSE;
}
/* TODO: add more check here */
void Renderer::processRequests()
{
	const juce::ScopedLock open(request_lock);
	while (!remove_queue.empty())
	{
		GLuint idx = remove_queue.front();
		access_table[idx].isempty = true;
		access_table[idx].isready = false;
		active_slots[idx].object.reset();
		remove_queue.pop();
	}
	while (!register_queue.empty()) 
	{
		auto& request = register_queue.front();
		GLuint idx = request.first;

		active_slots[idx].object = std::move(request.second);

		active_slots[idx].object->initialise();

		access_table[idx].isempty = false;
		access_table[idx].isready = true;
		register_queue.pop();
	}
	while (!set_state_queue.empty())
	{
		auto& request = set_state_queue.front();
		GLuint idx = request.first;
		access_table[idx] = request.second;
		set_state_queue.pop();
	}
}
//==============================================================================