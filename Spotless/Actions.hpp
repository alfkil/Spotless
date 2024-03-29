#ifndef SPOTLESS_ACTIONS_H
#define SPOTLESS_ACTIONS_H

#include "../ReAction/classes.h"
#include "Spotless.hpp"
#include "Sources.hpp"
#include "Console.hpp"
#include "Configure.hpp"
#include "../SimpleDebug/TextFile.hpp"
#include "../SimpleDebug/Roots.hpp"
#include "MainMenu.hpp"

class Actions : public Widget {
public:
    typedef enum {
        Load = 1,
        Start = 2,
        Stop = 3,
        StepOver = 4,
        StepInto = 5,
        StepOut = 6,
        Quit = 7
    } ButtonIds;

private:
    Spotless *spotless;
    Speedbar *actions;

    string file, path, unixPath, configFile;

public:
    Actions(Spotless *spotless) : Widget() { this->spotless = spotless; setName("Actions"); }
    void createGuiObject(Layout *layout) {
        layout->setParent(this);
        actions = layout->createSpeedbar();
        actions->addButton(Load, "Load", "open");
        actions->addSpacer();
        actions->addButton(Start, "Start", "debug");
        actions->addButton(Stop, "Stop", "stop");
        actions->addSpacer();
        actions->addButton(StepOver, "Step over", "stepover");
        actions->addButton(StepInto, "Step into", "stepinto");
        actions->addButton(StepOut, "Step out", "stepout");
        actions->addSpacer();
        actions->addButton(Quit, "Quit", "quit");
        clear();
    }

    void update() {
        if(spotless->debugger.lives()) {
            actions->enableButton(Load, false);
            actions->enableButton(Quit, false);
        } else {
            actions->enableButton(Load, true);
            actions->enableButton(Quit, true);
        }
        
        if(spotless->debugger.isRunning()) {
            actions->enableButton(Start, false);
            actions->enableButton(Stop, true);
        } else if(spotless->debugger.lives()) {
            actions->enableButton(Start, true);
            actions->enableButton(Stop, false);
        } else {
            actions->enableButton(Start, false);
            actions->enableButton(Stop, false);
        }

        if(spotless->debugger.hasFunction()) {
            actions->enableButton(StepOver, true);
            actions->enableButton(StepInto, true);
            actions->enableButton(StepOut, true);
        } else {
            actions->enableButton(StepOver, false);
            actions->enableButton(StepInto, false);
            actions->enableButton(StepOut, false);
        }
    }

    void clear() {
        actions->enableButton(1, true);
        for(int i = 2; i <= 6; i++)
            actions->enableButton(i, false);
        actions->enableButton(7, true);
    }

    bool handleEvent(Event *event, bool *exit) {
        if(event->eventClass() == Event::CLASS_ActionButtonPress) {
            switch(event->elementId()) {
                case Actions::Load: {
                    spotless->debugger.clearRoots();
                    file = Requesters::file(Requesters::REQUESTER_EXECUTABLE, "", path, "Select executable...");
                    if(!file.size()) break;
                    unixPath = Requesters::convertToUnixRelative(path);
                    configFile = Roots::append(unixPath, "spotless.conf");
                    if(!spotless->configure->openConfig(configFile)) {
                        spotless->debugger.addSourceRoot(unixPath);
                    }
                    string arguments;
                    // if(spotless->configure->askArguments())
                    if(spotless->menu->getAskArguments())
                        arguments = Requesters::requestString("Runtime", "Please provide arguments for %s:", file.c_str());
                    if(spotless->debugger.load(path, file, arguments)) { //Use amigaos path for LoadSeg
                        spotless->updateAll();
                        spotless->sources->update();
                        spotless->sources->showCurrent();
                    } else {
                        spotless->console->write(PublicScreen::PENTYPE_CRITICAL, "Failed to load selected file.");
                    }
                    break;
                }
                case Actions::Start:
                    spotless->debugger.start();
                    // debugger.justGo();
                    break;
                case Actions::Stop:
                    spotless->debugger.stop();
                    spotless->updateAll();
                    break;
                case Actions::StepOver:
                    spotless->debugger.stepOver();
                    break;
                case Actions::StepInto:
                    spotless->debugger.stepInto();
                    spotless->updateAll();
                    break;
                case Actions::StepOut:
                    spotless->debugger.stepOut();
                    spotless->updateAll();
                    break;
                case Actions::Quit:
                    *exit = true;
                    break;
            }
            update();
        }
        return false;
    }
    string getFile() {
        return file;
    }
    string getPath() {
        return path;
    }
    string getUnixPath() {
        return unixPath;
    }
};
#endif