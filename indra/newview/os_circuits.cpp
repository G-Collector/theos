#include "llviewerprecompiledheaders.h"

#include "os_circuits.h"

#include "llnotificationsutil.h"
#include "llfloaterchat.h"
#include "llchat.h"

OSCircuits::OSCircuits()
{
}

OSCircuits::~OSCircuits()
{
}

void OSCircuits::notifyEnabled(LLViewerRegion* regionp)
{
	static LLCachedControl<bool> we_want_notifications(gSavedSettings, "OSNotifyEnabledSimulator");
	if (!we_want_notifications) return;
	if (!regionp)
	{
		llwarns << "OSCircuits::notifyEnabled() region is invalid!" << llendl;
		return;
	}
	LLHost host = regionp->getHost();
	LLChat chat;
	chat.mText = llformat("Added region '%s' - Host:%s Colo:%s ClassID:%i cpuRatio:%i SKU:%s Type:%s",
		regionp->getName().c_str(),
		regionp->getHost().getIPandPort().c_str(),
		regionp->getSimColoName().c_str(),
		regionp->getSimClassID(),
		regionp->getSimCPURatio(),
		regionp->getSimProductSKU().c_str(),
		regionp->getLocalizedSimProductName().c_str());
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	LLFloaterChat::addChat(chat);
}

void OSCircuits::notifyDisabled(LLViewerRegion* regionp)
{
	static LLCachedControl<bool> we_want_notifications(gSavedSettings, "OSNotifyDisabledSimulator");
	if (!we_want_notifications) return;
	if (!regionp)
	{
		llwarns << "OSCircuits::notifyDisabled() region is invalid!" << llendl;
		return;
	}
	LLHost host = regionp->getHost();
	LLChat chat;
	chat.mText = llformat("Removed region '%s' - Host:%s Colo:%s ClassID:%i cpuRatio:%i SKU:%s Type:%s",
		regionp->getName().c_str(),
		regionp->getHost().getIPandPort().c_str(),
		regionp->getSimColoName().c_str(),
		regionp->getSimClassID(),
		regionp->getSimCPURatio(),
		regionp->getSimProductSKU().c_str(),
		regionp->getLocalizedSimProductName().c_str());
	chat.mSourceType = CHAT_SOURCE_SYSTEM;
	LLFloaterChat::addChat(chat);
}
