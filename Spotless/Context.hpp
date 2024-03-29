#ifndef SPOTLESS_VARIABLES_H
#define SPOTLESS_VARIABLES_H

#include "../ReAction/classes.h"
#include "Spotless.hpp"

#include "../SimpleDebug/Strings.hpp"

class Context : public Widget {
private:
    Spotless *spotless;
    Listbrowser *listbrowser;
    Checkbox *globalsCheckbox;

    bool showGlobals;
public:
    Context(Spotless *spotless) : Widget(), showGlobals(false) { setName("Variables"); this->spotless = spotless; }
    void createGuiObject(Layout *layout) {
                layout->setParent(this);

        Layout *vertical = layout->createVerticalLayout();
        listbrowser = vertical->createListbrowser();
        listbrowser->setHierachical(true);
        globalsCheckbox = vertical->createCheckbox("Show globals", showGlobals);
    }
    void add(vector<string> context, int generation) {
        if(!open()) return;
        listbrowser->detach();
        for(int i = 0; i < context.size(); i++) {
            astream str(context[i]);
            int nextGeneration = generation;
            if(str.endsWith('{')) nextGeneration++;
            if(str.endsWith('}')) nextGeneration--;
            else listbrowser->addNode(context[i], 0, nextGeneration > generation, generation);
            generation = nextGeneration;
        }
        listbrowser->attach();
    }
    void update() {
        if(!open()) return;
        clear();
        add(spotless->debugger.context(), 1);
        if(showGlobals) add(spotless->debugger.globals(), 1);
    }
    void globals() {
        showGlobals = globalsCheckbox->getChecked();
        update();
    }
    void clear() {
        if(!open()) return;
        listbrowser->clear();
    }
    unsigned int getGlobalsId() {
        return globalsCheckbox->getId();
    }
    bool handleEvent(Event *event, bool *exit) {
        if(event->eventClass() == Event::CLASS_CheckboxPress) {
            if(event->elementId() == getGlobalsId()) {
                globals();
            }
        }
        return false;
    }
};
#endif