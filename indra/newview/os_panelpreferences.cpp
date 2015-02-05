#include "llviewerprecompiledheaders.h"

#include "os_panelpreferences.h"

#include "llpanel.h"
#include "lluictrl.h"
#include "llcombobox.h"
#include "llcheckboxctrl.h"
#include "lllineeditor.h"
#include "lluictrlfactory.h"

OSPanelPreferences::OSPanelPreferences()
{
	LLUICtrlFactory::getInstance()->buildPanel(this, "os_panel_preferences.xml");
}

OSPanelPreferences::~OSPanelPreferences()
{
}
