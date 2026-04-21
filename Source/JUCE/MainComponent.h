//==============================================================================
#pragma once
#include <JuceHeader.h>
#include "../Graphic/Object.h"
#include "../Graphic/Renderer.h"
#include "../Graphic/Objects/Mesh.h"
//==============================================================================
/* Forward Declearation */
class Renderer;
struct Object_Handle;
//==============================================================================
class MainComponent : public juce::OpenGLAppComponent
{
public:
	//==============================================================================
	MainComponent();
	~MainComponent() override;

	void paint (juce::Graphics& g) override;
	void resized() override;

	void initialise() override;
	void shutdown() override;
	void render() override;
	bool keyPressed(const juce::KeyPress& key) override;
	void initialise_scene();

	//==============================================================================
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent);
	Renderer main_renderer;
	std::vector<Object_Handle> object_handles;
	//==============================================================================
};