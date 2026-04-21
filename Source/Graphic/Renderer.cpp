//==============================================================================
#include "Renderer.h"
#include "Object.h"
#include "Camera.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
/* TODO: add them in a struct */
constexpr uint8_t DEFAULT		= 0b0000'0001;
constexpr uint8_t REGISTERD		= 0b0000'0010;
constexpr uint8_t VISIBLE		= 0b0000'0110;
//==============================================================================
Renderer::Renderer(juce::OpenGLContext& context) : gl_context(context), render_slots(16), access_table(16)
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
				render_slots[idx].object->render(global_VP, camera_pos);
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
			auto history = render_slots[i].history;
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
Object_Handle Renderer::registerObject(std::unique_ptr<Object> object, GLuint target_slot)
{
	const juce::ScopedLock open(request_lock);

	render_slots[target_slot].history++;
	GLuint current_version = render_slots[target_slot].history;

	object->setHandle({ target_slot, current_version });
	register_queue.push({ target_slot, std::move(object) });
	return { target_slot, current_version };
}
void Renderer::setRenderSlotState(const Object_Handle& handle, const uint8_t state)
{
	GLuint target_slot = handle.index;
	CRYSTAL_CHECK(target_slot >= render_slots.size(), "Invalid param");
	CRYSTAL_CHECK(!isHandleValid(handle), "Invalid handle");
	{
		const juce::ScopedLock open(request_lock);
		set_state_queue.push({ handle.index, state });
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
		render_slots[handle.index].history++;
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
size_t Renderer::getTotalAllocatedSize() const
{
	size_t total_size = 0;
	for (size_t i = 0; i < render_slots.size(); i++)
		total_size += render_slots[i].object->getAllocatedSize();

	return total_size;
}
GLboolean Renderer::isHandleValid(const Object_Handle& handle)
{
	const juce::ScopedLock open(request_lock);
	if (handle.index >= render_slots.size()) 
	{
		DBG("[ERROR] Invalid index, array out of bound");
		jassertfalse;
		return GL_FALSE;
	}
	return (handle.history == render_slots[handle.index].history) ? GL_TRUE : GL_FALSE;
}
void Renderer::processRequests()
{
	const juce::ScopedLock open(request_lock);
	while (!remove_queue.empty())
	{
		GLuint target_slot = remove_queue.front();
		*(reinterpret_cast<uint8_t*>(&access_table[target_slot])) = DEFAULT;
		render_slots[target_slot].object.reset();
		remove_queue.pop();
	}
	while (!register_queue.empty())
	{
		auto& request = register_queue.front();
		GLuint target_slot = request.first;
		*(reinterpret_cast<uint8_t*>(&access_table[target_slot])) = REGISTERD;
		render_slots[target_slot].object = std::move(request.second);
		render_slots[target_slot].object->initialise();
		register_queue.pop();
	}
	while (!set_state_queue.empty())
	{
		auto& request = set_state_queue.front();
		*(reinterpret_cast<uint8_t*>(&access_table[request.first]))
			= request.second;
		set_state_queue.pop();
	}
}
//==============================================================================