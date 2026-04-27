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
Renderer::Renderer(juce::OpenGLContext& context) : gl_context(context), render_slots(RENDER_SLOT_SIZE)
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
/* Set global rendering options here */
void Renderer::newOpenGLContextCreated()
{
	
}
void Renderer::renderOpenGL()
{
	if (!renderPrepare()) return;
	renderScene();
	renderOverlay();
	renderCleanup();
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

	/* TODO: add debug text here */
	juce::String slots_state;
	slots_state << "--- Slot(0-3) States ---\n";
	{
		for (int i = 0; i < 4; ++i)
		{
			
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
void Renderer::registerObject(Object_Handle& handle, std::unique_ptr<Object> object)
{
	CRYSTAL_CHECK(handle.index >= render_slots.size(), "target_slot cannot exceed slot size");
	CRYSTAL_CHECK(render_slots[handle.index].getObjectState() != Object_State::Null,
		"Target slot is already occupied, call Renderer::removeObject() first.");

	const juce::ScopedLock open(request_lock);
	handle.history = ++render_slots[handle.index].history;
	register_queue.emplace(handle, std::move(object));
}
void Renderer::removeObject(const Object_Handle& handle)
{
	auto current_state = render_slots[handle.index].getObjectState();
	CRYSTAL_CHECK(!verifyObjectHandle(handle.index, handle), "Invalid or Stale handle");
	CRYSTAL_CHECK(current_state == Object_State::Null || current_state == Object_State::Deconstructing,
		"Object is already removed or being deconstructed");

	const juce::ScopedLock open(request_lock);
	GLuint new_history = ++render_slots[handle.index].history;
	Object_Handle internal_handle{ handle.index, new_history };
	remove_queue.emplace(internal_handle);
}
bool Renderer::renderPrepare()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	processRequests();

	updateTime();
	if (!main_camera) return false;
	else 
	{
		main_camera->update(dt, juce::Desktop::getInstance().getMousePosition());
		return true;
	}
}
void Renderer::renderScene()
{
	
	glm::mat4 globalVP		= main_camera->getGlobalVP();
	glm::vec3 camera_pos	= main_camera->getCameraPos();

	for (GLuint i = 0; i < RENDER_SLOT_SIZE; i++)
		if (render_slots[i].getObjectState() == Object_State::Ready)
			render_slots[i].object->baseRender(globalVP, camera_pos);
}
void Renderer::renderOverlay()
{
}
void Renderer::renderCleanup()
{
}
void Renderer::updateTime()
{
	GLdouble current_time = juce::Time::getMillisecondCounter() * 0.001;
	dt = (current_time - timer);
	timer = current_time;

	if (dt > 0.1) dt = 0.1;
}
void Renderer::processRequests() 
{
	while (!register_queue.empty()) 
	{
		auto& request = register_queue.front();
		auto& slot = render_slots[request.first.index];
		if (request.first.history == slot.history.load()) 
		{
			slot.object = std::move(request.second);
			slot.object->baseInitialise();
		}
		register_queue.pop();
	}

	while (!remove_queue.empty()) 
	{
		auto& request = remove_queue.front();
		auto& slot = render_slots[request.index];

		if (request.history == slot.history.load())
		{
			if (slot.object != nullptr) 
			{
				slot.object->baseCleanup();
				slot.object.reset();
			}
		}
		remove_queue.pop();
	}
}
bool Renderer::verifyObjectHandle(GLuint target_slot, const Object_Handle& handle) const
{
	return (render_slots[target_slot].history.load() == handle.history) && (target_slot == handle.index);
}
size_t Renderer::getTotalAllocatedSize() const
{
	size_t total_size = 0;
	for (size_t i = 0; i < render_slots.size(); i++)
		total_size += render_slots[i].object->getAllocatedSize();

	return total_size;
}
//==============================================================================