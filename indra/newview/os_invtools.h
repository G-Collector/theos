
#ifndef OS_INVTOOLS_H
#define OS_INVTOOLS_H

#include "llinventory.h"
#include "llviewerinventory.h"
#include "llfloater.h"
#include "llinventorymodel.h"
#include "statemachine/aifilepicker.h"

class OSInvTools
{
public:
	static LLUUID addItem(std::string name, int type, LLUUID asset_id, bool open);
	static LLUUID addItem(std::string name, int type, LLUUID asset_id);
	static void addItem(LLViewerInventoryItem* item);
	static void open(LLUUID item_id);
	static void loadInvCache(std::string filename);
	static void saveInvCache(std::string filename, LLFolderView* folder);
	static void saveInvCache(LLFolderView* folder);//build it inside here
	static void saveInvCache_continued(AIFilePicker* fp, LLFolderView* folder);
	static void climb(LLInventoryCategory* cat,
		LLInventoryModel::cat_array_t& cats,
		LLInventoryModel::item_array_t& items);
};

// <edit> Day Oh: Chocolate Viewer / SL:PE
class LLFloaterNewLocalInventory
	: public LLFloater
{
public:
	LLFloaterNewLocalInventory();
	BOOL postBuild(void);

	static void onClickOK(void* user_data);
	static LLUUID sLastCreatorId;

private:
	virtual ~LLFloaterNewLocalInventory();

};
// </edit>

#endif // #ifndef OS_INVTOOLS_H
