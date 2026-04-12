#pragma once
#include <JuceHeader.h>
#include "../Graphic/MainRenderer.h"
//==============================================================================
/*
	This component lives inside our window, and this is where you should put all
	your controls and content.
*/
//==============================================================================
class MainComponent : public juce::OpenGLAppComponent
{
public:
	//==============================================================================
	MainComponent();
	~MainComponent() override;
	//==============================================================================
	void paint (juce::Graphics& g) override;
	void resized() override;
	//==============================================================================
	void initialise() override;
	void shutdown() override;
	void render() override;
	//==============================================================================
private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent);
	MainRenderer main_renderer;
	//==============================================================================
};