/** 
 * @file llpanelobject.h
 * @brief Object editing (position, scale, etc.) in the tools floater
 *
 * $LicenseInfo:firstyear=2004&license=viewergpl$
 * 
 * Copyright (c) 2004-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLPANELOBJECT_H
#define LL_LLPANELOBJECT_H

#include "v3math.h"
#include "llpanel.h"
#include "llmemory.h"
#include "llvolume.h"

class LLSpinCtrl;
class LLCheckBoxCtrl;
class LLTextBox;
class LLUICtrl;
class LLButton;
class LLViewerObject;
class LLComboBox;
class LLPanelObjectInventory;
class LLColorSwatchCtrl;
class LLTextureCtrl;
class LLInventoryItem;
class LLUUID;
class LLFlexibleObjectData;
class LLLightParams;
class LLLightImageParams;
class LLSculptParams;

class LLPanelObject : public LLPanel
{
public:
	LLPanelObject(const std::string& name);
	virtual ~LLPanelObject();

	virtual BOOL	postBuild();
	virtual void	draw();
	virtual void 	clearCtrls();

	void			refresh();

	static bool		precommitValidate(const LLSD& data);

	static void		onCommitLock(LLUICtrl *ctrl, void *data);
	static void 	onCommitPosition(		LLUICtrl* ctrl, void* userdata);
	static void 	onCommitScale(			LLUICtrl* ctrl, void* userdata);
	static void 	onCommitRotation(		LLUICtrl* ctrl, void* userdata);
	static void 	onCommitTemporary(		LLUICtrl* ctrl, void* userdata);
	static void 	onCommitPhantom(		LLUICtrl* ctrl, void* userdata);
	static void 	onCommitPhysics(		LLUICtrl* ctrl, void* userdata);

	//static void		onLinkObj(				void* user_data);
	//static void		onUnlinkObj(			void* user_data);
	static void 	onCopyPos(				void* user_data);
	static void 	onPastePos(				void* user_data);
	static void 	onPastePosClip(			void* user_data);
	static void 	onCopySize(				void* user_data);
	static void 	onPasteSize(			void* user_data);
	static void 	onPasteSizeClip(		void* user_data);
	static void 	onCopyRot(				void* user_data);
	static void 	onPasteRot(				void* user_data);
	static void 	onPasteRotClip(			void* user_data);
	static void 	onCopyParams(			void* user_data);
	static void 	onPasteParams(			void* user_data);

	static void 	onCommitParametric(LLUICtrl* ctrl, void* userdata);

	static void 	onCommitMaterial(		LLUICtrl* ctrl, void* userdata);

	void     		onCommitSculpt(const LLSD& data);
	void     		onCancelSculpt(const LLSD& data);
	void     		onSelectSculpt(const LLSD& data);
	BOOL     		onDropSculpt(LLInventoryItem* item);
	static void     onCommitSculptType(    LLUICtrl *ctrl, void* userdata);

	static void		onClickBuildConstants(void *);
	static const 	LLUUID& findItemID(const LLUUID& asset_id);

protected:
	void			getState();

	void			sendRotation(BOOL btn_down);
	void			sendScale(BOOL btn_down);
	void			sendPosition(BOOL btn_down);
	void			sendIsPhysical();
	void			sendIsTemporary();
	void			sendIsPhantom();

	void            sendSculpt();
	
	void 			getVolumeParams(LLVolumeParams& volume_params);
	
protected:
	static LLVector3 mClipboardPos;
	static LLVector3 mClipboardSize;
	static LLVector3 mClipboardRot;
	static LLVolumeParams mClipboardVolumeParams;
	static LLFlexibleObjectData* mClipboardFlexiParams;
	static LLLightParams* mClipboardLightParams;
	static LLSculptParams* mClipboardSculptParams;
	static LLLightImageParams* mClipboardLightImageParams;
	static BOOL hasParamClipboard;

	S32				mComboMaterialItemCount;

	LLTextBox*		mLabelMaterial;
	LLComboBox*		mComboMaterial;

	// Per-object options
	LLTextBox*		mLabelBaseType;
	LLComboBox*		mComboBaseType;

	LLTextBox*		mLabelCut;
	LLSpinCtrl*		mSpinCutBegin;
	LLSpinCtrl*		mSpinCutEnd;

	LLTextBox*		mLabelHollow;
	LLSpinCtrl*		mSpinHollow;

	LLTextBox*		mLabelHoleType;
	LLComboBox*		mComboHoleType;

	LLTextBox*		mLabelTwist;
	LLSpinCtrl*		mSpinTwist;
	LLSpinCtrl*		mSpinTwistBegin;

	LLSpinCtrl*		mSpinScaleX;
	LLSpinCtrl*		mSpinScaleY;
	
	LLTextBox*		mLabelSkew;
	LLSpinCtrl*		mSpinSkew;

	LLTextBox*		mLabelShear;
	LLSpinCtrl*		mSpinShearX;
	LLSpinCtrl*		mSpinShearY;

	// Advanced Path
	LLSpinCtrl*		mCtrlPathBegin;
	LLSpinCtrl*		mCtrlPathEnd;

	LLTextBox*		mLabelTaper;
	LLSpinCtrl*		mSpinTaperX;
	LLSpinCtrl*		mSpinTaperY;

	LLTextBox*		mLabelRadiusOffset;
	LLSpinCtrl*		mSpinRadiusOffset;

	LLTextBox*		mLabelRevolutions;
	LLSpinCtrl*		mSpinRevolutions;

	LLTextBox*		mLabelPosition;
	LLSpinCtrl*		mCtrlPosX;
	LLSpinCtrl*		mCtrlPosY;
	LLSpinCtrl*		mCtrlPosZ;

	LLTextBox*		mLabelSize;
	LLSpinCtrl*		mCtrlScaleX;
	LLSpinCtrl*		mCtrlScaleY;
	LLSpinCtrl*		mCtrlScaleZ;

	LLTextBox*		mLabelRotation;
	LLSpinCtrl*		mCtrlRotX;
	LLSpinCtrl*		mCtrlRotY;
	LLSpinCtrl*		mCtrlRotZ;

	//LLButton		*mBtnLinkObj;
	//LLButton		*mBtnUnlinkObj;

	LLButton		*mBtnCopyPos;
	LLButton		*mBtnPastePos;
	LLButton		*mBtnPastePosClip;

	LLButton		*mBtnCopySize;
	LLButton		*mBtnPasteSize;
	LLButton		*mBtnPasteSizeClip;

	LLButton		*mBtnCopyRot;
	LLButton		*mBtnPasteRot;
	LLButton		*mBtnPasteRotClip;

	LLButton		*mBtnCopyParams;
	LLButton		*mBtnPasteParams;

	LLCheckBoxCtrl	*mCheckLock;
	LLCheckBoxCtrl	*mCheckPhysics;
	LLCheckBoxCtrl	*mCheckTemporary;
	LLCheckBoxCtrl	*mCheckPhantom;

	LLTextureCtrl   *mCtrlSculptTexture;
	LLTextBox       *mLabelSculptType;
	LLComboBox      *mCtrlSculptType;
	LLCheckBoxCtrl  *mCtrlSculptMirror;
	LLCheckBoxCtrl  *mCtrlSculptInvert;
	
	LLVector3		mCurEulerDegrees;		// to avoid sending rotation when not changed
	BOOL			mIsPhysical;			// to avoid sending "physical" when not changed
	BOOL			mIsTemporary;			// to avoid sending "temporary" when not changed
	BOOL			mIsPhantom;				// to avoid sending "phantom" when not changed
	S32				mSelectedType;			// So we know what selected type we last were

	LLUUID          mSculptTextureRevert;   // so we can revert the sculpt texture on cancel
	U8              mSculptTypeRevert;      // so we can revert the sculpt type on cancel

	LLPointer<LLViewerObject> mObject;
	LLPointer<LLViewerObject> mRootObject;
};

#endif
