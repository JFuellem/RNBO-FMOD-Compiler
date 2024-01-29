#pragma once

#include <JuceHeader.h>
#include "RNBOFMODWrapperCompiler.h"
#include "AdditionalWindows.h"
#include "RNBOFMODConsole.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent  : public juce::Component, public juce::FilenameComponentListener, public CompileListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void onCompileFinished(bool success) override;
    void filenameComponentChanged(juce::FilenameComponent* fileComponentThatHasChanged) override;
    void SaveState();
    void GetAppDir();
    void LoadAndApplyState();

private:
    //==============================================================================
    // Your private member variables go here...
    
    std::unique_ptr<RNBOFMODCompiler> rnboFmodCompiler;
    juce::Component::SafePointer<StaticCompileComponentsAlertWindow> alertWindow;
    juce::Component::SafePointer<ConsoleWindow> consoleWindow;
    
    std::unique_ptr<juce::FilenameComponent> rnboDir;
    std::unique_ptr<juce::FilenameComponent> fmodIncDir;
    std::unique_ptr<juce::FilenameComponent> exportDir;
    
    std::unique_ptr<juce::Label> titleLabel;
    std::unique_ptr<juce::Label> pluginNameInputLabel;
    std::unique_ptr<juce::Label> rnboDirLabel;
    std::unique_ptr<juce::Label> fmodIncLabel;
    std::unique_ptr<juce::Label> exportDirLabel;
    std::unique_ptr<juce::HyperlinkButton> fmodApiButton;
    
    std::unique_ptr<juce::TextButton> exportBtn;
    
    juce::ValueTree valueTree;
    
    juce::File appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    juce::File appDir;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

