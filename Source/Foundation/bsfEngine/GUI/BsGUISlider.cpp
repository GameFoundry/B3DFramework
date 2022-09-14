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

		mBackground->SetElementDepthInternal(mSliderHandle->GetRenderElementDepthRangeInternal() + mFillBackground->GetRenderElementDepthRangeInternal());
		mFillBackground->SetElementDepthInternal(mSliderHandle->GetRenderElementDepthRangeInternal());

		RegisterChildElementInternal(mSliderHandle);
		RegisterChildElementInternal(mBackground);
		RegisterChildElementInternal(mFillBackground);

		mHandleMovedConn = mSliderHandle->onHandleMovedOrResized.connect(std::bind(&GUISlider::onHandleMoved, this, _1, _2));
	}

	GUISlider::~GUISlider()
	{
		mHandleMovedConn.disconnect();
	}

	const String& GUISlider::getHandleStyleType()
	{
		static String HANDLE_STYLE_TYPE = "SliderHandle";
		return HANDLE_STYLE_TYPE;
	}

	const String& GUISlider::getBackgroundStyleType()
	{
		static String BACKGROUND_STYLE_TYPE = "SliderBackground";
		return BACKGROUND_STYLE_TYPE;
	}

	const String& GUISlider::getFillStyleType()
	{
		static String FILL_STYLE_TYPE = "SliderFill";
		return FILL_STYLE_TYPE;
	}

	Vector2I GUISlider::GetOptimalSizeInternal() const
	{
		Vector2I optimalSize = mSliderHandle->GetOptimalSizeInternal();

		Vector2I backgroundSize = mBackground->GetOptimalSizeInternal();
		optimalSize.x = std::max(optimalSize.x, backgroundSize.x);
		optimalSize.y = std::max(optimalSize.y, backgroundSize.y);

		return optimalSize;
	}

	void GUISlider::UpdateLayoutInternalInternal(const GUILayoutData& data)
	{
		GUILayoutData childData = data;

		if (mHorizontal)
		{
			Vector2I optimalSize = mBackground->GetOptimalSizeInternal();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mBackground->SetLayoutDataInternal(childData);

			optimalSize = mSliderHandle->GetOptimalSizeInternal();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mSliderHandle->SetLayoutDataInternal(childData);
			UINT32 handleWidth = optimalSize.x;

			optimalSize = mFillBackground->GetOptimalSizeInternal();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);
			childData.area.width = mSliderHandle->getHandlePosPx() + handleWidth / 2;

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mFillBackground->SetLayoutDataInternal(childData);
		}
		else
		{
			Vector2I optimalSize = mBackground->GetOptimalSizeInternal();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mBackground->SetLayoutDataInternal(childData);

			optimalSize = mSliderHandle->GetOptimalSizeInternal();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mSliderHandle->SetLayoutDataInternal(childData);
			UINT32 handleHeight = optimalSize.y;

			optimalSize = mFillBackground->GetOptimalSizeInternal();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);
			childData.area.height = mSliderHandle->getHandlePosPx() + handleHeight / 2;

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mFillBackground->SetLayoutDataInternal(childData);
		}
	}

	void GUISlider::styleUpdated()
	{
		mBackground->setStyle(getSubStyleName(getBackgroundStyleType()));
		mFillBackground->setStyle(getSubStyleName(getFillStyleType()));
		mSliderHandle->setStyle(getSubStyleName(getHandleStyleType()));

		const GUIElementStyle* bgStyle = mBackground->GetStyleInternal();
		if(mHasFocus)
			mBackground->setTexture(bgStyle->focused.texture);
		else
			mBackground->setTexture(bgStyle->normal.texture);
	}

	void GUISlider::setPercent(float pct)
	{
		float oldHandlePos = mSliderHandle->getHandlePos();
		mSliderHandle->SetHandlePosInternal(pct);

		if (oldHandlePos != mSliderHandle->getHandlePos())
			mSliderHandle->MarkLayoutAsDirtyInternal();
	}

	float GUISlider::getPercent() const
	{
		return mSliderHandle->getHandlePos();
	}

	float GUISlider::getValue() const
	{
		float diff = mMaxRange - mMinRange;
		return mMinRange + diff * mSliderHandle->getHandlePos();
	}

	void GUISlider::setValue(float value)
	{
		float diff = mMaxRange - mMinRange;
		float pct = (value - mMinRange) / diff;

		setPercent(pct);
	}

	void GUISlider::setRange(float min, float max)
	{
		mMinRange = min;
		mMaxRange = max;
	}

	float GUISlider::getRangeMaximum() const
	{
		return mMaxRange;
	}

	float GUISlider::getRangeMinimum() const
	{
		return mMinRange;
	}

	void GUISlider::setStep(float step)
	{
		float range = mMaxRange - mMinRange;
		if(range > 0.0f)
			step = step / range;
		else
			step = 0.0f;

		mSliderHandle->setStep(step);
	}

	float GUISlider::getStep() const
	{
		return mSliderHandle->getStep();
	}

	void GUISlider::setTint(const Color& color)
	{
		mBackground->setTint(color);
		mSliderHandle->setTint(color);
	}

	void GUISlider::onHandleMoved(float newPosition, float newSize)
	{
		onChanged(getValue());
	}

	bool GUISlider::CommandEventInternal(const GUICommandEvent& ev)
	{
		const bool baseReturnValue = GUIElement::CommandEventInternal(ev);

		const GUIElementStyle* bgStyle = mBackground->GetStyleInternal();
		if(ev.getType() == GUICommandEventType::FocusGained)
		{
			mHasFocus = true;

			if(!IsDisabledInternal())
				mBackground->setTexture(bgStyle->focused.texture);

			return true;
		}
		else if(ev.getType() == GUICommandEventType::FocusLost)
		{
			mHasFocus = false;
			mBackground->setTexture(bgStyle->normal.texture);

			return true;
		}
		else if(ev.getType() == GUICommandEventType::MoveLeft)
		{
			mSliderHandle->moveOneStep(false);
			return true;
		}
		else if(ev.getType() == GUICommandEventType::MoveRight)
		{
			mSliderHandle->moveOneStep(true);
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

	const String& GUISliderHorz::getGUITypeName()
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

	const String& GUISliderVert::getGUITypeName()
	{
		static String typeName = "SliderVert";
		return typeName;
	}
}
