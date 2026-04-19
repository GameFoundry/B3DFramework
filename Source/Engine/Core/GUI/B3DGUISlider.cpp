//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUISlider.h"

#include "B3DGUIUtility.h"
#include "GUI/B3DGUISliderHandle.h"
#include "GUI/B3DGUITexture.h"
#include "GUI/B3DGUISizeConstraints.h"
#include "GUI/B3DGUICommandEvent.h"

using namespace b3d;

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

	mHandleMovedConn = mSliderHandle->OnHandleMovedOrResized.Connect([this](float newPosition, float newSize)
	{
		OnHandleMoved(newPosition, newSize);
	});
}

GUISlider::~GUISlider()
{
	mHandleMovedConn.Disconnect();
}

GUILogicalSize GUISlider::CalculateUnconstrainedOptimalSize() const
{
	GUILogicalSize optimalSize = mSliderHandle->CalculateConstrainedOptimalSize();

	GUILogicalSize backgroundSize = mBackground->CalculateConstrainedOptimalSize();
	optimalSize.Width = std::max(optimalSize.Width, backgroundSize.Width);
	optimalSize.Height = std::max(optimalSize.Height, backgroundSize.Height);

	return optimalSize;
}

void GUISlider::UpdateLayoutForChildren()
{
	GUILayoutData childData = mLayoutData;

	const GUIPhysicalUnit physicalHandlePosition = mSliderHandle->GetHandlePositionInPixels();
	const GUILogicalUnit handlePosition = GUIUtility::PhysicalToLogical(physicalHandlePosition, GetAbsoluteScale());

	if(mHorizontal)
	{
		GUILogicalSize optimalSize = mBackground->CalculateConstrainedOptimalSize();
		childData.Size.Height = optimalSize.Height;
		childData.RelativePosition = GUILogicalPoint(0, (i32)((float)(mLayoutData.Size.Height - childData.Size.Height) * 0.5f));

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateConstrainedOptimalSize();
		childData.Size.Height = optimalSize.Height;
		childData.RelativePosition = GUILogicalPoint(0, (i32)((float)(mLayoutData.Size.Height - childData.Size.Height) * 0.5f));

		mSliderHandle->SetLayoutData(childData);

		GUILogicalUnit handleWidth = optimalSize.Width;

		optimalSize = mFillBackground->CalculateConstrainedOptimalSize();
		childData.Size = GUILogicalSize(handlePosition + handleWidth / 2, optimalSize.Height);
		childData.RelativePosition = GUILogicalPoint(0, (i32)((float)(mLayoutData.Size.Height - childData.Size.Height) * 0.5f));

		mFillBackground->SetLayoutData(childData);
	}
	else
	{
		GUILogicalSize optimalSize = mBackground->CalculateConstrainedOptimalSize();
		childData.Size.Width = optimalSize.Width;
		childData.RelativePosition = GUILogicalPoint((i32)((float)(mLayoutData.Size.Width - childData.Size.Width) * 0.5f), 0);

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateConstrainedOptimalSize();
		childData.Size.Width = optimalSize.Width;
		childData.RelativePosition = GUILogicalPoint((i32)((float)(mLayoutData.Size.Width - childData.Size.Width) * 0.5f), 0);

		mSliderHandle->SetLayoutData(childData);
		GUILogicalUnit handleHeight = optimalSize.Height;

		optimalSize = mFillBackground->CalculateConstrainedOptimalSize();
		childData.Size = GUILogicalSize(optimalSize.Width, handlePosition + handleHeight / 2);
		childData.RelativePosition = GUILogicalPoint((i32)((float)(mLayoutData.Size.Width - childData.Size.Width) * 0.5f), 0);

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
	float percent = (value - mMinRange) / diff;

	SetHandlePositionInPercent(percent);
}

void GUISlider::SetRange(float minimum, float maximum)
{
	mMinRange = minimum;
	mMaxRange = maximum;
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
