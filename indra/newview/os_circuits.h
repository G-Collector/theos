#ifndef OS_CIRCUITS_H
#define OS_CIRCUITS_H

#include "llsingleton.h"
#include "llviewerregion.h"

class OSCircuits : public LLSingleton<OSCircuits>
{
	friend class LLSingleton<OSCircuits>;
protected:
	OSCircuits();
	~OSCircuits();
public:
	void notifyEnabled(LLViewerRegion* regionp);
	void notifyDisabled(LLViewerRegion* regionp);
};

#endif
