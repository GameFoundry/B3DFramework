//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIDropDownMenu.h"
#include "GUI/BsGUIPanel.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUISpace.h"
#include "GUI/BsGUIContent.h"
#include "GUI/BsGUISkin.h"
#include "RenderAPI/BsViewport.h"
#include "GUI/BsGUIListBox.h"
#include "GUI/BsGUIDropDownBoxManager.h"
#include "Scene/BsSceneObject.h"
#include "GUI/BsGUIDropDownHitBox.h"
#include "GUI/BsGUIDropDownContent.h"
#include "Renderer/BsCamera.h"
#include "Debug/BsDebug.h"

using namespace std::placeholders;

namespace bs
{
	const u32 GUIDropDownMenu::DROP_DOWN_BOX_WIDTH = 250;

	GUIDropDownDataEntry GUIDropDownDataEntry::Separator()
	{
		GUIDropDownDataEntry data;
		data.mType = Type::Separator;
		data.mCallback = nullptr;

		return data;
	}

	GUIDropDownDataEntry GUIDropDownDataEntry::Button(const String& label, std::function<void()> callback, const String& shortcutTag)
	{
		GUIDropDownDataEntry data;
		data.mLabel = label;
		data.mType = Type::Entry;
		data.mCallback = callback;
		data.mShortcutTag = shortcutTag;

		return data;
	}

	GUIDropDownDataEntry GUIDropDownDataEntry::SubMenu(const String& label, const GUIDropDownData& data)
	{
		GUIDropDownDataEntry dataEntry;
		dataEntry.mLabel = label;
		dataEntry.mType = Type::SubMenu;
		dataEntry.mChildData = data;

		return dataEntry;
	}

	GUIDropDownMenu::GUIDropDownMenu(const HSceneObject& parent, const DROP_DOWN_BOX_DESC& desc, GUIDropDownType type)
		: CGUIWidget(parent, desc.Camera), mRootMenu(nullptr), mFrontHitBox(nullptr), mBackHitBox(nullptr), mCaptureHitBox(nullptr)
	{
		String stylePrefix = "";
		switch(type)
		{
		case GUIDropDownType::ContextMenu:
			stylePrefix = "ContextMenu";
			break;
		case GUIDropDownType::ListBox:
		case GUIDropDownType::MultiListBox:
			stylePrefix = "ListBox";
			break;
		case GUIDropDownType::MenuBar:
			stylePrefix = "MenuBar";
			break;
		}

		mScrollUpStyle = stylePrefix + "ScrollUpBtn";
		mScrollDownStyle = stylePrefix + "ScrollDownBtn";
		mBackgroundStyle = stylePrefix + "Frame";
		mContentStyle = stylePrefix + "Content";
		mSideBackgroundStyle = stylePrefix + "SidebarBg";
		mHandleStyle = stylePrefix + "Handle";

		SetDepth(0); // Needs to be in front of everything
		SetSkin(desc.Skin);

		mFrontHitBox = GUIDropDownHitBox::Create(false, false);
		mFrontHitBox->OnFocusLost.Connect(std::bind(&GUIDropDownMenu::DropDownFocusLost, this));
		mFrontHitBox->SetFocus(true);
		GUILayoutData hitboxLayoutData = mFrontHitBox->GetLayoutDataInternal();
		hitboxLayoutData.SetWidgetDepth(0);
		hitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::min());
		mFrontHitBox->SetLayoutDataInternal(hitboxLayoutData);
		mFrontHitBox->ChangeParentWidgetInternal(GetInternalInternal());
		mFrontHitBox->MarkLayoutAsDirtyInternal();

		mBackHitBox = GUIDropDownHitBox::Create(false, true);
		GUILayoutData backHitboxLayoutData = mBackHitBox->GetLayoutDataInternal();
		backHitboxLayoutData.SetWidgetDepth(0);
		backHitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::max());
		mBackHitBox->SetLayoutDataInternal(backHitboxLayoutData);
		mBackHitBox->ChangeParentWidgetInternal(GetInternalInternal());
		mBackHitBox->MarkLayoutAsDirtyInternal();

		SPtr<Viewport> viewport = desc.Camera->GetViewport();

		Rect2I targetBounds(0, 0, viewport->GetPixelArea().Width, viewport->GetPixelArea().Height);
		Vector<Rect2I> captureBounds;
		targetBounds.Cut(desc.AdditionalBounds, captureBounds);

		mCaptureHitBox = GUIDropDownHitBox::Create(true, false);
		mCaptureHitBox->SetBounds(captureBounds);
		GUILayoutData captureHitboxLayoutData = mCaptureHitBox->GetLayoutDataInternal();
		captureHitboxLayoutData.SetWidgetDepth(0);
		captureHitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::max());
		mCaptureHitBox->SetLayoutDataInternal(captureHitboxLayoutData);
		mCaptureHitBox->ChangeParentWidgetInternal(GetInternalInternal());
		mCaptureHitBox->MarkLayoutAsDirtyInternal();

		mAdditionalCaptureBounds = desc.AdditionalBounds;

		Rect2I availableBounds = viewport->GetPixelArea();
		mRootMenu = bs_new<DropDownSubMenu>(this, nullptr, desc.Placement, availableBounds, desc.DropDownData, type, 0);
	}

	GUIDropDownMenu::~GUIDropDownMenu()
	{
	}

	void GUIDropDownMenu::OnDestroyed()
	{
		GUIElement::Destroy(mFrontHitBox);
		GUIElement::Destroy(mBackHitBox);
		GUIElement::Destroy(mCaptureHitBox);
		bs_delete(mRootMenu);
		mRootMenu = nullptr;

		CGUIWidget::OnDestroyed();
	}

	void GUIDropDownMenu::DropDownFocusLost()
	{
		mRootMenu->CloseSubMenu();
		GUIDropDownBoxManager::Instance().CloseDropDownBox();
	}

	void GUIDropDownMenu::NotifySubMenuOpened(DropDownSubMenu* subMenu)
	{
		Vector<Rect2I> bounds;
		while(subMenu != nullptr)
		{
			bounds.push_back(subMenu->GetVisibleBounds());

			subMenu = subMenu->MParent;
		}

		mBackHitBox->SetBounds(bounds);

		for(auto& additionalBound : mAdditionalCaptureBounds)
			bounds.push_back(additionalBound);

		mFrontHitBox->SetBounds(bounds);
	}

	void GUIDropDownMenu::NotifySubMenuClosed(DropDownSubMenu* subMenu)
	{
		Vector<Rect2I> bounds;
		while(subMenu != nullptr)
		{
			bounds.push_back(subMenu->GetVisibleBounds());

			subMenu = subMenu->MParent;
		}

		mBackHitBox->SetBounds(bounds);

		for(auto& additionalBound : mAdditionalCaptureBounds)
			bounds.push_back(additionalBound);

		mFrontHitBox->SetBounds(bounds);
	}

	GUIDropDownMenu::DropDownSubMenu::DropDownSubMenu(GUIDropDownMenu* owner, DropDownSubMenu* parent, const DropDownAreaPlacement& placement, const Rect2I& availableBounds, const GUIDropDownData& dropDownData, GUIDropDownType type, u32 depthOffset)
		: MOwner(owner), MType(type), MData(dropDownData), MPage(0), X(0), Y(0), Width(0), Height(0), MDepthOffset(depthOffset), MOpenedUpward(false), MContent(nullptr), MBackgroundFrame(nullptr), MBackgroundPanel(nullptr), MContentPanel(nullptr), MContentLayout(nullptr), MSidebarPanel(nullptr), MParent(parent), MSubMenu(nullptr)
	{
		MAvailableBounds = availableBounds;

		const GUIElementStyle* backgroundStyle = MOwner->GetSkin().GetStyle(MOwner->mBackgroundStyle);
		const GUIElementStyle* sideBarStyle = MOwner->GetSkin().GetStyle(MOwner->mSideBackgroundStyle);

		// Create content GUI element
		MContent = GUIDropDownContent::Create(this, dropDownData, MOwner->mContentStyle);
		MContent->SetKeyboardFocus(true);

		// Content area
		MContentPanel = MOwner->GetPanel()->AddNewElement<GUIPanel>();
		MContentPanel->SetWidth(Width);
		MContentPanel->SetHeight(Height);
		MContentPanel->SetDepthRange(100 - depthOffset * 2 - 1);

		// Background frame
		MBackgroundPanel = MOwner->GetPanel()->AddNewElement<GUIPanel>();
		MBackgroundPanel->SetWidth(Width);
		MBackgroundPanel->SetHeight(Height);
		MBackgroundPanel->SetDepthRange(100 - depthOffset * 2);

		GUILayout* backgroundLayout = MBackgroundPanel->AddNewElement<GUILayoutX>();

		MBackgroundFrame = GUITexture::Create(TextureScaleMode::StretchToFit, MOwner->mBackgroundStyle);
		backgroundLayout->AddElement(MBackgroundFrame);

		MContentLayout = MContentPanel->AddNewElement<GUILayoutY>();
		MContentLayout->AddElement(MContent); // Note: It's important this is added to the layout before we
		// use it for size calculations, in order for its skin to be assigned

		u32 dropDownBoxWidth = DROP_DOWN_BOX_WIDTH + sideBarStyle->Width;

		u32 maxNeededHeight = backgroundStyle->Margins.Top + backgroundStyle->Margins.Bottom;
		u32 numElements = (u32)dropDownData.Entries.size();
		for(u32 i = 0; i < numElements; i++)
			maxNeededHeight += MContent->GetElementHeight(i);

		DropDownAreaPlacement::HorzDir horzDir;
		DropDownAreaPlacement::VertDir vertDir;
		Rect2I placementBounds = placement.GetOptimalBounds(dropDownBoxWidth, maxNeededHeight, availableBounds, horzDir, vertDir);

		MOpenedUpward = vertDir == DropDownAreaPlacement::VertDir::Up;

		u32 actualY = placementBounds.Y;
		if(MOpenedUpward)
			Y = placementBounds.Y + placementBounds.Height;
		else
			Y = placementBounds.Y;

		X = placementBounds.X;
		Width = placementBounds.Width;
		Height = placementBounds.Height;

		MContentPanel->SetPosition(X, actualY);
		MBackgroundPanel->SetPosition(X, actualY);

		UpdateGuiElements();

		MOwner->NotifySubMenuOpened(this);
	}

	GUIDropDownMenu::DropDownSubMenu::~DropDownSubMenu()
	{
		CloseSubMenu();

		MOwner->NotifySubMenuClosed(this);

		GUIElement::Destroy(MContent);

		GUIElement::Destroy(MBackgroundFrame);

		GUILayout::Destroy(MBackgroundPanel);
		GUILayout::Destroy(MContentPanel);

		if(MSidebarPanel != nullptr)
			GUIPanel::Destroy(MSidebarPanel);
	}

	Vector<GUIDropDownMenu::DropDownSubMenu::PageInfo> GUIDropDownMenu::DropDownSubMenu::GetPageInfos() const
	{
		const GUIElementStyle* backgroundStyle = MOwner->GetSkin().GetStyle(MOwner->mBackgroundStyle);

		i32 numElements = (i32)MData.Entries.size();

		PageInfo curPageInfo;
		curPageInfo.Start = 0;
		curPageInfo.End = 0;
		curPageInfo.Idx = 0;
		curPageInfo.Height = backgroundStyle->Margins.Top + backgroundStyle->Margins.Bottom;

		Vector<PageInfo> pageInfos;
		for(i32 i = 0; i < numElements; i++)
		{
			curPageInfo.Height += MContent->GetElementHeight((u32)i);
			curPageInfo.End++;

			if(curPageInfo.Height > Height)
			{
				// Remove last few elements until we fit again
				while(curPageInfo.Height > Height && i >= 0)
				{
					curPageInfo.Height -= MContent->GetElementHeight((u32)i);
					curPageInfo.End--;

					i--;
				}

				// Nothing fits, break out of infinite loop
				if(curPageInfo.Start >= curPageInfo.End)
					break;

				pageInfos.push_back(curPageInfo);

				curPageInfo.Start = curPageInfo.End;
				curPageInfo.Height = backgroundStyle->Margins.Top + backgroundStyle->Margins.Bottom;

				curPageInfo.Idx++;
			}
		}

		if(curPageInfo.Start < curPageInfo.End)
			pageInfos.push_back(curPageInfo);

		return pageInfos;
	}

	void GUIDropDownMenu::DropDownSubMenu::UpdateGuiElements()
	{
		// Remove all elements from content layout
		while(MContentLayout->GetNumChildren() > 0)
			MContentLayout->RemoveElementAt(MContentLayout->GetNumChildren() - 1);

		MContentLayout->AddElement(MContent); // Note: Needs to be added first so that size calculations have proper skin to work with

		const GUIElementStyle* backgroundStyle = MOwner->GetSkin().GetStyle(MOwner->mBackgroundStyle);
		const GUIElementStyle* sideBarStyle = MOwner->GetSkin().GetStyle(MOwner->mSideBackgroundStyle);
		const GUIElementStyle* scrollUpStyle = MOwner->GetSkin().GetStyle(MOwner->mScrollUpStyle);
		const GUIElementStyle* scrollDownStyle = MOwner->GetSkin().GetStyle(MOwner->mScrollDownStyle);

		Vector<PageInfo> pageInfos = GetPageInfos();

		u32 pageStart = 0, pageEnd = 0;
		u32 pageHeight = 0;
		u32 pageCount = (u32)pageInfos.size();
		if(pageCount > MPage)
		{
			pageStart = pageInfos[MPage].Start;
			pageEnd = pageInfos[MPage].End;
			pageHeight = pageInfos[MPage].Height;
		}

		i32 actualY = Y;

		if(MOpenedUpward)
			actualY -= (i32)pageHeight;

		// Add sidebar if needed
		u32 contentOffset = 0;
		if(pageInfos.size() > 1)
		{
			u32 sidebarHeight = pageHeight - 2;
			contentOffset = sideBarStyle->Width;

			if(MSidebarPanel == nullptr)
			{
				MSidebarPanel = MOwner->GetPanel()->AddNewElement<GUIPanel>();

				MScrollUpBtn = GUIButton::Create(HString(""), MOwner->mScrollUpStyle);
				MScrollUpBtn->OnClick.Connect(std::bind(&DropDownSubMenu::ScrollUp, this));

				GUIElementOptions scrollUpBtnOptions = MScrollUpBtn->GetOptionFlags();
				scrollUpBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

				MScrollUpBtn->SetOptionFlags(scrollUpBtnOptions);

				MScrollDownBtn = GUIButton::Create(HString(""), MOwner->mScrollDownStyle);
				MScrollDownBtn->OnClick.Connect(std::bind(&::bs::GUIDropDownMenu::DropDownSubMenu::ScrollDown, this));

				GUIElementOptions scrollDownBtnOptions = MScrollDownBtn->GetOptionFlags();
				scrollDownBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

				MScrollDownBtn->SetOptionFlags(scrollDownBtnOptions);

				MHandle = GUITexture::Create(MOwner->mHandleStyle);
				GUITexture* background = GUITexture::Create(MOwner->mSideBackgroundStyle);
				background->SetElementDepthInternal(2);

				MSidebarPanel->AddElement(background);
				MSidebarPanel->AddElement(MScrollUpBtn);
				MSidebarPanel->AddElement(MScrollDownBtn);
				MSidebarPanel->AddElement(MHandle);
			}

			MScrollUpBtn->SetPosition(1, 1);
			MScrollDownBtn->SetPosition(1, sidebarHeight - 1 - scrollDownStyle->Height);

			u32 maxHandleSize = std::max(0, (i32)sidebarHeight - (i32)scrollDownStyle->Height - (i32)scrollUpStyle->Height - 2);
			u32 handleSize = maxHandleSize / pageCount;

			i32 handlePos = 1 + scrollUpStyle->Height + MPage * handleSize;

			MHandle->SetPosition(1, handlePos);
			MHandle->SetHeight(handleSize);

			MSidebarPanel->SetPosition(X, actualY);
			MSidebarPanel->SetWidth(sideBarStyle->Width);
			MSidebarPanel->SetHeight(sidebarHeight);
		}
		else
		{
			if(MSidebarPanel != nullptr)
			{
				GUIPanel::Destroy(MSidebarPanel);
				MSidebarPanel = nullptr;
			}
		}

		MContent->SetRange(pageStart, pageEnd);

		if(MSubMenu == nullptr)
			MContent->SetKeyboardFocus(true);

		// Resize and reposition areas
		MBackgroundPanel->SetWidth(Width - contentOffset);
		MBackgroundPanel->SetHeight(pageHeight);
		MBackgroundPanel->SetPosition(X + contentOffset, actualY);

		MVisibleBounds = Rect2I(X, actualY, Width, pageHeight);

		u32 contentWidth = (u32)std::max(0, (i32)Width - (i32)backgroundStyle->Margins.Left - (i32)backgroundStyle->Margins.Right - (i32)contentOffset);
		u32 contentHeight = (u32)std::max(0, (i32)pageHeight - (i32)backgroundStyle->Margins.Top - (i32)backgroundStyle->Margins.Bottom);

		MContentPanel->SetWidth(contentWidth);
		MContentPanel->SetHeight(contentHeight);
		MContentPanel->SetPosition(X + contentOffset + backgroundStyle->Margins.Left, actualY + backgroundStyle->Margins.Top);
	}

	void GUIDropDownMenu::DropDownSubMenu::ScrollDown()
	{
		MPage++;
		if(MPage == (u32)GetPageInfos().size())
			MPage = 0;

		UpdateGuiElements();

		CloseSubMenu();
	}

	void GUIDropDownMenu::DropDownSubMenu::ScrollUp()
	{
		if(MPage > 0)
			MPage--;
		else
			MPage = (u32)GetPageInfos().size() - 1;

		UpdateGuiElements();
		CloseSubMenu();
	}

	void GUIDropDownMenu::DropDownSubMenu::ScrollToTop()
	{
		MPage = 0;
		UpdateGuiElements();

		CloseSubMenu();
	}

	void GUIDropDownMenu::DropDownSubMenu::ScrollToBottom()
	{
		MPage = (u32)(GetPageInfos().size() - 1);
		UpdateGuiElements();

		CloseSubMenu();
	}

	void GUIDropDownMenu::DropDownSubMenu::CloseSubMenu()
	{
		if(MSubMenu != nullptr)
		{
			bs_delete(MSubMenu);
			MSubMenu = nullptr;

			MContent->SetKeyboardFocus(true);
		}
	}

	void GUIDropDownMenu::DropDownSubMenu::ElementActivated(u32 idx, const Rect2I& bounds)
	{
		CloseSubMenu();

		if(!MData.Entries[idx].IsSubMenu())
		{
			auto callback = MData.Entries[idx].GetCallback();
			if(callback != nullptr)
				callback();

			if(MType != GUIDropDownType::MultiListBox)
				GUIDropDownBoxManager::Instance().CloseDropDownBox();
		}
		else
		{
			MContent->SetKeyboardFocus(false);

			MSubMenu = bs_new<DropDownSubMenu>(MOwner, this, DropDownAreaPlacement::AroundBoundsVert(bounds), MAvailableBounds, MData.Entries[idx].GetSubMenuData(), MType, MDepthOffset + 1);
		}
	}

	void GUIDropDownMenu::DropDownSubMenu::Close()
	{
		if(MParent != nullptr)
			MParent->CloseSubMenu();
		else // We're the last sub-menu, close the whole thing
			GUIDropDownBoxManager::Instance().CloseDropDownBox();
	}

	void GUIDropDownMenu::DropDownSubMenu::ElementSelected(u32 idx)
	{
		CloseSubMenu();
	}
} // namespace bs
