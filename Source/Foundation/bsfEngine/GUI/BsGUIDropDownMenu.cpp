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
#include "StyleSheet/BsGUIStyleSheet.h"
#include "Text/BsStockIcons.h"

using namespace std::placeholders;

using namespace bs;

const u32 GUIDropDownMenu::kDropDownBoxWidth = 250;

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
	SetDepth(0); // Needs to be in front of everything
	SetSkin(desc.Skin);

	const GUIStyleSheetRules frameStyleSheetRules = GetStyleSheetCascade().BuildRules(GUITexture::kElementType, kBackgroundFrameStyleClass);
	mBackgroundFramePadding = frameStyleSheetRules.Padding;

	const GUIStyleSheetRules scrollbarBackgroundStyleSheetRules = GetStyleSheetCascade().BuildRules(GUITexture::kElementType, kScrollbarBackgroundStyleClass);
	mScrollbarWidth = scrollbarBackgroundStyleSheetRules.Size.Width;

	const GUIStyleSheetRules scrollbarButtonStyleSheetRules = GetStyleSheetCascade().BuildRules(GUIButton::kElementType, kScrollbarButtonStyleClass);
	mScrollButtonHeight = scrollbarButtonStyleSheetRules.Size.Height;

	mFrontHitBox = GUIDropDownHitBox::Create(false, false);
	mFrontHitBox->OnFocusLost.Connect(std::bind(&GUIDropDownMenu::DropDownFocusLost, this));
	mFrontHitBox->SetFocus(true);
	GUILayoutData hitboxLayoutData = mFrontHitBox->GetLayoutData();
	hitboxLayoutData.SetWidgetDepth(0);
	hitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::min());
	mFrontHitBox->SetLayoutData(hitboxLayoutData);
	mFrontHitBox->ChangeParentWidget(GetInternalInternal());
	mFrontHitBox->MarkLayoutAsDirty();

	mBackHitBox = GUIDropDownHitBox::Create(false, true);
	GUILayoutData backHitboxLayoutData = mBackHitBox->GetLayoutData();
	backHitboxLayoutData.SetWidgetDepth(0);
	backHitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::max());
	mBackHitBox->SetLayoutData(backHitboxLayoutData);
	mBackHitBox->ChangeParentWidget(GetInternalInternal());
	mBackHitBox->MarkLayoutAsDirty();

	SPtr<Viewport> viewport = desc.Camera->GetViewport();

	Rect2I targetBounds(0, 0, viewport->GetPixelArea().Width, viewport->GetPixelArea().Height);
	Vector<Rect2I> captureBounds;
	targetBounds.Cut(desc.AdditionalBounds, captureBounds);

	mCaptureHitBox = GUIDropDownHitBox::Create(true, false);
	mCaptureHitBox->SetBounds(captureBounds);
	GUILayoutData captureHitboxLayoutData = mCaptureHitBox->GetLayoutData();
	captureHitboxLayoutData.SetWidgetDepth(0);
	captureHitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::max());
	mCaptureHitBox->SetLayoutData(captureHitboxLayoutData);
	mCaptureHitBox->ChangeParentWidget(GetInternalInternal());
	mCaptureHitBox->MarkLayoutAsDirty();

	mAdditionalCaptureBounds = desc.AdditionalBounds;

	Rect2I availableBounds = viewport->GetPixelArea();
	mRootMenu = B3DNew<DropDownSubMenu>(this, nullptr, desc.Placement, availableBounds, desc.DropDownData, type, 0);
}

GUIDropDownMenu::~GUIDropDownMenu()
{
}

void GUIDropDownMenu::OnDestroyed()
{
	mFrontHitBox->Destroy();
	mBackHitBox->Destroy();
	mCaptureHitBox->Destroy();
	B3DDelete(mRootMenu);
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

		subMenu = subMenu->ParentSubMenu;
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

		subMenu = subMenu->ParentSubMenu;
	}

	mBackHitBox->SetBounds(bounds);

	for(auto& additionalBound : mAdditionalCaptureBounds)
		bounds.push_back(additionalBound);

	mFrontHitBox->SetBounds(bounds);
}

GUIDropDownMenu::DropDownSubMenu::DropDownSubMenu(GUIDropDownMenu* owner, DropDownSubMenu* parent, const DropDownAreaPlacement& placement, const Rect2I& availableBounds, const GUIDropDownData& dropDownData, GUIDropDownType type, u32 depthOffset)
	: Owner(owner), Type(type), Data(dropDownData), Page(0), X(0), Y(0), Width(0), Height(0), DepthOffset(depthOffset), IsOpenedUpward(false), Content(nullptr), BackgroundFrame(nullptr), BackgroundPanel(nullptr), ContentPanel(nullptr), ContentLayout(nullptr), SidebarPanel(nullptr), ParentSubMenu(parent), ActiveChildSubMenu(nullptr)
{
	MAvailableBounds = availableBounds;

	const RectOffset& backgroundFramePadding = owner->mBackgroundFramePadding;
	const u32 scrollbarWidth = owner->mScrollbarWidth;

	// Create content GUI element
	Content = GUIDropDownContent::Create(this, dropDownData);
	Content->SetKeyboardFocus(true);

	// Content area
	ContentPanel = Owner->GetPanel()->AddNewElement<GUIPanel>();
	ContentPanel->SetWidth(Width);
	ContentPanel->SetHeight(Height);
	ContentPanel->SetDepthRange(100 - depthOffset * 2 - 1);

	// Background frame
	BackgroundPanel = Owner->GetPanel()->AddNewElement<GUIPanel>();
	BackgroundPanel->SetWidth(Width);
	BackgroundPanel->SetHeight(Height);
	BackgroundPanel->SetDepthRange(100 - depthOffset * 2);

	GUILayout* backgroundLayout = BackgroundPanel->AddNewElement<GUILayoutX>();

	BackgroundFrame = GUITexture::Create(GUITextureContents(nullptr, TextureScaleMode::StretchToFit), GUIDropDownMenu::kBackgroundFrameStyleClass);
	backgroundLayout->AddElement(BackgroundFrame);

	ContentLayout = ContentPanel->AddNewElement<GUILayoutY>();
	ContentLayout->AddElement(Content); // Note: It's important this is added to the layout before we
	// use it for size calculations, in order for its skin to be assigned

	u32 dropDownBoxWidth = kDropDownBoxWidth + scrollbarWidth;

	u32 maxNeededHeight = backgroundFramePadding.Top + backgroundFramePadding.Bottom;
	u32 numElements = (u32)dropDownData.Entries.size();
	for(u32 i = 0; i < numElements; i++)
		maxNeededHeight += Content->GetElementHeight(i);

	DropDownAreaPlacement::HorzDir horzDir;
	DropDownAreaPlacement::VertDir vertDir;
	Rect2I placementBounds = placement.GetOptimalBounds(dropDownBoxWidth, maxNeededHeight, availableBounds, horzDir, vertDir);

	IsOpenedUpward = vertDir == DropDownAreaPlacement::VertDir::Up;

	u32 actualY = placementBounds.Y;
	if(IsOpenedUpward)
		Y = placementBounds.Y + placementBounds.Height;
	else
		Y = placementBounds.Y;

	X = placementBounds.X;
	Width = placementBounds.Width;
	Height = placementBounds.Height;

	ContentPanel->SetPosition(X, actualY);
	BackgroundPanel->SetPosition(X, actualY);

	UpdateGuiElements();

	Owner->NotifySubMenuOpened(this);
}

GUIDropDownMenu::DropDownSubMenu::~DropDownSubMenu()
{
	CloseSubMenu();

	Owner->NotifySubMenuClosed(this);

	Content->Destroy();
	BackgroundFrame->Destroy();
	BackgroundPanel->Destroy();
	ContentPanel->Destroy();

	if(SidebarPanel != nullptr)
		SidebarPanel->Destroy();
}

Vector<GUIDropDownMenu::DropDownSubMenu::PageInfo> GUIDropDownMenu::DropDownSubMenu::GetPageInfos() const
{
	const RectOffset& backgroundFramePadding = Owner->mBackgroundFramePadding;

	i32 numElements = (i32)Data.Entries.size();

	PageInfo curPageInfo;
	curPageInfo.Start = 0;
	curPageInfo.End = 0;
	curPageInfo.Idx = 0;
	curPageInfo.Height = backgroundFramePadding.Top + backgroundFramePadding.Bottom;

	Vector<PageInfo> pageInfos;
	for(i32 i = 0; i < numElements; i++)
	{
		curPageInfo.Height += Content->GetElementHeight((u32)i);
		curPageInfo.End++;

		if(curPageInfo.Height > Height)
		{
			// Remove last few elements until we fit again
			while(curPageInfo.Height > Height && i >= 0)
			{
				curPageInfo.Height -= Content->GetElementHeight((u32)i);
				curPageInfo.End--;

				i--;
			}

			// Nothing fits, break out of infinite loop
			if(curPageInfo.Start >= curPageInfo.End)
				break;

			pageInfos.push_back(curPageInfo);

			curPageInfo.Start = curPageInfo.End;
			curPageInfo.Height = backgroundFramePadding.Top + backgroundFramePadding.Bottom;

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
	while(ContentLayout->GetNumChildren() > 0)
		ContentLayout->RemoveElementAt(ContentLayout->GetNumChildren() - 1);

	ContentLayout->AddElement(Content); // Note: Needs to be added first so that size calculations have proper skin to work with

	const RectOffset& backgroundFramePadding = Owner->mBackgroundFramePadding;
	const u32 scrollbarWidth = Owner->mScrollbarWidth;
	const u32 scrollButtonHeight = Owner->mScrollButtonHeight;

	Vector<PageInfo> pageInfos = GetPageInfos();

	u32 pageStart = 0, pageEnd = 0;
	u32 pageHeight = 0;
	u32 pageCount = (u32)pageInfos.size();
	if(pageCount > Page)
	{
		pageStart = pageInfos[Page].Start;
		pageEnd = pageInfos[Page].End;
		pageHeight = pageInfos[Page].Height;
	}

	i32 actualY = Y;

	if(IsOpenedUpward)
		actualY -= (i32)pageHeight;

	// Add sidebar if needed
	u32 contentOffset = 0;
	if(pageInfos.size() > 1)
	{
		u32 sidebarHeight = pageHeight - 2;
		contentOffset = scrollbarWidth;

		if(SidebarPanel == nullptr)
		{
			SidebarPanel = Owner->GetPanel()->AddNewElement<GUIPanel>();

			MScrollUpBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretUp)), kScrollbarButtonStyleClass);
			MScrollUpBtn->OnClick.Connect(std::bind(&DropDownSubMenu::ScrollUp, this));

			GUIElementOptions scrollUpBtnOptions = MScrollUpBtn->GetOptionFlags();
			scrollUpBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

			MScrollUpBtn->SetOptionFlags(scrollUpBtnOptions);

			MScrollDownBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretDown)), kScrollbarButtonStyleClass);
			MScrollDownBtn->OnClick.Connect(std::bind(&::bs::GUIDropDownMenu::DropDownSubMenu::ScrollDown, this));

			GUIElementOptions scrollDownBtnOptions = MScrollDownBtn->GetOptionFlags();
			scrollDownBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

			MScrollDownBtn->SetOptionFlags(scrollDownBtnOptions);

			MHandle = GUITexture::Create(kScrollbarHandleStyleClass);
			GUITexture* background = GUITexture::Create(kScrollbarBackgroundStyleClass);
			background->SetElementDepth(2);

			SidebarPanel->AddElement(background);
			SidebarPanel->AddElement(MScrollUpBtn);
			SidebarPanel->AddElement(MScrollDownBtn);
			SidebarPanel->AddElement(MHandle);
		}

		MScrollUpBtn->SetPosition(1, 1);
		MScrollDownBtn->SetPosition(1, sidebarHeight - 1 - scrollButtonHeight);

		u32 maxHandleSize = std::max(0, (i32)sidebarHeight - (i32)scrollButtonHeight * 2 - 2);
		u32 handleSize = maxHandleSize / pageCount;

		i32 handlePos = 1 + scrollButtonHeight + Page * handleSize;

		MHandle->SetPosition(1, handlePos);
		MHandle->SetHeight(handleSize);

		SidebarPanel->SetPosition(X, actualY);
		SidebarPanel->SetWidth(scrollbarWidth);
		SidebarPanel->SetHeight(sidebarHeight);
	}
	else
	{
		if(SidebarPanel != nullptr)
		{
			SidebarPanel->Destroy();
			SidebarPanel = nullptr;
		}
	}

	Content->SetRange(pageStart, pageEnd);

	if(ActiveChildSubMenu == nullptr)
		Content->SetKeyboardFocus(true);

	// Resize and reposition areas
	BackgroundPanel->SetWidth(Width - contentOffset);
	BackgroundPanel->SetHeight(pageHeight);
	BackgroundPanel->SetPosition(X + contentOffset, actualY);

	MVisibleBounds = Rect2I(X, actualY, Width, pageHeight);

	u32 contentWidth = (u32)std::max(0, (i32)Width - (i32)backgroundFramePadding.Left - (i32)backgroundFramePadding.Right - (i32)contentOffset);
	u32 contentHeight = (u32)std::max(0, (i32)pageHeight - (i32)backgroundFramePadding.Top - (i32)backgroundFramePadding.Bottom);

	ContentPanel->SetWidth(contentWidth);
	ContentPanel->SetHeight(contentHeight);
	ContentPanel->SetPosition(X + contentOffset + backgroundFramePadding.Left, actualY + backgroundFramePadding.Top);
}

void GUIDropDownMenu::DropDownSubMenu::ScrollDown()
{
	Page++;
	if(Page == (u32)GetPageInfos().size())
		Page = 0;

	UpdateGuiElements();

	CloseSubMenu();
}

void GUIDropDownMenu::DropDownSubMenu::ScrollUp()
{
	if(Page > 0)
		Page--;
	else
		Page = (u32)GetPageInfos().size() - 1;

	UpdateGuiElements();
	CloseSubMenu();
}

void GUIDropDownMenu::DropDownSubMenu::ScrollToTop()
{
	Page = 0;
	UpdateGuiElements();

	CloseSubMenu();
}

void GUIDropDownMenu::DropDownSubMenu::ScrollToBottom()
{
	Page = (u32)(GetPageInfos().size() - 1);
	UpdateGuiElements();

	CloseSubMenu();
}

void GUIDropDownMenu::DropDownSubMenu::CloseSubMenu()
{
	if(ActiveChildSubMenu != nullptr)
	{
		B3DDelete(ActiveChildSubMenu);
		ActiveChildSubMenu = nullptr;

		Content->SetKeyboardFocus(true);
	}
}

void GUIDropDownMenu::DropDownSubMenu::ElementActivated(u32 idx, const Rect2I& bounds)
{
	CloseSubMenu();

	if(!Data.Entries[idx].IsSubMenu())
	{
		auto callback = Data.Entries[idx].GetCallback();
		if(callback != nullptr)
			callback();

		if(Type != GUIDropDownType::MultiListBox)
			GUIDropDownBoxManager::Instance().CloseDropDownBox();
	}
	else
	{
		Content->SetKeyboardFocus(false);

		ActiveChildSubMenu = B3DNew<DropDownSubMenu>(Owner, this, DropDownAreaPlacement::AroundBoundsVert(bounds), MAvailableBounds, Data.Entries[idx].GetSubMenuData(), Type, DepthOffset + 1);
	}
}

void GUIDropDownMenu::DropDownSubMenu::Close()
{
	if(ParentSubMenu != nullptr)
		ParentSubMenu->CloseSubMenu();
	else // We're the last sub-menu, close the whole thing
		GUIDropDownBoxManager::Instance().CloseDropDownBox();
}

void GUIDropDownMenu::DropDownSubMenu::ElementSelected(u32 idx)
{
	CloseSubMenu();
}
