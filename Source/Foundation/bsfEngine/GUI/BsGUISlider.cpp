//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISlider.h"
#include "GUI/BsGUISliderHandle.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIElementStyle.h"

using namespace std::placeholders;

using namespace bs;

GUISlider::GUISlider(bool horizontal, const String& styleName, const GUISizeConstraints& dimensions)
	: GUIElementContainer(dimensions, styleName, GUIElementOption::AcceptsKeyFocus), mHorizontal(horizontal)
{
	GUISliderHandleFlags flags = horizontal ? GUISliderHandleFlag::Horizontal : GUISliderHandleFlag::Vertical;
	flags |= GUISliderHandleFlag::JumpOnClick;

	mSliderHandle = GUISliderHandle::Create(flags, GetSubStyleName(GetHandleStyleType()));
	mBackground = GUITexture::Create(GetSubStyleName(GetBackgroundStyleType()));
	mFillBackground = GUITexture::Create(GetSubStyleName(GetFillStyleType()));

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

const String& GUISlider::GetHandleStyleType()
{
	static String HANDLE_STYLE_TYPE = "SliderHandle";
	return HANDLE_STYLE_TYPE;
}

const String& GUISlider::GetBackgroundStyleType()
{
	static String BACKGROUND_STYLE_TYPE = "SliderBackground";
	return BACKGROUND_STYLE_TYPE;
}

const String& GUISlider::GetFillStyleType()
{
	static String FILL_STYLE_TYPE = "SliderFill";
	return FILL_STYLE_TYPE;
}

Vector2I GUISlider::CalculateUnconstrainedOptimalSize() const
{
	Vector2I optimalSize = mSliderHandle->CalculateUnconstrainedOptimalSize();

	Vector2I backgroundSize = mBackground->CalculateUnconstrainedOptimalSize();
	optimalSize.X = std::max(optimalSize.X, backgroundSize.X);
	optimalSize.Y = std::max(optimalSize.Y, backgroundSize.Y);

	return optimalSize;
}

void GUISlider::UpdateLayoutRecursive(const GUILayoutData& data)
{
	GUILayoutData childData = data;

	if(mHorizontal)
	{
		Vector2I optimalSize = mBackground->CalculateUnconstrainedOptimalSize();
		childData.Area.Height = optimalSize.Y;
		childData.Area.Y = data.Area.Y + (i32)((data.Area.Height - childData.Area.Height) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateUnconstrainedOptimalSize();
		childData.Area.Height = optimalSize.Y;
		childData.Area.Y = data.Area.Y + (i32)((data.Area.Height - childData.Area.Height) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mSliderHandle->SetLayoutData(childData);
		u32 handleWidth = optimalSize.X;

		optimalSize = mFillBackground->CalculateUnconstrainedOptimalSize();
		childData.Area.Height = optimalSize.Y;
		childData.Area.Y = data.Area.Y + (i32)((data.Area.Height - childData.Area.Height) * 0.5f);
		childData.Area.Width = mSliderHandle->GetHandlePositionInPixels() + handleWidth / 2;

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mFillBackground->SetLayoutData(childData);
	}
	else
	{
		Vector2I optimalSize = mBackground->CalculateUnconstrainedOptimalSize();
		childData.Area.Width = optimalSize.X;
		childData.Area.X = data.Area.X + (i32)((data.Area.Width - childData.Area.Width) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mBackground->SetLayoutData(childData);

		optimalSize = mSliderHandle->CalculateUnconstrainedOptimalSize();
		childData.Area.Width = optimalSize.X;
		childData.Area.X = data.Area.X + (i32)((data.Area.Width - childData.Area.Width) * 0.5f);

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mSliderHandle->SetLayoutData(childData);
		u32 handleHeight = optimalSize.Y;

		optimalSize = mFillBackground->CalculateUnconstrainedOptimalSize();
		childData.Area.Width = optimalSize.X;
		childData.Area.X = data.Area.X + (i32)((data.Area.Width - childData.Area.Width) * 0.5f);
		childData.Area.Height = mSliderHandle->GetHandlePositionInPixels() + handleHeight / 2;

		childData.ClipRect = data.Area;
		childData.ClipRect.Clip(data.ClipRect);

		mFillBackground->SetLayoutData(childData);
	}
}

void GUISlider::NotifyStyleChanged()
{
	mBackground->SetStyleSheetClass(GetSubStyleName(GetBackgroundStyleType()));
	mFillBackground->SetStyleSheetClass(GetSubStyleName(GetFillStyleType()));
	mSliderHandle->SetStyleSheetClass(GetSubStyleName(GetHandleStyleType()));

	const GUIElementStyle* bgStyle = mBackground->GetStyle();
	if(mHasFocus)
		mBackground->SetImage(bgStyle->Focused.Image);
	else
		mBackground->SetImage(bgStyle->Normal.Image);
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
	const bool baseReturnValue = GUIElement::DoOnCommandEvent(ev);

	const GUIElementStyle* bgStyle = mBackground->GetStyle();
	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		mHasFocus = true;

		if(!IsDisabled())
			mBackground->SetImage(bgStyle->Focused.Image);

		return true;
	}
	else if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		mHasFocus = false;
		mBackground->SetImage(bgStyle->Normal.Image);

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

GUISliderHorz::GUISliderHorz(const String& styleName, const GUISizeConstraints& dimensions)
	: GUISlider(true, styleName, dimensions)
{
}

GUISliderHorz* GUISliderHorz::Create(const String& styleName)
{
	return new(B3DAllocate<GUISliderHorz>()) GUISliderHorz(GetStyleName<GUISliderHorz>(styleName), GUISizeConstraints::Create());
}

GUISliderHorz* GUISliderHorz::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUISliderHorz>()) GUISliderHorz(GetStyleName<GUISliderHorz>(styleName), GUISizeConstraints::Create(options));
}

const String& GUISliderHorz::GetGuiTypeName()
{
	static String typeName = "SliderHorz";
	return typeName;
}

GUISliderVert::GUISliderVert(const String& styleName, const GUISizeConstraints& dimensions)
	: GUISlider(false, styleName, dimensions)
{
}

GUISliderVert* GUISliderVert::Create(const String& styleName)
{
	return new(B3DAllocate<GUISliderVert>()) GUISliderVert(GetStyleName<GUISliderVert>(styleName), GUISizeConstraints::Create());
}

GUISliderVert* GUISliderVert::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUISliderVert>()) GUISliderVert(GetStyleName<GUISliderVert>(styleName), GUISizeConstraints::Create(options));
}

const String& GUISliderVert::GetGuiTypeName()
{
	static String typeName = "SliderVert";
	return typeName;
}
