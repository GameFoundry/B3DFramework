//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIDropDownMenu.h"

#include "B3DGUIUtility.h"
#include "GUI/B3DGUIPanel.h"
#include "GUI/B3DGUILayoutY.h"
#include "GUI/B3DGUILayoutX.h"
#include "GUI/B3DGUITexture.h"
#include "GUI/B3DGUILabel.h"
#include "GUI/B3DGUIButton.h"
#include "GUI/B3DGUISpace.h"
#include "GUI/B3DGUIContent.h"
#include "GpuBackend/B3DViewport.h"
#include "GUI/B3DGUIListBox.h"
#include "GUI/B3DGUIDropDownBoxManager.h"
#include "Scene/B3DSceneObject.h"
#include "GUI/B3DGUIDropDownHitBox.h"
#include "GUI/B3DGUIDropDownContent.h"
#include "Components/B3DCamera.h"
#include "Debug/B3DDebug.h"
#include "StyleSheet/B3DGUIStyleSheet.h"
#include "Text/B3DStockIcons.h"

using namespace b3d;

const GUILogicalUnit GUIDropDownMenu::kDropDownBoxWidth = 250;

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

GUIDropDownMenu::GUIDropDownMenu(const HSceneObject& parent, const DropDownBoxCreateInformation& createInformation, GUIDropDownType type)
	: GUIWidget(parent, createInformation.Camera), mMenuCreateInformation(createInformation), mType(type)
{ }

void GUIDropDownMenu::OnCreated()
{
	GUIWidget::OnCreated();

	SetDepth(0); // Needs to be in front of everything
	SetStyleSheetCascade(mMenuCreateInformation.StyleSheetCascade);

	const GUIStyleSheetRules frameStyleSheetRules = GetStyleSheetCascade().BuildRules(GUITexture::kStyleSheetElementType, kBackgroundFrameStyleClass);
	mBackgroundFramePadding = frameStyleSheetRules.Padding;

	const GUIStyleSheetRules scrollbarBackgroundStyleSheetRules = GetStyleSheetCascade().BuildRules(GUITexture::kStyleSheetElementType, kScrollbarBackgroundStyleClass);
	mScrollbarWidth = scrollbarBackgroundStyleSheetRules.Size.Width;

	const GUIStyleSheetRules scrollbarButtonStyleSheetRules = GetStyleSheetCascade().BuildRules(GUIButton::kElementType, kScrollbarButtonStyleClass);
	mScrollButtonHeight = scrollbarButtonStyleSheetRules.Size.Height;

	mFrontHitBox = GUIDropDownHitBox::Create(false, false);
	mFrontHitBox->OnFocusLost.Connect([this]() { DropDownFocusLost(); });
	mFrontHitBox->SetFocus(true);
	GUILayoutData hitboxLayoutData = mFrontHitBox->GetLayoutData();
	hitboxLayoutData.RelativePosition = GUILogicalPoint::kZero;
	hitboxLayoutData.SetWidgetDepth(0);
	hitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::min());
	mFrontHitBox->SetLayoutData(hitboxLayoutData);
	mFrontHitBox->ChangeParentWidget(this);
	mFrontHitBox->MarkLayoutAsDirty();

	mBackHitBox = GUIDropDownHitBox::Create(false, true);
	GUILayoutData backHitboxLayoutData = mBackHitBox->GetLayoutData();
	backHitboxLayoutData.RelativePosition = GUILogicalPoint::kZero;
	backHitboxLayoutData.SetWidgetDepth(0);
	backHitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::max());
	mBackHitBox->SetLayoutData(backHitboxLayoutData);
	mBackHitBox->ChangeParentWidget(this);
	mBackHitBox->MarkLayoutAsDirty();

	TShared<Viewport> viewport = mMenuCreateInformation.Camera->GetViewport();

	GUIPhysicalArea targetBounds(0, 0, (i32)viewport->GetPixelArea().Width, (i32)viewport->GetPixelArea().Height);
	Vector<GUIPhysicalArea> captureBounds;
	targetBounds.Cut(mMenuCreateInformation.AdditionalBounds, captureBounds);

	mCaptureHitBox = GUIDropDownHitBox::Create(true, false);
	mCaptureHitBox->SetBounds(captureBounds);
	GUILayoutData captureHitboxLayoutData = mCaptureHitBox->GetLayoutData();
	captureHitboxLayoutData.RelativePosition = GUILogicalPoint::kZero;
	captureHitboxLayoutData.SetWidgetDepth(0);
	captureHitboxLayoutData.SetPanelDepth(std::numeric_limits<i16>::max());
	mCaptureHitBox->SetLayoutData(captureHitboxLayoutData);
	mCaptureHitBox->ChangeParentWidget(this);
	mCaptureHitBox->MarkLayoutAsDirty();

	mAdditionalCaptureBounds = mMenuCreateInformation.AdditionalBounds;

	const GUIPhysicalArea availableBounds = viewport->GetPixelArea().To<GUIPhysicalUnit>();
	mRootMenu = B3DNew<DropDownSubMenu>(this, nullptr, mMenuCreateInformation.Placement, availableBounds, mMenuCreateInformation.DropDownData, mType, 0);
}

void GUIDropDownMenu::OnDestroyed()
{
	mFrontHitBox->Destroy();
	mBackHitBox->Destroy();
	mCaptureHitBox->Destroy();
	B3DDelete(mRootMenu);
	mRootMenu = nullptr;

	GUIWidget::OnDestroyed();
}

void GUIDropDownMenu::DropDownFocusLost()
{
	mRootMenu->CloseSubMenu();
	GUIDropDownBoxManager::Instance().CloseDropDownBox();
}

void GUIDropDownMenu::NotifySubMenuOpened(DropDownSubMenu* subMenu)
{
	Vector<GUIPhysicalArea> bounds;
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
	Vector<GUIPhysicalArea> bounds;
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

GUIDropDownMenu::DropDownSubMenu::DropDownSubMenu(GUIDropDownMenu* owner, DropDownSubMenu* parent, const TDropDownAreaPlacement<GUIPhysicalUnit>& placement, const GUIPhysicalArea& availableBounds, const GUIDropDownData& dropDownData, GUIDropDownType type, u32 depthOffset)
	: Owner(owner), Type(type), Data(dropDownData), Page(0), DepthOffset(depthOffset), IsOpenedUpward(false), Content(nullptr), BackgroundFrame(nullptr), BackgroundPanel(nullptr), ContentPanel(nullptr), ContentLayout(nullptr), SidebarPanel(nullptr), ParentSubMenu(parent), ActiveChildSubMenu(nullptr)
{
	AvailableBounds = availableBounds;

	const RectOffset& backgroundFramePadding = owner->mBackgroundFramePadding;
	const GUILogicalUnit scrollbarWidth = owner->mScrollbarWidth;

	// Create content GUI element
	Content = GUIDropDownContent::Create(this, dropDownData);
	Content->SetKeyboardFocus(true);

	// Content area
	ContentPanel = Owner->GetPanel()->AddNewElement<GUIPanel>();
	ContentPanel->SetSize(Size);
	ContentPanel->SetDepthRange(100 - depthOffset * 2 - 1);

	// Background frame
	BackgroundPanel = Owner->GetPanel()->AddNewElement<GUIPanel>();
	BackgroundPanel->SetSize(Size);
	BackgroundPanel->SetDepthRange(100 - depthOffset * 2);

	GUILayout* backgroundLayout = BackgroundPanel->AddNewElement<GUILayoutX>();

	BackgroundFrame = GUITexture::Create(GUITextureContents(nullptr, TextureScaleMode::StretchToFit), GUIDropDownMenu::kBackgroundFrameStyleClass);
	backgroundLayout->AddElement(BackgroundFrame);

	ContentLayout = ContentPanel->AddNewElement<GUILayoutY>();
	ContentLayout->AddElement(Content); // Note: It's important this is added to the layout before we
	// use it for size calculations, in order for its skin to be assigned

	GUILogicalUnit dropDownBoxWidth = kDropDownBoxWidth + scrollbarWidth;

	GUILogicalUnit maxNeededHeight = backgroundFramePadding.Top + backgroundFramePadding.Bottom;
	u32 elementCount = (u32)dropDownData.Entries.size();
	for(u32 i = 0; i < elementCount; i++)
		maxNeededHeight += Content->GetElementHeight(i);

	const GUILogicalSize logicalMenuSize(dropDownBoxWidth, maxNeededHeight);
	const GUIPhysicalSize physicalMenuSize = GUIUtility::LogicalToPhysical(logicalMenuSize, owner->GetDPIScale());

	TDropDownAreaPlacement<GUIPhysicalUnit>::HorizontalDirection horizontalDirection;
	TDropDownAreaPlacement<GUIPhysicalUnit>::VerticalDirection verticalDirection;
	GUIPhysicalArea physicalPlacementBounds = placement.GetOptimalBounds(physicalMenuSize, availableBounds, horizontalDirection, verticalDirection);
	GUILogicalArea logicalPlacementBounds = GUIUtility::PhysicalToLogical(physicalPlacementBounds, owner->GetDPIScale());

	IsOpenedUpward = verticalDirection == TDropDownAreaPlacement<GUIPhysicalUnit>::VerticalDirection::Up;

	GUILogicalUnit actualY = logicalPlacementBounds.Y;
	if(IsOpenedUpward)
		Position.Y = logicalPlacementBounds.Y + logicalPlacementBounds.Height;
	else
		Position.Y = logicalPlacementBounds.Y;

	Position.X = logicalPlacementBounds.X;
	Size = logicalPlacementBounds.GetSize();

	ContentPanel->SetPosition(Position.X, actualY);
	BackgroundPanel->SetPosition(Position.X, actualY);

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

	i32 elementCount = (i32)Data.Entries.size();

	PageInfo curPageInfo;
	curPageInfo.Start = 0;
	curPageInfo.End = 0;
	curPageInfo.Idx = 0;
	curPageInfo.Height = backgroundFramePadding.Top + backgroundFramePadding.Bottom;

	Vector<PageInfo> pageInfos;
	for(i32 elementIndex = 0; elementIndex < elementCount; elementIndex++)
	{
		curPageInfo.Height += Content->GetElementHeight((u32)elementIndex);
		curPageInfo.End++;

		if(curPageInfo.Height > Size.Height)
		{
			// Remove last few elements until we fit again
			while(curPageInfo.Height > Size.Height && elementIndex >= 0)
			{
				curPageInfo.Height -= (u32)Content->GetElementHeight((u32)elementIndex);
				curPageInfo.End--;

				elementIndex--;
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
	while(ContentLayout->GetChildCount() > 0)
		ContentLayout->RemoveElementAt(ContentLayout->GetChildCount() - 1);

	ContentLayout->AddElement(Content); // Note: Needs to be added first so that size calculations have proper skin to work with

	const RectOffset& backgroundFramePadding = Owner->mBackgroundFramePadding;
	const GUILogicalUnit scrollbarWidth = Owner->mScrollbarWidth;
	const GUILogicalUnit scrollButtonHeight = Owner->mScrollButtonHeight;

	Vector<PageInfo> pageInfos = GetPageInfos();

	u32 pageStart = 0, pageEnd = 0;
	GUILogicalUnit pageHeight = 0;
	u32 pageCount = (u32)pageInfos.size();
	if(pageCount > Page)
	{
		pageStart = pageInfos[Page].Start;
		pageEnd = pageInfos[Page].End;
		pageHeight = pageInfos[Page].Height;
	}

	GUILogicalUnit actualY = Position.Y;

	if(IsOpenedUpward)
		actualY -= (i32)pageHeight;

	// Add sidebar if needed
	GUILogicalUnit contentOffset = 0;
	if(pageInfos.size() > 1)
	{
		GUILogicalUnit sidebarHeight = pageHeight - 2;
		contentOffset = scrollbarWidth;

		if(SidebarPanel == nullptr)
		{
			SidebarPanel = Owner->GetPanel()->AddNewElement<GUIPanel>();

			ScrollUpBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretUp)), kScrollbarButtonStyleClass);
			ScrollUpBtn->OnClick.Connect([this]() { ScrollUp(); });

			GUIElementOptions scrollUpBtnOptions = ScrollUpBtn->GetOptionFlags();
			scrollUpBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

			ScrollUpBtn->SetOptionFlags(scrollUpBtnOptions);

			ScrollDownBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretDown)), kScrollbarButtonStyleClass);
			ScrollDownBtn->OnClick.Connect([this]() { ScrollDown(); });

			GUIElementOptions scrollDownBtnOptions = ScrollDownBtn->GetOptionFlags();
			scrollDownBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

			ScrollDownBtn->SetOptionFlags(scrollDownBtnOptions);

			Handle = GUITexture::Create(kScrollbarHandleStyleClass);
			GUITexture* background = GUITexture::Create(kScrollbarBackgroundStyleClass);
			background->SetElementDepth(2);

			SidebarPanel->AddElement(background);
			SidebarPanel->AddElement(ScrollUpBtn);
			SidebarPanel->AddElement(ScrollDownBtn);
			SidebarPanel->AddElement(Handle);
		}

		ScrollUpBtn->SetPosition(1, 1);
		ScrollDownBtn->SetPosition(1, sidebarHeight - 1 - scrollButtonHeight);

		GUILogicalUnit maxHandleSize = Math::Max(sidebarHeight - scrollButtonHeight * 2 - 2, 0);
		GUILogicalUnit handleSize = maxHandleSize / pageCount;

		GUILogicalUnit handlePos = scrollButtonHeight + handleSize * Page + 1;

		Handle->SetPosition(1, handlePos);
		Handle->SetHeight(handleSize);

		SidebarPanel->SetPosition(Position.X, actualY);
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
	BackgroundPanel->SetWidth(Size.Width - contentOffset);
	BackgroundPanel->SetHeight(pageHeight);
	BackgroundPanel->SetPosition(Position.X + contentOffset, actualY);

	const GUILogicalArea logicalVisibleBounds(Position.X, actualY, Size.Width, pageHeight);

	VisibleBounds = GUIUtility::LogicalToPhysical(logicalVisibleBounds, Owner->GetDPIScale());

	GUILogicalUnit contentWidth = Math::Max((i32)Size.Width - (i32)backgroundFramePadding.Left - (i32)backgroundFramePadding.Right - (i32)contentOffset, 0);
	GUILogicalUnit contentHeight = Math::Max((i32)pageHeight - (i32)backgroundFramePadding.Top - (i32)backgroundFramePadding.Bottom, 0);

	ContentPanel->SetWidth(contentWidth);
	ContentPanel->SetHeight(contentHeight);
	ContentPanel->SetPosition(Position.X + contentOffset + backgroundFramePadding.Left, actualY + backgroundFramePadding.Top);
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

void GUIDropDownMenu::DropDownSubMenu::ElementActivated(u32 index, const GUIPhysicalArea& bounds)
{
	CloseSubMenu();

	if(!Data.Entries[index].IsSubMenu())
	{
		auto callback = Data.Entries[index].GetCallback();
		if(callback != nullptr)
			callback();

		if(Type != GUIDropDownType::MultiListBox)
			GUIDropDownBoxManager::Instance().CloseDropDownBox();
	}
	else
	{
		Content->SetKeyboardFocus(false);

		ActiveChildSubMenu = B3DNew<DropDownSubMenu>(Owner, this, TDropDownAreaPlacement<GUIPhysicalUnit>::AroundBoundsVertical(bounds), AvailableBounds, Data.Entries[index].GetSubMenuData(), Type, DepthOffset + 1);
	}
}

void GUIDropDownMenu::DropDownSubMenu::Close()
{
	if(ParentSubMenu != nullptr)
		ParentSubMenu->CloseSubMenu();
	else // We're the last sub-menu, close the whole thing
		GUIDropDownBoxManager::Instance().CloseDropDownBox();
}

void GUIDropDownMenu::DropDownSubMenu::ElementSelected(u32 index)
{
	CloseSubMenu();
}
