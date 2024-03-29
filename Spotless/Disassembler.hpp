#ifndef SPOTLESS_DISASSEMBLER_H
#define SPOTLESS_DISASSEMBLER_H

#include <proto/exec.h>

#include "../ReAction/classes.h"
#include "../SimpleDebug/Strings.hpp"
#include "Spotless.hpp"
#include "MemorySurfer.hpp"

class Disassembler : public Widget {
private:
    Spotless *spotless;
    Listbrowser *listbrowser;
    RButton *asmBackSkip, *asmStep, *asmSkip, *memSurf;
    
public:
    Disassembler(Spotless *spotless) : Widget() { setName("Disassembler"); this->spotless = spotless; }
    void createGuiObject(Layout *layout) {
                layout->setParent(this);
        Layout *hLayout = layout->createHorizontalLayout();
        listbrowser = hLayout->createListbrowser();
        Layout *buttonLayout = hLayout->createVerticalLayout(100, 0);
        asmBackSkip = buttonLayout->createButton("Back skip", "scrollstart");
        asmStep = buttonLayout->createButton("Asm step", "scrolldown");
        asmSkip = buttonLayout->createButton("Asm skip", "scrollend");
        buttonLayout->createSpace();
        memSurf = buttonLayout->createButton("Memory surfer", "scrollpan");
    } 
    void update() {
        if(!open()) return;
        clear();
        vector<string> disassembly = spotless->debugger.disassemble();
        listbrowser->detach();
        for(int i = 0; i < disassembly.size(); i++)
            listbrowser->addNode(disassembly[i]);
        listbrowser->attach();
        listbrowser->focus(spotless->debugger.getDisassebmlyLine());
    }
    void clear() {
        if(!open()) return;
        listbrowser->clear();
    }
    unsigned int getAsmStepId() {
        return asmStep->getId();
    }
    unsigned int getAsmSkipId() {
        return asmSkip->getId();
    }
    unsigned int getAsmBackSkipId() {
        return asmBackSkip->getId();
    }
    unsigned int getMemSurfId() {
        return memSurf->getId();
    }

    bool handleEvent(Event *event, bool *exit) {

        if(event->eventClass() == Event::CLASS_ButtonPress) {
            if(event->elementId() == spotless->disassembler->getAsmBackSkipId()) {
                // this is inherently unsafe
                spotless->debugger.backSkip();
                spotless->disassembler->update();
            }
            if(event->elementId() == spotless->disassembler->getAsmStepId()) {
                spotless->debugger.safeStep();
                spotless->disassembler->update();
                spotless->sources->update();
                spotless->context->update();
            }
            if(event->elementId() == spotless->disassembler->getAsmSkipId()) {
                // this is inherently unsafe
                spotless->debugger.skip();
                spotless->disassembler->update();
            }
            if(event->elementId() == spotless->disassembler->getMemSurfId()) {
                if(spotless->memorySurfer) {
                    // spotless->openExtraWindow(spotless->memorySurfer);
                    spotless->memorySurfer->openWindow();
                    spotless->memorySurfer->update();
                }
            }
        }
        return false;
    }

};
#endif