//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollBar.h"
#include "Image/BsSpriteTexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUISliderHandle.h"
#include "GUI/BsGUISpace.h"
#include "Text/BsStockIcons.h"

using namespace std::placeholders;

using namespace bs;

const GUIPhysicalUnit GUIScrollBar::kButtonScrollAmount = 10;

GUIScrollBar::GUIScrollBar(bool horizontal, bool resizable, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIInteractable(styleName, dimensions), mHorizontal(horizontal)
{
	GUISliderHandleFlags flags;
	if(resizable)
		flags |= GUISliderHandleFlag::Resizeable;

	if(mHorizontal)
	{
		mLayout = GUILayoutX::Create();
		RegisterChildElement(mLayout);

		mUpBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretLeft)), kScrollButtonStyleClass);
		mDownBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretRight)), kScrollButtonStyleClass);

		const char* handleStyleClass = resizable ? kResizableHorizontalHandleStyleClass : kHorizontalHandleStyleClass;
		mHandleBtn = GUISliderHandle::Create(flags | GUISliderHandleFlag::Horizontal, handleStyleClass);
	}
	else
	{
		mLayout = GUILayoutY::Create();
		RegisterChildElement(mLayout);

		mUpBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretUp)), kScrollButtonStyleClass);
		mDownBtn = GUIButton::Create(GUIContent(StockIcons::Instance().GetIcon(StockIcon::FontAwesomeCaretDown)), kScrollButtonStyleClass);

		const char* handleStyleClass = resizable ? kResizableVerticalHandleStyleClass : kVerticalHandleStyleClass;
		mHandleBtn = GUISliderHandle::Create(flags | GUISliderHandleFlag::Vertical, handleStyleClass);
	}

	GUIElementOptions scrollUpBtnOptions = mUpBtn->GetOptionFlags();
	scrollUpBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

	mUpBtn->SetOptionFlags(scrollUpBtnOptions);

	GUIElementOptions scrollDownBtnOptions = mDownBtn->GetOptionFlags();
	scrollDownBtnOptions.Unset(GUIElementOption::AcceptsKeyFocus);

	mDownBtn->SetOptionFlags(scrollDownBtnOptions);

	mLayout->AddNewElement<GUIFixedSpace>(2);
	mLayout->AddElement(mUpBtn);
	mLayout->AddElement(mHandleBtn);
	mLayout->AddElement(mDownBtn);
	mLayout->AddNewElement<GUIFixedSpace>(2);

	mHandleBtn->OnHandleMovedOrResized.Connect(std::bind(&::bs::GUIScrollBar::HandleMoved, this, _1, _2));

	mUpBtn->OnClick.Connect(std::bind(&::bs::GUIScrollBar::UpButtonClicked, this));
	mDownBtn->OnClick.Connect(std::bind(&::bs::GUIScrollBar::DownButtonClicked, this));
}

GUIScrollBar::~GUIScrollBar()
{
	mUpBtn->Destroy();
	mDownBtn->Destroy();
	mHandleBtn->Destroy();
}

void GUIScrollBar::UpdateRenderElements()
{
	mRenderElements.Clear();
	GUISpriteHelper::BuildSpriteRenderElements(*this, GUIElementState::Normal, mBackgroundSprite, Vector2I::kZero, 2); // Depth 2 because child buttons use depth 1

	GUIInteractable::UpdateRenderElements();
}

Vector2I GUIScrollBar::CalculateUnconstrainedOptimalSize() const
{
	return mLayout->CalculateUnconstrainedOptimalSize();
}

u32 GUIScrollBar::GetRenderElementDepthRange() const
{
	return 3;
}

void GUIScrollBar::UpdateLayoutForChildren()
{
	GUILayoutData childLayoutData = mLayoutData;
	childLayoutData.RelativePosition = GUILogicalPoint::kZero;

	mLayout->SetLayoutData(childLayoutData);
	mLayout->UpdateLayoutForChildren();
}

void GUIScrollBar::HandleMoved(float handlePct, float sizePct)
{
	if(!OnScrollOrResize.Empty())
		OnScrollOrResize(handlePct, sizePct);
}

void GUIScrollBar::UpButtonClicked()
{
	float handleOffset = 0.0f;
	GUIPhysicalUnitF scrollableSize = mHandleBtn->GetScrollableLength().To<float>();

	if(scrollableSize > 0.0f)
		handleOffset = (float)(kButtonScrollAmount.To<float>() / scrollableSize);

	Scroll(handleOffset);
}

void GUIScrollBar::DownButtonClicked()
{
	float handleOffset = 0.0f;
	GUIPhysicalUnitF scrollableSize = (float)mHandleBtn->GetScrollableLength();

	if(scrollableSize > 0.0f)
		handleOffset = (float)(kButtonScrollAmount.To<float>() / scrollableSize);

	Scroll(-handleOffset);
}

void GUIScrollBar::Scroll(float amount)
{
	float newHandlePos = Math::Clamp01(mHandleBtn->GetHandlePositionInPercent() - amount);

	float oldHandlePos = mHandleBtn->GetHandlePositionInPercent();
	mHandleBtn->SetHandlePositionInPercent(newHandlePos);

	if(oldHandlePos != mHandleBtn->GetHandlePositionInPercent())
	{
		mHandleBtn->MarkLayoutAsDirty();

		if(!OnScrollOrResize.Empty())
			OnScrollOrResize(newHandlePos, mHandleBtn->GetHandleSizeInPercent());
	}
}

void GUIScrollBar::SetHandleSizeInternal(float pct)
{
	mHandleBtn->SetHandleSizeInPercent(pct);
}

void GUIScrollBar::SetScrollPosInternal(float pct)
{
	mHandleBtn->SetHandlePositionInPercent(pct);
}

float GUIScrollBar::GetScrollHandlePosition() const
{
	return mHandleBtn->GetHandlePositionInPercent();
}

void GUIScrollBar::SetScrollHandlePosition(float pct)
{
	float oldHandlePos = mHandleBtn->GetHandlePositionInPercent();
	mHandleBtn->SetHandlePositionInPercent(pct);

	if(oldHandlePos != mHandleBtn->GetHandlePositionInPercent())
		mHandleBtn->MarkLayoutAsDirty();
}

float GUIScrollBar::GetScrollHandleSize() const
{
	return mHandleBtn->GetHandleSizeInPercent();
}

void GUIScrollBar::SetScrollHandleSize(float pct)
{
	mHandleBtn->SetHandleSizeInPercent(pct);
	mHandleBtn->MarkLayoutAsDirty();
}

GUIPhysicalUnit GUIScrollBar::GetScrollableSize() const
{
	return mHandleBtn->GetScrollableLength();
}

void GUIScrollBar::SetTint(const Color& color)
{
	mUpBtn->SetTint(color);
	mDownBtn->SetTint(color);
	mHandleBtn->SetTint(color);

	GUIInteractable::SetTint(color);
}
