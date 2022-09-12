//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUISlider.h"
#include "GUI/BsGUISliderHandle.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUICommandEvent.h"
#include "GUI/BsGUIElementStyle.h"

using namespace std::placeholders;

namespace bs
{
	GUISlider::GUISlider(bool horizontal, const String& styleName, const GUIDimensions& dimensions)
		:GUIElementContainer(dimensions, styleName, GUIElementOption::AcceptsKeyFocus), mHorizontal(horizontal)
	{
		GUISliderHandleFlags flags = horizontal ? GUISliderHandleFlag::Horizontal : GUISliderHandleFlag::Vertical;
		flags |= GUISliderHandleFlag::JumpOnClick;

		mSliderHandle = GUISliderHandle::create(flags, getSubStyleName(getHandleStyleType()));
		mBackground = GUITexture::create(getSubStyleName(getBackgroundStyleType()));
		mFillBackground = GUITexture::create(getSubStyleName(getFillStyleType()));

		mBackground->_setElementDepth(mSliderHandle->_getRenderElementDepthRange() + mFillBackground->_getRenderElementDepthRange());
		mFillBackground->_setElementDepth(mSliderHandle->_getRenderElementDepthRange());

		_registerChildElement(mSliderHandle);
		_registerChildElement(mBackground);
		_registerChildElement(mFillBackground);

		mHandleMovedConn = mSliderHandle->onHandleMovedOrResized.Connect(std::bind(&GUISlider::onHandleMoved, this, _1, _2));
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

	Vector2I GUISlider::_getOptimalSize() const
	{
		Vector2I optimalSize = mSliderHandle->_getOptimalSize();

		Vector2I backgroundSize = mBackground->_getOptimalSize();
		optimalSize.x = std::max(optimalSize.x, backgroundSize.x);
		optimalSize.y = std::max(optimalSize.y, backgroundSize.y);

		return optimalSize;
	}

	void GUISlider::_updateLayoutInternal(const GUILayoutData& data)
	{
		GUILayoutData childData = data;

		if (mHorizontal)
		{
			Vector2I optimalSize = mBackground->_getOptimalSize();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.Clip(data.clipRect);

			mBackground->_setLayoutData(childData);

			optimalSize = mSliderHandle->_getOptimalSize();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.Clip(data.clipRect);

			mSliderHandle->_setLayoutData(childData);
			UINT32 handleWidth = optimalSize.x;

			optimalSize = mFillBackground->_getOptimalSize();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);
			childData.area.width = mSliderHandle->GetHandlePosPx() + handleWidth / 2;

			childData.clipRect = data.area;
			childData.clipRect.Clip(data.clipRect);

			mFillBackground->_setLayoutData(childData);
		}
		else
		{
			Vector2I optimalSize = mBackground->_getOptimalSize();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.Clip(data.clipRect);

			mBackground->_setLayoutData(childData);

			optimalSize = mSliderHandle->_getOptimalSize();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.Clip(data.clipRect);

			mSliderHandle->_setLayoutData(childData);
			UINT32 handleHeight = optimalSize.y;

			optimalSize = mFillBackground->_getOptimalSize();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);
			childData.area.height = mSliderHandle->GetHandlePosPx() + handleHeight / 2;

			childData.clipRect = data.area;
			childData.clipRect.Clip(data.clipRect);

			mFillBackground->_setLayoutData(childData);
		}
	}

	void GUISlider::StyleUpdated()
	{
		mBackground->SetStyle(getSubStyleName(getBackgroundStyleType()));
		mFillBackground->SetStyle(getSubStyleName(getFillStyleType()));
		mSliderHandle->SetStyle(getSubStyleName(getHandleStyleType()));

		const GUIElementStyle* bgStyle = mBackground->_getStyle();
		if(mHasFocus)
			mBackground->SetTexture(bgStyle->focused.texture);
		else
			mBackground->SetTexture(bgStyle->normal.texture);
	}

	void GUISlider::SetPercent(float pct)
	{
		float oldHandlePos = mSliderHandle->GetHandlePos();
		mSliderHandle->_setHandlePos(pct);

		if (oldHandlePos != mSliderHandle->GetHandlePos())
			mSliderHandle->_markLayoutAsDirty();
	}

	float GUISlider::GetPercent() const
	{
		return mSliderHandle->GetHandlePos();
	}

	float GUISlider::GetValue() const
	{
		float diff = mMaxRange - mMinRange;
		return mMinRange + diff * mSliderHandle->GetHandlePos();
	}

	void GUISlider::SetValue(float value)
	{
		float diff = mMaxRange - mMinRange;
		float pct = (value - mMinRange) / diff;

		setPercent(pct);
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

		mSliderHandle->SetStep(step);
	}

	float GUISlider::GetStep() const
	{
		return mSliderHandle->GetStep();
	}

	void GUISlider::SetTint(const Color& color)
	{
		mBackground->SetTint(color);
		mSliderHandle->SetTint(color);
	}

	void GUISlider::OnHandleMoved(float newPosition, float newSize)
	{
		onChanged(getValue());
	}

	bool GUISlider::_commandEvent(const GUICommandEvent& ev)
	{
		const bool baseReturnValue = GUIElement::_commandEvent(ev);

		const GUIElementStyle* bgStyle = mBackground->_getStyle();
		if(ev.GetType() == GUICommandEventType::FocusGained)
		{
			mHasFocus = true;

			if(!_isDisabled())
				mBackground->SetTexture(bgStyle->focused.texture);

			return true;
		}
		else If(ev.GetType() == GUICommandEventType::FocusLost)
		{
			mHasFocus = false;
			mBackground->SetTexture(bgStyle->normal.texture);

			return true;
		}
		else If(ev.GetType() == GUICommandEventType::MoveLeft)
		{
			mSliderHandle->MoveOneStep(false);
			return true;
		}
		else If(ev.GetType() == GUICommandEventType::MoveRight)
		{
			mSliderHandle->MoveOneStep(true);
			return true;
		}

		return baseReturnValue;
	}

	GUISliderHorz::GUISliderHorz(const String& styleName, const GUIDimensions& dimensions)
		:GUISlider(true, styleName, dimensions)
	{

	}

	GUISliderHorz* GUISliderHorz::create(const String& styleName)
	{
		return new (bs_alloc<GUISliderHorz>()) GUISliderHorz(getStyleName<GUISliderHorz>(styleName), GUIDimensions::create());
	}

	GUISliderHorz* GUISliderHorz::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUISliderHorz>()) GUISliderHorz(getStyleName<GUISliderHorz>(styleName), GUIDimensions::create(options));
	}

	const String& GUISliderHorz::GetGUITypeName()
	{
		static String typeName = "SliderHorz";
		return typeName;
	}

	GUISliderVert::GUISliderVert(const String& styleName, const GUIDimensions& dimensions)
		:GUISlider(false, styleName, dimensions)
	{

	}

	GUISliderVert* GUISliderVert::create(const String& styleName)
	{
		return new (bs_alloc<GUISliderVert>()) GUISliderVert(getStyleName<GUISliderVert>(styleName), GUIDimensions::create());
	}

	GUISliderVert* GUISliderVert::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUISliderVert>()) GUISliderVert(getStyleName<GUISliderVert>(styleName), GUIDimensions::create(options));
	}

	const String& GUISliderVert::GetGUITypeName()
	{
		static String typeName = "SliderVert";
		return typeName;
	}
}
