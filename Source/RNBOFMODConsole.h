/*
  ==============================================================================

    RNBOFMODConsole.h
    Created: 29 Jan 2024 11:16:23am
    Author:  Jonas Füllemann

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <ctime>

enum LogType
{
    LOG,
    ERR,
    WARN,
    SUCCESS
};

class CustomStreamBuffer : public std::streambuf
{
public:
    CustomStreamBuffer(juce::TextEditor& editor, LogType logType) : textEditor(editor), logType(logType) {};
        
protected:
    virtual std::streamsize xsputn(const char_type* s, std::streamsize n) override
    {
        std::string text(s, n);
        
        time(&rawTime);
        timeinfo = localtime(&rawTime);
        strftime(buffer, sizeof(buffer), "%H:%M:%S: ", timeinfo);
        std::string timestr(buffer);
        
        
        juce::MessageManager::callAsync([text, n, timestr, this]()
        {
            if(logType == LogType::LOG)
                textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::white);
            else if (logType == LogType::ERR)
                textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::red);
            else if (logType == LogType::WARN)
                textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::yellow);
            else if (logType == LogType::SUCCESS)
                textEditor.setColour(juce::TextEditor::textColourId, juce::Colours::green);
            
            textEditor.setCaretPosition(textEditor.getTotalNumChars());

            if(n > 1)
                textEditor.insertTextAtCaret(timestr + text);
            else
                textEditor.insertTextAtCaret(text);
        });
        
        
        return n;
    }
    virtual int_type overflow(int_type c = traits_type::eof()) override
    {
        if (traits_type::eq_int_type(c, traits_type::eof()))
        {
            return traits_type::not_eof(c);
        }
        char_type ch = traits_type::to_char_type(c);
        xsputn(&ch, 1);
        return c;
    }
private:
    juce::String output;
    juce::TextEditor& textEditor;
    LogType logType;
    time_t rawTime;
    struct tm * timeinfo;
    char buffer[80];
};

class JRFConsole
{
public:
    static void init(juce::TextEditor& consoleEditor)
    {
        static CustomStreamBuffer consoleBufferLog(consoleEditor, LogType::LOG);
        static CustomStreamBuffer consoleBufferErr(consoleEditor, LogType::ERR);
        static CustomStreamBuffer consoleBufferWarn(consoleEditor, LogType::WARN);
        static CustomStreamBuffer consoleBufferSucc(consoleEditor, LogType::SUCCESS);
        log.rdbuf(&consoleBufferLog);
        err.rdbuf(&consoleBufferErr);
        warn.rdbuf(&consoleBufferWarn);
        succ.rdbuf(&consoleBufferSucc);
    }
    
    static std::ostream log;
    static std::ostream err;
    static std::ostream warn;
    static std::ostream succ;
};


