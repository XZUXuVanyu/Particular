//==============================================================================
#include <memory>
#include "MainComponent.h"
//==============================================================================
constexpr uint8_t DEFAULT		= 0b0000'0001;
constexpr uint8_t REGISTERD		= 0b0000'0010;
constexpr uint8_t VISIBLE		= 0b0000'0110;
//==============================================================================
MainComponent::MainComponent() : main_renderer(openGLContext)
{
	setWantsKeyboardFocus(true);

	openGLContext.setContinuousRepainting(true);
    addAndMakeVisible(main_renderer);
    centreWithSize(720, 480);
	DBG("[INFO] MainComponent constructed");
}
MainComponent::~MainComponent()
{
	shutdownOpenGL();
}
//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
}
void MainComponent::resized()
{
	//setBounds(getLocalBounds());
	main_renderer.setBounds(getLocalBounds());
	//main_renderer.resized();
}
//==============================================================================
void MainComponent::initialise()
{
    main_renderer.newOpenGLContextCreated();
	initialise_scene();
}
void MainComponent::shutdown()
{
}
void MainComponent::render()
{
	main_renderer.renderOpenGL();
}
bool MainComponent::keyPressed(const juce::KeyPress& key)
{
	if (key.isKeyCode('H')) main_renderer.setRenderSlotState(object_handles[0], VISIBLE);
	if (key.isKeyCode('I')) main_renderer.setRenderSlotState(object_handles[0], REGISTERD);
	if (key.isKeyCode('J'))
	{
		main_renderer.removeObject(object_handles[0]);
		object_handles[0] = Object_Handle();
	}
	if (key.isKeyCode('K'))
	{
		auto& gl_context = main_renderer.getglContext();
		Object_Handle mesh_handle = main_renderer.registerObject(
			std::make_unique<Mesh>(gl_context), 0);
		object_handles[0] = mesh_handle;
		main_renderer.setRenderSlotState(object_handles[0], VISIBLE);
	}
	return true;
}
void MainComponent::initialise_scene()
{
	auto& gl_context = main_renderer.getglContext();
	Object_Handle mesh_handle = main_renderer.registerObject(
		std::make_unique<Mesh>(gl_context), 0);
	object_handles.push_back(mesh_handle);
	main_renderer.setRenderSlotState(mesh_handle, VISIBLE);
}
//==============================================================================