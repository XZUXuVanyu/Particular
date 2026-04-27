//==============================================================================
#include <memory>
#include "MainComponent.h"
#include "../Graphic/Objects/Mesh.h"
#include "../Graphic/Objects/Sphere.h"
#include "../Graphic/Objects/Triangle.h"
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
	main_renderer.setBounds(getLocalBounds());
}
//==============================================================================
void MainComponent::initialise()
{
    main_renderer.newOpenGLContextCreated();
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
	return true;
}
void MainComponent::initialise_scene()
{
	auto& gl_context = main_renderer.getglContext();
	Object_Handle mesh_handle{ .index = 0, .history = 0 };
	std::vector<juce::String> mesh_shader_src = { "mesh.vert", "mesh.frag", "" };
	main_renderer.registerObject(mesh_handle, std::make_unique<Mesh>(gl_context, mesh_shader_src));
	object_handles.push_back(mesh_handle);

	Object_Handle triangle_handle{ .index = 1, .history = 0 };
	std::vector<juce::String> triangle_shader_src = { "triangle.vert", "triangle.frag", "" };
	main_renderer.registerObject(triangle_handle, std::make_unique<Triangle>(gl_context, triangle_shader_src));
	object_handles.push_back(triangle_handle);

	Object_Handle sphere_handle{ .index = 2, .history = 0 };
	std::vector<juce::String> sphere_shader_src = { "sphere.vert", "sphere.frag", "" };
	main_renderer.registerObject(sphere_handle, std::make_unique<UVSphere>(gl_context, sphere_shader_src));
	object_handles.push_back(sphere_handle);
}
void MainComponent::newOpenGLContextCreated()
{
	initialise_scene();
}
//==============================================================================