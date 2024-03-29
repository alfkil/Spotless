#include "Menubar.hpp"
#include "Event.hpp"
#include "Layout.hpp"
#include "Widget.hpp"
#include "Speedbar.hpp"
#include "reaction.h"
#include "Screen.hpp"
#include "Config.hpp"
#include "MainWindow.hpp"

#include <libraries/keymap.h>

//iconify
#include <proto/icon.h>
#include <proto/wb.h>

#include <string.h>
#include <iostream>
#include <algorithm>

unsigned int Widget::gadgetId = 1;
list<Object *> Widget::children; //We need this to delegate input events
list<Widget *> Widget::openedWindows;

Widget::Widget(Widget *parentWidget)
	:	object(0),
		window(0),
		isOpen(false),
		parentLayout(0),
		mainMenu(0),
		// gadgetId(1),
		appPort(0)
{
	this->parentWidget = parentWidget;
}

Widget::~Widget()
{
    if(window) closeWindow();
	handlers.clear();
}

bool Widget::openWindow()
{
	if (window) return true;

	struct Screen *publicScreen = PublicScreen::instance()->screenPointer();

	Config config("config.prefs");

	object = WindowObject,
		WA_PubScreen,		publicScreen,
		WA_ScreenTitle,		"Spotless",
		WA_Title,			widgetName.size() ? widgetName.c_str() : "Spotless",
 		WA_DepthGadget,		TRUE,
		WA_SizeGadget,		TRUE,
		WA_DragBar,			TRUE,
		WA_CloseGadget,		TRUE,
		WA_Activate,		TRUE,
		WA_IDCMP,			IDCMP_RAWKEY,

		WA_Top,				config.getValue(widgetName, "Top", 10),
		WA_Left,			config.getValue(widgetName, "Left", 0),
		WA_Width,			config.getValue(widgetName, "Width", 320),
		WA_Height,			config.getValue(widgetName, "Height", 240),

		// WINDOW_Position,	WPOS_CENTERSCREEN,
		WINDOW_ParentLayout,	createContent(),
		WINDOW_MenuStrip,		mainMenu ? mainMenu->systemObject() : 0,
        WINDOW_GadgetHelp,      TRUE,

	EndWindow;
	if(!object) cout << "Failed to create object.\n";
	if (object) window = (struct Window *) RA_OpenWindow (object);
	if(!window) cout << "Failed to open window\n";
	openedWindows.push_back(this);

	if(window) isOpen = true;

	if(mainMenu) mainMenu->update();

	return window != 0;
}

Object *Widget::createContent()
{
    parentLayout = new Layout(this, Layout::LAYOUT_Vertical);
	createGuiObject(parentLayout);
	return parentLayout->systemObject();
}

void Widget::destroyContent()
{
	delete parentLayout;
	parentLayout = 0;
}

void Widget::setMenubar(Menubar *menu)
{
	mainMenu = menu;
}

void Widget::closeWindow ()
{
	Config config("config.prefs");

	uint32 top, left, width, height;

	IIntuition->GetWindowAttrs (window,
		WA_Top, &top,
		WA_Left, &left,
		WA_Width, &width,
		WA_Height, &height,
		TAG_DONE);
	config.setValue(widgetName, "Top", top);
	config.setValue(widgetName, "Left", left);
	config.setValue(widgetName, "Width", width);
	config.setValue(widgetName, "Height", height);

	if (object) IIntuition->DisposeObject (object);
	object = 0;
	window = 0;
	isOpen = false;

	destroyContent();
	
	openedWindows.remove(this);

	if(mainMenu) mainMenu->update();
}

void Widget::closeAllWindows()
{
	if(!window) return;
	closeWindow();
	list<Widget *> windows = openedWindows;
	for (list<Widget *>::iterator it = windows.begin(); it != windows.end(); it++)
		if((*it) != this) (*it)->closeWindow();
	// destroyContent();
	if(mainMenu) mainMenu->destroyMenuStrip();
	openedWindows.clear();
	children.clear();
	RButton::clean();
	Checkbox::clean();
	Listbrowser::clean();
	RString::clean();
	Speedbar::clean();
}

extern struct MsgPort *AppPort;

int Widget::waitForClose()
{
	bool exit = false;

	while (!exit) {
		uint32 result = IExec->Wait (handlerSignals() | openedWindowsSignalMask() | SIGBREAKF_CTRL_C );

		if (result & SIGBREAKF_CTRL_C) {
			exit = true;
		}

		for(int i = 0; i < handlers.size(); i++) {
			if(result & handlers[i]->signal)
				handlers[i]->handler();
		}

		for(list<Widget *>::iterator it = openedWindows.begin(); it != openedWindows.end(); it++) {
			// if we open or close windows, we need to break out of the above loop
			bool openClose = false;

			Widget *target = (*it);
			if(!target) { cout << "target==0\n"; continue; }
			if(!target->windowPointer()) { cout << "target->windowPointer() == 0\n"; continue; }
			Object *object = target->windowObject();

			bool done = false;
			while(!exit && !openClose && !done) {

				uint32 Class;
				uint16 Code;
				Class = IIntuition->IDoMethod (object, WM_HANDLEINPUT, &Code);
				if(Class == WMHI_LASTMSG) {
					done = true;
				} else {
					switch (Class & WMHI_CLASSMASK) {
						case WMHI_CLOSEWINDOW: {
							// MainWindow *mw = dynamic_cast<MainWindow *>(this);
							// if(mw && target != this) {
							// 	if(!mw->closeExtraWindow(target))
							// 		target->closeWindow();
							// 	openClose = true;
							// 	break;
							// }
							done = true;
							openClose = true;
							if(target == this) { exit = true; }
							if(target != this) { target->closeWindow(); }
							break;
						}
						case WMHI_MENUPICK: {
							uint32 id = NO_MENU_ID;
							while ((id = IIntuition->IDoMethod(mainMenu->systemObject(),MM_NEXTSELECT,0,id)) != NO_MENU_ID) {
								mainMenu->handleMenuPick(id, &openClose, &exit);
								if(openClose) { break; }
							}
							// while(IIntuition->IDoMethod(mainMenu->systemObject(),MM_NEXTSELECT,0,id) != NO_MENU_ID)
							// 	;
							break;
						}

						case WMHI_ICONIFY : {
							if(!appPort) break;

							iconify();

							// list<Widget *> windows = openedWindows;
							// for(list<Widget *>::iterator it = windows.begin(); it != windows.end(); it++)
							// 	if((*it) != this) (*it)->closeWindow();
							// iconify();
							// result = 0x0;

							// uint32 newResult = IExec->Wait (1L << appPort->mp_SigBit);

							// for(list<Widget *>::iterator it = windows.begin(); it != windows.end(); it++)
							// 	if((*it) != this) (*it)->openWindow();
							// uniconify();

							// updateAll(true); //do sources update

							openClose = true;
							break;
						}

						case WMHI_RAWKEY :
							if ((Code & WMHI_KEYMASK) == RAWKEY_ESC) {
								openClose = true;
								exit = true;
								// done = true;
							} else {
								target->processEvent(Class, Code, &exit);
							}
							break;

						default:
							openClose = target->processEvent(Class, Code, &exit);
							break;
					}
				}
			}
			// here is the break
			if(openClose) break;
		}
	}
	// closeWindow ();
	closeAllWindows();
	return 0;
}

Widget *Widget::topLevelParent()
{
	Widget *top = parentWidget;
	while(top && top->parentWidget)
		top = top->parentWidget;
	return top ? top : this;
}

unsigned int Widget::addChild(Object *object)
{
	// cout << "addChild " << gadgetId << "\n";
	children.push_back(object);
	IIntuition->SetAttrs(object, GA_ID, gadgetId, TAG_DONE);
	return gadgetId++;
}

Object *Widget::findChild(unsigned int id)
{
	for (list<Object *>::iterator it = children.begin(); it != children.end(); it++) {
		unsigned int childId;
		IIntuition->GetAttrs ((*it), GA_ID, &childId, TAG_DONE);
		if (childId == id)
			return (*it);
	}
	return 0;
}

void Widget::setName(string name)
{
	widgetName = name;
	if(window) IIntuition->SetWindowTitles(window, name.c_str(), "Spotless");
}

void Widget::iconify()
{
	// if(object) {
	// 	RA_Iconify(object);
	// 	// cout << "RA_Iconify.\n";
	// 	window = 0;
	// }

	struct DiskObject *diskObject = IIcon->GetDiskObject("Spotless");
	if(diskObject) {
		struct AppIcon *appIcon = IWorkbench->AddAppIcon(0, 0, "Spotless", appPort, (BPTR)NULL, diskObject, TAG_END);
		if (appIcon) {
			for(list<Widget *>::iterator it = openedWindows.begin(); it != openedWindows.end(); it++)
				IIntuition->HideWindow((*it)->windowPointer());
			bool done = false;
			while (!done) {
				IExec->WaitPort(appPort);
				struct AppMessage *imsg = 0;
				while ((imsg = (struct AppMessage *)IExec->GetMsg(appPort))) {
					if (imsg->am_Type == AMTYPE_APPICON) {
						if (imsg->am_Class == AMCLASSICON_Open && imsg->am_NumArgs == 0)
							done = true;
					}
					if(imsg->am_Message.mn_ReplyPort)
						IExec->ReplyMsg((struct Message *)imsg);
				}
			}
			IWorkbench->RemoveAppIcon(appIcon);
			for(list<Widget *>::iterator it = openedWindows.begin(); it != openedWindows.end(); it++)
				IIntuition->ShowWindow((*it)->windowPointer(), WINDOW_FRONTMOST);
			IIntuition->ActivateWindow(window);
		}
		if(diskObject)
			IIcon->FreeDiskObject(diskObject);
	}

}

// void Widget::uniconify()
// {
// 	if(object)
// 		window = RA_Uniconify(object);
// }

void Widget::windowToFront ()
{
	if (window) IIntuition->WindowToFront (window);
}

uint32 Widget::windowSignalMask ()
{
	uint32 mask = 0x0;
	if (object) IIntuition->GetAttr (WINDOW_SigMask, object, &mask);
	return mask;
}

uint32 Widget::openedWindowsSignalMask()
{
	uint32 mask = 0x0;
	for (list<Widget *>::iterator it = openedWindows.begin(); it != openedWindows.end(); it++) {
		mask |= (*it)->windowSignalMask();
	}
	return mask;
}

bool Widget::processEvent (uint32 Class, uint16 Code, bool *exit)
{
	if(!window) return false; // if iconified

	int MouseX = window->MouseX;
	int MouseY = window->MouseY;

	Event *event = 0;
	Widget *parent = 0;
	switch (Class & WMHI_CLASSMASK) {
		case WMHI_RAWKEY : {
			event = new Event (Event::CLASS_KeyPress);
			event->setElementId (Code & WMHI_KEYMASK);
			event->setElementDescription ("");
		}
		break;

		case WMHI_GADGETUP: {
			// cout << "WMHI_GADGETUP\n";
			uint32 gadgetId = Class & WMHI_GADGETMASK;
				// cout << "gadgetId : " << gadgetId << "\n";
			
			Object *gadget = findChild(gadgetId);
			if(!gadget) break;

			// cout << "gadget : " << (void *) gadget << "\n";			
			IIntuition->GetAttrs(gadget,
				GA_UserData, &parent,
				TAG_DONE);

			// cout << "parent : " << (void *)parent << "\n";

			if(Checkbox::isCheckbox(gadget)) {
				event = new Event (Event::CLASS_CheckboxPress);
				event->setElementId (gadgetId);
				event->setElementDescription ("");
			} else if(RButton::isButton(gadget)) {
				// cout << "isButton().\n";
				// char *text;

				// IIntuition->GetAttrs(gadget,
				// 	GA_Text, &text,
				// TAG_DONE);

				event = new Event (Event::CLASS_ButtonPress);
				event->setElementId (gadgetId);
				event->setElementDescription ("");
			} else if(RString::isString(gadget)) {
				// cout << "isString()\n";
				char *text;

				IIntuition->GetAttrs(gadget,
					STRINGA_TextVal, &text,
				TAG_DONE);

				event = new Event (Event::CLASS_StringEntry);
				event->setElementId (gadgetId);
				event->setElementDescription(text);
			} else if(Listbrowser::isListbrowser(gadget)) {
				// cout << "isListbrowser()\n";
				uint32 relEvent, selected;

				IIntuition->GetAttrs(gadget,
					LISTBROWSER_RelEvent,	&relEvent,
					LISTBROWSER_Selected,	&selected,
				TAG_DONE);
			
				event = new Event(Event::CLASS_SelectNode);

				if (relEvent == LBRE_CHECKED)
					event->setEventClass (Event::CLASS_CheckboxCheck);
				else if (relEvent == LBRE_UNCHECKED)
					event->setEventClass (Event::CLASS_CheckboxUncheck);

				event->setElementId(gadgetId);
				event->setItemId (selected);
			} else if(Speedbar::isSpeedbar(gadget)) {
				// cout << "isSpeedbar()\n";
				event = new Event (Event::CLASS_ActionButtonPress);
				event->setElementId (Code);
			}
			break;
		}
		case WMHI_MOUSEBUTTONS: {
			event = new Event(Event::CLASS_MouseButtonDown);
			event->setMousePosition (MouseX, MouseY);
			break;
		}
		case WMHI_MOUSEMOVE: {
			event = new Event(Event::CLASS_MouseMove);
			event->setMousePosition (MouseX, MouseY);
			break;
		}
	}
	bool result = false;
	if (event) {
		result = parent ? parent->handleEvent(event, exit) : handleEvent(event, exit);
		delete event;
	}
	return result;
}