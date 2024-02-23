#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : valueTree("Directories")
{
    rnboDir.reset(new juce::FilenameComponent ("rnboDir",
                  {},
                  true,
                  true,
                  false,
                  {},
                  {},
                  "Select the Folder exported from RNBO"));
    
    fmodIncDir.reset(new juce::FilenameComponent ("fmodIncDir",
                  {},
                  true,
                  true,
                  false,
                  {},
                  {},
                  "Select the FMOD Inc Folder from the API"));
    
    exportDir.reset(new juce::FilenameComponent ("exportDir",
                  {},
                  true,
                  true,
                  false,
                  {},
                  {},
                  "Select the export Directory"));
    
    rnboFmodCompiler.reset(new RNBOFMODCompiler());
    rnboFmodCompiler->addCompileListener(this);
    
    rnboDir->addListener(this);
    fmodIncDir->addListener(this);
    exportDir->addListener(this);
    
    pluginNameInputLabel.reset(new juce::Label());
    pluginNameInputLabel->setFont(juce::Font (27.0f));
    pluginNameInputLabel->setColour (juce::Label::backgroundColourId, juce::Colours::darkgrey);
    pluginNameInputLabel->setText("Plugin_Name", juce::sendNotification);
    pluginNameInputLabel->setJustificationType(juce::Justification::centred);
    pluginNameInputLabel->setEditable(true);
    pluginNameInputLabel->onTextChange = [this]
    {
        juce::String inputText = pluginNameInputLabel->getText();
        juce::String filteredText = inputText.removeCharacters(" ");
        if(inputText != filteredText)
            pluginNameInputLabel->setText(filteredText, juce::NotificationType::dontSendNotification);
        valueTree.setProperty("PluginName", pluginNameInputLabel->getText(), nullptr);
        rnboFmodCompiler->PluginName = pluginNameInputLabel->getText().toStdString();
        
    };
    
    titleLabel.reset(new juce::Label());
    titleLabel->setFont(juce::Font (32.0f));
    titleLabel->setText("RNBO Plugin Compiler", juce::dontSendNotification);
    titleLabel->setJustificationType(juce::Justification::centred);
   
    rnboDirLabel.reset(new juce::Label());
    rnboDirLabel->setFont(juce::Font (16.0f));
    rnboDirLabel->setText("RNBO Path", juce::dontSendNotification);
    
    fmodIncLabel.reset(new juce::Label());
    fmodIncLabel->setFont(juce::Font (16.0f));
    fmodIncLabel->setText("FMOD Inc Folder Path", juce::dontSendNotification);
    fmodIncLabel->setJustificationType(juce::Justification::left);
    
    exportDirLabel.reset(new juce::Label());
    exportDirLabel->setFont(juce::Font (16.0f));
    exportDirLabel->setText("Export Directory", juce::dontSendNotification);
    exportDirLabel->setJustificationType(juce::Justification::left);
    
    fmodApiButton.reset(new juce::HyperlinkButton("Click me to go to download (Fmod Engine).", juce::URL("https://www.fmod.com/download#fmodengine")));
    fmodApiButton->setFont(juce::Font(10.0f), true);
    fmodApiButton->setJustificationType(juce::Justification::left);
    
    exportBtn.reset(new juce::TextButton());
    exportBtn->setButtonText("Export Plugin");
    exportBtn->onClick = [this]
    {
        rnboFmodCompiler->CompilePluginInThread();
        exportBtn->setEnabled(false);
    };
    
    addAndMakeVisible(rnboDir.get());
    addAndMakeVisible(fmodIncDir.get());
    addAndMakeVisible(exportDir.get());
    addAndMakeVisible(titleLabel.get());
    addAndMakeVisible(rnboDirLabel.get());
    addAndMakeVisible(fmodApiButton.get());
    addAndMakeVisible(fmodIncLabel.get());
    addAndMakeVisible(exportDirLabel.get());
    addAndMakeVisible(pluginNameInputLabel.get());
    addAndMakeVisible(exportBtn.get());
    
    GetAppDir();
    LoadAndApplyState();
    
    consoleWindow = new ConsoleWindow("Console");
    
    if (!rnboFmodCompiler->CheckStaticCompileComponents())
    {
        alertWindow = new StaticCompileComponentsAlertWindow();
    }
    else
        JRFConsole::log << "Welcome!" << std::endl;
    
    setSize (600, 400);
}

MainComponent::~MainComponent()
{
    alertWindow.deleteAndZero();
    consoleWindow.deleteAndZero();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (32.0f));
    g.setColour (juce::Colours::white);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    juce::FlexBox fb;
    fb.flexWrap = juce::FlexBox::Wrap::noWrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    fb.alignContent  = juce::FlexBox::AlignContent::center;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.alignItems = juce::FlexBox::AlignItems::center;
    
    float width = getWidth()-100;
    
    fb.items.add(juce::FlexItem(*titleLabel).withMinWidth(width).withMinHeight(50.0f));
    fb.items.add(juce::FlexItem(*pluginNameInputLabel).withMinWidth(width/2.0f).withMinHeight(50.0f));
    fb.items.add(juce::FlexItem(*rnboDirLabel).withMinWidth(width).withMinHeight(25.0f).withMargin(juce::FlexItem::Margin(15, 0, 0, 0)));
    fb.items.add(juce::FlexItem(*rnboDir).withMinWidth(width).withMinHeight(25.0f));
    fb.items.add(juce::FlexItem(*fmodIncLabel).withMinWidth(width).withMinHeight(25.0f).withMargin(juce::FlexItem::Margin(15, 0, 0, 0)));
    fb.items.add(juce::FlexItem(*fmodApiButton).withMinWidth(width).withMinHeight(18.0f));
    fb.items.add(juce::FlexItem(*fmodIncDir).withMinWidth(width).withMinHeight(25.0f));
    fb.items.add(juce::FlexItem(*exportDirLabel).withMinWidth(width).withMinHeight(25.0f).withMargin(juce::FlexItem::Margin(15, 0, 0, 0)));
    fb.items.add(juce::FlexItem(*exportDir).withMinWidth(width).withMinHeight(25.0f));
    fb.items.add(juce::FlexItem(*exportBtn).withMinWidth(width/2).withMinHeight(42).withMargin(juce::FlexItem::Margin(25, 0, 0, 0)));

    fb.performLayout(getLocalBounds());
}

void MainComponent::filenameComponentChanged(juce::FilenameComponent *fileComponentThatHasChanged)
{
    if(fileComponentThatHasChanged == rnboDir.get())
    {
        juce::String temp = rnboDir->getCurrentFile().getFullPathName();
        valueTree.setProperty("rnboDir", temp, nullptr);
        rnboFmodCompiler->rnboDirectory = temp.toStdString();
        
        pluginNameInputLabel->setText(TryGetPluginName(), juce::NotificationType::sendNotification);
    }
    else if(fileComponentThatHasChanged == fmodIncDir.get())
    {
        juce::String temp = fmodIncDir->getCurrentFile().getFullPathName();
        valueTree.setProperty("fmodIncDir", temp, nullptr);
        rnboFmodCompiler->fmodIncDirectory = temp.toStdString();
    }
    else if(fileComponentThatHasChanged == exportDir.get())
    {
        juce::String temp = exportDir->getCurrentFile().getFullPathName();
        valueTree.setProperty("exportDir", temp, nullptr);
        rnboFmodCompiler->exportDirectory = temp.toStdString();
    }
}

std::string MainComponent::TryGetPluginName()
{
    //juce::File rnboSrc = juce::File(rnboFmodCompiler->rnboDirectory).getChildFile("rnbo_source.cpp");
    auto cppFiles = juce::File(rnboFmodCompiler->rnboDirectory).findChildFiles(juce::File::TypesOfFileToFind::findFiles, false, "*.cpp");
    if(cppFiles.size() > 1)
        JRFConsole::warn << "The RNBO Export Directory contains more than one .cpp file. This can cause issues." << std::endl;
    
    if(cppFiles[0].existsAsFile())
    {
        juce::FileInputStream instream(cppFiles[0]);
        if(!instream.openedOk())
            return "DefaultName";
        juce::String line;
        while(!instream.isExhausted())
        {
            line = instream.readNextLine();
            if(line.contains("Name_"))
            {
                juce::String temp = line.fromFirstOccurrenceOf("Name_", false, true);
                return temp.upToFirstOccurrenceOf("\"", false, true).toStdString();
            }
        }
    }
    else
        JRFConsole::warn << "The selected RNBO Directory is likely wrong." << std::endl;
    
    return "DefaultName";
}

void MainComponent::SaveState()
{
    std::unique_ptr<juce::XmlElement> xml = valueTree.createXml();
    if(xml != nullptr)
    {
        if(appDir.getChildFile("config.xml").exists())
            appDir.getChildFile("config.xml").deleteFile();
        
        xml->writeTo(appDir.getNonexistentChildFile("config", ".xml"));
       
    }
}

void MainComponent::LoadAndApplyState()
{
    juce::File config = appDir.getChildFile("config.xml");
    if(config.exists())
    {
        std::unique_ptr<juce::XmlElement> xml(juce::XmlDocument::parse(config));
        valueTree = juce::ValueTree::fromXml(*xml);

        if(valueTree.hasProperty("rnboDir"))
            rnboDir->setCurrentFile(juce::File(valueTree.getProperty("rnboDir")), false);
        if(valueTree.hasProperty("fmodIncDir"))
            fmodIncDir->setCurrentFile(juce::File(valueTree.getProperty("fmodIncDir")), false);
        if(valueTree.hasProperty("exportDir"))
            exportDir->setCurrentFile(juce::File(valueTree.getProperty("exportDir")), false);
        if(valueTree.hasProperty("PluginName"))
            pluginNameInputLabel->setText(juce::String(valueTree.getProperty("PluginName").toString()), juce::sendNotification);

    }
        
}

void MainComponent::GetAppDir()
{
    appDir = appDataDir.getChildFile("RNBOFmodCompiler");
    if(!appDir.exists())
        appDir.createDirectory();
}

void MainComponent::onCompileFinished(bool success)
{
    exportBtn->setEnabled(true);
    if(success)
        JRFConsole::succ << "Compilation succeeded." << std::endl;
    else
        JRFConsole::err << "Compilation failed." << std::endl;
}

