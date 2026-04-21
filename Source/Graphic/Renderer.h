//==============================================================================
/* This program is dedicated in rendering particles. */
//==============================================================================
#pragma once
#include <JuceHeader.h>
#include "../Utilities.h"
//==============================================================================
using namespace juce::gl;
//==============================================================================
class Camera;
class Object;
struct Object_Handle;
//==============================================================================
/* Render slot */
struct Render_Slot
{
	std::unique_ptr<Object>	object;
	GLuint history = 0;
};
struct Render_Slot_Flags {
	uint8_t isempty		: 1 = true;
	uint8_t isready		: 1 = false;
	uint8_t isvisible	: 1 = false;
	uint8_t reserved	: 5 = false;
};
/* Renderer that manage all objects */
class Renderer : public juce::OpenGLRenderer, public juce::Component, public juce::Timer
{
public:
	//==============================================================================
	Renderer(juce::OpenGLContext& context);
	~Renderer() override;

	/* JUCE OpenGL */
	juce::OpenGLContext& getglContext();
	void newOpenGLContextCreated() override;
	void renderOpenGL() override;
	void openGLContextClosing() override;

	/* JUCE Component */
	void paint(juce::Graphics& g) override;
	void resized() override;

	/* JUCE Timer */
	void timerCallback() override;

	/* Render Objects */
	/* TODO: here exist an overwrap problem, though it isn't an urgent, better finds out a way to fix this */
	Object_Handle registerObject(std::unique_ptr<Object> object, GLuint target_slot);
	/* TODO: add request overlap fix */
	void setRenderSlotState(const Object_Handle& handle, const uint8_t state);
	/* TODO: add more check here */
	void removeObject(const Object_Handle& handle);
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Renderer);
	juce::OpenGLContext& gl_context;
	juce::TextEditor debug_info;
	GLdouble timer;
	GLdouble dt;
	void updateTime();

	/* Main camera */
	std::unique_ptr<Camera> main_camera;

	/* Independent objects to render */
	/* TODO: add sorting logic based on it */
	std::vector<Render_Slot> render_slots;
	std::vector<Render_Slot_Flags> access_table;
	/* TODO: add VRAM control */
	size_t getTotalAllocatedSize() const;

	std::queue<std::pair<GLuint, uint8_t>> set_state_queue;
	std::queue<std::pair<GLuint, std::unique_ptr<Object>>> register_queue;
	std::queue<GLuint> remove_queue;
	juce::CriticalSection request_lock;
	GLboolean isHandleValid(const Object_Handle& handle);
	void processRequests();
};
//==============================================================================