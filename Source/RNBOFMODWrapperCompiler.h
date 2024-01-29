/*
  ==============================================================================

    RNBOFMODWrapperCompiler.h
    Created: 21 Jan 2024 2:24:51pm
    Author:  Jonas Füllemann

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <string>
#include <cstdlib>
#include "RNBOFMODConsole.h"

class CommandlineThread : public juce::Thread
{
public:
    CommandlineThread() : juce::Thread ("Cmd Thread") {}
    ~CommandlineThread() {stopThread(1000);}
    
    void startThreadWithCommand(juce::String s_command)
    {
        cmdCommand = s_command;
        startThread();
    }
    
    void run() override;
    std::atomic<bool> success {false};
    
private:
    juce::String cmdCommand;
};

struct CompileListener
{
    virtual void onCompileFinished(bool success) = 0;
    virtual ~CompileListener() = default;
};

class RNBOFMODCompiler
{
public:
    RNBOFMODCompiler();
    ~RNBOFMODCompiler();
    bool CompilePluginInThread();
    bool CheckStaticCompileComponents();
    bool CheckDynamicCompileComponents();
    bool CheckCmakeInstall();
    bool CheckCompilerInstall();
    bool TryRemoveNinjaQuarantine();
    
    void addCompileListener(CompileListener* listener) { compileListeners.add(listener); }
    void removeCompileListener(CompileListener* listener) { compileListeners.remove(listener); }
    
    std::string PluginName;
    std::string exportDirectory;
    std::string fmodIncDirectory;
    std::string rnboDirectory;
    
    juce::File cmakeDir;
    juce::File ninjaDir;
private:
    void CompilePlugin();
    bool CheckRNBOSrc();
    bool CheckFMODAPIDir();
    bool CheckRNBOFMODSrcs();
    bool CreateCMakeLists();
    bool CreateCMake();
    bool CompileCMake();
    
    void onCompilationFinished(bool success)
    {
        if(compileThead.joinable())
            compileThead.join();
        std::cout << "All finished" << std::endl;
        compileListeners.call([success](CompileListener& listener)
        {
            listener.onCompileFinished(success);
        });
    }
    
    juce::File rnbo_src;
    juce::File rnbo_src2;
    juce::File rnbo_src3;
    juce::File rnbowrapperHpp;
    juce::File rnborwapperCpp;
    juce::File rnboFmod;
    
    CommandlineThread cmdThread;
    std::thread compileThead;
    juce::ListenerList<CompileListener> compileListeners;
};

class CompileThread : private juce::Thread
{
    CompileThread(RNBOFMODCompiler *comp) : juce::Thread ("Compile Thread")
    {
        rnboFmodCompiler = comp;
    }
    ~CompileThread() {stopThread(100);}
    void run() override;
    RNBOFMODCompiler* rnboFmodCompiler;
};


