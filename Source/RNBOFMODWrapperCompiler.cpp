/*
  ==============================================================================

    RNBOFMODWrapperCompiler.cpp
    Created: 21 Jan 2024 2:24:51pm
    Author:  Jonas Füllemann

  ==============================================================================
*/

#include "RNBOFMODWrapperCompiler.h"

RNBOFMODCompiler::RNBOFMODCompiler()
{
    juce::File workingDirectory = juce::File::getCurrentWorkingDirectory();
    PluginName = "DefaultName";
    exportDirectory = workingDirectory.getFullPathName().toStdString();
    fmodIncDirectory = workingDirectory.getFullPathName().toStdString();
    rnboDirectory = workingDirectory.getFullPathName().toStdString();
}

RNBOFMODCompiler::~RNBOFMODCompiler()
{
    if(compileThead.joinable())
        compileThead.join();
}

bool RNBOFMODCompiler::CompilePluginInThread()
{
    compileThead = std::thread(&RNBOFMODCompiler::CompilePlugin, this);
    return false;
}

void RNBOFMODCompiler::CompilePlugin()
{
    //checkphase
    if(!CheckDynamicCompileComponents())
    {
        juce::MessageManager::callAsync([this] {onCompilationFinished(false);});
        return;
    }
    
    //std::cout << "All components ready." << std::endl;
    JRFConsole::log << "All components ready." << std::endl;
    
    //cmakecreatephase
    if(!CreateCMakeLists())
    {
        juce::MessageManager::callAsync([this] {onCompilationFinished(false);});
        return;
    }
    
    //std::cout << "Sucessfully made CMakeLists.txt" << std::endl;
    JRFConsole::log << "Sucessfully made CMakeLists.txt" << std::endl;
    
    //build cmake phase
    if(!CreateCMake())
    {
        juce::MessageManager::callAsync([this] {onCompilationFinished(false);});
        return;
    }
    
    JRFConsole::log << "CMake files built nicely" << std::endl;
    
    //run compiler phase
    if(!CompileCMake())
    {
        juce::MessageManager::callAsync([this] {onCompilationFinished(false);});
        return;
    }
    
    JRFConsole::log << "Plugin is served" << std::endl;
    
    //cleanup phase
    
    juce::MessageManager::callAsync([this] {onCompilationFinished(true);});
    
}

bool RNBOFMODCompiler::CreateCMakeLists()
{
    juce::File cmakelistsFile = juce::File(rnboDirectory).getChildFile("CmakeLists.txt");
    juce::File rnboDir = juce::File(rnboDirectory);

    if(cmakelistsFile.existsAsFile())
        JRFConsole::warn << "Cmakefile exists and will be overridden. This is not a problem." << std::endl;
    
    juce::TemporaryFile tempFile (cmakelistsFile);
    juce::FileOutputStream outStream (tempFile.getFile());

    std::string trnboWrapperHpp = rnbowrapperHpp.getFullPathName().toStdString() + " ";
    std::string trnboWrapperCpp = rnborwapperCpp.getFullPathName().toStdString() + " ";
    std::string trnboFmodCpp = rnboFmod.getFullPathName().toStdString() + " ";
    std::string trnboSrc1 = rnbo_src.getFullPathName().toStdString() + " ";
    std::string trnboSrc2 = rnbo_src2.getFullPathName().toStdString() + " ";
    std::string trnboSrc3 = rnbo_src3.getFullPathName().toStdString() + " ";
    
    if(!outStream.openedOk())
    {
        JRFConsole::err << "Error opening stream." << std::endl;
        return false;
    }
    
    outStream.setNewLineString("\n");

    outStream.writeText(juce::String("#This file was created with the RNBO-FMOD-Wrapper-Compiler. It's safe to delete if you feel like it.") + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("#CMAKE_HOST_SYSTEM_NAME") + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("cmake_minimum_required(VERSION 3.10.0)") + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("project(") + PluginName +")" + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("set(CMAKE_CXX_STANDARD 14)") + juce::newLine, false, false, nullptr);
    //outStream.writeText(juce::String("set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ") + exportDirectory + ")" + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("add_library(${PROJECT_NAME} SHARED ") + trnboWrapperHpp  + trnboWrapperCpp + trnboFmodCpp + trnboSrc1 + trnboSrc2 + trnboSrc3 + ")" + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ") + exportDirectory + ")" + juce::newLine, false, false, nullptr);
    
    outStream.writeText(juce::String("target_include_directories(${PROJECT_NAME} PRIVATE ") + rnboDir.getChildFile("rnbo").getFullPathName().toStdString() + ")" + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("target_include_directories(${PROJECT_NAME} PRIVATE ") + rnboDir.getChildFile("rnbo/common").getFullPathName().toStdString() + ")" + juce::newLine, false, false, nullptr);
    outStream.writeText(juce::String("target_include_directories(${PROJECT_NAME} PRIVATE ") + fmodIncDirectory + ")" + juce::newLine, false, false, nullptr);
    
    outStream.flush();
    
    if(outStream.getStatus().failed())
    {
        JRFConsole::err << "Outputstream failed." << std::endl;
        return false;
    }
    
    bool result = tempFile.overwriteTargetFileWithTemporary();
    
    if(!result)
        JRFConsole::err << "Failed to override CMakeLists.txt" << std::endl;
    
    return result;
}

bool RNBOFMODCompiler::CreateCMake()
{
    juce::File cmakebuildfolder = juce::File(rnboDirectory).getChildFile("CMake");
    juce::String command = cmakeDir.getFullPathName().toStdString() + " -S " + rnboDirectory + " -B " + cmakebuildfolder.getFullPathName().toStdString();
    
    cmdThread.startThreadWithCommand(command);
    cmdThread.waitForThreadToExit(60000);
    
    bool result = !cmdThread.success.load();
    
    if(!result)
        JRFConsole::err << "Failed to create Cmake Files" << std::endl;
    
    return result;
}

bool RNBOFMODCompiler::CompileCMake()
{
    juce::File cmakebuildfolder = juce::File(rnboDirectory).getChildFile("CMake");
    
    juce::String command = cmakeDir.getFullPathName().toStdString() + " --build " + cmakebuildfolder.getFullPathName().toStdString();
    
    cmdThread.startThreadWithCommand(command);
    cmdThread.waitForThreadToExit(60000);
    
    bool result = !cmdThread.success.load();
    
    if(!result)
        JRFConsole::err << "Failed to compile plugin..." << std::endl;
    
    return result;
}

bool RNBOFMODCompiler::CheckCmakeInstall()
{
    juce::File appDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile);

    cmakeDir = appDir.getChildFile("Contents/Resources/External_Tools/CMake.app/Contents/bin/cmake");


    std::string command = cmakeDir.getFullPathName().toStdString() + " --version";
    
    juce::ChildProcess process;
    int result = -1;
    if(process.start(command.c_str()))
    {
        //juce::String result = process.readAllProcessOutput();
        process.waitForProcessToFinish(3000);
        result = process.getExitCode();
    }
    
    return !result;
}

bool RNBOFMODCompiler::CheckCompilerInstall()
{
#if JUCE_MAC
    std::string command = "clang --version";
#elif JUCE_WINDOWS
    std::string command = "gcc --version";
#else
    std::string command = "error";
#endif
    
    juce::ChildProcess process;
    int result = -1;
    if(process.start(command.c_str()))
    {
        //juce::String result = process.readAllProcessOutput();
        process.waitForProcessToFinish(3000);
        result = process.getExitCode();
    }
    
    return !result;
}

bool RNBOFMODCompiler::CheckRNBOSrc()
{
    //the file with the rnbo export class
    rnbo_src = juce::File(rnboDirectory).getChildFile("rnbo_source.cpp");
    rnbo_src2 = juce::File(rnboDirectory).getChildFile("rnbo").getChildFile("RNBO.h");
    rnbo_src3 = juce::File(rnboDirectory).getChildFile("rnbo").getChildFile("RNBO.cpp");
    auto result = rnbo_src.existsAsFile() && rnbo_src2.existsAsFile() && rnbo_src3.existsAsFile();
    
    if(!result)
        JRFConsole::err << "RNBOSRC_ERROR\n";
    
    return result;
}

bool RNBOFMODCompiler::CheckFMODAPIDir()
{
    //look if the main header exists, if yes, boldly assume everything is ok
    juce::File fmodHpp = juce::File(fmodIncDirectory).getChildFile("fmod.hpp");
    auto result = fmodHpp.existsAsFile();
    if(!result)
        JRFConsole::err << "FMODAPIDIR_ERROR\n";
    return result;
}

bool RNBOFMODCompiler::CheckRNBOFMODSrcs()
{
    juce::File appDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::currentApplicationFile);

    rnbowrapperHpp = appDir.getChildFile("Contents/Resources/RNBOFMODSrc/RNBOWrapper.hpp");
    rnborwapperCpp = appDir.getChildFile("Contents/Resources/RNBOFMODSrc/RNBOWrapper.cpp");
    rnboFmod = appDir.getChildFile("Contents/Resources/RNBOFMODSrc/RNBO_FMOD.cpp");
    
    auto result = rnbowrapperHpp.existsAsFile() && rnborwapperCpp.existsAsFile() && rnboFmod.existsAsFile();
    
    if(!result)
        JRFConsole::err << "RNBOFMODCOMPONTENTS_ERROR\n";
    
    return result;
    
}

bool RNBOFMODCompiler::CheckStaticCompileComponents()
{
    return CheckCmakeInstall() && CheckCompilerInstall() && CheckRNBOFMODSrcs();
}

bool RNBOFMODCompiler::CheckDynamicCompileComponents()
{
    return CheckFMODAPIDir() && CheckRNBOSrc() && CheckFMODAPIDir();
}

void CommandlineThread::run()
{
    setPriority(juce::Thread::Priority::highest);
    success.store(false);
    
    //std::cout << cmdCommand.toStdString().c_str() << std::endl;
    //child Process taking too damn long... taking system for now
    
    bool result = system(cmdCommand.toStdString().c_str());
    
    /*
    juce::ChildProcess process;
    if(!process.start(cmdCommand.toStdString().c_str()))
        return;
    while(process.isRunning() && !threadShouldExit())
    {
        char buffer[32];
        auto numBytes = process.readProcessOutput(buffer, sizeof(buffer));
    
        if(numBytes > 0)
        {
            std::cout << buffer;
        }
        juce::Thread::sleep(10);
    }
     //std::cout << "Thread finished with return code: " << process.getExitCode() << std::endl;
     //success.store(process.getExitCode());
*/
    success.store(result);
    return;

}
