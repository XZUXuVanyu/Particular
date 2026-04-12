#include "MainComponent.h"
//==============================================================================
MainComponent::MainComponent() : main_renderer(openGLContext)
{
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
}
void MainComponent::shutdown()
{
}
void MainComponent::render()
{
	main_renderer.renderOpenGL();
}
//==============================================================================