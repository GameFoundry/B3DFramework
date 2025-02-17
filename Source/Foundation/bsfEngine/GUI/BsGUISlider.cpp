//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISlider.h"
#include "GUI/BsGUISliderHandle.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUICommandEvent.h"

using namespace std::placeholders;

using namespace bs;

GUISlider::GUISlider(bool horizontal, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIElementContainer(dimensions, styleName, GUIElementOption::AcceptsKeyFocus), mHorizontal(horizontal)
{
	GUISliderHandleFlags flags = horizontal ? GUISliderHandleFlag::Horizontal : GUISliderHandleFlag::Vertical;
	flags |= GUISliderHandleFlag::JumpOnClick;

	mSliderHandle = GUISliderHandle::Create(flags, kHandleClassStyle);
	mBackground = GUITexture::Create(kBackgroundClassStyle);
	mFillBackground = GUITexture::Create(kFillClassStyle);

	mBackground->SetElementDepth(mSliderHandle->GetRenderElementDepthRange() + mFillBackground->GetRenderElementDepthRange());
	mFillBackground->SetElementDepth(mSliderHandle->GetRenderElementDepthRange());

	RegisterChildElement(mSliderHandle);
	RegisterChildElement(mBackground);
	RegisterChildElement(mFillBackground);

	mHandleMovedConn = mSliderHandle->OnHandleMovedOrResized.Connect(std::bind(&GUISlider::OnHandleMoved, this, _1, _2));
}

GUISlider::~GUISlider()
{
	mHandleMovedConn.Disconnect();
}

Vector2I GUISlider::CalculateUnconstrainedOptimalSize() const
{
	Vector2I optimalSize = mSliderHandle->CalculateConstrainedSize().Optimal;

	Vector2I backgroundSize = mBackground->CalculateConstrainedSize().Optimal;
	optimalSize.X = std::max(optimalSize.X, backgroundSize.X);
	optimalSize.Y = std::max(optimalSize.Y, backgroundSize.Y);

	return optimalSize;
}

void GUISlider::UpdateLayoutForChildren()
{
	GUILayoutData childData = mLayoutData;

	if(mHorizontal)
	{
		Vector2I optimalSize = mBackground->CalculateConstrainedSize().Optimal;
		childData.Size.Height = optimalSize.Y;
		childData.RelativePosition = GUILogicalPoint(0, (i32)((mLayoutData.Size.Height - childData.Size.Height) * 0.5f));

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateConstrainedSize().Optimal;
		childData.Size.Height = optimalSize.Y;
		childData.RelativePosition = GUILogicalPoint(0, (i32)((mLayoutData.Size.Height - childData.Size.Height) * 0.5f));

		mSliderHandle->SetLayoutData(childData);

		u32 handleWidth = optimalSize.X;

		optimalSize = mFillBackground->CalculateConstrainedSize().Optimal;
		childData.Size = Size2UI(mSliderHandle->GetHandlePositionInPixels() + handleWidth / 2, optimalSize.Y);
		childData.RelativePosition = GUILogicalPoint(0, (i32)((mLayoutData.Size.Height - childData.Size.Height) * 0.5f));

		mFillBackground->SetLayoutData(childData);
	}
	else
	{
		Vector2I optimalSize = mBackground->CalculateConstrainedSize().Optimal;
		childData.Size.Width = optimalSize.X;
		childData.RelativePosition = GUILogicalPoint((i32)((mLayoutData.Size.Width - childData.Size.Width) * 0.5f), 0);

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateConstrainedSize().Optimal;
		childData.Size.Width = optimalSize.X;
		childData.RelativePosition = GUILogicalPoint((i32)((mLayoutData.Size.Width - childData.Size.Width) * 0.5f), 0);

		mSliderHandle->SetLayoutData(childData);
		u32 handleHeight = optimalSize.Y;

		optimalSize = mFillBackground->CalculateConstrainedSize().Optimal;
		childData.Size = Size2UI(optimalSize.X, mSliderHandle->GetHandlePositionInPixels() + handleHeight / 2);
		childData.RelativePosition = GUILogicalPoint((i32)((mLayoutData.Size.Width - childData.Size.Width) * 0.5f), 0);

		mFillBackground->SetLayoutData(childData);
	}
}

void GUISlider::SetHandlePositionInPercent(float percent)
{
	float oldHandlePos = mSliderHandle->GetHandlePositionInPercent();
	mSliderHandle->SetHandlePositionInPercent(percent);

	if(oldHandlePos != mSliderHandle->GetHandlePositionInPercent())
		mSliderHandle->MarkLayoutAsDirty();
}

float GUISlider::GetHandlePositionInPercent() const
{
	return mSliderHandle->GetHandlePositionInPercent();
}

float GUISlider::GetHandlePositionInRange() const
{
	float diff = mMaxRange - mMinRange;
	return mMinRange + diff * mSliderHandle->GetHandlePositionInPercent();
}

void GUISlider::SetHandlePositionInRange(float value)
{
	float diff = mMaxRange - mMinRange;
	float pct = (value - mMinRange) / diff;

	SetHandlePositionInPercent(pct);
}

void GUISlider::SetRange(float min, float max)
{
	mMinRange = min;
	mMaxRange = max;
}

float GUISlider::GetRangeMaximum() const
{
	return mMaxRange;
}

float GUISlider::GetRangeMinimum() const
{
	return mMinRange;
}

void GUISlider::SetStep(float step)
{
	float range = mMaxRange - mMinRange;
	if(range > 0.0f)
		step = step / range;
	else
		step = 0.0f;

	mSliderHandle->SetMinimumStepIncrement(step);
}

float GUISlider::GetStep() const
{
	return mSliderHandle->GetMinimumStepIncrement();
}

void GUISlider::SetTint(const Color& color)
{
	mBackground->SetTint(color);
	mSliderHandle->SetTint(color);
}

void GUISlider::OnHandleMoved(float newPosition, float newSize)
{
	OnChanged(GetHandlePositionInRange());
}

bool GUISlider::DoOnCommandEvent(const GUICommandEvent& ev)
{
	const bool baseReturnValue = GUIInteractable::DoOnCommandEvent(ev);

	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		AddStateFlags(GUIElementStateFlag::Focus);
		mHasFocus = true;

		return true;
	}
	else if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		RemoveStateFlags(GUIElementStateFlag::Focus);
		mHasFocus = false;

		return true;
	}
	else if(ev.GetType() == GUICommandEventType::MoveLeft)
	{
		mSliderHandle->MoveOneStep(false);
		return true;
	}
	else if(ev.GetType() == GUICommandEventType::MoveRight)
	{
		mSliderHandle->MoveOneStep(true);
		return true;
	}

	return baseReturnValue;
}

GUIHorizontalSlider::GUIHorizontalSlider(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& dimensions)
	: GUISlider(true, styleName, dimensions)
{
}

const String& GUIHorizontalSlider::GetGuiTypeName()
{
	static String typeName = "HorizontalSlider";
	return typeName;
}

GUIVerticalSlider::GUIVerticalSlider(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& dimensions)
	: GUISlider(false, styleName, dimensions)
{
}

const String& GUIVerticalSlider::GetGuiTypeName()
{
	static String typeName = "VerticalSlider";
	return typeName;
}
