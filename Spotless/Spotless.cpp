#include "Actions.hpp"
#include "Code.hpp"
#include "Console.hpp"
#include "Context.hpp"
#include "MainMenu.hpp"
#include "Sources.hpp"
#include "Stacktrace.hpp"
#include "Disassembler.hpp"
#include "Spotless.hpp"
#include "MemorySurfer.hpp"
#include "Registers.hpp"
#include "../ReAction/Config.hpp"

#include <libraries/keymap.h>

#include <proto/exec.h>

#include "../SimpleDebug/Strings.hpp"

// Listbrowser *Console::listbrowser = 0;
// Spotless *Console::spotless = 0;
Spotless *Spotless::spotless = 0;

// vector<Console::Line *> Console::buffer;

void Spotless::create() {
    spotless = this;

    menu = new MainMenu(spotless);
    actions = new Actions(spotless);
    sources = new Sources(spotless);
    context = new Context(spotless);
    stacktrace = new Stacktrace(spotless);
    code = new Code(spotless);
    console = new Console(spotless);
    disassembler = new Disassembler(spotless);
    registers = new Registers(spotless);

    memorySurfer = new MemorySurfer(spotless);
    configure = new Configure(spotless);

    setMenubar(menu);
    setTopBar(actions);
    setMainView(code);
    addLeftPanelWidget(sources);
    addLeftPanelWidget(context);
    addLeftPanelWidget(stacktrace);
    addBottomPanelWidget(disassembler);
    addBottomPanelWidget(registers);
    addBottomPanelWidget(console);

    addExtraWidget(configure);
    addExtraWidget(memorySurfer);
    
    // addSignalHandler(deathHandler, SIGF_CHILD);
    // addSignalHandler(trapHandler, debugger.getTrapSignal());
    addSignalHandler(portHandler, debugger.getPortSignal());
    // addSignalHandler(pipeHandler, debugger.getPipeSignal());
}

vector<Widget *> Spotless::getAllPanelWidgets()
{
    vector<Widget *> result;
    result.push_back(sources);
    result.push_back(context);
    result.push_back(stacktrace);
    result.push_back(disassembler);
    result.push_back(registers);
    result.push_back(console);
    return result;
}

void Spotless::destroy() {

    delete menu;
    delete actions;
    delete sources;
    delete context;
    delete stacktrace;
    delete code;
    delete console;
    delete disassembler;
    delete registers;

    delete memorySurfer;
    delete configure;
}

int Spotless::unfold() {
    {
        Config config("config.prefs");
        setSplit(config.getBool("Split mode", false));
        if(config.getBool("Using public screen", false))
            PublicScreen::instance()->openPublicScreen("Spotless", "Spotless - Copyright (c) 2020, 2022 by Alfkil Thorbjoern Wennermark");
        // vector<Widget *> windows = getAllPanelWidgets();
        // for(int i = 0; i < windows.size(); i++) {
        //     mainMenu->setWindowSelected(windows[i]->name(), config.getBool(windows[i]->name(), "Window selected", true));
        // }
        menu->setAskArguments(config.getBool("Ask for arguments", true));
    } //to not interfere with below:

    if(isSplit()) {
        showSplit();    
    } else {
        openWindow();
    }

    bool result = waitForClose();

    { // to not interfere with the above
        Config config("config.prefs");
        config.setBool("Using public screen", PublicScreen::usingPublicScreen());
        // vector<Widget*> windows = getAllPanelWidgets();
        // for(int i = 0; i < windows.size(); i++) {
        //     config.setBool(windows[i]->name(), "Window selected", mainMenu->getWindowSelected(windows[i]->name()));
        // }
        config.setBool("Ask for arguments", menu->getAskArguments());
    }

    return result;
}

// void Spotless::trapHandler() {
//     if(spotless) {
//         spotless->debugger.suspendBreaks();
//         spotless->updateAll();
//         Console::write(PublicScreen::PENTYPE_EVENT, "At break : " + spotless->debugger.printLocation());
//     }
// }

void Spotless::portHandler() {
    if(spotless) {
        // vector<string> messages = spotless->debugger.getMessages();
        // for(int i = 0; i < messages.size(); i++)
        //     Console::write(PublicScreen::PENTYPE_INFO, messages[i]);
        // if(spotless->debugger.isDead()) 
        if(spotless->debugger.handleMessages()) { // if trap or exception
                spotless->debugger.suspendBreaks();
                spotless->console->write(PublicScreen::PENTYPE_EVENT, "At break : " + spotless->debugger.printLocation());
        }
        // if(!spotless->debugger.lives()) {
        //     spotless->clearAll();
        //     spotless->childLives = false;
        // }
        if(!spotless->debugger.lives()) {
            spotless->clearAll();
            // spotless->childLives = false;
        }
        if(spotless->debugger.lives())
            spotless->updateAll();
    }
}

// void Spotless::pipeHandler() {
//     if(spotless) {
//         vector<string> output = spotless->debugger.emptyPipe();
//         for(int i = 0; i < output.size(); i++)
//             Console::write(PublicScreen::PENTYPE_OUTPUT, "--] " + formatRawString(output[i]));
//     }
// }

// void Spotless::deathHandler() {
//     if(spotless) {
//         spotless->debugger.handleDeath();
//         spotless->updateAll();
//     }
// }

bool Spotless::handleEvent(Event *event, bool *exit) {
    if(event->eventClass() == Event::CLASS_KeyPress) {
        if(event->elementId() == RAWKEY_F1) {
                            debugger.justGo();
        // actions->update();

        }
        if(event->elementId() == RAWKEY_F2) {
                            debugger.stop();
            // actions->update();
        }
    }
    return false;
}

void Spotless::updateAll(bool doSources) {
    actions->update();
    code->update();
    context->update();
    stacktrace->update();
    disassembler->update();
    registers->update();
    console->update();
    configure->update();

    /*if(doSources)*/ sources->update(); /* this is normally done in Actions.hpp, except for when doing iconify */
    sources->showCurrent();

    if(memorySurfer->open()) memorySurfer->update();
}

void Spotless::clearAll() {
    actions->clear();
    code->clear();
    context->clear();
    sources->clear();
    stacktrace->clear();
    disassembler->clear();
    debugger.clear();
    configure->clear();

    if(memorySurfer->open()) memorySurfer->clear();
}