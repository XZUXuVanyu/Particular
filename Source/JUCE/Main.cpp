#include <iostream>
#include <JuceHeader.h>
#include "MainComponent.h"
//==============================================================================
class NewProjectApplication  : public juce::JUCEApplication
{
public:
	//==============================================================================
	NewProjectApplication() { }
	const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
	const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
	bool moreThanOneInstanceAllowed() override             { return true; }
	//==============================================================================
	void initialise (const juce::String& commandLine) override 
	{
		mainWindow.reset(new MainWindow("Hello"));
	}
	void shutdown() override
	{
		mainWindow = nullptr;
	}
	//==============================================================================
	void systemRequestedQuit() override
	{
		// This is called when the app is being asked to quit: you can ignore this
		// request and let the app carry on running, or call quit() to allow the app to close.
		quit();
	}
	void anotherInstanceStarted (const juce::String& commandLine) override
	{
		// When another instance of the app is launched while this one is running,
		// this method is invoked, and the commandLine parameter tells you what
		// the other instance's command-line arguments were.
	}
	//==============================================================================
	/*
		This class implements the desktop window that contains an instance of
		our MainComponent class.
	*/
	//==============================================================================
	class MainWindow    : public juce::DocumentWindow
	{
	public:
		MainWindow (juce::String name)
			: DocumentWindow (name,
							  juce::Desktop::getInstance().getDefaultLookAndFeel()
														  .findColour (juce::ResizableWindow::backgroundColourId),
							  DocumentWindow::allButtons)
		{
			setUsingNativeTitleBar (true);
			setContentOwned (new MainComponent(), true);
		   #if JUCE_IOS || JUCE_ANDROID
			setFullScreen (true);
		   #else
			setResizable (true, true);
			centreWithSize (getWidth(), getHeight());
		   #endif

			setVisible (true);
		}
		void closeButtonPressed() override
		{
			// This is called when the user tries to close this window. Here, we'll just
			// ask the app to quit when this happens, but you can change this to do
			// whatever you need.
			JUCEApplication::getInstance()->systemRequestedQuit();
		}
	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
	};
	//==============================================================================
private:
	std::unique_ptr<MainWindow> mainWindow;
};
//==============================================================================
START_JUCE_APPLICATION(NewProjectApplication);