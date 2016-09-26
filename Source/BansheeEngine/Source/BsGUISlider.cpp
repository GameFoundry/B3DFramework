//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUISlider.h"
#include "BsGUISliderHandle.h"
#include "BsGUITexture.h"
#include "BsGUIDimensions.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	GUISlider::GUISlider(bool horizontal, const String& styleName, const GUIDimensions& dimensions)
		:GUIElementContainer(dimensions, styleName), mHorizontal(horizontal), mMinRange(0.0f), mMaxRange(1.0f)
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
			childData.clipRect.clip(data.clipRect);

			mBackground->_setLayoutData(childData);

			optimalSize = mSliderHandle->_getOptimalSize();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mSliderHandle->_setLayoutData(childData);
			UINT32 handleWidth = optimalSize.x;

			optimalSize = mFillBackground->_getOptimalSize();
			childData.area.height = optimalSize.y;
			childData.area.y = data.area.y + (INT32)((data.area.height - childData.area.height) * 0.5f);
			childData.area.width = mSliderHandle->getHandlePosPx() + handleWidth / 2;

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mFillBackground->_setLayoutData(childData);
		}
		else
		{
			Vector2I optimalSize = mBackground->_getOptimalSize();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mBackground->_setLayoutData(childData);

			optimalSize = mSliderHandle->_getOptimalSize();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mSliderHandle->_setLayoutData(childData);
			UINT32 handleHeight = optimalSize.y;

			optimalSize = mFillBackground->_getOptimalSize();
			childData.area.width = optimalSize.x;
			childData.area.x = data.area.x + (INT32)((data.area.width - childData.area.width) * 0.5f);
			childData.area.height = mSliderHandle->getHandlePosPx() + handleHeight / 2;

			childData.clipRect = data.area;
			childData.clipRect.clip(data.clipRect);

			mFillBackground->_setLayoutData(childData);
		}
	}

	void GUISlider::styleUpdated()
	{
		mBackground->setStyle(getSubStyleName(getBackgroundStyleType()));
		mFillBackground->setStyle(getSubStyleName(getFillStyleType()));
		mSliderHandle->setStyle(getSubStyleName(getHandleStyleType()));
	}

	void GUISlider::setPercent(float pct)
	{
		float oldHandlePos = mSliderHandle->getHandlePos();
		mSliderHandle->_setHandlePos(pct);

		if (oldHandlePos != mSliderHandle->getHandlePos())
			mSliderHandle->_markLayoutAsDirty();
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