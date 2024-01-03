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

void GUISlider::UpdateLayoutRecursive(const GUILayoutData& data)
{
	GUILayoutData childData = data;

	if(mHorizontal)
	{
		Vector2I optimalSize = mBackground->CalculateConstrainedSize().Optimal;
		childData.Area.Height = optimalSize.Y;
		childData.Area.Y = data.Area.Y + (i32)((data.Area.Height - childData.Area.Height) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateConstrainedSize().Optimal;
		childData.Area.Height = optimalSize.Y;
		childData.Area.Y = data.Area.Y + (i32)((data.Area.Height - childData.Area.Height) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mSliderHandle->SetLayoutData(childData);
		u32 handleWidth = optimalSize.X;

		optimalSize = mFillBackground->CalculateConstrainedSize().Optimal;
		childData.Area.Height = optimalSize.Y;
		childData.Area.Y = data.Area.Y + (i32)((data.Area.Height - childData.Area.Height) * 0.5f);
		childData.Area.Width = mSliderHandle->GetHandlePositionInPixels() + handleWidth / 2;

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mFillBackground->SetLayoutData(childData);
	}
	else
	{
		Vector2I optimalSize = mBackground->CalculateConstrainedSize().Optimal;
		childData.Area.Width = optimalSize.X;
		childData.Area.X = data.Area.X + (i32)((data.Area.Width - childData.Area.Width) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateConstrainedSize().Optimal;
		childData.Area.Width = optimalSize.X;
		childData.Area.X = data.Area.X + (i32)((data.Area.Width - childData.Area.Width) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mSliderHandle->SetLayoutData(childData);
		u32 handleHeight = optimalSize.Y;

		optimalSize = mFillBackground->CalculateConstrainedSize().Optimal;
		childData.Area.Width = optimalSize.X;
		childData.Area.X = data.Area.X + (i32)((data.Area.Width - childData.Area.Width) * 0.5f);
		childData.Area.Height = mSliderHandle->GetHandlePositionInPixels() + handleHeight / 2;

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mFillBackground->SetLayoutData(childData);
	}
}

void GUISlider::SetPercent(float pct)
{
	float oldHandlePos = mSliderHandle->GetHandlePositionInPercent();
	mSliderHandle->SetHandlePositionInPercent(pct);

	if(oldHandlePos != mSliderHandle->GetHandlePositionInPercent())
		mSliderHandle->MarkLayoutAsDirty();
}

float GUISlider::GetPercent() const
{
	return mSliderHandle->GetHandlePositionInPercent();
}

float GUISlider::GetValue() const
{
	float diff = mMaxRange - mMinRange;
	return mMinRange + diff * mSliderHandle->GetHandlePositionInPercent();
}

void GUISlider::SetValue(float value)
{
	float diff = mMaxRange - mMinRange;
	float pct = (value - mMinRange) / diff;

	SetPercent(pct);
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
	OnChanged(GetValue());
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

GUIHorizontalSlider::GUIHorizontalSlider(const String& styleName, const GUISizeConstraints& dimensions)
	: GUISlider(true, styleName, dimensions)
{
}

GUIHorizontalSlider* GUIHorizontalSlider::Create(const String& styleName)
{
	return new(B3DAllocate<GUIHorizontalSlider>()) GUIHorizontalSlider(GetStyleClass<GUIHorizontalSlider>(styleName), GUISizeConstraints::Create());
}

GUIHorizontalSlider* GUIHorizontalSlider::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIHorizontalSlider>()) GUIHorizontalSlider(GetStyleClass<GUIHorizontalSlider>(styleName), GUISizeConstraints::Create(options));
}

const String& GUIHorizontalSlider::GetGuiTypeName()
{
	static String typeName = "HorizontalSlider";
	return typeName;
}

GUIVerticalSlider::GUIVerticalSlider(const String& styleName, const GUISizeConstraints& dimensions)
	: GUISlider(false, styleName, dimensions)
{
}

GUIVerticalSlider* GUIVerticalSlider::Create(const String& styleName)
{
	return new(B3DAllocate<GUIVerticalSlider>()) GUIVerticalSlider(GetStyleClass<GUIVerticalSlider>(styleName), GUISizeConstraints::Create());
}

GUIVerticalSlider* GUIVerticalSlider::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIVerticalSlider>()) GUIVerticalSlider(GetStyleClass<GUIVerticalSlider>(styleName), GUISizeConstraints::Create(options));
}

const String& GUIVerticalSlider::GetGuiTypeName()
{
	static String typeName = "VerticalSlider";
	return typeName;
}
