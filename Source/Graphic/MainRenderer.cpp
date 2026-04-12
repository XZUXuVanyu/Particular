//==============================================================================
#include "MainRenderer.h"
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
MainRenderer::MainRenderer(juce::OpenGLContext& context) : gl_context(context)
{
	mesh = std::make_unique<GlobalMesh>(10, 0);
	setWantsKeyboardFocus(true);

	addAndMakeVisible(debug_info);
	debug_info.setMultiLine(true);
	debug_info.setReadOnly(true);
	debug_info.setAlpha(0.6);

	DBG("[INFO] MainRenderer constructed");
}
MainRenderer::~MainRenderer()
{
}
void MainRenderer::newOpenGLContextCreated()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	mesh.get()->initialise();
}
void MainRenderer::renderOpenGL()
{
	glClearColor(0.2f, 0.2f, 0.2f, 0.2f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	global_VP = projection_mat * view_mat;

	if (mesh != nullptr) mesh.get()->render(global_VP, camera_pos);
}
void MainRenderer::openGLContextClosing()
{
}
void MainRenderer::paint(juce::Graphics& g)
{
}
void MainRenderer::resized()
{
	sensitivity_x = juce::MathConstants<GLfloat>::pi / getWidth();
	sensitivity_y = juce::MathConstants<GLfloat>::pi / getHeight();
	update_Pmat();

	debug_info.setBoundsRelative(0.0, 0.0, 0.4, 0.3);
}
bool MainRenderer::keyPressed(const juce::KeyPress& key)
{
	float step = 0.2f;

	if		(key.getKeyCode() == 'W')
	{
		camera_pos += camera_f.v * step;
	}
	else if (key.getKeyCode() == 'S')
	{
		camera_pos -= camera_f.v * step;
	}
	else if (key.getKeyCode() == 'A')
	{
		camera_pos -= camera_r.v * step;
	}
	else if (key.getKeyCode() == 'D')
	{
		camera_pos += camera_r.v * step;
	}
	else if (key.getKeyCode() == 'Q')
	{
		camera_pos.z -= step;
	}
	else if (key.getKeyCode() == 'E')
	{
		camera_pos.z += step;
	}
	else if (key.getKeyCode() == '-') fov *= 1.05;
	else if (key.getKeyCode() == '=') fov /= 1.05;

	/* Make graph updated */
	update_Vmat();
	update_Pmat();
	repaint();
	gl_context.triggerRepaint();
	return true;
}
void MainRenderer::mouseDown(const juce::MouseEvent& event)
{
	grabKeyboardFocus();
	last_mouse_pos = event.getMouseDownPosition();
}
void MainRenderer::mouseDrag(const juce::MouseEvent& event)
{
	auto current_pos = event.getPosition();
	GLfloat delta_x = current_pos.x - last_mouse_pos.x;
	GLfloat delta_y = current_pos.y - last_mouse_pos.y;
	last_mouse_pos = current_pos;

	float lr_angle = -delta_x * sensitivity_x;
	float ud_angle = -delta_y * sensitivity_y;

	camera_rotation = Quaternion::gen_rotater(lr_angle, glm::vec3{ 0,0,1 }) * camera_rotation;
	camera_rotation = camera_rotation * Quaternion::gen_rotater(ud_angle, glm::vec3{ 1,0,0 });
	Quaternion::normalize(camera_rotation);

	Quaternion inv_q{ camera_rotation.get_conjugate() };
	camera_r = (camera_rotation * Quaternion{ 0.0, glm::vec3{1,0,0} } * inv_q);
	camera_f = (camera_rotation * Quaternion{ 0.0, glm::vec3{0,1,0} } * inv_q);
	camera_u = (camera_rotation * Quaternion{ 0.0, glm::vec3{0,0,1} } * inv_q);
	update_Vmat();
	repaint();
	gl_context.triggerRepaint();
}
void MainRenderer::update_Vmat()
{
	glm::mat4 rotation_mat = glm::mat4(
		glm::vec4(camera_r.v.x, camera_u.v.x, -camera_f.v.x, 0.0f),
		glm::vec4(camera_r.v.y, camera_u.v.y, -camera_f.v.y, 0.0f),
		glm::vec4(camera_r.v.z, camera_u.v.z, -camera_f.v.z, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)                          
	);

	glm::mat4 traslation_mat = glm::mat4(
		glm::vec4(1, 0, 0, 0),
		glm::vec4(0, 1, 0, 0),
		glm::vec4(0, 0, 1, 0),
		glm::vec4(-camera_pos.x, -camera_pos.y, -camera_pos.z, 1)
	);
	view_mat = rotation_mat * traslation_mat;

	std::stringstream text;
	text << "Camera R " << camera_r << "\n";
	text << "Camera F " << camera_f << "\n";
	text << "Camera U " << camera_u;

	juce::MessageManagerLock mmLock;
	debug_info.setText(text.str());

}
void MainRenderer::update_Pmat()
{
	aspect = ((float)getWidth() / (float)getHeight());
	projection_mat = glm::perspective(fov, aspect, dnear, dfar);
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