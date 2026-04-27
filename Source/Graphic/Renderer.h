//==============================================================================
/* This program is dedicated in rendering particles. */
//==============================================================================
#pragma once
#include <JuceHeader.h>
#include "../Utilities.h"
#include "Object.h"
//==============================================================================
constexpr GLuint RENDER_SLOT_SIZE = 16;
//==============================================================================
using namespace juce::gl;
//==============================================================================
class Camera;
enum class Renderer_State
{
	Idle, Preparing, Rendering, Cleaning
};
struct Render_Slot
{
	std::unique_ptr<Object>		object;
	std::atomic<GLuint>			history;

	Object_State getObjectState() const
	{
		if (object == nullptr) return Object_State::Null;
		return object->getCurrentState();
	}
};
/* Renderer that manage all objects */
class Renderer : public juce::OpenGLRenderer, public juce::Component, public juce::Timer
{
public:
	//==============================================================================
	Renderer(juce::OpenGLContext& context);
	~Renderer() override;
	/* JUCE OpenGL */
	void					newOpenGLContextCreated() override;
	void					renderOpenGL() override;
	void					openGLContextClosing() override;

	/* JUCE Component */
	void					paint(juce::Graphics& g) override;
	void					resized() override;

	/* JUCE Timer */
	void					timerCallback() override;
public:
	//==============================================================================
	/* Render Objects */
	void					registerObject(Object_Handle& handle, std::unique_ptr<Object> object);
	void					removeObject(const Object_Handle& handle);
	juce::OpenGLContext&	getglContext();
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Renderer);
	juce::OpenGLContext&			gl_context;
	juce::TextEditor				debug_info;
	GLdouble						timer, dt;

	std::unique_ptr<Camera>			main_camera;

	/* TODO: add sorting logic based on it */
	std::vector<Render_Slot>		render_slots;

	std::queue<std::pair<Object_Handle, std::unique_ptr<Object>>>
									register_queue;
	std::queue<Object_Handle>		remove_queue;
	
	juce::CriticalSection			request_lock;
private:
	//==============================================================================
	bool renderPrepare();
	void renderScene();
	void renderOverlay();
	void renderCleanup();

	Renderer_State current_state;
private:
	//==============================================================================
	void processRequests();
	void updateTime();

	bool verifyObjectHandle(GLuint target_slot, const Object_Handle& handle) const;
	/* TODO: add VRAM control */
	size_t getTotalAllocatedSize() const;
};
//==============================================================================