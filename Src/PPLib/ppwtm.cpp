// PPWTM.CPP
// Copyright (c) A.Sobolev 2010, 2011, 2012, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021
//
#include <pp.h>
#pragma hdrstop

LayoutEntryDialogBlock::LayoutEntryDialogBlock(const AbstractLayoutBlock * pS)
{
	Setup(pS);
}
	
void LayoutEntryDialogBlock::Setup(const AbstractLayoutBlock * pS)
{
	if(pS)
		*static_cast<AbstractLayoutBlock *>(this) = *pS;
}

class LayoutContainerDialog : public TDialog {
	DECL_DIALOG_DATA(LayoutEntryDialogBlock);
public:
	LayoutContainerDialog() : TDialog(DLG_WOLAYC), Data(0)
	{
	}
	DECL_DIALOG_SETDTS()
	{
		int    ok = 1;
		SString temp_buf;
		RVALUEPTR(Data, pData);
		setCtrlString(CTL_WOLAYC_IDENT, Data.OwnLayoutSymb);
		AddClusterAssocDef(CTL_WOLAYC_LAY, 0, DIREC_UNKN);
		AddClusterAssoc(CTL_WOLAYC_LAY, 1, DIREC_HORZ);
		AddClusterAssoc(CTL_WOLAYC_LAY, 2, DIREC_VERT);
		SetClusterData(CTL_WOLAYC_LAY, Data.GetContainerDirection());
		AddClusterAssoc(CTL_WOLAYC_FLAGS, 0, AbstractLayoutBlock::fContainerWrap);
		AddClusterAssoc(CTL_WOLAYC_FLAGS, 1, AbstractLayoutBlock::fContainerReverseDir);
		AddClusterAssoc(CTL_WOLAYC_FLAGS, 2, AbstractLayoutBlock::fContainerWrapReverse);
		SetClusterData(CTL_WOLAYC_FLAGS, Data.Flags);
		{
			SetupStringCombo(this, CTLSEL_WOLAYC_JUSTC, PPTXT_LAYOUTALIGNMENT, Data.JustifyContent);
			SetupStringCombo(this, CTLSEL_WOLAYC_ALGNC, PPTXT_LAYOUTALIGNMENT, Data.AlignContent);
			SetupStringCombo(this, CTLSEL_WOLAYC_ALGNI, PPTXT_LAYOUTALIGNMENT, Data.AlignItems);
		}
		//
		AbstractLayoutBlock::MarginsToString(Data.Padding, temp_buf);
		setCtrlString(CTL_WOLAYC_MARG, temp_buf);
		return ok;
	}
	DECL_DIALOG_GETDTS()
	{
		int    ok = 1;
		uint   sel = 0;
		SString temp_buf;
		getCtrlString(CTL_WOLAYC_IDENT, Data.OwnLayoutSymb);
		Data.SetContainerDirection(GetClusterData(CTL_WOLAYC_LAY));
		// GetClusterData(CTL_WOLAYC_ADJ, &Data.ContainerAdjustment);
		{
			long   flags = 0;
			GetClusterData(CTL_WOLAYC_FLAGS, &flags);
			SETFLAGBYSAMPLE(Data.Flags, AbstractLayoutBlock::fContainerWrap, flags);
			SETFLAGBYSAMPLE(Data.Flags, AbstractLayoutBlock::fContainerReverseDir, flags);
			SETFLAGBYSAMPLE(Data.Flags, AbstractLayoutBlock::fContainerWrapReverse, flags);
		}
		{
			long   temp_alignment = 0;
			temp_alignment = static_cast<long>(Data.JustifyContent);
			if(getCtrlData(CTLSEL_WOLAYC_JUSTC, &temp_alignment))
				Data.JustifyContent = static_cast<uint16>(temp_alignment);
			temp_alignment = static_cast<long>(Data.AlignContent);
			if(getCtrlData(CTLSEL_WOLAYC_ALGNC, &temp_alignment))
				Data.AlignContent = static_cast<uint16>(temp_alignment);
			temp_alignment = static_cast<long>(Data.AlignItems);
			if(getCtrlData(CTLSEL_WOLAYC_ALGNI, &temp_alignment))
				Data.AlignItems = static_cast<uint16>(temp_alignment);
		}
		//
		getCtrlString(sel = CTL_WOLAYC_MARG, temp_buf);
		THROW_PP(AbstractLayoutBlock::MarginsFromString(temp_buf, Data.Padding), PPERR_USERINPUT);
		//
		ASSIGN_PTR(pData, Data);
		CATCHZOKPPERRBYDLG
		return ok;
	}
};

class LayoutEntryDialog : public TDialog {
	DECL_DIALOG_DATA(LayoutEntryDialogBlock);
public:
	LayoutEntryDialog(const TWhatman * pWtm) : TDialog(DLG_LAYOENTRY), P_Wtm(pWtm), Data(0)
	{
	}
	DECL_DIALOG_SETDTS()
	{
		int    ok = 1;
		SString temp_buf;
		RVALUEPTR(Data, pData);
		if(P_Wtm) {
			long    init_parent_id = 0;
			StrAssocArray layout_symb_list;
			P_Wtm->GetLayoutSymbList(layout_symb_list);
			if(Data.ParentLayoutSymb.NotEmpty()) {
				uint pos = 0;
				if(layout_symb_list.SearchByText(Data.ParentLayoutSymb, 0, &pos)) {
					init_parent_id = layout_symb_list.Get(pos).Id;
				}
			}
			SetupStrAssocCombo(this, CTLSEL_LAYOENTRY_PARENT, &layout_symb_list, init_parent_id, 0, 0, 0);
		}
		else {
			disableCtrl(CTLSEL_LAYOENTRY_PARENT, 1);
		}
		Data.SizeToString(temp_buf);
		setCtrlString(CTL_LAYOENTRY_SIZE, temp_buf);
		AbstractLayoutBlock::MarginsToString(Data.Margin, temp_buf);
		setCtrlString(CTL_LAYOENTRY_MARG, temp_buf);
		setCtrlData(CTL_LAYOENTRY_GROWF, &Data.GrowFactor);
		setCtrlData(CTL_LAYOENTRY_SHRNKF, &Data.ShrinkFactor);
		setCtrlData(CTL_LAYOENTRY_ASPR, &Data.AspectRatio);
		//
		{
			long   area = Data.GetVArea();
			SetupStringCombo(this, CTLSEL_LAYOENTRY_GRAVITY, PPTXT_LAYOUTGRAVITYAREA, area+1);
		}
		SetupStringCombo(this, CTLSEL_LAYOENTRY_ALGNSLF, PPTXT_LAYOUTALIGNMENT, Data.AlignSelf);
		//
		/*
		AddClusterAssocDef(CTL_LAYOENTRY_GRAVITY, 0, Data.bfLeft);
		AddClusterAssoc(CTL_LAYOENTRY_GRAVITY, 1, Data.bfRight);
		AddClusterAssoc(CTL_LAYOENTRY_GRAVITY, 2, Data.bfTop);
		AddClusterAssoc(CTL_LAYOENTRY_GRAVITY, 3, Data.bfBottom);
		AddClusterAssoc(CTL_LAYOENTRY_GRAVITY, 4, Data.bfHFill);
		AddClusterAssoc(CTL_LAYOENTRY_GRAVITY, 5, Data.bfVFill);
		AddClusterAssoc(CTL_LAYOENTRY_GRAVITY, 6, Data.bfFill);
		long bf = (Data.BehaveFlags & (Data.bfLeft|Data.bfRight|Data.bfTop|Data.bfBottom|Data.bfHFill|Data.bfVFill|Data.bfFill));
		SetClusterData(CTL_LAYOENTRY_GRAVITY, bf);
		*/
		return ok;
	}
	DECL_DIALOG_GETDTS()
	{
		int    ok = 1;
		uint   sel = 0;
		long   bf = 0;
		SString temp_buf;
		getCtrlString(sel = CTL_LAYOENTRY_SIZE, temp_buf);
		THROW_PP(Data.SizeFromString(temp_buf), PPERR_USERINPUT);
		getCtrlString(CTL_LAYOENTRY_MARG, temp_buf);
		THROW_PP(AbstractLayoutBlock::MarginsFromString(temp_buf, Data.Margin), PPERR_USERINPUT);
		getCtrlData(CTL_LAYOENTRY_GROWF, &Data.GrowFactor);
		getCtrlData(CTL_LAYOENTRY_SHRNKF, &Data.ShrinkFactor);
		getCtrlData(CTL_LAYOENTRY_ASPR, &Data.AspectRatio);
		{
			long   area = 0;
			getCtrlData(CTLSEL_LAYOENTRY_GRAVITY, &area);
			if(area) {
				Data.SetVArea(area-1);
			}
		}
		{
			long align_self = static_cast<long>(Data.AlignSelf);
			if(getCtrlData(CTLSEL_LAYOENTRY_ALGNSLF, &align_self))
				Data.AlignSelf = static_cast<uint16>(align_self);
		}
		//GetClusterData(CTL_LAYOENTRY_GRAVITY, &bf);
		//Data.BehaveFlags &= ~(Data.BehaveFlags & (Data.bfLeft|Data.bfRight|Data.bfTop|Data.bfBottom|Data.bfHFill|Data.bfVFill|Data.bfFill));
		//Data.BehaveFlags |= bf;
		if(P_Wtm) {
			long   parent_id = getCtrlLong(CTLSEL_LAYOENTRY_PARENT);
			if(parent_id) {
				StrAssocArray layout_symb_list;
				P_Wtm->GetLayoutSymbList(layout_symb_list);
				uint pos = 0;
				if(layout_symb_list.Search(parent_id, &pos))
					Data.ParentLayoutSymb = layout_symb_list.Get(pos).Txt;
			}
		}
		ASSIGN_PTR(pData, Data);
		CATCH
			PPErrorByDialog(this, sel);
		ENDCATCH
		return ok;
	}
	const TWhatman * P_Wtm;
};

int LayoutEntryDialogBlock::EditEntry(const TWhatman * pWtm)
{
	DIALOG_PROC_BODY_P1(LayoutEntryDialog, pWtm, this);
}

int LayoutEntryDialogBlock::EditContainer()
{
	DIALOG_PROC_BODY(LayoutContainerDialog, this);
}

class BaseWtmToolDialog : public TDialog {
	DECL_DIALOG_DATA(TWhatmanToolArray::Item);
public:
	explicit BaseWtmToolDialog(uint dlgID) : TDialog(dlgID/*DLG_WTMTOOL*/)
	{
		FileBrowseCtrlGroup::Setup(this, CTLBRW_WTMTOOL_FIGPATH, CTL_WTMTOOL_FIGPATH, 1, 0, PPTXT_FILPAT_SPIC,
			FileBrowseCtrlGroup::fbcgfFile|FileBrowseCtrlGroup::fbcgfSaveLastPath);
		FileBrowseCtrlGroup::Setup(this, CTLBRW_WTMTOOL_PICPATH, CTL_WTMTOOL_PICPATH, 2, 0, PPTXT_FILPAT_SPIC,
			FileBrowseCtrlGroup::fbcgfFile|FileBrowseCtrlGroup::fbcgfSaveLastPath);
	}
	DECL_DIALOG_SETDTS()
	{
		int    ok = 1;
		StrAssocArray * p_wtmo_list = 0;
		RVALUEPTR(Data, pData);
		setCtrlString(CTL_WTMTOOL_NAME, Data.Text);
		setCtrlString(CTL_WTMTOOL_SYMB, Data.Symb);
		disableCtrl(CTL_WTMTOOL_SYMB, Data.Symb.NotEmpty());
		setCtrlString(CTL_WTMTOOL_FIGPATH, Data.FigPath);
		setCtrlString(CTL_WTMTOOL_PICPATH, Data.PicPath);
		setCtrlData(CTL_WTMTOOL_FIGSIZE, &Data.FigSize);
		setCtrlData(CTL_WTMTOOL_PICSIZE, &Data.PicSize);
		AddClusterAssoc(CTL_WTMTOOL_FLAGS, 0, TWhatmanToolArray::Item::fDontEnlarge);
		AddClusterAssoc(CTL_WTMTOOL_FLAGS, 1, TWhatmanToolArray::Item::fDontKeepRatio);
		AddClusterAssoc(CTL_WTMTOOL_FLAGS, 2, TWhatmanToolArray::Item::fGrayscale);
		SetClusterData(CTL_WTMTOOL_FLAGS, Data.Flags);
		{
			ComboBox * p_combo = static_cast<ComboBox *>(getCtrlView(CTLSEL_WTMTOOL_WTMOBJ));
			if(p_combo) {
				p_combo->setListWindow(CreateListWindow(TWhatmanObject::MakeStrAssocList(), lbtDisposeData|lbtDblClkNotify),
					TWhatmanObject::GetRegIdBySymb(Data.WtmObjSymb));
				disableCtrl(CTLSEL_WTMTOOL_WTMOBJ, 1);
			}
		}
		return ok;
	}
	DECL_DIALOG_GETDTS()
	{
		int    ok = 1;
		long   wtmo_id = getCtrlLong(CTLSEL_WTMTOOL_WTMOBJ);
		if(wtmo_id)
			TWhatmanObject::GetRegSymbById(wtmo_id, Data.WtmObjSymb);
		getCtrlString(CTL_WTMTOOL_NAME, Data.Text);
		getCtrlString(CTL_WTMTOOL_SYMB, Data.Symb);
		getCtrlString(CTL_WTMTOOL_FIGPATH, Data.FigPath);
		getCtrlString(CTL_WTMTOOL_PICPATH, Data.PicPath);
		getCtrlData(CTL_WTMTOOL_FIGSIZE, &Data.FigSize);
		getCtrlData(CTL_WTMTOOL_PICSIZE, &Data.PicSize);
		GetClusterData(CTL_WTMTOOL_FLAGS, &Data.Flags);
		ASSIGN_PTR(pData, Data);
		return ok;
	}
private:
	DECL_HANDLE_EVENT
	{
		TDialog::handleEvent(event);
		if(event.isCmd(cmLayoutEntry)) {
			LayoutEntryDialogBlock lodb(&Data.Alb);
			if(lodb.EditEntry(0) > 0) {
				Data.Alb = lodb;
			}
		}
		else if(event.isCmd(cmLayoutContainer)) {
			LayoutEntryDialogBlock lodb(&Data.Alb);
			if(lodb.EditContainer() > 0) {
				Data.Alb = lodb;
			}
		}
	}
};

int BaseEditWhatmanToolItem(TWhatmanToolArray::Item * pItem)
{
	int    ok = -1;
	BaseWtmToolDialog * dlg = new BaseWtmToolDialog(DLG_WTMTOOL);
	if(CheckDialogPtrErr(&dlg)) {
		dlg->setDTS(pItem);
		while(ok < 0 && ExecView(dlg) == cmOK) {
			if(dlg->getDTS(pItem))
				ok = 1;
		}
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}

class WhatmanObjectBaseDialog : public TDialog {
public:
	WhatmanObjectBaseDialog(uint dlgId) : TDialog(dlgId), P_Wtm(0), Data(0)
	{
	}
	int    setRef(TWhatmanObject * pData)
	{
		if(pData) {
			Data.Setup(&pData->GetLayoutBlock());
			Data.ParentLayoutSymb = pData->GetLayoutContainerIdent();
			if(pData->HasOption(TWhatmanObject::oContainer)) {
				Data.OwnLayoutSymb = static_cast<const WhatmanObjectLayoutBase *>(pData)->GetContainerIdent();
			}
			P_Wtm = pData->GetOwner();
		}
		return 1;
	}
	int    updateRef(TWhatmanObject * pData)
	{
		int    ok = 1;
		if(pData) {
			pData->SetLayoutBlock(&Data);
			pData->SetLayoutContainerIdent(Data.ParentLayoutSymb);
			if(pData->HasOption(TWhatmanObject::oContainer)) {
				static_cast<WhatmanObjectLayoutBase *>(pData)->SetContainerIdent(Data.OwnLayoutSymb);
			}
		}
		return ok;
	}
protected:
	DECL_HANDLE_EVENT
	{
		TDialog::handleEvent(event);
		if(event.isCmd(cmLayoutEntry)) {
			Data.EditEntry(P_Wtm);
		}
		else if(event.isCmd(cmLayoutContainer)) { // @v10.9.8
			Data.EditContainer();
		}
		else
			return;
		clearEvent(event);
	}
	LayoutEntryDialogBlock Data;
	const TWhatman * P_Wtm;
};
//
//
//
class WhatmanObjectLayout : public WhatmanObjectLayoutBase {
public:
	WhatmanObjectLayout() : WhatmanObjectLayoutBase()
	{
		Options |= (oMovable | oResizable | oMultSelectable | oContainer); // @v10.9.6 oContainer
	}
	~WhatmanObjectLayout()
	{
	}
protected:
	virtual int Draw(TCanvas2 & rCanv)
	{
		TRect b = GetBounds();
		const TWhatman * p_wtm = GetOwner();
		int   tool = 0;
		if(p_wtm) {
			if(State & stContainerCandidate)
				tool = p_wtm->GetTool(TWhatman::toolPenContainerCandidateBorder);
			else
				tool = p_wtm->GetTool(TWhatman::toolPenLayoutBorder);
		}
		rCanv.Rect(b.grow(-1, -1), tool, 0);
		return 1;
	}
	virtual int HandleCommand(int cmd, void * pExt)
	{
		int    ok = -1;
		switch(cmd) {
			case cmdSetupByTool:
				{
					const TWhatmanToolArray::Item * p_item = static_cast<const TWhatmanToolArray::Item *>(pExt);
					if(p_item && p_item->P_Owner) {
						ok = 1;
					}
				}
				break;
			case cmdEditTool:
				ok = BaseEditWhatmanToolItem(static_cast<TWhatmanToolArray::Item *>(pExt));
				break;
			case cmdEdit:
				{
					class WoLayoutDialog : public WhatmanObjectBaseDialog {
						DECL_DIALOG_DATA(WhatmanObjectLayout);
					public:
						WoLayoutDialog() : WhatmanObjectBaseDialog(DLG_WOLAYOUT)
						{
						}
						DECL_DIALOG_SETDTS()
						{
							int    ok = 1;
							RVALUEPTR(Data, pData);
							WhatmanObjectBaseDialog::setRef(&Data); // @v10.4.9
							setCtrlString(CTL_WOLAYOUT_SYMB, Data.GetSymb());
							return ok;
						}
						DECL_DIALOG_GETDTS()
						{
							int    ok = 1;
							uint   sel = 0;
							AbstractLayoutBlock lb = Data.GetLayoutBlock();
							getCtrlString(sel = CTL_WOLAYOUT_SYMB, Data.Symb);
							const TWhatman * p_wtm = Data.GetOwner();
							THROW_SL(!p_wtm || p_wtm->CheckUniqLayoutSymb(pData));
							WhatmanObjectBaseDialog::updateRef(&Data);
							ASSIGN_PTR(pData, Data);
							CATCH
								ok = PPErrorByDialog(this, sel);
							ENDCATCH
							return ok;
						}
					};
					WoLayoutDialog * dlg = new WoLayoutDialog();
					if(CheckDialogPtrErr(&dlg)) {
						dlg->setDTS(this);
						while(ok < 0 && ExecView(dlg) == cmOK) {
							if(dlg->getDTS(this)) {
								ok = 1;
							}
						}
					}
					else
						ok = 0;
					delete dlg;
				}
				break;
		}
		return ok;
	}
};

IMPLEMENT_WTMOBJ_FACTORY(Layout, "@wtmo_layout");

class WhatmanObjectDrawFigure : public TWhatmanObject {
public:
	explicit WhatmanObjectDrawFigure(SDrawFigure * pFig = 0) : TWhatmanObject("DrawFigure"), P_Fig(pFig)
	{
		Options |= (oMovable | oResizable | oMultSelectable);
		if(P_Fig) {
			FPoint sz = P_Fig->GetSize();
			TRect b(0, 0, fceili(sz.X), fceili(sz.Y));
			SetBounds(b);
		}
	}
	~WhatmanObjectDrawFigure()
	{
		ZDELETE(P_Fig);
	}
protected:
	explicit WhatmanObjectDrawFigure(const char * pSymb) : TWhatmanObject(pSymb), P_Fig(0)
	{
		Options |= (oMovable | oResizable);
	}
	virtual TWhatmanObject * Dup() const;
	virtual int Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	virtual int Draw(TCanvas2 & rCanv);
	virtual int HandleCommand(int cmd, void * pExt);
	int    FASTCALL Copy(const WhatmanObjectDrawFigure & rS)
	{
		int    ok = 1;
		TWhatmanObject::Copy(rS);
		ZDELETE(P_Fig);
		if(rS.P_Fig)
			P_Fig = rS.P_Fig->Dup();
		return ok;
	}
	SDrawFigure * P_Fig;
};

IMPLEMENT_WTMOBJ_FACTORY(DrawFigure, "@wtmo_drawfigure");

TWhatmanObject * WhatmanObjectDrawFigure::Dup() const
{
	WhatmanObjectDrawFigure * p_obj = new WhatmanObjectDrawFigure(static_cast<const char *>(0));
	CALLPTRMEMB(p_obj, Copy(*this));
	return p_obj;
}

int WhatmanObjectDrawFigure::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	uint8  ind = 0;
	THROW(TWhatmanObject::Serialize(dir, rBuf, pCtx));
	if(dir > 0) {
		THROW(pCtx->Serialize(dir, (ind = P_Fig ? 0 : 1), rBuf));
		if(P_Fig) {
			THROW(P_Fig->Serialize(dir, rBuf, pCtx));
		}
	}
	else if(dir < 0) {
		ZDELETE(P_Fig);
		THROW(pCtx->Serialize(dir, ind, rBuf));
		if(ind == 0) {
			THROW(P_Fig = SDrawFigure::Unserialize(rBuf, pCtx));
		}
	}
	CATCHZOK
	return ok;
}

int WhatmanObjectDrawFigure::HandleCommand(int cmd, void * pExt)
{
	int    ok = -1;
	switch(cmd) {
		case cmdSetupByTool:
			{
				const TWhatmanToolArray::Item * p_item = (const TWhatmanToolArray::Item *)pExt;
				if(p_item && p_item->P_Owner) {
					const SDrawFigure * p_fig = p_item->P_Owner->GetFig(1, p_item->Symb, 0);
					if(p_fig) {
						P_Fig = p_fig->Dup();
						// @construction {
						if(p_item->Flags & TWhatmanToolArray::Item::fGrayscale) {
							if(P_Fig->GetKind() == SDrawFigure::kImage) {
								SDrawImage * p_img = static_cast<SDrawImage *>(P_Fig);
								p_img->TransformToGrayscale();
							}
						}
						// } @construction
						if(p_item->Flags & TWhatmanToolArray::Item::fDontKeepRatio) {
							SViewPort vp;
							P_Fig->GetViewPort(&vp);
							vp.ParX = SViewPort::parNone;
							vp.ParY = SViewPort::parNone;
							P_Fig->SetViewPort(&vp);
						}
						else if(p_item->Flags & TWhatmanToolArray::Item::fDontEnlarge) {
							SViewPort vp;
							P_Fig->GetViewPort(&vp);
							vp.Flags |= SViewPort::fDontEnlarge;
							P_Fig->SetViewPort(&vp);
						}
						ok = 1;
					}
				}
			}
			break;
		case cmdEditTool:
			ok = BaseEditWhatmanToolItem((TWhatmanToolArray::Item *)pExt);
			break;
		/*case cmdEdit:
			break;*/
	}
	return ok;
}

int WhatmanObjectDrawFigure::Draw(TCanvas2 & rCanv)
{
	int    ok = 1;
	if(P_Fig) {
		LMatrix2D mtx;
		rCanv.PushTransform();
		SViewPort vp;
		P_Fig->GetViewPort(&vp);
		rCanv.AddTransform(vp.GetMatrix(GetBounds(), mtx));
		rCanv.Draw(P_Fig);
		rCanv.PopTransform();
	}
	else
		ok = -1;
	return ok;
}
//
//
//
class WhatmanObjectBackground : public WhatmanObjectDrawFigure {
public:
	enum {
		bkgoFull   = 0x0001,
		bkgoTile   = 0x0002,
		bkgoCenter = 0x0004
	};

	WhatmanObjectBackground();
protected:
	virtual TWhatmanObject * Dup() const;
	virtual int Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	//virtual int Draw(TCanvas2 & rCanv);
	virtual int HandleCommand(int cmd, void * pExt);
	int    FASTCALL Copy(const WhatmanObjectBackground & rS);
private:
	int    EditTool(TWhatmanToolArray::Item * pItem);

	long   BkgOptions;
};

IMPLEMENT_WTMOBJ_FACTORY(Background, "@wtmo_background");

WhatmanObjectBackground::WhatmanObjectBackground() : WhatmanObjectDrawFigure("Background"), BkgOptions(bkgoFull)
{
	Options &= ~(oMovable | oResizable);
	Options |= oBackground;
}

int FASTCALL WhatmanObjectBackground::Copy(const WhatmanObjectBackground & rS)
{
	int    ok = 1;
	WhatmanObjectDrawFigure::Copy(rS);
	BkgOptions = rS.BkgOptions;
	return ok;
}

TWhatmanObject * WhatmanObjectBackground::Dup() const
{
	WhatmanObjectBackground * p_obj = new WhatmanObjectBackground();
	CALLPTRMEMB(p_obj, Copy(*this));
	return p_obj;
}

int WhatmanObjectBackground::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	THROW(WhatmanObjectDrawFigure::Serialize(dir, rBuf, pCtx));
	THROW_SL(pCtx->Serialize(dir, BkgOptions, rBuf));
	CATCHZOK
	return ok;
}

int WhatmanObjectBackground::EditTool(TWhatmanToolArray::Item * pItem)
{
	class BkgToolDialog : public TDialog {
		DECL_DIALOG_DATA(TWhatmanToolArray::Item);
	public:
		BkgToolDialog() : TDialog(DLG_WTMTOOLBKG)
		{
			FileBrowseCtrlGroup::Setup(this, CTLBRW_WTMTOOL_FIGPATH, CTL_WTMTOOL_FIGPATH, 1, 0, PPTXT_FILPAT_SPIC,
				FileBrowseCtrlGroup::fbcgfFile|FileBrowseCtrlGroup::fbcgfSaveLastPath);
		}
		DECL_DIALOG_SETDTS()
		{
			int    ok = 1;
			RVALUEPTR(Data, pData);
			long   bkg_options = *(long *)Data.ExtData;
			setCtrlString(CTL_WTMTOOL_NAME, Data.Text);
			setCtrlString(CTL_WTMTOOL_SYMB, Data.Symb);
			disableCtrl(CTL_WTMTOOL_SYMB, Data.Symb.NotEmpty());
			setCtrlString(CTL_WTMTOOL_FIGPATH, Data.FigPath);
			setCtrlData(CTL_WTMTOOL_FIGSIZE, &Data.FigSize);
			AddClusterAssocDef(CTL_WTMTOOL_BKGOPTIONS, 0, WhatmanObjectBackground::bkgoFull);
			AddClusterAssoc(CTL_WTMTOOL_BKGOPTIONS, 1, WhatmanObjectBackground::bkgoTile);
			AddClusterAssoc(CTL_WTMTOOL_BKGOPTIONS, 2, WhatmanObjectBackground::bkgoCenter);
			SetClusterData(CTL_WTMTOOL_BKGOPTIONS, bkg_options);
			return ok;
		}
		DECL_DIALOG_GETDTS()
		{
			int    ok = 1;
			long   bkg_options = 0;
			getCtrlString(CTL_WTMTOOL_NAME, Data.Text);
			getCtrlString(CTL_WTMTOOL_SYMB, Data.Symb);
			getCtrlString(CTL_WTMTOOL_FIGPATH, Data.FigPath);
			getCtrlData(CTL_WTMTOOL_FIGSIZE, &Data.FigSize);
			GetClusterData(CTL_WTMTOOL_BKGOPTIONS, &bkg_options);
			Data.ExtSize = sizeof(long);
			*(long *)Data.ExtData = bkg_options;
			ASSIGN_PTR(pData, Data);
			return ok;
		}
	};
	int    ok = -1;
	BkgToolDialog * dlg = new BkgToolDialog();
	if(CheckDialogPtrErr(&dlg)) {
		dlg->setDTS(pItem);
		while(ok < 0 && ExecView(dlg) == cmOK) {
			if(dlg->getDTS(pItem))
				ok = 1;
		}
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}

int WhatmanObjectBackground::HandleCommand(int cmd, void * pExt)
{
	int    ok = -1;
	switch(cmd) {
		case cmdSetupByTool:
			{
				const TWhatmanToolArray::Item * p_item = static_cast<const TWhatmanToolArray::Item *>(pExt);
				if(p_item) {
					BkgOptions = *reinterpret_cast<const long *>(p_item->ExtData);
					if(p_item->P_Owner) {
						const SDrawFigure * p_fig = p_item->P_Owner->GetFig(1, p_item->Symb, 0);
						if(p_fig) {
							P_Fig = p_fig->Dup();
							{
								//
								// ������ �� ������ ���������� ��������� �������
								//
								SViewPort vp;
								P_Fig->GetViewPort(&vp);
								vp.ParX = SViewPort::parNone;
								vp.ParY = SViewPort::parNone;
								P_Fig->SetViewPort(&vp);
							}
						}
					}
					ok = 1;
				}
			}
			break;
		case cmdEditTool:
			ok = EditTool(static_cast<TWhatmanToolArray::Item *>(pExt));
			break;
	}
	return ok;
}
//
//
//
class WhatmanObjectText : public WhatmanObjectDrawFigure {
public:
	WhatmanObjectText();
	~WhatmanObjectText();
protected:
	virtual TWhatmanObject * Dup() const;
	virtual int Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	virtual int Draw(TCanvas2 & rCanv);
	virtual int HandleCommand(int cmd, void * pExt);
private:
	STextLayout Tlo;
};

IMPLEMENT_WTMOBJ_FACTORY(Text, "Text");

WhatmanObjectText::WhatmanObjectText() : WhatmanObjectDrawFigure("Text")
{
}

WhatmanObjectText::~WhatmanObjectText()
{
}

TWhatmanObject * WhatmanObjectText::Dup() const
{
	WhatmanObjectText * p_obj = new WhatmanObjectText;
	if(p_obj) {
		p_obj->WhatmanObjectDrawFigure::Copy(*this);
		p_obj->Tlo.Copy(Tlo);
	}
	return p_obj;
}

int WhatmanObjectText::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	THROW(WhatmanObjectDrawFigure::Serialize(dir, rBuf, pCtx));
	THROW(Tlo.Serialize(dir, rBuf, pCtx));
	CATCHZOK
	return ok;
}

int WhatmanObjectText::HandleCommand(int cmd, void * pExt)
{
	int    ok = -1;
	switch(cmd) {
		case cmdSetBounds:
			{
				const TRect * p_rect = static_cast<const TRect *>(pExt);
				Tlo.SetBounds(FRect(static_cast<float>(p_rect->width()), static_cast<float>(p_rect->height())));
			}
			ok = 1;
			break;
		case cmdSetupByTool:
			ok = WhatmanObjectDrawFigure::HandleCommand(cmd, pExt);
			if(ok) {
				SString text("����� �����\n��� - ����� ������!\n� ��� � ������ ������");
				Tlo.SetOptions(Tlo.fWrap);
				Tlo.SetText(text.ToOem());
			}
			break;
		case cmdEditTool:
			ok = WhatmanObjectDrawFigure::HandleCommand(cmd, pExt);
			break;
		case cmdEdit:
			{
				WhatmanObjectBaseDialog * dlg = new WhatmanObjectBaseDialog(DLG_WOTEXT);
				if(CheckDialogPtrErr(&dlg)) {
					SString text;
					Tlo.GetText().CopyToUtf8(text, 0);
					text.Transf(CTRANSF_UTF8_TO_INNER);
					TInputLine * p_il = static_cast<TInputLine *>(dlg->getCtrlView(CTL_WOTEXT_TEXT));
					if(p_il) {
						p_il->setMaxLen(2048);
						dlg->setCtrlString(CTL_WOTEXT_TEXT, text);
						dlg->setRef(this); // @v10.4.9
						if(ExecView(dlg) == cmOK) {
							dlg->getCtrlString(CTL_WOTEXT_TEXT, text);
							Tlo.SetText(text);
							dlg->updateRef(this); // @v10.4.9
							ok = 1;
						}
					}
				}
				else
					ok = 0;
			}
			break;
	}
	return ok;
}

int WhatmanObjectText::Draw(TCanvas2 & rCanv)
{
	int    ok = 1;
	if(P_Fig) {
		LMatrix2D mtx;
		rCanv.PushTransform();
		mtx.InitTranslate(TWhatmanObject::GetBounds().a);
		rCanv.AddTransform(mtx);
		Tlo.Arrange(rCanv.operator SDrawContext(), rCanv.GetToolBox());
		ok = rCanv.DrawTextLayout(&Tlo);
		rCanv.PopTransform();
	}
	else
		ok = -1;
	return ok;
}
//
//
//
class WhatmanObjectProcessor : public WhatmanObjectDrawFigure {
public:
	WhatmanObjectProcessor();
	~WhatmanObjectProcessor();
	int    SetPrc(PPID prcID);
private:
	virtual TWhatmanObject * Dup() const;
	virtual int Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	virtual int Draw(TCanvas2 & rCanv);
	virtual int GetTextLayout(STextLayout & rTl, int options) const;
	virtual int HandleCommand(int cmd, void * pExt);
	static int Refresh(int kind, const PPNotifyEvent * pEv, void * procExtPtr);
	int    RefreshBusyStatus(int force);

	PPID   PrcID;     // @persistent
	ProcessorTbl::Rec PrcRec;
	long   AdvCookie;
	SCycleTimer Tmr;
	int    BusyStatus; // 0 - ��������, 1 - ����� (BusyDtm - ����� �� �������� �����), 2 - ��������, �� ����� ����� � BusyDtm
	LDATETIME BusyDtm;
	PPObjTSession TSesObj;
};

IMPLEMENT_WTMOBJ_FACTORY(Processor, "@wtmo_processor");

WhatmanObjectProcessor::WhatmanObjectProcessor() : WhatmanObjectDrawFigure("Processor"), Tmr(30000), PrcID(0), BusyStatus(0),
	AdvCookie(0), BusyDtm(ZERODATETIME)
{
	Options |= oSelectable;
	// @v10.7.9 @ctr MEMSZERO(PrcRec);
	TextParam tp;
	tp.Side = SIDE_BOTTOM;
	tp.AlongSize = -1.0f;
	tp.AcrossSize = 0.0f; //40.0f;
	SetTextOptions(&tp);
	//
	PPAdviseBlock adv_blk;
	adv_blk.Kind = PPAdviseBlock::evQuartz;
	adv_blk.ProcExtPtr = this;
	adv_blk.Proc = WhatmanObjectProcessor::Refresh;
	DS.Advise(&AdvCookie, &adv_blk);
}

WhatmanObjectProcessor::~WhatmanObjectProcessor()
{
	DS.Unadvise(AdvCookie);
}

/*static*/int WhatmanObjectProcessor::Refresh(int kind, const PPNotifyEvent * pEv, void * procExtPtr)
{
	int    ok = -1;
	if(kind == PPAdviseBlock::evQuartz) {
		WhatmanObjectProcessor * p_self = static_cast<WhatmanObjectProcessor *>(procExtPtr);
		if(p_self) {
			p_self->RefreshBusyStatus(0);
			ok = 1;
		}
	}
	return ok;
}

int WhatmanObjectProcessor::RefreshBusyStatus(int force)
{
	int    ok = 1;
	if(PrcID == 0) {
		if(BusyStatus != 1 || !!BusyDtm) {
			BusyStatus = 1;
			BusyDtm.Z();
			Redraw();
			ok = 1;
		}
		else
			ok = -1;
	}
	else if(force || Tmr.Check(0)) {
		LDATETIME curdtm = getcurdatetime_();
		PPID   tsess_id = 0;
		if(TSesObj.IsProcessorBusy(PrcID, 0, TSESK_SESSION, curdtm, 60*60*24, &tsess_id) > 0) {
			TSessionTbl::Rec tses_rec;
			if(TSesObj.Search(tsess_id, &tses_rec) > 0) {
				if(cmp(curdtm, tses_rec.StDt, tses_rec.StTm) < 0) {
					//
					// ������ ��� �� ��������.
					//
					BusyStatus = 2;
					BusyDtm.Set(tses_rec.StDt, tses_rec.StTm);
				}
				else {
					BusyStatus = 1;
					BusyDtm.Set(tses_rec.FinDt, tses_rec.FinTm);
				}
			}
			else {
				BusyStatus = 1;
				BusyDtm.Z();
			}
		}
		else {
			BusyStatus = 0;
			BusyDtm.Z();
		}
		Redraw();
	}
	else
		ok = -1;
	return ok;
}

int WhatmanObjectProcessor::SetPrc(PPID prcID)
{
	int    ok = -1;
	PrcID = prcID;
	if(TSesObj.GetPrc(PrcID, &PrcRec, 0, 1) > 0)
		ok = 1;
	else {
		MEMSZERO(PrcRec);
	}
	return ok;
}

TWhatmanObject * WhatmanObjectProcessor::Dup() const
{
	WhatmanObjectProcessor * p_obj = new WhatmanObjectProcessor;
	if(p_obj) {
		p_obj->WhatmanObjectDrawFigure::Copy(*this);
		p_obj->SetPrc(PrcID);
	}
	return p_obj;
}

int WhatmanObjectProcessor::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	THROW(WhatmanObjectDrawFigure::Serialize(dir, rBuf, pCtx));
	THROW(pCtx->Serialize(dir, PrcID, rBuf));
	if(dir < 0) {
		SetPrc(PrcID);
		RefreshBusyStatus(1);
	}
	CATCHZOK
	return ok;
}

int WhatmanObjectProcessor::Draw(TCanvas2 & rCanv)
{
	WhatmanObjectDrawFigure::Draw(rCanv);
	TRect b = GetBounds();
	const float _p = 8.0f;
	const float w = (float)b.width();
	const float h = (float)b.height();
	FPoint c((float)b.b.x - w/_p + w/(2*_p), (float)b.b.y - h/_p + h/(2*_p));
	float r = (MIN(w/_p, h/_p)) / 2.0f;
	rCanv.Arc(c, r, 0.0, SMathConst::Pi2_f);
	SColor clr;
	if(BusyStatus == 1)
		clr = SClrRed;
	else if(BusyStatus == 2)
		clr = SClrOrange;
	else
		clr = SClrGreen;
	rCanv.Fill(clr, 0);
	return 1;
}

int WhatmanObjectProcessor::GetTextLayout(STextLayout & rTlo, int options) const
{
	int    ok = 1;
	SString text, temp_buf;
	if(PrcRec.ID) {
		text = PrcRec.Name;
	}
	else {
		PPLoadString("processor", text);
		text.Space().CatChar('#').Cat(PrcID);
	}
	if(BusyStatus == 1) {
		if(!!BusyDtm) {
			PPLoadString("busyuntil", temp_buf);
			text.CR().Cat(temp_buf).Space();
			if(BusyDtm.d == getcurdate_())
				text.Cat(BusyDtm.t, TIMF_HM);
			else
				text.Cat(BusyDtm, DATF_DMY, TIMF_HM);
		}
	}
	else if(BusyStatus == 2) {
		if(!!BusyDtm) {
			PPLoadString("freeuntil", temp_buf);
			text.CR().Cat(temp_buf).Space();
			if(BusyDtm.d == getcurdate_())
				text.Cat(BusyDtm.t, TIMF_HM);
			else
				text.Cat(BusyDtm, DATF_DMY, TIMF_HM);
		}
	}
	if(!(options & (gtloBoundsOnly | gtloQueryForArrangeObject))) {
		rTlo.SetText(text);
		rTlo.SetOptions(STextLayout::fWrap, PPWhatmanWindow::paraToolText,
			(State & stCurrent) ? PPWhatmanWindow::csToolTextSel : PPWhatmanWindow::csToolText);
	}
	rTlo.SetBounds(FRect(GetTextBounds()));
	return ok;
}

int WhatmanObjectProcessor::HandleCommand(int cmd, void * pExt)
{
	int    ok = -1;
	if(cmd == cmdEdit) {
		WhatmanObjectBaseDialog * dlg = new WhatmanObjectBaseDialog(DLG_WOPRC);
		if(CheckDialogPtrErr(&dlg)) {
			SetupPPObjCombo(dlg, CTLSEL_WOPRC_PRC, PPOBJ_PROCESSOR, PrcID, OLW_CANINSERT, 0);
			dlg->setRef(this); // @v10.4.9
			if(ExecView(dlg) == cmOK) {
				SetPrc(dlg->getCtrlLong(CTLSEL_WOPRC_PRC));
				dlg->updateRef(this); // @v10.4.9
				ok = 1;
			}
		}
		else
			ok = 0;
		delete dlg;
	}
	else if(cmd == cmdGetSelRetBlock) {
		SelectObjRetBlock * p_blk = (SelectObjRetBlock *)pExt;
		if(p_blk) {
			p_blk->WtmObjTypeSymb = Symb;
			p_blk->Val1 = PPOBJ_PROCESSOR;
			p_blk->Val2 = PrcID;
			p_blk->ExtString.Z().Cat(PrcID);
		}
		ok = 1;
	}
	else
		ok = WhatmanObjectDrawFigure::HandleCommand(cmd, pExt);
	return ok;
}
//
//
//
class WhatmanObjectBarcode : public WhatmanObjectDrawFigure {
public:
	WhatmanObjectBarcode();
	~WhatmanObjectBarcode();
private:
	virtual TWhatmanObject * Dup() const;
	virtual int Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	virtual int Draw(TCanvas2 & rCanv);
	virtual int GetTextLayout(STextLayout & rTl, int options) const;
	virtual int HandleCommand(int cmd, void * pExt);

	PPBarcode::BarcodeImageParam P;
};

IMPLEMENT_WTMOBJ_FACTORY(Barcode, "@wtmo_barcode");

WhatmanObjectBarcode::WhatmanObjectBarcode() : WhatmanObjectDrawFigure("Barcode")
{
	Options |= oSelectable;
	P.Std = BARCSTD_EAN13;
	TextParam tp;
	tp.Side = SIDE_BOTTOM;
	tp.AlongSize = -1.0f;
	tp.AcrossSize = 32.0f;
	SetTextOptions(&tp);
}

WhatmanObjectBarcode::~WhatmanObjectBarcode()
{
}

TWhatmanObject * WhatmanObjectBarcode::Dup() const
{
	WhatmanObjectBarcode * p_obj = new WhatmanObjectBarcode;
	if(p_obj) {
		p_obj->WhatmanObjectDrawFigure::Copy(*this);
		p_obj->P = P;
	}
	return p_obj;
}

int WhatmanObjectBarcode::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	THROW(WhatmanObjectDrawFigure::Serialize(dir, rBuf, pCtx));
	THROW(pCtx->Serialize(dir, P.Std, rBuf));
	THROW(pCtx->Serialize(dir, P.Code, rBuf));
	THROW(pCtx->Serialize(dir, P.Size, rBuf));
	THROW(pCtx->Serialize(dir, P.Angle, rBuf));
	CATCHZOK
	return ok;
}

int WhatmanObjectBarcode::Draw(TCanvas2 & rCanv)
{
	int    r = 0;
	if(P.Code.NotEmpty()) {
		TRect b = GetBounds();
		if(oneof4(P.Std, BARCSTD_EAN13, BARCSTD_EAN8, BARCSTD_UPCA, BARCSTD_UPCE))
			P.Size.Z();
		else
			P.Size.Set(b.width(), b.height());
		P.ColorFg = SClrDarkcyan;
		P.ColorBg = SClrYellow;
		if(PPBarcode::CreateImage(P)) {
			LMatrix2D mtx;
			rCanv.PushTransform();
			SViewPort vp;
			vp.a.SetZero();
			vp.b = P.Buffer.GetDimF();
			vp.ParX = SViewPort::parMid;
			vp.ParY = SViewPort::parMax;
			vp.Flags &= ~SViewPort::fEmpty;
			rCanv.AddTransform(vp.GetMatrix(GetBounds(), mtx));
			rCanv.Draw(&P.Buffer);
			rCanv.PopTransform();
			r = 1;
		}
	}
	if(!r)
		WhatmanObjectDrawFigure::Draw(rCanv);
	return 1;
}

int WhatmanObjectBarcode::GetTextLayout(STextLayout & rTlo, int options) const
{
	return -1;
}

class WoBarcodeParamDialog : public WhatmanObjectBaseDialog {
public:
	WoBarcodeParamDialog() : WhatmanObjectBaseDialog(DLG_WOBARCODE)
	{
	}
	int    setDTS(const PPBarcode::BarcodeImageParam * pData)
	{
		RVALUEPTR(Data, pData);
		setCtrlString(CTL_WOBARCODE_CODE, Data.Code);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 0, 0);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 1, BARCSTD_EAN13);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 2, BARCSTD_EAN8);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 3, BARCSTD_UPCA);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 4, BARCSTD_UPCE);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 5, BARCSTD_CODE39);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 6, BARCSTD_PDF417);
		AddClusterAssocDef(CTL_WOBARCODE_STD, 7, BARCSTD_QR);
		SetClusterData(CTL_WOBARCODE_STD, Data.Std);
		AddClusterAssocDef(CTL_WOBARCODE_ANGLE, 0, 0);
		AddClusterAssocDef(CTL_WOBARCODE_ANGLE, 1, 90);
		AddClusterAssocDef(CTL_WOBARCODE_ANGLE, 2, 180);
		AddClusterAssocDef(CTL_WOBARCODE_ANGLE, 3, 270);
		SetClusterData(CTL_WOBARCODE_ANGLE, Data.Angle);
		return 1;
	}
	int    getDTS(PPBarcode::BarcodeImageParam * pData)
	{
		int    ok = 1;
		getCtrlString(CTL_WOBARCODE_CODE, Data.Code);
		Data.Std = GetClusterData(CTL_WOBARCODE_STD);
		Data.Angle = GetClusterData(CTL_WOBARCODE_ANGLE);
		ASSIGN_PTR(pData, Data);
		return ok;
	}
private:
	DECL_HANDLE_EVENT
	{
		TDialog::handleEvent(event);
		if(event.isCmd(cmInputUpdated)) {
			if(event.isCtlEvent(CTL_WOBARCODE_CODE)) {
				getCtrlString(CTL_WOBARCODE_CODE, Data.Code);
				if(Data.Code.NotEmptyS()) {
					int    code_std = 0;
					int    code_diag = 0;
					SString norm_code;
					int    dcr = PPObjGoods::DiagBarcode(Data.Code, &code_diag, &code_std, &norm_code);
					if(dcr > 0 && oneof4(code_std, BARCSTD_EAN13, BARCSTD_EAN8, BARCSTD_UPCA, BARCSTD_UPCE)) {
						if(code_std != Data.Std) {
							Data.Std = code_std;
							SetClusterData(CTL_WOBARCODE_STD, Data.Std);
						}
					}
					else {
						if(Data.Code.IsDigit()) {
							if(Data.Std == 0) {
								Data.Std = BARCSTD_CODE39;
								SetClusterData(CTL_WOBARCODE_STD, Data.Std);
							}
						}
						else {
							if(Data.Std == 0) {
								Data.Std = BARCSTD_QR;
								SetClusterData(CTL_WOBARCODE_STD, Data.Std);
							}
						}
					}
				}
			}
			else
				return;
		}
		else
			return;
		clearEvent(event);
	}
	PPBarcode::BarcodeImageParam Data;
};

int WhatmanObjectBarcode::HandleCommand(int cmd, void * pExt)
{
	int    ok = -1;
	if(cmd == cmdEdit) {
		WoBarcodeParamDialog * dlg = new WoBarcodeParamDialog();
		if(CheckDialogPtrErr(&dlg)) {
			dlg->setRef(this); // @v10.4.9
			dlg->setDTS(&P);
			if(ExecView(dlg) == cmOK) {
				if(dlg->getDTS(&P)) {
					dlg->updateRef(this); // @v10.4.9
					ok = 1;
				}
			}
		}
		else
			ok = 0;
		delete dlg;
	}
	else
		ok = WhatmanObjectDrawFigure::HandleCommand(cmd, pExt);
	return ok;
}
//
//
//
class WhatmanObjectCafeTable : public WhatmanObjectDrawFigure {
public:
	WhatmanObjectCafeTable();
	~WhatmanObjectCafeTable();
private:
	virtual TWhatmanObject * Dup() const;
	virtual int Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx);
	virtual int Draw(TCanvas2 & rCanv);
	virtual int GetTextLayout(STextLayout & rTl, int options) const;
	virtual int HandleCommand(int cmd, void * pExt);
	static int Refresh(int kind, const PPNotifyEvent * pEv, void * procExtPtr);
	int    RefreshBusyStatus(int force);

	long   TableNo;
	CTableStatus Status; // @transient
	long   AdvCookie; // @transient
	SCycleTimer Tmr;  // @transient
	CTableOrder * P_Cto;
};

/*static*/int WhatmanObjectCafeTable::Refresh(int kind, const PPNotifyEvent * pEv, void * procExtPtr)
{
	int    ok = -1;
	if(kind == PPAdviseBlock::evQuartz) {
		WhatmanObjectCafeTable * p_self = (WhatmanObjectCafeTable *)procExtPtr;
		if(p_self) {
			p_self->RefreshBusyStatus(0);
			ok = 1;
		}
	}
	return ok;
}

int WhatmanObjectCafeTable::RefreshBusyStatus(int force)
{
	int    ok = 1;
	if(force || Tmr.Check(0)) {
		CCheckCore::FetchCTableStatus(TableNo, &Status);
		Redraw();
	}
	else
		ok = -1;
	return ok;
}

IMPLEMENT_WTMOBJ_FACTORY(CafeTable, "@wtmo_cafetable");

WhatmanObjectCafeTable::WhatmanObjectCafeTable() : WhatmanObjectDrawFigure("CafeTable"), Tmr(30000), TableNo(0), AdvCookie(0), P_Cto(0)
{
	Options |= oSelectable;
	TextParam tp;
	tp.Side = SIDE_BOTTOM;
	tp.AlongSize = -1.0f;
	tp.AcrossSize = 32.0f;
	SetTextOptions(&tp);
	//
	PPAdviseBlock adv_blk;
	adv_blk.Kind = PPAdviseBlock::evQuartz;
	adv_blk.ProcExtPtr = this;
	adv_blk.Proc = WhatmanObjectCafeTable::Refresh;
	DS.Advise(&AdvCookie, &adv_blk);
}

WhatmanObjectCafeTable::~WhatmanObjectCafeTable()
{
	DS.Unadvise(AdvCookie);
	delete P_Cto;
}

TWhatmanObject * WhatmanObjectCafeTable::Dup() const
{
	WhatmanObjectCafeTable * p_obj = new WhatmanObjectCafeTable;
	if(p_obj) {
		p_obj->WhatmanObjectDrawFigure::Copy(*this);
		p_obj->TableNo = TableNo;
	}
	return p_obj;
}

int WhatmanObjectCafeTable::Serialize(int dir, SBuffer & rBuf, SSerializeContext * pCtx)
{
	int    ok = 1;
	THROW(WhatmanObjectDrawFigure::Serialize(dir, rBuf, pCtx));
	THROW(pCtx->Serialize(dir, TableNo, rBuf));
	if(dir < 0)
		RefreshBusyStatus(1);
	CATCHZOK
	return ok;
}

int WhatmanObjectCafeTable::Draw(TCanvas2 & rCanv)
{
	WhatmanObjectDrawFigure::Draw(rCanv);
	TRect b = GetBounds();
	const float _p = 6.0f;
	const float w = (float)b.width();
	const float h = (float)b.height();
	FPoint c((float)b.b.x - w/_p + w/(2*_p), (float)b.b.y - h/_p + h/(2*_p));
	float r = (MIN(w/_p, h/_p)) / 2.0f;
	rCanv.Arc(c, r, 0.0, (float)SMathConst::Pi2);
	SColor clr;
	if(Status.Status == CTableStatus::sBusy)
		clr = SColor(SClrRed);
	else if(Status.Status == CTableStatus::sOnPayment)
		clr = SColor(SClrOrange);
	else if(Status.Status == CTableStatus::sOrder)
		clr = SColor(SClrBlue);
	else
		clr = SColor(SClrGreen);
	rCanv.Fill(clr, 0);
	return 1;
}

int WhatmanObjectCafeTable::GetTextLayout(STextLayout & rTlo, int options) const
{
	int    ok = 1;
	SString text, temp_buf;
	PPLoadString("ftable", text);
	PPObjCashNode::GetCafeTableName(TableNo, temp_buf);
	text.Space().Cat(temp_buf);
	if(Status.Status == CTableStatus::sOrder) {
		temp_buf.Z().Cat("Order").Space();
		Status.OrderTime.ToStr(temp_buf, STimeChunk::fmtOmitSec);
		text.CR().Cat(temp_buf);
	}
	if(!(options & (gtloBoundsOnly | gtloQueryForArrangeObject))) {
		rTlo.SetText(text);
		rTlo.SetOptions(STextLayout::fWrap, PPWhatmanWindow::paraToolText,
			(State & stCurrent) ? PPWhatmanWindow::csToolTextSel : PPWhatmanWindow::csToolText);
	}
	rTlo.SetBounds(FRect(GetTextBounds()));
	return ok;
}

int WhatmanObjectCafeTable::HandleCommand(int cmd, void * pExt)
{
	int    ok = -1;
	if(cmd == cmdEdit) {
		WhatmanObjectBaseDialog * dlg = new WhatmanObjectBaseDialog(DLG_WOCAFETBL);
		if(CheckDialogPtrErr(&dlg)) {
			dlg->setRef(this); // @v10.4.9
			dlg->setCtrlLong(CTL_WOCAFETBL_N, TableNo);
			if(ExecView(dlg) == cmOK) {
				dlg->updateRef(this); // @v10.4.9
				TableNo = dlg->getCtrlLong(CTL_WOCAFETBL_N);
				ok = 1;
			}
		}
		else
			ok = 0;
		delete dlg;
	}
	else if(cmd == cmdGetSelRetBlock) {
		SelectObjRetBlock * p_blk = static_cast<SelectObjRetBlock *>(pExt);
		if(p_blk) {
			p_blk->WtmObjTypeSymb = Symb;
			p_blk->Val1 = PPOBJ_CAFETABLE;
			p_blk->Val2 = TableNo;
			p_blk->ExtString.Z().Cat(TableNo);
		}
		ok = 1;
	}
	else if(cmd == cmdMouseHover) {
		TWindow * p_owner_win = GetOwnerWindow();
		if(p_owner_win) {
			SString text, temp_buf;
			PPLoadString("ftable", text);
			text.Space().Cat(TableNo).CR();
			switch(Status.Status) {
				case CTableStatus::sFree: text.Cat(PPLoadTextS(PPTXT_CTABLEFREE, temp_buf)); break;
				case CTableStatus::sBusy: text.Cat(PPLoadTextS(PPTXT_CTABLEBUSY, temp_buf)); break;
				case CTableStatus::sOnPayment: text.Cat(PPLoadTextS(PPTXT_CTABLEONPAYMENT, temp_buf)); break;
				case CTableStatus::sOrder:
					if(Status.CheckID) {
						SETIFZ(P_Cto, new CTableOrder);
						if(P_Cto) {
							CTableOrder::Packet ord;
							if(P_Cto->GetCheck(Status.CheckID, &ord) > 0) {
								text.Z();
								SCardTbl::Rec sc_rec;
								PPLoadString("booking", temp_buf);
								text.Cat(temp_buf).CR();
								ord.Chunk.ToStr(temp_buf.Z(), STimeChunk::fmtOmitSec);
								text.Cat(temp_buf).CR();
								temp_buf.Z();
								if(ord.SCardID && P_Cto->GetSCard(ord.SCardID, &sc_rec) > 0) {
									text.Cat(sc_rec.Code).Space();
									if(sc_rec.PersonID) {
										GetPersonName(sc_rec.PersonID, temp_buf);
										text.Cat(temp_buf);
									}

								}
								if(ord.PrepayAmount != 0.0) {
									PPLoadString("prepay", temp_buf);
									text.CR().Cat(temp_buf).CatDiv(':', 2).Cat(ord.PrepayAmount, SFMT_MONEY);
								}
							}
						}
					}
					break;
			}
			if(text.NotEmpty()) {
				const long flags = SMessageWindow::fShowOnCursor|SMessageWindow::fCloseOnMouseLeave|SMessageWindow::fTextAlignLeft|
					SMessageWindow::fOpaque|SMessageWindow::fSizeByText|SMessageWindow::fChildWindow;
				SMessageWindow * p_win = new SMessageWindow;
				if(p_win) {
					text.ReplaceChar('\003', ' ').Strip();
					p_win->Open(text, 0, p_owner_win->H(), 0, 5000, 0, flags, 0);
				}
			}
		}
	}
	else
		ok = WhatmanObjectDrawFigure::HandleCommand(cmd, pExt);
	return ok;
}
//
//
//
PPWhatmanWindow::ToolObject::ToolObject(const char * pToolSymb, const TWhatmanToolArray * pTools, const TRect & rBounds) :
	TWhatmanObject("Tool"), ToolSymb(pToolSymb), P_Tools(pTools)
{
	Options |= TWhatmanObject::oDraggable;
	SetBounds(rBounds);
	TextParam tp;
	tp.Side = SIDE_BOTTOM;
	tp.AlongSize = -2.0f;
	tp.AcrossSize = -1.0f;
	SetTextOptions(&tp);
}

TWhatmanObject * PPWhatmanWindow::ToolObject::Dup() const
{
	PPWhatmanWindow::ToolObject * p_obj = new PPWhatmanWindow::ToolObject(ToolSymb, P_Tools, GetBounds());
	CALLPTRMEMB(p_obj, TWhatmanObject::Copy(*this));
	return p_obj;
}

int PPWhatmanWindow::ToolObject::Draw(TCanvas2 & rCanv)
{
	int    ok = 1;
	LMatrix2D mtx;
	SViewPort vp;
	const SDrawFigure * p_fig = P_Tools->GetFig(0, ToolSymb, 0);
	THROW(p_fig);
	rCanv.PushTransform();
	p_fig->GetViewPort(&vp);
	rCanv.AddTransform(vp.GetMatrix(GetBounds(), mtx));
	rCanv.Draw(p_fig);
	rCanv.PopTransform();
	CATCHZOK
	return ok;
}

int PPWhatmanWindow::ToolObject::GetTextLayout(STextLayout & rTlo, int options) const
{
	int    ok = -1;
	TWhatmanToolArray::Item tool_item;
	if(P_Tools && P_Tools->GetBySymb(ToolSymb, &tool_item)) {
		if(tool_item.Text.NotEmpty()) {
			if(!(options & (gtloBoundsOnly | gtloQueryForArrangeObject))) {
				rTlo.SetText(tool_item.Text);
				rTlo.SetOptions(STextLayout::fWrap, paraToolText, (State & stCurrent) ? csToolTextSel : csToolText);
			}
			rTlo.SetBounds(FRect(GetTextBounds()));
			ok = 1;
		}
	}
	else
		ok = 0;
	return ok;
}

PPWhatmanWindow::ResizeState::ResizeState() : P_MovedObjCopy(0)
{
	Reset();
}

PPWhatmanWindow::ResizeState::~ResizeState()
{
	delete P_MovedObjCopy;
}

int PPWhatmanWindow::ResizeState::operator ! () const
{
	return (Kind == kNone);
}

void PPWhatmanWindow::ResizeState::Reset()
{
	Kind = kNone;
	ObjIdx = -1;
	ObjRszDir = 0;
	Flags = 0;
	StartPt.Z();
	EndPt.Z();
	ZDELETE(P_MovedObjCopy);
}

int PPWhatmanWindow::ResizeState::Setup(int mode, const Loc & rLoc, int * pCursorIdent)
{
	int    result = 0;
	int    cur_ident = 0;
	if(rLoc.Kind == Loc::kObject) {
		ObjIdx = rLoc.ObjIdx;
		if(rLoc.ObjRszDir) {
			if(rLoc.P_Obj && rLoc.P_Obj->HasOption(TWhatmanObject::oResizable) && mode == modeEdit) {
				Kind = kObjResize;
				ObjRszDir = rLoc.ObjRszDir;
				if(oneof2(ObjRszDir, SOW_NORD, SOW_SOUTH))
					cur_ident = curResizeNS;
				else if(oneof2(ObjRszDir, SOW_WEST, SOW_EAST))
					cur_ident = curResizeEW;
				else if(oneof2(ObjRszDir, SOW_NORDEAST, SOW_SOUTHWEST))
					cur_ident = curResizeNESW;
				else if(oneof2(ObjRszDir, SOW_SOUTHEAST, SOW_NORDWEST))
					cur_ident = curResizeNWSE;
				StartPt = EndPt = rLoc.Pt;
				result = 1;
			}
		}
		else if(rLoc.P_Obj) {
			if(rLoc.P_Obj->HasOption(TWhatmanObject::oMovable) && mode == modeEdit) {
				Kind = kObjMove;
				ObjRszDir = 0;
				cur_ident = PPWhatmanWindow::curMove;
				StartPt = EndPt = rLoc.Pt;
				result = 1;
			}
			else if(rLoc.P_Obj->HasOption(TWhatmanObject::oDraggable) && mode == modeToolbox) {
				Kind = kObjDrag;
				ObjRszDir = 0;
				cur_ident = PPWhatmanWindow::curOdious;
				StartPt = EndPt = rLoc.Pt;
				result = 1;
			}
			else if(rLoc.P_Obj->HasOption(TWhatmanObject::oSelectable)) {
				Kind = kNone;
				cur_ident = PPWhatmanWindow::curOdious;
			}
		}
	}
	else if(rLoc.Kind == Loc::kObjectInMultSel && mode == modeEdit) {
		Kind = kMultObjMove;
		ObjRszDir = 0;
		cur_ident = PPWhatmanWindow::curMove;
		StartPt = EndPt = rLoc.Pt;
		result = 1;
	}
	else if(rLoc.Kind == Loc::kWorkspace && mode == modeEdit) {
		Reset();
		Kind = kRectSelection;
		StartPt = EndPt = rLoc.Pt;
		result = 1;
	}
	else
		Reset();
	ASSIGN_PTR(pCursorIdent, cur_ident);
	return result;
}

#pragma warning(disable : 4355) // ������ ��������� � ���, ��� this ������������ � ������ �������������

PPWhatmanWindow::PPWhatmanWindow(int mode) : TWindowBase(_T("SLibWindowBase"), wbcDrawBuffer), W(this)
{
	St.Mode = mode;
	St.SelectedObjIdx = -1;
	Tb.CreatePen(penMain, SPaintObj::psSolid, 5, SColor(SClrGreen));
	Tb.CreateBrush(brBackg, SPaintObj::bsSolid, SColor(SClrWhite), 0);
	Tb.CreateBrush(brGreen, SPaintObj::bsSolid, SColor(SClrGreen), 0);
	Tb.CreateFont_(fontMain, "Arial Cyr", SDrawContext::CalcScreenFontSizePt(9), SFontDescr::fAntialias);
	{
		int    tool_text_font_id = Tb.CreateFont_(0, "Arial Narrow", SDrawContext::CalcScreenFontSizePt(8), SFontDescr::fAntialias);
		int    tool_text_pen_id = Tb.CreateColor(0, SColor(SClrBlack));
		int    tool_text_sel_pen_id = Tb.CreateColor(0, SColor(SClrWhite));
		int    tool_text_brush_id = 0;
		int    tool_text_sel_brush_id = Tb.CreateColor(0, SColor(SClrNavy));
		Tb.CreateCStyle(csToolText, tool_text_font_id, tool_text_pen_id, tool_text_brush_id);
		Tb.CreateCStyle(csToolTextSel, tool_text_font_id, tool_text_sel_pen_id, tool_text_sel_brush_id);
	}
	{
		SParaDescr pd;
		pd.Flags |= SParaDescr::fJustCenter;
		Tb.CreateParagraph(paraToolText, &pd);
	}
	Tb.CreateCursor(curMove, IDC_MOVE);
	Tb.CreateCursor(curResizeEW, IDC_SIZE_EW);
	Tb.CreateCursor(curResizeNS, IDC_SIZE_NS);
	Tb.CreateCursor(curResizeNESW, IDC_SIZE_NESW);
	Tb.CreateCursor(curResizeNWSE, IDC_SIZE_NWSE);
	Tb.CreateCursor(curOdious, IDC_PPYPOINT); // @debug

	Tb.CreatePen(penObjBorder, SPaintObj::psSolid, 0.0f, SColor(SClrBlack));
	Tb.CreatePen(penObjBorderSel, SPaintObj::psDash, 1.0f, SColor(SClrBlue));
	Tb.CreatePen(penObjBorderCur, SPaintObj::psDash, 1.0f, SColor(SClrLime));
	Tb.CreatePen(penObjRszSq, SPaintObj::psSolid, 1.0f, SColor(SClrBlack));
	Tb.CreateBrush(brObjRszSq, SPaintObj::bsSolid, SColor(SClrLime), 0);
	Tb.CreatePen(penObjNonmovBorder, SPaintObj::psSolid, 1.0f, SColor(SClrDeepskyblue));
	Tb.CreatePen(penRule, SPaintObj::psSolid, 1.0f, SColor(SClrBlack));
	Tb.CreateBrush(brRule, SPaintObj::bsSolid, SColor(SClrYellow).Lighten(0.5f), 0);

	Tb.CreatePen(penGrid, SPaintObj::psDash, 1.0f, SColor(0.7f));
	Tb.CreatePen(penSubGrid, SPaintObj::psDot, 1.0f, SColor(0.7f));
	Tb.CreatePen(penLayoutBorder, SPaintObj::psDot, 2.0f, SColor(SClrDarkgreen)); // @v10.4.8 // @v10.9.6 1.0f-->2.0f, SClrGreen-->SClrDarkgreen
	Tb.CreatePen(penContainerCandidateBorder, SPaintObj::psSolid, 2.0f, SColor(SClrCoral)); // @v10.9.6

	W.SetTool(TWhatman::toolPenObjBorder, penObjBorder);
	W.SetTool(TWhatman::toolPenObjBorderSel, penObjBorderSel);
	W.SetTool(TWhatman::toolPenObjBorderCur, penObjBorderCur);
	W.SetTool(TWhatman::toolPenObjRszSq, penObjRszSq);
	W.SetTool(TWhatman::toolBrushObjRszSq, brObjRszSq);
	W.SetTool(TWhatman::toolPenObjNonmovBorder, penObjNonmovBorder);
	W.SetTool(TWhatman::toolPenRule, penRule);
	W.SetTool(TWhatman::toolBrushRule, brRule);
	W.SetTool(TWhatman::toolPenGrid, penGrid);
	W.SetTool(TWhatman::toolPenSubGrid, penSubGrid);
	W.SetTool(TWhatman::toolPenLayoutBorder, penLayoutBorder); // @v10.4.8
	W.SetTool(TWhatman::toolPenContainerCandidateBorder, penContainerCandidateBorder); // @v10.4.8
	if(St.Mode == modeToolbox) {
		TWhatmanToolArray::Param param;
		Tools.GetParam(param);
		param.Ap.Dir = DIREC_HORZ;
		param.Ap.UlGap = 8;
		param.Ap.LrGap = 8;
		param.Ap.InnerGap = 5;
		Tools.SetParam(param);
	}
	else if(St.Mode == modeEdit) {
		TWhatman::Param param = W.GetParam();
		param.Flags |= (TWhatman::Param::fRule | TWhatman::Param::fGrid | TWhatman::Param::fSnapToGrid);
		W.SetParam(param);
	}
}

#pragma warning(default : 4355)

PPWhatmanWindow::~PPWhatmanWindow()
{
}

SPaintToolBox & PPWhatmanWindow::GetToolBox()
{
	return Tb;
}

int PPWhatmanWindow::AddObject(TWhatmanObject * pObj, const TRect * pBounds)
{
	int    ok = 1;
	if(pObj) {
		if(pBounds)
			pObj->SetBounds(*pBounds);
		W.InsertObject(pObj);
	}
	else
		ok = -1;
	return ok;
}

int PPWhatmanWindow::ArrangeObjects(const LongArray * pObjPosList, TArrangeParam & rParam)
{
	int    ok = 1;
	W.ArrangeObjects2(pObjPosList, rParam); // @v10.9.3 ArrangeObjects-->ArrangeObjects2
	return ok;
}

int PPWhatmanWindow::Rearrange()
{
	int    ok = -1;
	if(St.Mode == modeToolbox) {
		TWhatmanToolArray::Param param;
		Tools.GetParam(param);
		ArrangeObjects(0, param.Ap);
		invalidateAll(1);
		::UpdateWindow(H());
		ok = 1;
	}
	return ok;
}

int PPWhatmanWindow::Locate(TPoint p, Loc * pLoc) const
{
	int    ok = 0;
	int    obj_idx = 0;
	pLoc->Pt = p;
	int    r = W.FindObjectByPoint(p, &obj_idx);
	if(r > 0) {
		if(W.IsMultSelObject(obj_idx) && W.HaveMultSelObjectsOption(TWhatmanObject::oMovable)) {
			ok = pLoc->Kind = Loc::kObjectInMultSel;
			pLoc->ObjRszDir = 0;
		}
		else {
			ok = pLoc->Kind = Loc::kObject;
			pLoc->ObjRszDir = (r > 100) ? (r - 100) : 0;
		}
		pLoc->P_Obj = W.GetObjectByIndexC(obj_idx);
		pLoc->ObjIdx = obj_idx;
	}
	else {
		pLoc->P_Obj = 0;
		if(getClientRect().contains(p)) {
			ok = pLoc->Kind = Loc::kWorkspace;
			pLoc->ObjIdx = 0;
			pLoc->ObjRszDir = 0;
		}
		else {
			ok = pLoc->Kind = Loc::kNone;
			pLoc->ObjIdx = 0;
			pLoc->ObjRszDir = 0;
		}
	}
	return ok;
}

int PPWhatmanWindow::InvalidateObjScope(const TWhatmanObject * pObj)
{
	return W.InvalidateObjScope(pObj);
}

int PPWhatmanWindow::Resize(int mode, TPoint p)
{
	int    ok = -1;
	int    do_update_win = 0;
	TRect  b, temp_rect;
	if(mode == 1) {
		//
		// ������ ������ ��������� ��������
		//
		if(!St.Rsz) {
			int    cur_ident = 0;
			Loc    loc;
			if(Locate(p, &loc)) {
				if(St.Rsz.Setup(St.Mode, loc, &cur_ident)) {
					if(St.Rsz.ObjIdx >= 0) {
						if(St.Rsz.Kind == ResizeState::kObjMove) {
							const TWhatmanObject * p_obj = W.GetObjectByIndexC(St.Rsz.ObjIdx);
							if(p_obj)
								St.Rsz.P_MovedObjCopy = p_obj->Dup();
						}
						else if(St.Rsz.Kind == ResizeState::kMultObjMove) {
							W.InvalidateMultSelContour(0);
							do_update_win = 1;
						}
						else if(St.Rsz.Kind == ResizeState::kObjDrag) {
							::ReleaseCapture();
							TWhatmanObject * p_obj = W.GetObjectByIndex(St.Rsz.ObjIdx);
							if(p_obj) {
								SImageBuffer img_buf(32, 32);
								p_obj->DrawToImage(Tb, img_buf);
								St.Rsz.P_MovedObjCopy = new WhatmanObjectDrawFigure(new SDrawImage(img_buf, 0));
								if(St.Mode == modeToolbox) {
									Dib.P_SrcWin = this;
									const char * p_symb = ((const ToolObject *)p_obj)->ToolSymb;
									TWhatmanToolArray::Item wta_item;
									if(Tools.GetBySymb(p_symb, &Dib.WtaItem))
										SLS.SetupDragndropObj(SlSession::ddotLocalWhatmanToolArrayItem, &Dib);
								}
							}
						}
						if(St.Rsz.P_MovedObjCopy) {
							InvalidateObjScope(St.Rsz.P_MovedObjCopy);
							do_update_win = 1;
						}
					}
					else if(St.Rsz.Kind == ResizeState::kRectSelection) {
						W.SetSelArea(p, 1);
					}
					if(cur_ident) {
						::SetCursor(Tb.GetCursor(cur_ident));
					}
				}
			}
		}
	}
	else if(mode == 3) {
		//
		// ������ ������ �������������� ������ ������� ��� Drag'N'Drop
		//
		if(!St.Rsz) {
			DdotInfoBlock * p_dot_blk = 0;
			if(SLS.IsThereDragndropObj((void **)&p_dot_blk) == SlSession::ddotLocalWhatmanToolArrayItem) {
				const TWhatmanToolArray * p_wta = p_dot_blk->WtaItem.P_Owner;
				if(p_wta) {
					const SDrawFigure * p_fig = p_wta->GetFig(1, p_dot_blk->WtaItem.Symb, 0);
					if(p_fig) {
						DragndropEvent ev;
						ev.Action = DragndropEvent::acnGet;
						if(TView::messageCommand(p_dot_blk->P_SrcWin, cmDragndropObj, &ev)) {
							St.Rsz.P_MovedObjCopy = new WhatmanObjectDrawFigure(p_fig->Dup());
							(b = p_dot_blk->WtaItem.FigSize).movecenterto(p);
							b.a = W.TransformScreenToPoint(b.a);
							b.b = W.TransformScreenToPoint(b.b);
							W.MoveObject(St.Rsz.P_MovedObjCopy, b);
							St.Rsz.Kind = ResizeState::kObjDragTarget;
							InvalidateObjScope(St.Rsz.P_MovedObjCopy);
							do_update_win = 1;
							ok = 1;
						}
					}
				}
			}
		}
	}
	else if(mode == -1) {
		//
		// ���� ���������������� ������� �� ������� ����-����
		//
		if(!!St.Rsz) {
			if(St.Rsz.Kind == ResizeState::kObjDragTarget) {
				DdotInfoBlock * p_dot_blk = 0;
				if(SLS.IsThereDragndropObj((void **)&p_dot_blk) == SlSession::ddotLocalWhatmanToolArrayItem) {
					DragndropEvent ev;
					ev.Action = DragndropEvent::acnLeave;
					TView::messageCommand(p_dot_blk->P_SrcWin, cmDragndropObj, &ev);
				}
				if(St.Rsz.P_MovedObjCopy) {
					St.Rsz.Flags |= ResizeState::fDontDrawMovedObj;
					InvalidateObjScope(St.Rsz.P_MovedObjCopy);
					do_update_win = 1;
				}
				St.Rsz.Reset();
				SetDefaultCursor();
				ok = 1;
			}
		}
	}
	else if(mode == 0) {
		//
		// ���������� ������ ��������� ��������
		//
		if(!!St.Rsz) {
			if(St.Rsz.P_MovedObjCopy) {
				InvalidateObjScope(St.Rsz.P_MovedObjCopy);
				if(St.Rsz.Kind == ResizeState::kObjDragTarget) {
					DdotInfoBlock * p_dot_blk = 0;
					if(SLS.IsThereDragndropObj((void **)&p_dot_blk) == SlSession::ddotLocalWhatmanToolArrayItem) {
						TWhatmanObject * p_obj = TWhatmanObject::CreateInstance(p_dot_blk->WtaItem.WtmObjSymb);
						if(p_obj && p_obj->Setup(&p_dot_blk->WtaItem)) {
							if(p_obj->HasOption(TWhatmanObject::oBackground)) {
								b.setwidthrel(0, W.GetArea().width());
								b.setheightrel(0, W.GetArea().height());
							}
							else {
								b = p_obj->GetBounds().movecenterto(p);
								b.a = W.TransformScreenToPoint(b.a);
								b.b = W.TransformScreenToPoint(b.b);
							}
							AddObject(p_obj, &b);
							W.MoveObject(p_obj, b);
							// @v10.9.10 {
							{
								const int current_contaiter_candidate_idx = W.GetContaiterCandidateIdx();
								if(current_contaiter_candidate_idx >= 0) {
									TWhatmanObject * p_cc_obj = W.GetObjectByIndex(current_contaiter_candidate_idx);
									if(p_cc_obj) {
										assert(p_cc_obj->HasOption(TWhatmanObject::oContainer));
										if(p_cc_obj->HasOption(TWhatmanObject::oContainer)) {
											WhatmanObjectLayoutBase * p_cc_obj_as_layout = static_cast<WhatmanObjectLayoutBase *>(p_cc_obj);
											p_obj->SetLayoutContainerIdent(p_cc_obj_as_layout->GetContainerIdent());
										}
									}
								}
							}
							// } @v10.9.10
							InvalidateObjScope(p_obj);
							SLS.SetupDragndropObj(0, 0);
						}
						else
							delete p_obj;
						//
						DragndropEvent ev;
						ev.Action = DragndropEvent::acnAccept;
						TView::messageCommand(p_dot_blk->P_SrcWin, cmDragndropObj, &ev);
						//
						ok = 1;
					}
				}
				else if(oneof2(St.Rsz.Kind, ResizeState::kObjResize, ResizeState::kObjMove) && getClientRect().contains(p)) {
					TWhatmanObject * p_obj = W.GetObjectByIndex(St.Rsz.ObjIdx);
					if(p_obj) {
						InvalidateObjScope(p_obj);
						W.MoveObject(p_obj, St.Rsz.P_MovedObjCopy->GetBounds());
						// @v10.9.8 {
						{
							const int current_contaiter_candidate_idx = W.GetContaiterCandidateIdx();
							if(current_contaiter_candidate_idx >= 0) {
								TWhatmanObject * p_cc_obj = W.GetObjectByIndex(current_contaiter_candidate_idx);
								if(p_cc_obj) {
									assert(p_cc_obj->HasOption(TWhatmanObject::oContainer));
									if(p_cc_obj->HasOption(TWhatmanObject::oContainer)) {
										WhatmanObjectLayoutBase * p_cc_obj_as_layout = static_cast<WhatmanObjectLayoutBase *>(p_cc_obj);
										p_obj->SetLayoutContainerIdent(p_cc_obj_as_layout->GetContainerIdent());
									}
								}
							}
						}
						// } @v10.9.8
						InvalidateObjScope(p_obj);
					}
				}
			}
			else if(oneof2(St.Rsz.Kind, ResizeState::kObjResize, ResizeState::kObjMove)) {
				InvalidateObjScope(W.GetObjectByIndexC(St.Rsz.ObjIdx));
			}
			else if(St.Rsz.Kind == ResizeState::kRectSelection) {
				if(W.SetupMultSelBySelArea() > 0) {
					invalidateAll(1);
				}
				else {
					SRegion rgn;
					rgn.AddFrame(W.GetSelArea(), 5, SCOMBINE_OR);
					invalidateRegion(rgn, 1);
				}
				W.SetSelArea(p, 0);
				do_update_win = 1;
			}
			else if(St.Rsz.Kind == ResizeState::kMultObjMove) {
				const LongArray * p_list = W.GetMultSelIdxList();
				const uint c = SVectorBase::GetCount(p_list);
				const TPoint coffs = p - St.Rsz.StartPt;
				for(uint i = 0; i < c; i++) {
					TWhatmanObject * p_obj = W.GetObjectByIndex(p_list->get(i));
					if(p_obj) {
						InvalidateObjScope(p_obj);
						W.MoveObject(p_obj, p_obj->GetBounds().move(coffs));
						InvalidateObjScope(p_obj);
					}
				}
				{
					TPoint offs;
					W.InvalidateMultSelContour(&(offs = St.Rsz.EndPt - St.Rsz.StartPt));
				}
				do_update_win = 1;
			}
			if(St.Rsz.Kind == ResizeState::kObjDrag && St.Mode == modeToolbox) {
				Dib.P_SrcWin = 0;
				SLS.SetupDragndropObj(0, 0);
			}
			St.Rsz.Reset();
			// @v10.9.7 {
			{
				const int current_contaiter_candidate_idx = W.GetContaiterCandidateIdx();
				if(current_contaiter_candidate_idx >= 0) {
					TWhatmanObject * p_cc_obj = W.GetObjectByIndex(current_contaiter_candidate_idx);
					if(p_cc_obj)
						InvalidateObjScope(p_cc_obj);
				}
			}
			// } @v10.9.7 
			W.SetupContainerCandidate(-1, false); // @v10.9.6 ���������� ������� ���������-��������
			SetDefaultCursor();
		}
	}
	else if(mode == 2) {
		//
		// ��������� ��������
		//
		if(!!St.Rsz) {
			if(St.Rsz.Kind == ResizeState::kMultObjMove) {
				TPoint offs;
				W.InvalidateMultSelContour(&(offs = St.Rsz.EndPt - St.Rsz.StartPt));
				W.InvalidateMultSelContour(&(offs = p - St.Rsz.StartPt));
			}
			else {
				TWhatmanObject * p_obj = W.GetObjectByIndex(St.Rsz.ObjIdx);
				if(St.Rsz.P_MovedObjCopy) {
					//
					// �� ����� ������������ ��������� ������, ���� ���� ���� �� ������� ����
					//
					SETFLAG(St.Rsz.Flags, ResizeState::fDontDrawMovedObj, !getClientRect().contains(p));
				}
				if(p_obj) {
					if(St.Rsz.P_MovedObjCopy) {
						InvalidateObjScope(St.Rsz.P_MovedObjCopy);
						if(oneof2(St.Rsz.Kind, ResizeState::kObjMove, ResizeState::kObjDrag)) {
							// @v10.9.7 {
							const int current_contaiter_candidate_idx = W.GetContaiterCandidateIdx();
							if(current_contaiter_candidate_idx >= 0) {
								TWhatmanObject * p_cc_obj = W.GetObjectByIndex(current_contaiter_candidate_idx);
								if(p_cc_obj) {
									W.SetupContainerCandidate(current_contaiter_candidate_idx, false);
									InvalidateObjScope(p_cc_obj);
								}
							}
							// } @v10.9.7 
							W.MoveObject(St.Rsz.P_MovedObjCopy, (b = p_obj->GetBounds()).move(p.x - St.Rsz.StartPt.x, p.y - St.Rsz.StartPt.y));
							// @v10.9.6 {
							int container_candidate_idx = 0;
							TWhatmanObject * p_container_candidate = 0;
							if(W.FindContainerCandidateForObjectByPoint(p, p_obj, &container_candidate_idx)) {
								p_container_candidate = W.GetObjectByIndex(container_candidate_idx);
								assert(p_container_candidate);
								W.SetupContainerCandidate(container_candidate_idx, true);
								InvalidateObjScope(p_container_candidate);
							}
							// } @v10.9.6 
							InvalidateObjScope(St.Rsz.P_MovedObjCopy);
						}
					}
					else if(oneof2(St.Rsz.Kind, ResizeState::kObjResize, ResizeState::kObjMove)) {
						InvalidateObjScope(p_obj);
						if(St.Rsz.Kind == ResizeState::kObjMove) {
							b = p_obj->GetBounds();
							W.MoveObject(p_obj, b.move(p.x - St.Rsz.StartPt.x, p.y - St.Rsz.StartPt.y));
						}
						else if(St.Rsz.Kind == ResizeState::kObjResize)
							W.ResizeObject(p_obj, St.Rsz.ObjRszDir, p, 0);
						InvalidateObjScope(p_obj);
					}
					do_update_win = 1;
				}
				else if(St.Rsz.Kind == ResizeState::kObjDragTarget && St.Rsz.P_MovedObjCopy) {
					// @v10.9.10 {
					const int current_contaiter_candidate_idx = W.GetContaiterCandidateIdx();
					if(current_contaiter_candidate_idx >= 0) {
						TWhatmanObject * p_cc_obj = W.GetObjectByIndex(current_contaiter_candidate_idx);
						if(p_cc_obj) {
							W.SetupContainerCandidate(current_contaiter_candidate_idx, false);
							InvalidateObjScope(p_cc_obj);
						}
					}
					// } @v10.9.10 
					InvalidateObjScope(St.Rsz.P_MovedObjCopy);
					(b = St.Rsz.P_MovedObjCopy->GetBounds()).movecenterto(p);
					b.a = W.TransformScreenToPoint(b.a);
					b.b = W.TransformScreenToPoint(b.b);
					W.MoveObject(St.Rsz.P_MovedObjCopy, b);
					// @v10.9.10 {
					int container_candidate_idx = 0;
					TWhatmanObject * p_container_candidate = 0;
					if(W.FindContainerCandidateForObjectByPoint(p, St.Rsz.P_MovedObjCopy, &container_candidate_idx)) {
						p_container_candidate = W.GetObjectByIndex(container_candidate_idx);
						assert(p_container_candidate);
						W.SetupContainerCandidate(container_candidate_idx, true);
						InvalidateObjScope(p_container_candidate);
					}
					// } @v10.9.10 
					InvalidateObjScope(St.Rsz.P_MovedObjCopy);
					do_update_win = 1;
				}
				else if(St.Rsz.Kind == ResizeState::kRectSelection) {
					SRegion rgn;
					rgn.AddFrame(W.GetSelArea(), 5, SCOMBINE_OR);
					W.SetSelArea(p, 2);
					rgn.AddFrame(W.GetSelArea(), 5, SCOMBINE_OR);
					invalidateRegion(rgn, 1);
					do_update_win = 1;
				}
			}
			St.Rsz.EndPt = p;
		}
	}
	if(do_update_win)
		::UpdateWindow(H());
	return ok;
}

int PPWhatmanWindow::LocalMenu(int objIdx)
{
	int    ok = -1;
	if(St.Mode == modeToolbox) {
		TMenuPopup menu;
		menu.Add("Stencil params", cmaMore);
		menu.AddSeparator();
		menu.Add("@fileopen", cmFileOpen);
		menu.Add("@filesave", cmFileSave);
		{
			StrAssocArray * p_wtmo_list = TWhatmanObject::MakeStrAssocList();
			if(p_wtmo_list && p_wtmo_list->getCount()) {
				menu.AddSeparator();
				SString cmd_text;
				for(uint i = 0; i < p_wtmo_list->getCount(); i++) {
					StrAssocArray::Item item = p_wtmo_list->Get(i);
					(cmd_text = item.Txt).Transf(CTRANSF_INNER_TO_OUTER);
					menu.Add(cmd_text, MAKELONG(cmaInsert, (uint16)item.Id));
				}
			}
			ZDELETE(p_wtmo_list);
		}
		if(objIdx >= 0) {
			menu.AddSeparator();
			menu.Add("@edit",   cmaEdit);
			menu.Add("@delete", cmaDelete);
			menu.Add("@tlayout", cmLayoutEntry);
		}
		uint cmd = 0;
		if(menu.Execute(H(), TMenuPopup::efRet, &cmd, 0)) {
			switch(LoWord(cmd)) {
				case cmaMore:
					//EditParams();
					break;
				case cmaInsert:
					{
						SString obj_symb;
						if(TWhatmanObject::GetRegSymbById(HiWord(cmd), obj_symb))
							ok = AddTool(obj_symb);
					}
					break;
				case cmaEdit: ok = EditTool(objIdx); break;
				case cmaDelete: ok = DeleteTool(objIdx); break;
				case cmLayoutEntry:
					{
						ToolObject * p_tool_obj = static_cast<ToolObject *>(W.GetObjectByIndex(objIdx));
						if(p_tool_obj) {
							uint pos = 0;
							TWhatmanToolArray::Item item;
							if(Tools.SearchBySymb(p_tool_obj->ToolSymb, &pos) && Tools.Get(pos, &item)) {
								LayoutEntryDialogBlock lodb(&item.Alb);
								if(lodb.EditEntry(0) > 0) {
									item.Alb = lodb;
									if(Tools.Set(item, 0)) {
										Rearrange();
										ok = 1;
									}
									else
										PPError(PPERR_SLIB);
								}
							}
						}
					}
					break;
				case cmFileOpen: ok = FileOpen(); break;
				case cmFileSave: ok = FileSave(); break;
			}
		}
	}
	else if(oneof2(St.Mode, modeEdit, modeView)) {
		TMenuPopup menu;
		menu.Add("@fileopen", cmFileOpen);
		if(St.Mode == modeEdit) {
			menu.Add("@filesave", cmFileSave);
			if(objIdx >= 0) {
				menu.AddSeparator();
				menu.Add("@edit", cmaEdit);
				menu.Add("@delete", cmaDelete);
				menu.AddSeparator();
				menu.Add("@bringtofront", cmBringToFront);
				menu.Add("@sendtoback",   cmSendToBack);
			}
			if(St.Mode == modeEdit) {
				menu.AddSeparator();
				menu.Add("@properties", cmProperties);
			}
		}
		uint cmd = 0;
		if(menu.Execute(H(), TMenuPopup::efRet, &cmd, 0)) {
			int do_redraw = 0;
			switch(LoWord(cmd)) {
				case cmFileOpen: ok = FileOpen(); break;
				case cmFileSave: ok = FileSave(); break;
				case cmaEdit:
					if(W.EditObject(objIdx) > 0)
						do_redraw = 1;
					break;
				case cmaDelete:
					if(W.RemoveObject(objIdx) > 0)
						do_redraw = 1;
					break;
				case cmBringToFront:
					if(W.BringObjToFront(objIdx) > 0)
						do_redraw = 1;
					break;
				case cmSendToBack:
					if(W.SendObjToBack(objIdx) > 0)
						do_redraw = 1;
					break;
				case cmProperties:
					if(EditParam() > 0)
						do_redraw = 1;
					break;
			}
			if(do_redraw) {
				invalidateAll(0);
				::UpdateWindow(H());
			}
		}
	}
	return ok;
}

IMPL_HANDLE_EVENT(PPWhatmanWindow)
{
	TRect  b;
	TWindowBase::handleEvent(event);
	if(TVKEYDOWN) {
		int    cur_obj_idx = -1;
		int    new_obj_idx = -1;
		switch(TVKEY) {
			//case kbUp:
			//case kbLeft:
			case kbShiftTab:
				if(W.GetCurrentObject(&cur_obj_idx)) {
					new_obj_idx = (cur_obj_idx > 0) ? (cur_obj_idx-1) : (int)(W.GetObjectsCount()-1);
				}
				break;
			//case kbDown:
			//case kbRight:
			case kbTab:
				if(W.GetCurrentObject(&cur_obj_idx)) {
					new_obj_idx = ((cur_obj_idx+1) < (int)W.GetObjectsCount()) ? (cur_obj_idx+1) : 0;
				}
				break;
			case kbDel:
				if(W.GetCurrentObject(&cur_obj_idx)) {
					if(W.RemoveObject(cur_obj_idx) > 0) {
						invalidateAll(0);
						::UpdateWindow(H());
					}
				}
				break;
			default:
				return;
		}
		if(new_obj_idx >= 0 && cur_obj_idx >= 0) {
			W.SetCurrentObject(new_obj_idx, 0);
			InvalidateObjScope(W.GetObjectByIndexC(cur_obj_idx));
			InvalidateObjScope(W.GetObjectByIndexC(new_obj_idx));
			::UpdateWindow(H());
		}
	}
	else if(TVINFOPTR) {
		if(event.isCmd(cmInit)) {
			CreateBlock * p_blk = static_cast<CreateBlock *>(TVINFOPTR);
			W.SetArea(getClientRect());
			W.GetScrollRange(&Sb.Rx, &Sb.Ry);
			Sb.SetupWindow(H());
		}
		else if(event.isCmd(cmPaint)) {
			PaintEvent * p_pe = static_cast<PaintEvent *>(TVINFOPTR);
			if(p_pe->PaintType == PaintEvent::tPaint) {
				MemLeakTracer mlt;
				{
					TCanvas2 canv(Tb, static_cast<HDC>(p_pe->H_DeviceContext));
					TRect rect_cli = getClientRect();
					canv.Rect(rect_cli, 0, brBackg);
					W.Draw(canv);
					if(!!St.Rsz) {
						if(!(St.Rsz.Flags & ResizeState::fDontDrawMovedObj))
							W.DrawSingleObject(canv, St.Rsz.P_MovedObjCopy);
						if(St.Rsz.Kind == ResizeState::kMultObjMove) {
							TPoint offs = St.Rsz.EndPt - St.Rsz.StartPt;
							W.DrawMultSelContour(canv, &offs);
						}
					}
				}
			}
			else if(p_pe->PaintType == PaintEvent::tEraseBackground) {
				/*
				TCanvas2 canv(Tb, (HDC)p_pe->H_DeviceContext);
				canv.Rect(p_pe->Rect, 0, brBackg);
				*/
			}
			else
				return;
		}
		else if(event.isCmd(cmSize)) {
			const SizeEvent * p_se = static_cast<const SizeEvent *>(TVINFOPTR);
			W.SetArea(getClientRect());
			W.GetScrollRange(&Sb.Rx, &Sb.Ry);
			Sb.SetupWindow(H());
			if(St.Mode == modeToolbox) {
				TWhatmanToolArray::Param param;
				Tools.GetParam(param);
				if(ArrangeObjects(0, param.Ap) > 0)
					invalidateAll(1);
			}
			else {
				if(p_se->NewSize.x > p_se->PrevSize.x)
					invalidateRect(b.set(p_se->PrevSize.x, 0, ViewSize.x, ViewSize.y), 1);
				if(p_se->NewSize.y > p_se->PrevSize.y)
					invalidateRect(b.set(0, p_se->PrevSize.y, ViewSize.x, ViewSize.y), 1);
			}
			::UpdateWindow(H());
			return;
		}
		else if(event.isCmd(cmDragndropObj)) {
			DragndropEvent * p_ev = static_cast<DragndropEvent *>(TVINFOPTR);
			if(p_ev) {
				if(p_ev->Action == DragndropEvent::acnGet) {
					if(St.Rsz.P_MovedObjCopy) {
						St.Rsz.Flags |= ResizeState::fDontDrawMovedObj;
						InvalidateObjScope(St.Rsz.P_MovedObjCopy);
						::UpdateWindow(H());
					}
				}
				else if(p_ev->Action == DragndropEvent::acnAccept) {
					TPoint zero_point;
					Resize(0, zero_point.Z());
				}
			}
		}
		else if(event.isCmd(cmMouse)) {
			MouseEvent * p_me = static_cast<MouseEvent *>(TVINFOPTR);
			int    obj_idx = 0, prev_idx = -1;
			switch(p_me->Type) {
				case MouseEvent::tRDown:
					LocalMenu(W.FindObjectByPoint(p_me->Coord, &obj_idx) ? obj_idx : -1);
					break;
				case MouseEvent::tLDown:
					if(W.FindObjectByPoint(p_me->Coord, &obj_idx)) {
						TWhatmanObject * p_obj = W.GetObjectByIndex(obj_idx);
						if(!p_obj || !p_obj->HasState(TWhatmanObject::stSelected)) {
							if(W.RmvMultSelObject(-1) > 0) {
								invalidateAll(1);
								::UpdateWindow(H());
							}
						}
						if(St.Mode == modeView && W.GetParam().Flags & TWhatman::Param::fOneClickActivation &&
							p_obj && p_obj->HasOption(TWhatmanObject::oSelectable) && St.Flags & stfModalSelector) {
							St.SelectedObjIdx = obj_idx;
							if(IsInState(sfModal))
								endModal(cmOK);
						}
						else {
							W.SetCurrentObject(obj_idx, &prev_idx);
							InvalidateObjScope(W.GetObjectByIndexC(prev_idx));
							InvalidateObjScope(W.GetObjectByIndexC(obj_idx));
							Resize(1, p_me->Coord);
							::UpdateWindow(H());
						}
					}
					else {
						if(W.RmvMultSelObject(-1) > 0) {
							invalidateAll(1);
							::UpdateWindow(H());
						}
						if(W.GetCurrentObject(&prev_idx)) {
							W.SetCurrentObject(-1, &prev_idx);
							InvalidateObjScope(W.GetObjectByIndexC(prev_idx));
							::UpdateWindow(H());
						}
						else {
							Resize(1, p_me->Coord);
						}
					}
					break;
				case MouseEvent::tLUp:
					if(!!St.Rsz)
						Resize(0, p_me->Coord);
					break;
				case MouseEvent::tLDblClk:
					if(W.FindObjectByPoint(p_me->Coord, &obj_idx)) {
						switch(St.Mode) {
							case modeToolbox:
								EditTool(obj_idx);
								break;
							case modeEdit:
								if(W.EditObject(obj_idx) > 0) {
									invalidateAll(0);
									::UpdateWindow(H());
								}
								break;
							case modeView:
								{
									TWhatmanObject * p_obj = W.GetObjectByIndex(obj_idx);
									if(p_obj && p_obj->HasOption(TWhatmanObject::oSelectable) && St.Flags & stfModalSelector) {
										St.SelectedObjIdx = obj_idx;
										if(IsInState(sfModal))
											endModal(cmOK);
									}
								}
								break;
							default:
								return;
						}
					}
					break;
				case MouseEvent::tMove:
					{
						int    cur_ident = 0;
						Loc    loc;
						if(!St.Rsz) {
							if(DragDetect(H(), p_me->Coord)) {
								if(Resize(3, p_me->Coord) > 0)
									Resize(2, p_me->Coord);
							}
							else if(Locate(p_me->Coord, &loc) && St.Rsz.Setup(St.Mode, loc, &cur_ident)) {
								if(cur_ident) {
									::SetCursor(Tb.GetCursor(cur_ident));
								}
								St.Rsz.Reset();
							}
						}
						else
							Resize(2, p_me->Coord);
					}
					break;
				case MouseEvent::tLeave:
					if(St.Rsz.Kind == ResizeState::kObjDragTarget) {
						Resize(-1, p_me->Coord);
					}
					else if(St.Rsz.P_MovedObjCopy) {
						St.Rsz.Flags |= ResizeState::fDontDrawMovedObj;
						InvalidateObjScope(St.Rsz.P_MovedObjCopy);
						::UpdateWindow(H());
						//Resize(2, p_me->Coord);
					}
					break;
				case MouseEvent::tHover:
					if(!St.Rsz) {
						if(W.FindObjectByPoint(p_me->Coord, &obj_idx)) {
							TWhatmanObject * p_obj = W.GetObjectByIndex(obj_idx);
							CALLPTRMEMB(p_obj, HandleCommand(TWhatmanObject::cmdMouseHover, 0));
						}
					}
					break;
				case MouseEvent::tWeel: // @v10.5.9 @todo ��� ����� ����� ��������. ���� ���������� ��� ����������� � ���� ���� ������
					{
						int r = 0;
						if(p_me->WeelDelta < 0)
							r = Sb.Move(SIDE_TOP, -p_me->WeelDelta);
						else if(p_me->WeelDelta > 0)
							r = Sb.Move(SIDE_BOTTOM, p_me->WeelDelta);
						if(r > 0) {
							TPoint sp;
							W.SetScrollPos(sp.Set(Sb.ScX, Sb.ScY));
							Sb.SetupWindow(H());
							invalidateAll(0);
							::UpdateWindow(H());
						}
					}
					break;
				default:
					return;
			}
		}
		else if(event.isCmd(cmScroll)) {
			ScrollEvent * p_se = static_cast<ScrollEvent *>(TVINFOPTR);
			if(oneof2(p_se->Dir, DIREC_VERT, DIREC_HORZ)) {
				int    r = 0;
				switch(p_se->Type) {
					case ScrollEvent::tTop:
						r = Sb.MoveToEdge((p_se->Dir == DIREC_VERT) ? SIDE_TOP : SIDE_LEFT);
						break;
					case ScrollEvent::tBottom:
						r = Sb.MoveToEdge((p_se->Dir == DIREC_VERT) ? SIDE_BOTTOM : SIDE_RIGHT);
						break;
					case ScrollEvent::tLineDown:
					case ScrollEvent::tPageDown:
						{
							const TPoint delta = W.GetScrollDelta();
							if(p_se->Dir == DIREC_VERT)
								r = Sb.Move(SIDE_BOTTOM, NZOR(delta.y, 1));
							else
								r = Sb.Move(SIDE_RIGHT, NZOR(delta.x, 1));
						}
						break;
					case ScrollEvent::tLineUp:
					case ScrollEvent::tPageUp:
						{
							const TPoint delta = W.GetScrollDelta();
							if(p_se->Dir == DIREC_VERT)
								r = Sb.Move(SIDE_TOP, NZOR(delta.y, 1));
							else
								r = Sb.Move(SIDE_LEFT, NZOR(delta.x, 1));
						}
						break;
					case ScrollEvent::tThumbPos:
						if(p_se->Dir == DIREC_VERT)
							r = Sb.Set(Sb.ScX, p_se->TrackPos);
						else
							r = Sb.Set(p_se->TrackPos, Sb.ScY);
						break;
					case ScrollEvent::tThumbTrack:
						break;
				}
				if(r > 0) {
					TPoint sp;
					W.SetScrollPos(sp.Set(Sb.ScX, Sb.ScY));
					Sb.SetupWindow(H());
					invalidateAll(0);
					::UpdateWindow(H());
				}
			}
		}
		else
			return;
	}
	else
		return;
	clearEvent(event);
}

int PPWhatmanWindow::EditParam()
{
	int    ok = -1;
	TDialog * dlg = new TDialog(DLG_WTMPARAM);
	TWhatman::Param param;
	if(CheckDialogPtrErr(&dlg)) {
		param = W.GetParam();
		dlg->setCtrlString(CTL_WTMPARAM_NAME, param.Name);
		dlg->setCtrlString(CTL_WTMPARAM_SYMB, param.Symb);
		//UNIT_METER
		SetupStringCombo(dlg, CTLSEL_WTMPARAM_UNIT, PPTXT_WTMUNITLIST, param.Unit);
		dlg->setCtrlReal(CTL_WTMPARAM_UNITFACTOR, param.UnitFactor);
		dlg->AddClusterAssoc(CTL_WTMPARAM_FLAGS, 0, TWhatman::Param::fSnapToGrid);
		dlg->AddClusterAssoc(CTL_WTMPARAM_FLAGS, 1, TWhatman::Param::fOneClickActivation);
		dlg->SetClusterData(CTL_WTMPARAM_FLAGS, param.Flags);
		while(ok < 0 && ExecView(dlg) == cmOK) {
			dlg->getCtrlString(CTL_WTMPARAM_NAME, param.Name);
			dlg->getCtrlString(CTL_WTMPARAM_SYMB, param.Symb);
			param.Unit = dlg->getCtrlLong(CTLSEL_WTMPARAM_UNIT);
			param.UnitFactor = dlg->getCtrlReal(CTL_WTMPARAM_UNITFACTOR);
			dlg->GetClusterData(CTL_WTMPARAM_FLAGS, &param.Flags);
			W.SetParam(param);
			ok = 1;
		}
	}
	else
		ok = 0;
	delete dlg;
	return ok;
}

int PPWhatmanWindow::AddTool(const char * pObjSymb)
{
	int    ok = -1, r = 1;
	TWhatmanObject * p_obj = 0;
	if(St.Mode == modeToolbox) {
		TWhatmanToolArray::Item item;
		THROW_SL(p_obj = TWhatmanObject::CreateInstance(pObjSymb));
		item.WtmObjSymb = pObjSymb;
		item.PicSize = 32;
		item.FigSize = 64;
		while(ok < 0 && (r = p_obj->EditTool(&item)) > 0) {
			if(Tools.Set(item, 0)) {
				ToolObject * p_tool_obj = new ToolObject(item.Symb, &Tools, TRect(item.PicSize));
				THROW_MEM(p_tool_obj);
				W.InsertObject(p_tool_obj);
				Rearrange();
				ok = 1;
			}
			else
				PPError(PPERR_SLIB);
		}
		THROW(r);
	}
	CATCHZOKPPERR
	delete p_obj;
	return ok;
}

int PPWhatmanWindow::EditTool(uint objIdx)
{
	int    ok = -1, r = 1;
	TWhatmanObject * p_obj = 0;
	if(St.Mode == modeToolbox) {
		const ToolObject * p_tool_obj = static_cast<const ToolObject *>(W.GetObjectByIndexC(objIdx));
		if(p_tool_obj) {
			uint pos = 0;
			TWhatmanToolArray::Item item;
			if(Tools.SearchBySymb(p_tool_obj->ToolSymb, &pos) && Tools.Get(pos, &item)) {
				THROW_SL(p_obj = TWhatmanObject::CreateInstance(item.WtmObjSymb));
				while(ok < 0 && (r = p_obj->EditTool(&item)) > 0) {
					if(Tools.Set(item, 0)) {
						Rearrange();
						ok = 1;
					}
					else
						PPError(PPERR_SLIB);
				}
			}
		}
	}
	CATCHZOKPPERR
	delete p_obj;
	return ok;
}

int PPWhatmanWindow::DeleteTool(uint objIdx)
{
	int    ok = -1;
	if(St.Mode == modeToolbox) {
		const ToolObject * p_obj = static_cast<const ToolObject *>(W.GetObjectByIndexC(objIdx));
		if(p_obj) {
			uint pos = 0;
			if(Tools.SearchBySymb(p_obj->ToolSymb, &pos)) {
				Tools.Remove(pos);
			}
			p_obj = 0;
			W.RemoveObject(objIdx);
			Rearrange();
			ok = 1;
		}
	}
	return ok;
}

int PPWhatmanWindow::FileSave()
{
	int    ok = -1;
	SString path;
	if(St.Mode == modeToolbox) {
		TWhatmanToolArray::Param param;
		Tools.GetParam(param);
		path = param.FileName;
		if(PPOpenFile(PPTXT_FILPAT_WTA, path, ofilfNExist, 0) > 0) {
			SPathStruc::ReplaceExt(path, "wta", 0);
			if(!Tools.Store(path)) {
				PPSetErrorSLib();
				ok = PPErrorZ();
			}
		}
	}
	else if(oneof2(St.Mode, modeEdit, modeView)) {
		TWhatmanToolArray::Param param;
		if(PPOpenFile(PPTXT_FILPAT_WTM, path, ofilfNExist, 0) > 0) {
			SPathStruc::ReplaceExt(path, "wtm", 0);
			if(!W.Store(path)) {
				PPSetErrorSLib();
				ok = PPErrorZ();
			}
		}
	}
	return ok;
}

int PPWhatmanWindow::LoadTools(const char * pFileName)
{
	int    ok = 1;
	if(St.Mode == modeToolbox) {
		TWhatmanToolArray::Item item;
		THROW_SL(Tools.Load(pFileName));
		W.Clear();
		for(uint i = 0; i < Tools.GetCount(); ++i) {
			if(Tools.Get(i, &item)) {
				ToolObject * p_obj = new ToolObject(item.Symb, &Tools, TRect(item.PicSize));
				THROW_MEM(p_obj);
				THROW_SL(W.InsertObject(p_obj));
			}
		}
		{
			TWhatmanToolArray::Param param;
			Tools.GetParam(param);
			param.Ap.Dir = DIREC_HORZ;
			param.Ap.UlGap = 8;
			param.Ap.LrGap = 8;
			param.Ap.InnerGap = 5;
			Tools.SetParam(param);
			Rearrange();
		}
	}
	else
		ok = -1;
	CATCHZOK
	return ok;
}

int PPWhatmanWindow::FileOpen()
{
	int    ok = -1;
	SString path;
	if(St.Mode == modeToolbox) {
		if(PPOpenFile(PPTXT_FILPAT_WTA, path, 0, 0) > 0) {
			if(LoadTools(path))
				ok = 1;
			else
				ok = PPErrorZ();
		}
	}
	else if(oneof2(St.Mode, modeEdit, modeView)) {
		if(PPOpenFile(PPTXT_FILPAT_WTM, path, 0, 0) > 0) {
			W.Clear();
			if(W.Load(path)) {
				invalidateAll(1);
				::UpdateWindow(H());
			}
			else {
				PPSetErrorSLib();
				ok = PPErrorZ();
			}
		}
	}
	return ok;
}

/*static*/int PPWhatmanWindow::Launch(const char * pWtmFileName, long options, TWhatmanObject::SelectObjRetBlock * pSelRetBlk)
{
	int    ok = -1;
	HWND   w_preserve_focus = ::GetFocus();
	HWND   w_preserve_capt  = ::GetCapture();
	MemLeakTracer mlt;
	PPWhatmanWindow * p_win = new PPWhatmanWindow(PPWhatmanWindow::modeView);
	THROW_MEM(p_win);
	p_win->setBounds(TRect(0, 0, 200, 200));
	THROW(p_win->W.Load(pWtmFileName));
	{
		TWhatman::Param p = p_win->W.GetParam();
		p.Flags &= ~(TWhatman::Param::fRule | TWhatman::Param::fGrid);
		p.Flags |= (TWhatman::Param::fDisableReszObj | TWhatman::Param::fDisableMoveObj);
		p_win->W.SetParam(p);
		p_win->setTitle(p.Name);
	}
	p_win->St.Flags |= stfModalSelector; // execView
	int    ret = APPL->P_DeskTop->execView(p_win);
	if(ret == cmOK) {
		TWhatmanObject * p_obj = p_win->W.GetObjectByIndex(p_win->St.SelectedObjIdx);
		CALLPTRMEMB(p_obj, HandleCommand(TWhatmanObject::cmdGetSelRetBlock, pSelRetBlk));
		ok = 1;
	}
	/*
	p_win->Create((long)APPL->H_MainWnd, TWindowBase::coPopup);
	ShowWindow(*p_win, SW_SHOWNORMAL);
	UpdateWindow(*p_win);
	*/
	CATCHZOKPPERR
	delete p_win;
	if(w_preserve_capt)
		::SetCapture(w_preserve_capt);
	if(w_preserve_focus)
		::SetFocus(w_preserve_focus);
	return ok;
}

/*static*/int PPWhatmanWindow::Helper_MakeFrameWindow(TWindowBase * pFrame, const char * pWtmFileName, const char * pWtaFileName)
{
	int    ok = 1;
	if(pFrame) {
		PPWhatmanWindow * p_tool_win = 0;
		PPWhatmanWindow * p_edit_win = 0;
		LayoutFlexItem * p_frame_layout = static_cast<LayoutFlexItem *>(pFrame->GetLayout());
		{
			THROW_MEM(p_tool_win = new PPWhatmanWindow(PPWhatmanWindow::modeToolbox));
			{
				LayoutFlexItem * p_lo = 0;
				if(p_frame_layout) {
					AbstractLayoutBlock alb;
					p_lo = p_frame_layout->InsertItem();
					alb.GrowFactor = 1.0f;
					p_lo->SetLayoutBlock(alb);
					//p_lo->align_items = FLEX_ALIGN_STRETCH;
					//p_lo->align_self = FLEX_ALIGN_AUTO;
				}
				pFrame->AddChildWithLayout(p_tool_win, TWindowBase::coChild, p_lo);
			}
		}
		{
			THROW_MEM(p_edit_win = new PPWhatmanWindow(PPWhatmanWindow::modeEdit));
			{
				LayoutFlexItem * p_lo = 0;
				if(p_frame_layout) {
					AbstractLayoutBlock alb;
					p_lo = p_frame_layout->InsertItem();
					alb.GrowFactor = 3.0f;
					p_lo->SetLayoutBlock(alb);
					//p_lo->align_items = FLEX_ALIGN_STRETCH;
					//p_lo->align_self = FLEX_ALIGN_AUTO;
				}
				pFrame->AddChildWithLayout(p_edit_win, TWindowBase::coChild, p_lo);
			}
			{
				TWhatman::Param p = p_edit_win->W.GetParam();
				p.Flags |= (TWhatman::Param::fRule | TWhatman::Param::fGrid);
				p.Flags &= ~(TWhatman::Param::fDisableReszObj | TWhatman::Param::fDisableMoveObj);
				p_edit_win->W.SetParam(p);
			}
		}
		{
			SString file_name(pWtaFileName);
			if(file_name.IsEmpty())
				PPGetFilePath(PPPATH_WTM, "cafetable.wta", file_name);
			THROW(p_tool_win->LoadTools(file_name));
		}
		if(pWtmFileName)
			THROW(p_edit_win->W.Load(pWtmFileName));
	}
	CATCHZOK
	return ok;
}

/*static*/LPCTSTR TWhatmanBrowser::WndClsName = _T("TWhatmanBrowser"); // @global

TWhatmanBrowser::TWhatmanBrowser(Param * pP) : TBaseBrowserWindow(WndClsName)
{
	RVALUEPTR(P, pP);
	InitLayout();
	BbState |= bbsWoScrollbars;
}

TWhatmanBrowser::TWhatmanBrowser(const char * pFileName, int toolbarId/*= -1*/) : TBaseBrowserWindow(WndClsName)
{
	InitLayout();
}

TWhatmanBrowser::~TWhatmanBrowser()
{
}

void TWhatmanBrowser::InitLayout()
{
	//SRectLayout::Item li;
	AbstractLayoutBlock alb;
	P_Lfc = new LayoutFlexItem();
	alb.SetContainerDirection(DIREC_HORZ);
	alb.AlignContent = AbstractLayoutBlock::alignStretch;
	P_Lfc->SetLayoutBlock(alb);
	P_Lfc->SetCallbacks(0, TWindowBase::SetupLayoutItemFrame, this);
}

int TWhatmanBrowser::WMHCreate()
{
	RECT rc;
	GetWindowRect(H(), &rc);
	P_Toolbar = new TToolbar(H(), TBS_NOMOVE);
	if(P_Toolbar && LoadToolbarResource(ToolbarID) > 0) {
		P_Toolbar->Init(ToolbarID, &Toolbar);
		if(P_Toolbar->IsValid()) {
			RECT tbr;
			::GetWindowRect(P_Toolbar->H(), &tbr);
			ToolBarWidth = tbr.bottom - tbr.top;
		}
	}
	PPWhatmanWindow::Helper_MakeFrameWindow(this, P.WtmFileName, P.WtaFileName);
#if 0 // 
	HwndSci = ::CreateWindowEx(WS_EX_CLIENTEDGE, _T("Scintilla"), _T(""), WS_CHILD|WS_VISIBLE|WS_TABSTOP|WS_CLIPCHILDREN,
		0, ToolBarWidth, rc.right - rc.left, rc.bottom - rc.top, H(), 0/*(HMENU)GuiID*/, APPL->GetInst(), 0);
	SScEditorBase::Init(HwndSci, 1/*preserveFileName*/);
	TView::SetWindowProp(HwndSci, GWLP_USERDATA, this);
	OrgScintillaWndProc = static_cast<WNDPROC>(TView::SetWindowProp(HwndSci, GWLP_WNDPROC, ScintillaWindowProc));
	// @v8.6.2 (SCI_SETKEYSUNICODE deprecated in sci 3.5.5) CallFunc(SCI_SETKEYSUNICODE, 1, 0);
	CallFunc(SCI_SETCARETLINEVISIBLE, 1);
	CallFunc(SCI_SETCARETLINEBACK, RGB(232,232,255));
	CallFunc(SCI_SETSELBACK, 1, RGB(117,217,117));
	CallFunc(SCI_SETFONTQUALITY, SC_EFF_QUALITY_LCD_OPTIMIZED); // @v9.8.2 SC_EFF_QUALITY_ANTIALIASED-->SC_EFF_QUALITY_LCD_OPTIMIZED
	// CallFunc(SCI_SETTECHNOLOGY, /*SC_TECHNOLOGY_DIRECTWRITERETAIN*/SC_TECHNOLOGY_DIRECTWRITEDC, 0); // @v9.8.2
	//
	CallFunc(SCI_SETMOUSEDWELLTIME, 500);
	//
	{
		KeyAccel.clear();
		{
			for(uint i = 0; i < OuterKeyAccel.getCount(); i++) {
				const LAssoc & r_accel_item = OuterKeyAccel.at(i);
				const KeyDownCommand & r_k = *reinterpret_cast<const KeyDownCommand *>(&r_accel_item.Key);
				KeyAccel.Set(r_k, r_accel_item.Val);
			}
		}
		if(P_Toolbar) {
			const uint tbc = P_Toolbar->getItemsCount();
			for(uint i = 0; i < tbc; i++) {
				const ToolbarItem & r_tbi = P_Toolbar->getItem(i);
				if(!(r_tbi.Flags & r_tbi.fHidden) && r_tbi.KeyCode && r_tbi.KeyCode != TV_MENUSEPARATOR && r_tbi.Cmd) {
					KeyDownCommand k;
					if(k.SetTvKeyCode(r_tbi.KeyCode))
						KeyAccel.Set(k, r_tbi.Cmd);
				}
			}
		}
		KeyAccel.Sort();
	}
	FileLoad(Doc.FileName, cpUTF8, 0);
	return BIN(P_SciFn && P_SciPtr);
#endif
	return 1;
}

/*static*/LRESULT CALLBACK TWhatmanBrowser::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CREATESTRUCT * p_init_data;
	TWhatmanBrowser * p_view = 0;
	switch(message) {
		case WM_CREATE:
			p_view = static_cast<TWhatmanBrowser *>(Helper_InitCreation(lParam, (void **)&p_init_data));
			if(p_view) {
				p_view->HW = hWnd;
				TView::SetWindowProp(hWnd, GWLP_USERDATA, p_view);
				::SetFocus(hWnd);
				::SendMessage(hWnd, WM_NCACTIVATE, TRUE, 0L);
				p_view->WMHCreate();
				PostMessage(hWnd, WM_PAINT, 0, 0);
				{
					SString temp_buf;
					TView::SGetWindowText(hWnd, temp_buf);
					APPL->AddItemToMenu(temp_buf, p_view);
				}
				return 0;
			}
			else
				return -1;
		case WM_COMMAND:
			{
				p_view = static_cast<TWhatmanBrowser *>(TView::GetWindowUserData(hWnd));
				if(p_view) {
					if(HIWORD(wParam) == 0) {
						/*if(p_view->KeyAccel.getCount()) {
							long   cmd = 0;
							KeyDownCommand k;
							k.SetTvKeyCode(LOWORD(wParam));
							if(p_view->KeyAccel.BSearch(*reinterpret_cast<const long *>(&k), &cmd, 0)) {
								p_view->ProcessCommand(cmd, 0, p_view);
							}
						}*/
					}
					/*
					if(LOWORD(wParam))
						p_view->ProcessCommand(LOWORD(wParam), 0, p_view);
					*/
				}
			}
			break;
		case WM_DESTROY:
			p_view = static_cast<TWhatmanBrowser *>(TView::GetWindowUserData(hWnd));
			if(p_view) {
				//p_view->SaveChanges();
				TWindowBase::Helper_Finalize(hWnd, p_view);
			}
			return 0;
		case WM_SETFOCUS:
			if(!(TView::GetWindowStyle(hWnd) & WS_CAPTION)) {
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
				APPL->NotifyFrame(0);
			}
			p_view = static_cast<TWhatmanBrowser *>(TView::GetWindowUserData(hWnd));
			if(p_view) {
				//::SetFocus(p_view->HwndSci);
				APPL->SelectTabItem(p_view);
				TView::messageBroadcast(p_view, cmReceivedFocus);
				p_view->select();
			}
			break;
		case WM_KILLFOCUS:
			if(!(TView::GetWindowStyle(hWnd) & WS_CAPTION))
				APPL->NotifyFrame(0);
			p_view = static_cast<TWhatmanBrowser *>(TView::GetWindowUserData(hWnd));
			if(p_view) {
				TView::messageBroadcast(p_view, cmReleasedFocus);
				p_view->ResetOwnerCurrent();
			}
			break;
		case WM_KEYDOWN:
			if(wParam == VK_ESCAPE) {
				p_view = static_cast<TWhatmanBrowser *>(TView::GetWindowUserData(hWnd));
				if(p_view) {
					p_view->endModal(cmCancel);
					return 0;
				}
			}
			else if(wParam == VK_TAB) {
				p_view = static_cast<TWhatmanBrowser *>(TView::GetWindowUserData(hWnd));
				if(p_view && GetKeyState(VK_CONTROL) & 0x8000 && !p_view->IsInState(sfModal)) {
					SetFocus(GetNextBrowser(hWnd, (GetKeyState(VK_SHIFT) & 0x8000) ? 0 : 1));
					return 0;
				}
			}
			return 0;
		case WM_SIZE:
			p_view = static_cast<TWhatmanBrowser *>(TView::GetWindowUserData(hWnd));
			{ // ����� - ������ ����� ��������� ������ ������� �� TWindowBase::WndProc
				SizeEvent se;
				switch(wParam) {
					case SIZE_MAXHIDE:   se.ResizeType = SizeEvent::tMaxHide;   break;
					case SIZE_MAXIMIZED: se.ResizeType = SizeEvent::tMaximized; break;
					case SIZE_MAXSHOW:   se.ResizeType = SizeEvent::tMaxShow;   break;
					case SIZE_MINIMIZED: se.ResizeType = SizeEvent::tMinimized; break;
					case SIZE_RESTORED:  se.ResizeType = SizeEvent::tRestored;  break;
					default: se.ResizeType = 0; break;
				}
				se.PrevSize = p_view->ViewSize;
				p_view->ViewSize = se.NewSize.setwparam(lParam);
				if(TView::messageCommand(p_view, cmSize, &se))
					return 0;
			}
			break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/*static*/int TWhatmanBrowser::RegWindowClass(HINSTANCE hInst)
{
	WNDCLASSEX wc;
	INITWINAPISTRUCT(wc);
	wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_DBLCLKS;
	wc.lpfnWndProc   = TWhatmanBrowser::WndProc;
	wc.cbClsExtra    = BRWCLASS_CEXTRA;
	wc.cbWndExtra    = BRWCLASS_WEXTRA;
	wc.hInstance     = hInst;
	wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(/*ICON_TIMEGRID*/172));
	wc.hCursor       = NULL;
	wc.hbrBackground = ::CreateSolidBrush(RGB(0xEE, 0xEE, 0xEE));
	wc.lpszClassName = TWhatmanBrowser::WndClsName;
	return RegisterClassEx(&wc);
}

/*static*/int PPWhatmanWindow::Edit(const char * pWtmFileName, const char * pWtaFileName)
{
	const int use_base_browser_window = 1;
	int    ok = -1;
	if(use_base_browser_window) {
		TWhatmanBrowser::Param param;
		param.Flags |= param.fEditMode;
		param.WtmFileName = pWtmFileName;
		param.WtaFileName = pWtaFileName;
		TWhatmanBrowser * p_frame_win = new TWhatmanBrowser(&param);
		InsertView(p_frame_win);
	}
	else {
		class FrameWindow : public TWindowBase {
		public:
			FrameWindow() : TWindowBase(_T("SLibWindowBase"), 0)
			{
				AbstractLayoutBlock alb;
				P_Lfc = new LayoutFlexItem();
				alb.SetContainerDirection(DIREC_HORZ);
				alb.AlignContent = AbstractLayoutBlock::alignStretch;
				P_Lfc->SetLayoutBlock(alb);
				P_Lfc->SetCallbacks(0, TWindowBase::SetupLayoutItemFrame, this);
			}
		};
		FrameWindow * p_frame_win = new FrameWindow;
		p_frame_win->changeBounds(TRect(10, 10, 900, 900));
		p_frame_win->Create(APPL->H_MainWnd, TWindowBase::coPopup);
		THROW(PPWhatmanWindow::Helper_MakeFrameWindow(p_frame_win, pWtmFileName, pWtaFileName));
		::ShowWindow(p_frame_win->H(), SW_SHOWNORMAL);
		p_frame_win->invalidateAll(1);
		::UpdateWindow(p_frame_win->H());
	}
	CATCH
		ok = PPErrorZ();
	ENDCATCH
	return ok;
}

void PPViewWhatmanBrowser(const char * pTitle)
{
	TWhatmanBrowser * p_brw = 0;//PPFindLastTextBrowser(pFileName);
	if(p_brw) {
		PPCloseBrowser(p_brw);
		p_brw = 0;
	}
	p_brw = new TWhatmanBrowser((TWhatmanBrowser::Param *)0);
	{
		SString title_buf;
		if(!isempty(pTitle))
			title_buf = pTitle;
		else {
			title_buf = "Whatman Window";
		}
		p_brw->setTitle(title_buf);
	}
	InsertView(p_brw);
}