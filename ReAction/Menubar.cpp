#include <intuition/menuclass.h>

#include <string.h>

#include "Menubar.hpp"
#include "Widget.hpp"

Menubar::Menubar(Widget *parent)
    : menu(0),
    created(false)
{
    this->parent = parent;
    createMenuStrip();
    // this->doScreenSwitch = false;
}

Menubar::~Menubar()
{
    destroyMenuStrip();
}

void Menubar::createMenuStrip () {
    menu = IIntuition->NewObject(NULL, "menuclass", MA_Type, T_ROOT, TAG_END);
    // created = true;
}

void Menubar::destroyMenuStrip () {
    if (menu) IIntuition->DisposeObject(menu);
    menu = 0;
    created = false;
}

void Menubar::attach() {
    if(parent && parent->windowPointer()) IIntuition->SetMenuStrip(parent->topLevelParent()->windowPointer(), (struct Menu *)menu);
}

void Menubar::detach() {
    if(parent && parent->windowPointer()) IIntuition->ClearMenuStrip(parent->topLevelParent()->windowPointer());
}

Object *Menubar::addCreateMenu (string label) {
    detach();
    Object *panel = IIntuition->NewObject(NULL, "menuclass",
        MA_Type, 		T_MENU,
        MA_Label,		strdup(label.c_str()), //hiddeous hack
        TAG_END);

    if (menu) IIntuition->SetAttrs (menu, MA_AddChild, panel, TAG_END);
    attach();
    
    return panel;
}

void Menubar::addCreateMenuItem (Object *panel, string label, string shortCut, int itemId) {
    detach();
    Object *item = IIntuition->NewObject(NULL, "menuclass",
        MA_ID,			                            itemId,
        MA_Type, 		                            T_ITEM,
        MA_Label,		                            strdup(label.c_str()),
        shortCut.length() ? MA_Key : TAG_IGNORE,    strdup(shortCut.c_str()),
        TAG_END);

    if (item) IIntuition->SetAttrs (panel, MA_AddChild, item, TAG_END);
    attach();
}
