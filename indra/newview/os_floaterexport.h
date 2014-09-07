/** 
* @file os_floaterexport.h
* Simms - 2014
* $/LicenseInfo$
*/

#ifndef OS_LLFLOATEREXPORT_H
#define OS_LLFLOATEREXPORT_H

#include "llfloater.h"
#include "llselectmgr.h"
#include "llvoavatar.h"
#include "llavatarappearancedefines.h"
#include "statemachine/aifilepicker.h"

class LLScrollListCtrl;
class OSExportable
{
	enum EXPORTABLE_TYPE
	{
		EXPORTABLE_OBJECT,
		EXPORTABLE_WEARABLE
	};

public:
	OSExportable(LLViewerObject* object, std::string name, std::map<U32,std::pair<std::string, std::string> >& primNameMap);
	OSExportable(LLVOAvatar* avatar, LLWearableType::EType type, std::map<U32,std::pair<std::string, std::string> >& primNameMap);

	LLSD asLLSD();

	EXPORTABLE_TYPE mType;
	LLWearableType::EType mWearableType;
	LLViewerObject* mObject;
	LLVOAvatar* mAvatar;
	std::map<U32,std::pair<std::string, std::string> >* mPrimNameMap;
};

class OSFloaterExport : public LLFloater, public LLFloaterSingleton<OSFloaterExport>
{
	friend class LLUISingleton<OSFloaterExport, VisibilityPolicy<LLFloater> >;
public:

	void onOpen();
	virtual BOOL postBuild();

	virtual void draw();
	virtual void refresh();

	void updateSelection();
	void updateAvatarList();
	void updateNamesProgress();
	void onClickSelectAll();
	void onClickSelectObjects();
	void onClickSelectWearables();
	void onClickMakeCopy();
	void onClickSaveAs();

	void addObjectStuff();
	void addAvatarStuff(LLVOAvatar* avatarp);
	void receivePrimName(LLViewerObject* object, std::string name, std::string desc);

	static void onClickSaveAs_Callback(OSFloaterExport* floater, AIFilePicker* filepicker);
	static void receiveObjectProperties(LLUUID fullid, std::string name, std::string desc);
	static std::vector<OSFloaterExport*> instances; // for callback-type use

	LLSD getLLSD();
	std::vector<U32> mPrimList;
	LLScrollListCtrl* mExportList;
	std::map<U32, std::pair<std::string, std::string> > mPrimNameMap;
private:
	std::map<LLViewerObject*, bool> avatars;
	LLUUID mObjectID;
	bool mIsAvatar;
	bool mDirty;
	void dirty();

	OSFloaterExport(const LLSD&);
	virtual ~OSFloaterExport(void);

	void addToPrimList(LLViewerObject* object);

	enum LIST_COLUMN_ORDER
	{
		LIST_CHECKED,
		LIST_TYPE,
		LIST_NAME,
		LIST_AVATARID
	};

	std::map<LLUUID, LLSD> mExportables;
	LLSafeHandle<LLObjectSelection> mObjectSelection;
};

#endif //OS_FLOATEREXPORT_H
