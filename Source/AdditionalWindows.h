
#pragma once

#include <JuceHeader.h>
#include "RNBOFMODWrapperCompiler.h"
#include "RNBOFMODConsole.h"


static auto getDisplayArea()
{
    return juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea.reduced (20);
}


class StaticCompileComponentsAlertWindow : public juce::AlertWindow
{
public:
    StaticCompileComponentsAlertWindow() : juce::AlertWindow("Some required components not found", "It looks like you're missing some components: ", juce::MessageBoxIconType::WarningIcon)
    {
        
        if (rnbofmodCompiler.CheckCmakeInstall())
        {
            setColour(juce::AlertWindow::textColourId, juce::Colours::green);
            addTextBlock("Cmake perfect to go!");
        }
        else
        {
            setColour(juce::AlertWindow::textColourId, juce::Colours::red);
            addTextBlock("Cmake not detected: The developer of this program has failed.");
        }
        
        if (rnbofmodCompiler.CheckCompilerInstall())
        {
            setColour(juce::AlertWindow::textColourId, juce::Colours::green);
            addTextBlock("Compiler ready to go!");
        }
        else
        {
            setColour(juce::AlertWindow::textColourId, juce::Colours::red);
            addTextBlock("Compiler not detected or some write permission stuff...");
#ifdef JUCE_MAC
            setColour(juce::AlertWindow::textColourId, juce::Colours::white);
            addTextBlock("We're using ninja which is bundled in this application. You can try removing the quarantine by pressing this button:");
            addButton("Try Remove Quarantine", 0);
#endif
        }
        
        
        setColour(juce::AlertWindow::textColourId, juce::Colours::white);
        addTextBlock("Press esc to close this warning.");
        
        juce::RectanglePlacement placement { juce::RectanglePlacement::yMid
                                       | juce::RectanglePlacement::xMid
                                       | juce::RectanglePlacement::doNotResize };
        setBounds(placement.appliedTo (getBounds(), getDisplayArea()));
        setVisible(true);
        setEscapeKeyCancels(true);
        enterModalState(true, juce::ModalCallbackFunction::create([this](bool result)
        {
            rnbofmodCompiler.TryRemoveNinjaQuarantine();
        }), true);
    }
private:
    RNBOFMODCompiler rnbofmodCompiler;
};

class ConsoleWindow : public juce::DocumentWindow
{
public:
    ConsoleWindow(juce::String name) : DocumentWindow (name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons), area(0, 0, 400, 900)
    {
    
        
        
        juce::RectanglePlacement placement { juce::RectanglePlacement::yTop
                                       | juce::RectanglePlacement::xRight
                                       | juce::RectanglePlacement::doNotResize };
        setBounds(placement.appliedTo (area, getDisplayArea()));

        setUsingNativeTitleBar (true);
        setAlwaysOnTop(true);
        setResizable(true, false);
        setVisible(true);
        
    
        consoleEditor.setReadOnly(true);
        consoleEditor.setBounds(area);
        consoleEditor.setVisible(true);
        consoleEditor.setMultiLine(true);
        consoleEditor.setFont(juce::Font(16));
        
        setContentOwned(&consoleEditor, true);
        
        JRFConsole::init(consoleEditor);
    }
    void closeButtonPressed() override {}
    
private:
    juce::TextEditor consoleEditor;
    juce::Rectangle<int> area ;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleWindow)
};

