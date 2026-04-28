//==============================================================================
#include "Renderer.h"
#include "Camera.h"
#include "../Foundation.h"
#include "../Utilities.h"
#include <glm-master/glm/gtc/type_ptr.hpp>
//==============================================================================
using namespace Crystal;
Renderer::Renderer(juce::OpenGLContext& context) 
	: gl_context(context), render_slots(RENDER_SLOT_SIZE), Processor(Timer::getTimer())
{
	addAndMakeVisible(debug_info);
	debug_info.setMultiLine(true); debug_info.setReadOnly(true); debug_info.setAlpha(0.3);

	main_camera = std::make_unique<Camera>();
	DBG("[INFO] Renderer constructed");
}
Renderer::~Renderer()
{
	shutdown();
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
bool Crystal::Renderer::prepare()
{
	return true;
}
void Crystal::Renderer::processing()
{
}
void Crystal::Renderer::synchronize()
{
}
void Crystal::Renderer::shutdown()
{
	for (GLuint i = 0; i < RENDER_SLOT_SIZE; i++)
	{
		auto& object = render_slots[i].object;
		if (object) object.reset();
	}
}
void Renderer::registerObject(Object_Handle& handle, std::unique_ptr<Object> object)
{
	CRYSTAL_CHECK(handle.index >= render_slots.size(), "target_slot cannot exceed slot size");
	const juce::ScopedLock open(request_lock);
	handle.history = ++render_slots[handle.index].history;
	register_queue.emplace(handle, std::move(object));
}
void Renderer::removeObject(Object_Handle& handle)
{
	CRYSTAL_CHECK(!verifyObjectHandle(handle.index, handle), "Invalid or Stale handle");
	const juce::ScopedLock open(request_lock);
	GLuint new_history = ++render_slots[handle.index].history;
	handle.history = new_history;
	remove_queue.emplace(handle);
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
		if (render_slots[i].object)
			if (render_slots[i].object->getState() == Entity_Initialised)
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