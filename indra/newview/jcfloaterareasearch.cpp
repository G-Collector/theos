/* Copyright (c) 2009
 *
 * Modular Systems Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *   3. Neither the name Modular Systems Ltd nor the names of its contributors
 *      may be used to endorse or promote products derived from this
 *      software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MODULAR SYSTEMS LTD AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MODULAR SYSTEMS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Modified, debugged, optimized and improved by Henri Beauchamp Feb 2010.
 */

#include "llviewerprecompiledheaders.h"

#include "jcfloaterareasearch.h"

#include "llfiltereditor.h"
#include "llscrolllistctrl.h"
#include "llscrolllistitem.h"
#include "lluictrlfactory.h"

#include "llagent.h"
#include "lltracker.h"
#include "llviewerobjectlist.h"
// <os>
#include "llscrolllistitem.h"
#include "llcombobox.h"
#include "lltrans.h"
#include "llmenugl.h"
#include "lltoolmgr.h"
#include "lltoolpie.h"
#include "lltoolcomp.h"
#include "llselectmgr.h"
#include "llviewermenu.h"
#include "llagentcamera.h"
#include "llfloatertools.h"
#include "llviewerregion.h"
// </os>
const std::string request_string = "JCFloaterAreaSearch::Requested_\xF8\xA7\xB5";
const F32 min_refresh_interval = 0.25f;	// Minimum interval between list refreshes in seconds.

JCFloaterAreaSearch::JCFloaterAreaSearch(const LLSD& data) :
	LLFloater(),
	//<os>
	mPopupMenuHandle(),
	mSelectedObjectID(LLUUID::null),
	mObjectFilter(LLCachedControl<std::string>(gSavedSettings, "ObjectAreaSearchFilter", "all")),
	//</os>
	mCounterText(0),
	mResultList(0),
	mLastRegion(0),
	mStopped(false)
{
	mLastUpdateTimer.reset();
	LLUICtrlFactory::getInstance()->buildFloater(this, "floater_area_search.xml");
}

JCFloaterAreaSearch::~JCFloaterAreaSearch()
{
}

void JCFloaterAreaSearch::close(bool app)
{
	if (app || mStopped)
	{
		LLFloater::close(app);
	}
	else
	{
		setVisible(FALSE);
	}
}

// <os>
BOOL JCFloaterAreaSearch::handleRightMouseDown(S32 x, S32 y, MASK mask)
{
	LLScrollListItem *item = mResultList->getFirstSelected();
	if (!item) return TRUE;
	mSelectedObjectID = item->getUUID();

	setFocus(TRUE);
	LLMenuGL* menu = (LLMenuGL*)mPopupMenuHandle.get();
	if (menu)
	{
		menu->buildDrawLabels();
		menu->updateParent(LLMenuGL::sMenuContainer);
		LLMenuGL::showPopup(this, menu, x, y);
	}
	return TRUE;
}
// <os>

BOOL JCFloaterAreaSearch::postBuild()
{
	mResultList = getChild<LLScrollListCtrl>("result_list");
	mResultList->setDoubleClickCallback(boost::bind(&JCFloaterAreaSearch::onDoubleClick,this));
	mResultList->sortByColumn("Name", TRUE);

	mCounterText = getChild<LLTextBox>("counter");

	getChild<LLButton>("Refresh")->setClickedCallback(boost::bind(&JCFloaterAreaSearch::onRefresh,this));
	getChild<LLButton>("Stop")->setClickedCallback(boost::bind(&JCFloaterAreaSearch::onStop,this));

	getChild<LLFilterEditor>("Name query chunk")->setCommitCallback(boost::bind(&JCFloaterAreaSearch::onCommitLine,this,_1,_2,LIST_OBJECT_NAME));
	getChild<LLFilterEditor>("Description query chunk")->setCommitCallback(boost::bind(&JCFloaterAreaSearch::onCommitLine,this,_1,_2,LIST_OBJECT_DESC));
	getChild<LLFilterEditor>("Owner query chunk")->setCommitCallback(boost::bind(&JCFloaterAreaSearch::onCommitLine,this,_1,_2,LIST_OBJECT_OWNER));
	getChild<LLFilterEditor>("Group query chunk")->setCommitCallback(boost::bind(&JCFloaterAreaSearch::onCommitLine,this,_1,_2,LIST_OBJECT_GROUP));

	// <os>
	mFilterComboBox = getChild<LLComboBox>("filter_combobox");
	mFilterComboBox->add("All Objects", std::string("all"));
	mFilterComboBox->add("All Scripted Objects", std::string("scripted"));
	mFilterComboBox->add("Scripted Touch Only", std::string("touch_only"));
	mFilterComboBox->add("Physical Objects", std::string("physical"));
	mFilterComboBox->add("Phantom Objects", std::string("phantom"));
	mFilterComboBox->add("Flexible Objects", std::string("flexible"));
	mFilterComboBox->add("Sculpted Objects", std::string("sculpted"));
	mFilterComboBox->add("Payable Objects", std::string("payable"));

	mFilterComboBox->add("Animation Sources", std::string("animations"));
	mFilterComboBox->add("Sound Sources", std::string("sounds"));
	mFilterComboBox->add("Particle Sources", std::string("particles"));
	mFilterComboBox->add("Camera Sources", std::string("camera"));

	mFilterComboBox->add("Attachment", std::string("attachment"));
	mFilterComboBox->add("HUD Attachment", std::string("hudattachment"));

	mFilterComboBox->add("Copyable Objects", std::string("copyable"));
	mFilterComboBox->add("Modifiable Objects", std::string("modifiable"));
	mFilterComboBox->add("Can Add Inventory", std::string("inventoryadd"));
	mFilterComboBox->add("Inventory Is Empty", std::string("inventoryempty"));
	mFilterComboBox->add("Any Owner", std::string("any_owner"));
	mFilterComboBox->add("Owned by You", std::string("you_owner"));
	mFilterComboBox->add("Owned by a Group", std::string("group_owner"));

	mFilterComboBox->setCommitCallback(boost::bind(&JCFloaterAreaSearch::onCommitFilterComboBox, this, _1));

	LLMenuGL* menu = new LLMenuGL("rclickmenu");
	menu->addChild(new LLMenuItemCallGL("Edit", handleRightClickEdit, NULL, this));
	menu->addChild(new LLMenuItemCallGL("Track", handleRightClickTrack, NULL, this));
	menu->addChild(new LLMenuItemCallGL("Teleport To", handleRightClickTeleport, NULL, this));
	menu->addChild(new LLMenuItemCallGL("Take Copy", handleRightClickTakeCopy, NULL, this));
	//menu->addSeparator();
	menu->setCanTearOff(FALSE);
	menu->setVisible(FALSE);
	mPopupMenuHandle = menu->getHandle();

	getChild<LLButton>("SelectAll")->setClickedCallback(boost::bind(&JCFloaterAreaSearch::onSelectAll, this));
	// </os>

	return TRUE;
}

void JCFloaterAreaSearch::onOpen()
{
	mFilterComboBox->setValue((std::string)gSavedSettings.getString("ObjectAreaSearchFilter"));
	checkRegion();
	results();
}

void JCFloaterAreaSearch::checkRegion(bool force_clear)
{
	// Check if we changed region, and if we did, clear the object details cache.
	LLViewerRegion* region = gAgent.getRegion();
	if (force_clear || region != mLastRegion)
	{
		mLastRegion = region;
		mPendingObjects.clear();
		mCachedObjects.clear();
		mResultList->deleteAllItems();
		mCounterText->setText(std::string("Listed/Pending/Total"));
	}
}

// <os>
void JCFloaterAreaSearch::onCommitFilterComboBox(LLUICtrl* ctrl)
{
	gSavedSettings.setString("ObjectAreaSearchFilter", mFilterComboBox->getValue().asString());
	onRefresh();
}
// <os>

void JCFloaterAreaSearch::onDoubleClick()
{
	LLScrollListItem *item = mResultList->getFirstSelected();
	if (!item) return;
	LLUUID object_id = item->getUUID();
	// <os>
	/*
	std::map<LLUUID,ObjectData>::iterator it = mCachedObjects.find(object_id);
	if(it != mCachedObjects.end())
	{
		LLViewerObject* objectp = gObjectList.findObject(object_id);
		if (objectp)
		{
			LLTracker::trackLocation(objectp->getPositionGlobal(), it->second.name, "", LLTracker::LOCATION_ITEM);
		}
	}
	*/
	if ( !gAgentCamera.lookAtObject(object_id, false) )
		gAgentCamera.resetCamera();
	// </os>
}

// <os>
void JCFloaterAreaSearch::onSelectAll()
{
	LLSelectMgr::getInstance()->deselectAll();
	std::vector<LLScrollListItem*> items = mResultList->getAllData();
	std::vector<LLScrollListItem*>::iterator item_iter = items.begin();
	std::vector<LLScrollListItem*>::iterator items_end = items.end();
	for (; item_iter != items_end; ++item_iter)
	{
		LLScrollListItem* item = (*item_iter);

		LLViewerObject* objectp = gObjectList.findObject(item->getUUID());
		if (objectp)
		{
			//open Tool Manager (Edit) to select all
			if (!LLToolMgr::getInstance()->inBuildMode())
			{
				gFloaterTools->open();
				LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
				gFloaterTools->setEditTool(LLToolCompTranslate::getInstance());
			}

			LLSelectMgr::getInstance()->selectObjectAndFamily(objectp);
		}
	}
}
// </os>

void JCFloaterAreaSearch::onStop()
{
	mStopped = true;
	mPendingObjects.clear();
	mCounterText->setText(std::string("Stopped"));
}

void JCFloaterAreaSearch::onRefresh()
{
	//llinfos << "Clicked search" << llendl;
	mStopped = false;
	checkRegion(true);
	results();
}

void JCFloaterAreaSearch::onCommitLine(LLUICtrl* caller, const LLSD& value, OBJECT_COLUMN_ORDER type)
{
	std::string text = value.asString();
	LLStringUtil::toLower(text);
	caller->setValue(text);
 	mFilterStrings[type] = text;
	//llinfos << "loaded " << name << " with "<< text << llendl;
	checkRegion();
	results();
}

bool JCFloaterAreaSearch::requestIfNeeded(LLUUID object_id)
{
	if (!mCachedObjects.count(object_id) && !mPendingObjects.count(object_id))
	{
		if(mStopped)
			return true;

		//llinfos << "not in list" << llendl;
		mPendingObjects.insert(object_id);

		LLMessageSystem* msg = gMessageSystem;
		msg->newMessageFast(_PREHASH_RequestObjectPropertiesFamily);
		msg->nextBlockFast(_PREHASH_AgentData);
		msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
		msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
		msg->nextBlockFast(_PREHASH_ObjectData);
		msg->addU32Fast(_PREHASH_RequestFlags, 0 );
		msg->addUUIDFast(_PREHASH_ObjectID, object_id);
		gAgent.sendReliableMessage();
		//llinfos << "Sent data request for object " << object_id << llendl;
		return true;
	}
	return false;
}

// <os>
void JCFloaterAreaSearch::handleRightClickEdit(void* userdata)
{
	JCFloaterAreaSearch* floaterp = (JCFloaterAreaSearch*)userdata;
	if (!floaterp) return;
	LLUUID object_id = floaterp->mSelectedObjectID;
	std::map<LLUUID, ObjectData>::iterator it = floaterp->mCachedObjects.find(object_id);
	if (it != floaterp->mCachedObjects.end())
	{
		LLViewerObject* objectp = gObjectList.findObject(object_id);
		if (objectp)
		{
			//BACKUP SelectOwned Value
			BOOL SelectionSettingBackup = gSavedSettings.getBOOL("SelectOwnedOnly");
			//Set to false to select all objects
			gSavedSettings.setBOOL("SelectOwnedOnly", FALSE);

			LLSelectMgr::getInstance()->deselectAll();
			//open Tool Manager (Edit) to select all
			if (!LLToolMgr::getInstance()->inBuildMode())
			{
				gFloaterTools->open();
				LLToolMgr::getInstance()->setCurrentToolset(gBasicToolset);
				gFloaterTools->setEditTool(LLToolCompTranslate::getInstance());
			}

			LLSelectMgr::getInstance()->selectObjectAndFamily(objectp);


			//SET BACK TO Settings as it was before
			gSavedSettings.setBOOL("SelectOwnedOnly", SelectionSettingBackup);;
		}
	}
}

void JCFloaterAreaSearch::handleRightClickTrack(void* userdata)
{
	JCFloaterAreaSearch* floaterp = (JCFloaterAreaSearch*)userdata;
	if (!floaterp) return;
	LLUUID object_id = floaterp->mSelectedObjectID;
	std::map<LLUUID, ObjectData>::iterator it = floaterp->mCachedObjects.find(object_id);
	if (it != floaterp->mCachedObjects.end())
	{
		LLViewerObject* objectp = gObjectList.findObject(object_id);
		if (objectp)
		{
			LLTracker::trackLocation(objectp->getPositionGlobal(), it->second.name, "", LLTracker::LOCATION_ITEM);
		}
	}
}

void JCFloaterAreaSearch::handleRightClickTeleport(void* userdata)
{
	JCFloaterAreaSearch* floaterp = (JCFloaterAreaSearch*)userdata;
	if (!floaterp) return;
	LLUUID object_id = floaterp->mSelectedObjectID;
	std::map<LLUUID, ObjectData>::iterator it = floaterp->mCachedObjects.find(object_id);
	if (it != floaterp->mCachedObjects.end())
	{
		LLViewerObject* objectp = gObjectList.findObject(object_id);
		if (objectp)
		{
			LLTracker::trackLocation(objectp->getPositionGlobal(), it->second.name, "", LLTracker::LOCATION_ITEM);
			gAgent.teleportViaLocation(objectp->getPositionGlobal());
		}
	}
}

void JCFloaterAreaSearch::handleRightClickTakeCopy(void* userdata)
{
	JCFloaterAreaSearch* floaterp = (JCFloaterAreaSearch*)userdata;
	if (!floaterp) return;
	LLUUID object_id = floaterp->mSelectedObjectID;
	std::map<LLUUID, ObjectData>::iterator it = floaterp->mCachedObjects.find(object_id);
	if (it != floaterp->mCachedObjects.end())
	{
		LLViewerObject* objectp = gObjectList.findObject(object_id);
		if (objectp)
		{
			LLUUID tid;
			tid.generate();
			LLMessageSystem* msg = gMessageSystem;
			const LLUUID& category_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_OBJECT);
			U8 packet_number = 0;
			U8 packet_count = 1;

			msg->newMessageFast(_PREHASH_DeRezObject);
			msg->nextBlockFast(_PREHASH_AgentData);
			msg->addUUIDFast(_PREHASH_AgentID, gAgent.getID());
			msg->addUUIDFast(_PREHASH_SessionID, gAgent.getSessionID());
			msg->nextBlockFast(_PREHASH_AgentBlock);
			msg->addUUIDFast(_PREHASH_GroupID, gAgent.getGroupID());
			msg->addU8Fast(_PREHASH_Destination, (U8)DRD_ACQUIRE_TO_AGENT_INVENTORY);
			msg->addUUIDFast(_PREHASH_DestinationID, category_id);
			msg->addUUIDFast(_PREHASH_TransactionID, tid);
			msg->addU8Fast(_PREHASH_PacketCount, packet_count);
			msg->addU8Fast(_PREHASH_PacketNumber, packet_number);
			msg->nextBlockFast(_PREHASH_ObjectData);
			msg->addU32Fast(_PREHASH_ObjectLocalID, objectp->getLocalID());
			msg->sendReliable(objectp->getRegion()->getHost());
			make_ui_sound("UISndObjectRezOut");
		}
	}
}
// </os>

void JCFloaterAreaSearch::results()
{
	if (!getVisible()) return;

	if (mPendingObjects.size() > 0 && mLastUpdateTimer.getElapsedTimeF32() < min_refresh_interval) return;
	//llinfos << "results()" << llendl;
	uuid_vec_t selected = mResultList->getSelectedIDs();
	S32 scrollpos = mResultList->getScrollPos();
	mResultList->deleteAllItems();
	S32 i;
	S32 total = gObjectList.getNumObjects();

	LLViewerRegion* our_region = gAgent.getRegion();
	for (i = 0; i < total; i++)
	{
		LLViewerObject *objectp = gObjectList.getObject(i);
		if (objectp)
		{
			if (objectp->getRegion() == our_region && !objectp->isAvatar() && objectp->isRoot() &&
				!objectp->flagTemporary() && !objectp->flagTemporaryOnRez())
			{
				// <os>
				const std::string filter = mObjectFilter.get();
				if (filter != "all")
				{
					if (filter == "payable" && (bool)objectp->flagTakesMoney() != true) continue;
					else if (filter == "touch_only" && (bool)objectp->flagHandleTouch() != true) continue;
					else if (filter == "scripted" && (bool)objectp->flagScripted() != true) continue;
					else if (filter == "sounds" && (bool)objectp->isAudioSource() != true) continue;
					else if (filter == "particles" && (bool)objectp->isParticleSource() != true) continue;
					else if (filter == "animations" && (bool)objectp->flagAnimSource() != true) continue;
					else if (filter == "camera" && (bool)objectp->flagCameraSource() != true) continue;
					else if (filter == "sculpted" && (bool)objectp->isSculpted() != true) continue;
					else if (filter == "flexible" && (bool)objectp->isFlexible() != true) continue;
					else if (filter == "phantom" && (bool)objectp->flagPhantom() != true) continue;
					else if (filter == "physical" && (bool)objectp->flagUsePhysics() != true) continue;
					else if (filter == "inventoryadd" && (bool)objectp->flagAllowInventoryAdd() != true) continue;
					else if (filter == "inventoryempty" && (bool)objectp->flagInventoryEmpty() != true) continue;
					else if (filter == "attachment" && (bool)objectp->isAttachment() != true) continue;
					else if (filter == "hudattachment" && (bool)objectp->isHUDAttachment() != true) continue;
					else if (filter == "copyable" && (bool)objectp->flagObjectCopy() != true) continue;
					else if (filter == "modifiable" && (bool)objectp->flagObjectModify() != true) continue;
					else if (filter == "any_owner" && (bool)objectp->flagObjectAnyOwner() != true) continue;
					else if (filter == "you_owner" && (bool)objectp->flagObjectYouOwner() != true) continue;
					else if (filter == "group_owner" && (bool)objectp->flagObjectGroupOwned() != true) continue;
					
				}
				// </os>
				LLUUID object_id = objectp->getID();
				if(!requestIfNeeded(object_id))
				{
					std::map<LLUUID,ObjectData>::iterator it = mCachedObjects.find(object_id);
					if(it != mCachedObjects.end())
					{
						//llinfos << "all entries are \"\" or we have data" << llendl;
						std::string object_name = it->second.name;
						std::string object_desc = it->second.desc;
						std::string object_owner;
						std::string object_group;
						gCacheName->getFullName(it->second.owner_id, object_owner);
						gCacheName->getGroupName(it->second.group_id, object_group);
						//llinfos << "both names are loaded or aren't needed" << llendl;
						std::string onU = object_owner;
						std::string cnU = object_group;
						LLStringUtil::toLower(object_name);
						LLStringUtil::toLower(object_desc);
						LLStringUtil::toLower(object_owner);
						LLStringUtil::toLower(object_group);
						if ((mFilterStrings[LIST_OBJECT_NAME].empty() || object_name.find(mFilterStrings[LIST_OBJECT_NAME]) != std::string::npos) &&
							(mFilterStrings[LIST_OBJECT_DESC].empty() || object_desc.find(mFilterStrings[LIST_OBJECT_DESC]) != std::string::npos) &&
							(mFilterStrings[LIST_OBJECT_OWNER].empty() || object_owner.find(mFilterStrings[LIST_OBJECT_OWNER]) != std::string::npos) &&
							(mFilterStrings[LIST_OBJECT_GROUP].empty() || object_group.find(mFilterStrings[LIST_OBJECT_GROUP]) != std::string::npos))
						{
							//llinfos << "pass" << llendl;
							LLSD element;
							element["id"] = object_id;
							element["columns"][LIST_OBJECT_NAME]["column"] = "Name";
							element["columns"][LIST_OBJECT_NAME]["type"] = "text";
							element["columns"][LIST_OBJECT_NAME]["value"] = it->second.name;
							element["columns"][LIST_OBJECT_DESC]["column"] = "Description";
							element["columns"][LIST_OBJECT_DESC]["type"] = "text";
							element["columns"][LIST_OBJECT_DESC]["value"] = it->second.desc;
							element["columns"][LIST_OBJECT_OWNER]["column"] = "Owner";
							element["columns"][LIST_OBJECT_OWNER]["type"] = "text";
							element["columns"][LIST_OBJECT_OWNER]["value"] = onU;
							element["columns"][LIST_OBJECT_GROUP]["column"] = "Group";
							element["columns"][LIST_OBJECT_GROUP]["type"] = "text";
							element["columns"][LIST_OBJECT_GROUP]["value"] = cnU;			//ai->second;
							mResultList->addElement(element, ADD_BOTTOM);
						}
						
					}
				}
			}
		}
	}

	mResultList->updateSort();
	mResultList->selectMultiple(selected);
	mResultList->setScrollPos(scrollpos);
	mCounterText->setText(llformat("%d listed/%d pending/%d total", mResultList->getItemCount(), mPendingObjects.size(), mPendingObjects.size()+mCachedObjects.size()));
	mLastUpdateTimer.reset();
}

// static
void JCFloaterAreaSearch::processObjectPropertiesFamily(LLMessageSystem* msg, void** user_data)
{
	JCFloaterAreaSearch* floater = findInstance();
	if(!floater)
		return;
	floater->checkRegion();

	LLUUID object_id;
	msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_ObjectID, object_id);

	std::set<LLUUID>::iterator it = floater->mPendingObjects.find(object_id);
	if(it != floater->mPendingObjects.end())
		floater->mPendingObjects.erase(it);
	//else if(floater->mCachedObjects.count(object_id)) //Let entries update.
	//	return;

	ObjectData* data = &floater->mCachedObjects[object_id];
	// We cache unknown objects (to avoid having to request them later)
	// and requested objects.
	msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_OwnerID, data->owner_id);
	msg->getUUIDFast(_PREHASH_ObjectData, _PREHASH_GroupID, data->group_id);
	msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Name, data->name);
	msg->getStringFast(_PREHASH_ObjectData, _PREHASH_Description, data->desc);
	gCacheName->get(data->owner_id, false, boost::bind(&JCFloaterAreaSearch::results,floater));
	gCacheName->get(data->group_id, true, boost::bind(&JCFloaterAreaSearch::results,floater));
	//llinfos << "Got info for " << (exists ? "requested" : "unknown") << " object " << object_id << llendl;
}
