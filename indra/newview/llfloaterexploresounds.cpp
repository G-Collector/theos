// <edit>

#include "llviewerprecompiledheaders.h"

#include "llfloaterexploresounds.h"

#include "llscrolllistctrl.h"
#include "llscrolllistitem.h"
#include "lluictrlfactory.h"

#include "llagent.h"
#include "llagentcamera.h"
#include "llfloaterblacklist.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
//<os>
#include "os_invtools.h"
#include "llwindow.h"
#include "llviewerwindow.h"
//</os>
static const size_t num_collision_sounds = 29;
const LLUUID collision_sounds[num_collision_sounds] =
{
	LLUUID("dce5fdd4-afe4-4ea1-822f-dd52cac46b08"),
	LLUUID("51011582-fbca-4580-ae9e-1a5593f094ec"),
	LLUUID("68d62208-e257-4d0c-bbe2-20c9ea9760bb"),
	LLUUID("75872e8c-bc39-451b-9b0b-042d7ba36cba"),
	LLUUID("6a45ba0b-5775-4ea8-8513-26008a17f873"),
	LLUUID("992a6d1b-8c77-40e0-9495-4098ce539694"),
	LLUUID("2de4da5a-faf8-46be-bac6-c4d74f1e5767"),
	LLUUID("6e3fb0f7-6d9c-42ca-b86b-1122ff562d7d"),
	LLUUID("14209133-4961-4acc-9649-53fc38ee1667"),
	LLUUID("bc4a4348-cfcc-4e5e-908e-8a52a8915fe6"),
	LLUUID("9e5c1297-6eed-40c0-825a-d9bcd86e3193"),
	LLUUID("e534761c-1894-4b61-b20c-658a6fb68157"),
	LLUUID("8761f73f-6cf9-4186-8aaa-0948ed002db1"),
	LLUUID("874a26fd-142f-4173-8c5b-890cd846c74d"),
	LLUUID("0e24a717-b97e-4b77-9c94-b59a5a88b2da"),
	LLUUID("75cf3ade-9a5b-4c4d-bb35-f9799bda7fb2"),
	LLUUID("153c8bf7-fb89-4d89-b263-47e58b1b4774"),
	LLUUID("55c3e0ce-275a-46fa-82ff-e0465f5e8703"),
	LLUUID("24babf58-7156-4841-9a3f-761bdbb8e237"),
	LLUUID("aca261d8-e145-4610-9e20-9eff990f2c12"),
	LLUUID("0642fba6-5dcf-4d62-8e7b-94dbb529d117"),
	LLUUID("25a863e8-dc42-4e8a-a357-e76422ace9b5"),
	LLUUID("9538f37c-456e-4047-81be-6435045608d4"),
	LLUUID("8c0f84c3-9afd-4396-b5f5-9bca2c911c20"),
	LLUUID("be582e5d-b123-41a2-a150-454c39e961c8"),
	LLUUID("c70141d4-ba06-41ea-bcbc-35ea81cb8335"),
	LLUUID("7d1826f4-24c4-4aac-8c2e-eff45df37783"),
	LLUUID("063c97d3-033a-4e9b-98d8-05c8074922cb"),
	LLUUID("e8af4a28-aa83-4310-a7c4-c047e15ea0df") //Step sound
};

LLFloaterExploreSounds* LLFloaterExploreSounds::sInstance;

LLFloaterExploreSounds::LLFloaterExploreSounds()
:	LLFloater(), LLEventTimer(0.25f)
{
	LLFloaterExploreSounds::sInstance = this;
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_explore_sounds.xml");
}

LLFloaterExploreSounds::~LLFloaterExploreSounds()
{
	LLFloaterExploreSounds::sInstance = NULL;
}

// static
void LLFloaterExploreSounds::toggle()
{
	if(LLFloaterExploreSounds::sInstance) LLFloaterExploreSounds::sInstance->close(FALSE);
	else LLFloaterExploreSounds::sInstance = new LLFloaterExploreSounds();
}

BOOL LLFloaterExploreSounds::visible()
{
	if (LLFloaterExploreSounds::sInstance)
		return TRUE;
	return FALSE;
}

void LLFloaterExploreSounds::close(bool app_quitting)
{
	LLFloater::close(app_quitting);
}

BOOL LLFloaterExploreSounds::postBuild(void)
{
	LLScrollListCtrl* soundlist = getChild<LLScrollListCtrl>("sound_list");
	soundlist->setDoubleClickCallback(boost::bind(&LLFloaterExploreSounds::handle_play_locally,this));
	soundlist->sortByColumn("playing", TRUE);

	childSetAction("play_locally_btn", handle_play_locally, this);
	childSetAction("look_at_btn", handle_look_at, this);
	childSetAction("stop_btn", handle_stop, this);
	childSetAction("bl_btn", blacklistSound, this);

	// <os>
	childSetAction("play_in_world", handle_play_in_world, this);
	childSetAction("play_ambient_btn", handle_play_ambient, this);
	childSetAction("open_btn", handle_open, this);
	childSetAction("copy_uuid_btn", handle_copy_uuid, this);
	// </os>

	return TRUE;
}

LLSoundHistoryItem LLFloaterExploreSounds::getItem(LLUUID itemID)
{
	if(gSoundHistory.find(itemID) != gSoundHistory.end())
		return gSoundHistory[itemID];
	else
	{
		// If log is paused, hopefully we can find it in mLastHistory
		std::list<LLSoundHistoryItem>::iterator iter = mLastHistory.begin();
		std::list<LLSoundHistoryItem>::iterator end = mLastHistory.end();
		for( ; iter != end; ++iter)
		{
			if((*iter).mID == itemID) return (*iter);
		}
	}
	LLSoundHistoryItem item;
	item.mID = LLUUID::null;
	return item;
}

class LLSoundHistoryItemCompare
{
public:
	bool operator() (LLSoundHistoryItem first, LLSoundHistoryItem second)
	{
		if(first.isPlaying())
		{
			if(second.isPlaying())
			{
				return (first.mTimeStarted > second.mTimeStarted);
			}
			else
			{
				return true;
			}
		}
		else if(second.isPlaying())
		{
			return false;
		}
		else
		{
			return (first.mTimeStopped > second.mTimeStopped);
		}
	}
};

// static
BOOL LLFloaterExploreSounds::tick()
{
	//if(childGetValue("pause_chk").asBoolean()) return FALSE;

	bool show_collision_sounds = childGetValue("collision_chk").asBoolean();
	bool show_repeated_assets = childGetValue("repeated_asset_chk").asBoolean();
	bool show_avatars = childGetValue("avatars_chk").asBoolean();
	bool show_objects = childGetValue("objects_chk").asBoolean();

	std::list<LLSoundHistoryItem> history;
	if(childGetValue("pause_chk").asBoolean())
	{
		history = mLastHistory;
	}
	else
	{
		std::map<LLUUID, LLSoundHistoryItem>::iterator map_iter = gSoundHistory.begin();
		std::map<LLUUID, LLSoundHistoryItem>::iterator map_end = gSoundHistory.end();
		for( ; map_iter != map_end; ++map_iter)
		{
			history.push_back((*map_iter).second);
		}
		LLSoundHistoryItemCompare c;
		history.sort(c);
		mLastHistory = history;
	}

	LLScrollListCtrl* list = getChild<LLScrollListCtrl>("sound_list");

	// Save scroll pos and selection so they can be restored
	S32 scroll_pos = list->getScrollPos();
	uuid_vec_t selected_ids = list->getSelectedIDs();
	list->clearRows();

	std::list<LLUUID> unique_asset_list;

	std::list<LLSoundHistoryItem>::iterator iter = history.begin();
	std::list<LLSoundHistoryItem>::iterator end = history.end();
	for( ; iter != end; ++iter)
	{
		LLSoundHistoryItem item = (*iter);

		bool is_avatar = item.mOwnerID == item.mSourceID;
		if(is_avatar && !show_avatars) continue;

		bool is_object = !is_avatar;
		if(is_object && !show_objects) continue;

		bool is_repeated_asset = std::find(unique_asset_list.begin(), unique_asset_list.end(), item.mAssetID) != unique_asset_list.end();
		if(is_repeated_asset && !show_repeated_assets) continue;

		if(!item.mReviewed)
		{
			item.mReviewedCollision	= std::find(&collision_sounds[0], &collision_sounds[num_collision_sounds], item.mAssetID) != &collision_sounds[num_collision_sounds];
			item.mReviewed = true;
		}
		bool is_collision_sound = item.mReviewedCollision;
		if(is_collision_sound && !show_collision_sounds) continue;

		unique_asset_list.push_back(item.mAssetID);

		LLSD element;
		element["id"] = item.mID;

		LLSD& playing_column = element["columns"][0];
		playing_column["column"] = "playing";
		if(item.isPlaying())
		{
			playing_column["value"] = item.mIsLooped ? " Looping" : " Playing";
		}
		else
		{
			S32 time = (LLTimer::getElapsedSeconds() - item.mTimeStopped);
			S32 hours = time / 3600;
			S32 mins = time / 60;
			S32 secs = time % 60;
			playing_column["value"] = llformat("%d:%02d:%02d", hours,mins,secs);
		}

		LLSD& type_column = element["columns"][1];
		type_column["column"] = "type";
		type_column["type"] = "icon";
		if(item.mType == LLAudioEngine::AUDIO_TYPE_UI)
		{
			// this shouldn't happen for now, as UI is forbidden in the log
			type_column["value"] = "UI";
		}
		else
		{
			std::string type;
			std::string icon;
			if(is_avatar)
			{
				type = "Avatar";
				icon = "inv_item_gesture.tga";
			}
			else
			{
				if(item.mIsTrigger)
				{
					type = "llTriggerSound";
					icon = "inv_item_sound.tga";
				}
				else
				{
					if(item.mIsLooped)
						type = "llLoopSound";
					else
						type = "llPlaySound";
					icon = "inv_item_object.tga";
				}
			}

			type_column["value"] = icon;
			type_column["tool_tip"] = type;
		}

		LLSD& owner_column = element["columns"][2];
		owner_column["column"] = "owner";
		std::string fullname;
		BOOL is_group;
		if(gCacheName->getIfThere(item.mOwnerID, fullname, is_group))
		{
			if(is_group) fullname += " (Group)";
			owner_column["value"] = fullname;
		}
		else
			owner_column["value"] = item.mOwnerID.asString();

		LLSD& sound_column = element["columns"][3];
		sound_column["column"] = "sound";

		std::string uuid = item.mAssetID.asString();
		sound_column["value"] = uuid;// uuid.substr(0, uuid.find_first_of("-")) + "..."; // <os />

		list->addElement(element, ADD_BOTTOM);
	}

	list->selectMultiple(selected_ids);
	list->setScrollPos(scroll_pos);

	return FALSE;
}

// static
void LLFloaterExploreSounds::handle_play_locally(void* user_data)
{
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");
	std::vector<LLScrollListItem*> selection = list->getAllSelected();
	std::vector<LLScrollListItem*>::iterator selection_iter = selection.begin();
	std::vector<LLScrollListItem*>::iterator selection_end = selection.end();
	std::vector<LLUUID> asset_list;
	for( ; selection_iter != selection_end; ++selection_iter)
	{
		LLSoundHistoryItem item = floater->getItem((*selection_iter)->getValue());
		if(item.mID.isNull()) continue;
		// Unique assets only
		if(std::find(asset_list.begin(), asset_list.end(), item.mAssetID) == asset_list.end())
		{
			asset_list.push_back(item.mAssetID);
			if(gAudiop)
			gAudiop->triggerSound(item.mAssetID, LLUUID::null, 1.0f, LLAudioEngine::AUDIO_TYPE_UI);
		}
	}
}

// <os>
// static
void LLFloaterExploreSounds::handle_play_in_world(void* user_data)
{
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");
	std::vector<LLScrollListItem*> selection = list->getAllSelected();
	std::vector<LLScrollListItem*>::iterator selection_iter = selection.begin();
	std::vector<LLScrollListItem*>::iterator selection_end = selection.end();
	for (; selection_iter != selection_end; ++selection_iter)
	{
		LLSoundHistoryItem item = floater->getItem((*selection_iter)->getValue());
		if (item.mID.isNull()) continue;

		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_SoundTrigger);
		msg->nextBlockFast(_PREHASH_SoundData);
		msg->addUUIDFast(_PREHASH_SoundID, item.mAssetID);
		// Client untrusted, ids set on sim
		msg->addUUIDFast(_PREHASH_OwnerID, LLUUID::null);
		msg->addUUIDFast(_PREHASH_ObjectID, LLUUID::null);
		msg->addUUIDFast(_PREHASH_ParentID, LLUUID::null);
		msg->addU64Fast(_PREHASH_Handle, gAgent.getRegion()->getHandle());
		LLVector3 position = gAgent.getPositionAgent();
		msg->addVector3Fast(_PREHASH_Position, position);
		msg->addF32Fast(_PREHASH_Gain, 1.0f);

		gAgent.sendMessage();
	}
}

// static
void LLFloaterExploreSounds::handle_play_ambient(void* user_data)
{
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");
	std::vector<LLScrollListItem*> selection = list->getAllSelected();
	std::vector<LLScrollListItem*>::iterator selection_iter = selection.begin();
	std::vector<LLScrollListItem*>::iterator selection_end = selection.end();
	for (; selection_iter != selection_end; ++selection_iter)
	{
		LLSoundHistoryItem item = floater->getItem((*selection_iter)->getValue());
		if (item.mID.isNull()) continue;
		int gain = 0.01f;
		for (int i = 0; i < 2; i++)
		{
			LLMessageSystem* msg = gMessageSystem;
			msg->newMessageFast(_PREHASH_SoundTrigger);
			msg->nextBlockFast(_PREHASH_SoundData);
			msg->addUUIDFast(_PREHASH_SoundID, item.mAssetID);
			// Client untrusted, ids set on sim
			msg->addUUIDFast(_PREHASH_OwnerID, LLUUID::null);
			msg->addUUIDFast(_PREHASH_ObjectID, LLUUID::null);
			msg->addUUIDFast(_PREHASH_ParentID, LLUUID::null);
			msg->addU64Fast(_PREHASH_Handle, gAgent.getRegion()->getHandle());

			LLVector3d position = -from_region_handle(gAgent.getRegion()->getHandle()); // PinkiePie: What actually makes this "ambient," lol.
			msg->addVector3Fast(_PREHASH_Position, (LLVector3)position);

			msg->addF32Fast(_PREHASH_Gain, 1.0f);
			msg->sendReliable(gAgent.getRegionHost());

			gain = 1.0f;
		}
	}
}

// static
void LLFloaterExploreSounds::handle_open(void* user_data)
{
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");
	std::vector<LLScrollListItem*> selection = list->getAllSelected();
	std::vector<LLScrollListItem*>::iterator selection_iter = selection.begin();
	std::vector<LLScrollListItem*>::iterator selection_end = selection.end();
	std::vector<LLUUID> asset_list;
	for (; selection_iter != selection_end; ++selection_iter)
	{
		LLSoundHistoryItem item = floater->getItem((*selection_iter)->getValue());
		if (item.mID.isNull()) continue;
		// Unique assets only
		if (std::find(asset_list.begin(), asset_list.end(), item.mAssetID) == asset_list.end())
		{
			asset_list.push_back(item.mAssetID);
			LLUUID inv_item = OSInvTools::addItem(item.mAssetID.asString(), LLAssetType::AT_SOUND, item.mAssetID, true);
		}
	}
}

// static
void LLFloaterExploreSounds::handle_copy_uuid(void* user_data)
{
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");
	LLUUID selection = list->getSelectedValue().asUUID(); // Single item only
	LLSoundHistoryItem item = floater->getItem(selection);
	if (item.mID.isNull()) return;
	gViewerWindow->getWindow()->copyTextToClipboard(utf8str_to_wstring(item.mAssetID.asString()));
}
// </os>

// static
void LLFloaterExploreSounds::handle_look_at(void* user_data)
{
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");
	LLUUID selection = list->getSelectedValue().asUUID();
	LLSoundHistoryItem item = floater->getItem(selection); // Single item only
	if(item.mID.isNull()) return;

	LLVector3d pos_global = item.mPosition;

	// Try to find object position
	if(item.mSourceID.notNull())
	{
		LLViewerObject* object = gObjectList.findObject(item.mSourceID);
		if(object)
		{
			pos_global = object->getPositionGlobal();
		}
	}

	// Move the camera
	// Find direction to self (reverse)
	LLVector3d cam = gAgent.getPositionGlobal() - pos_global;
	cam.normalize();
	// Go 4 meters back and 3 meters up
	cam *= 4.0f;
	cam += pos_global;
	cam += LLVector3d(0.f, 0.f, 3.0f);

	gAgentCamera.setFocusOnAvatar(FALSE, FALSE);
	gAgentCamera.setCameraPosAndFocusGlobal(cam, pos_global, item.mSourceID);
	gAgentCamera.setCameraAnimating(FALSE);
}

// static
void LLFloaterExploreSounds::handle_stop(void* user_data)
{
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");
	std::vector<LLScrollListItem*> selection = list->getAllSelected();
	std::vector<LLScrollListItem*>::iterator selection_iter = selection.begin();
	std::vector<LLScrollListItem*>::iterator selection_end = selection.end();
	std::vector<LLUUID> asset_list;
	for( ; selection_iter != selection_end; ++selection_iter)
	{
		LLSoundHistoryItem item = floater->getItem((*selection_iter)->getValue());
		if(item.mID.isNull()) continue;
		if(item.isPlaying())
		{
			item.mAudioSource->play(LLUUID::null);
		}
	}
}

void LLFloaterExploreSounds::blacklistSound(void* user_data)
{
	LLFloaterBlacklist::show();
	LLFloaterExploreSounds* floater = (LLFloaterExploreSounds*)user_data;
	LLScrollListCtrl* list = floater->getChild<LLScrollListCtrl>("sound_list");

	typedef std::vector<LLScrollListItem*> item_map_t;
	item_map_t selection = list->getAllSelected();
	item_map_t::iterator selection_end = selection.end();
	
	for(item_map_t::iterator selection_iter = selection.begin(); selection_iter != selection_end; ++selection_iter)
	{
		LLSoundHistoryItem item = floater->getItem((*selection_iter)->getValue());
		if(item.mID.isNull()) continue;

		LLSD sound_data;
		std::string agent;
		gCacheName->getFullName(item.mOwnerID, agent);
		LLViewerRegion* cur_region = gAgent.getRegion();

		if(cur_region)
		  sound_data["entry_name"] = llformat("Sound played by %s in region %s",agent.c_str(),cur_region->getName().c_str());
		else
		  sound_data["entry_name"] = llformat("Sound played by %s",agent.c_str());
		sound_data["entry_type"] = (LLAssetType::EType)item.mType;
		sound_data["entry_agent"] = gAgent.getID();
		LLFloaterBlacklist::addEntry(item.mAssetID,sound_data);
	}
}

// </edit>
