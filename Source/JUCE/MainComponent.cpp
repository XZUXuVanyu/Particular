//==============================================================================
#include <memory>
#include "MainComponent.h"
#include "../Graphic/Objects/Mesh.h"
#include "../Graphic/Objects/Sphere.h"
#include "../Graphic/Objects/Triangle.h"
//==============================================================================
using namespace Crystal;
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
	if (juce::KeyPress::isKeyCurrentlyDown('k') || juce::KeyPress::isKeyCurrentlyDown('K'))
	{
		for (int i = 0; i <= 2; ++i)
		{
			if (juce::KeyPress::isKeyCurrentlyDown('0' + i))
			{
				if (i < object_handles.size())
				{
					main_renderer.removeObject(object_handles[i]);
				}
			}
		}
	}
	if (juce::KeyPress::isKeyCurrentlyDown('n') || juce::KeyPress::isKeyCurrentlyDown('N'))
	{
		auto& gl_context = main_renderer.getglContext();
		if (juce::KeyPress::isKeyCurrentlyDown('0'))
		{
			Crystal::Object_Handle mesh_handle = object_handles[0];
			std::vector<juce::String> mesh_shader_src = { "mesh.vert", "mesh.frag", "" };
			main_renderer.registerObject(mesh_handle, std::make_unique<Mesh>(gl_context, mesh_shader_src));
			object_handles[0] = mesh_handle;
		}
		if (juce::KeyPress::isKeyCurrentlyDown('1'))
		{
			Crystal::Object_Handle triangle_handle = object_handles[1];
			std::vector<juce::String> triangle_shader_src = { "triangle.vert", "triangle.frag", "" };
			main_renderer.registerObject(triangle_handle, std::make_unique<Triangle>(gl_context, triangle_shader_src));
			object_handles[1] = triangle_handle;
		}
		if (juce::KeyPress::isKeyCurrentlyDown('2'))
		{
			Crystal::Object_Handle sphere_handle = object_handles[2];
			std::vector<juce::String> sphere_shader_src = { "sphere.vert", "sphere.frag", "" };
			main_renderer.registerObject(sphere_handle, std::make_unique<UVSphere>(gl_context, sphere_shader_src));
			object_handles[2] = sphere_handle;
		}
	}
	if (juce::KeyPress::isKeyCurrentlyDown('p') || juce::KeyPress::isKeyCurrentlyDown('P'))
	{
		if (!object_handles.empty())
		{
			if (juce::KeyPress::isKeyCurrentlyDown('0')) {
				auto& target_obj = main_renderer.debug_getRenderSlot(0);
				target_obj.object->debug_forceSetState(Entity_Constructed);
			}
			if (juce::KeyPress::isKeyCurrentlyDown('1')) {
				auto& target_obj = main_renderer.debug_getRenderSlot(0);
				target_obj.object->debug_forceSetState(Entity_Initialised);
			}
			if (juce::KeyPress::isKeyCurrentlyDown('2')) {
				auto& target_obj = main_renderer.debug_getRenderSlot(0);
				target_obj.object->debug_forceSetState(Entity_Initialising);
			}
			if (juce::KeyPress::isKeyCurrentlyDown('3')) {
				auto& target_obj = main_renderer.debug_getRenderSlot(0);
				target_obj.object->debug_forceSetState(Entity_OnProcessing);
			}
			if (juce::KeyPress::isKeyCurrentlyDown('4')) {
				auto& target_obj = main_renderer.debug_getRenderSlot(0);
				target_obj.object->debug_forceSetState(Entity_Deconstructing);
			}
			if (juce::KeyPress::isKeyCurrentlyDown('5')) {
				auto& target_obj = main_renderer.debug_getRenderSlot(0);
				target_obj.object->debug_forceSetState(Entity_Deconstructed);
			}
		}
		return true;
	}
}
void MainComponent::initialise_scene()
{
	auto& gl_context = main_renderer.getglContext();
	Crystal::Object_Handle mesh_handle{ .index = 0, .history = 0 };
	std::vector<juce::String> mesh_shader_src = { "mesh.vert", "mesh.frag", "" };
	main_renderer.registerObject(mesh_handle, std::make_unique<Mesh>(gl_context, mesh_shader_src));
	object_handles.push_back(mesh_handle);

	Crystal::Object_Handle triangle_handle{ .index = 1, .history = 0 };
	std::vector<juce::String> triangle_shader_src = { "triangle.vert", "triangle.frag", "" };
	main_renderer.registerObject(triangle_handle, std::make_unique<Triangle>(gl_context, triangle_shader_src));
	object_handles.push_back(triangle_handle);

	Crystal::Object_Handle sphere_handle{ .index = 2, .history = 0 };
	std::vector<juce::String> sphere_shader_src = { "sphere.vert", "sphere.frag", "" };
	main_renderer.registerObject(sphere_handle, std::make_unique<UVSphere>(gl_context, sphere_shader_src));
	object_handles.push_back(sphere_handle);
}
void MainComponent::newOpenGLContextCreated()
{
	initialise_scene();
}
void MainComponent::openGLContextClosing()
{
	main_renderer.shutdown();
}
//==============================================================================