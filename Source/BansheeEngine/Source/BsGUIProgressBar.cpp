//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUIProgressBar.h"
#include "BsGUIWidget.h"
#include "BsGUISkin.h"
#include "BsGUITexture.h"
#include "BsGUIDimensions.h"

using namespace std::placeholders;

namespace bs
{
	GUIProgressBar::GUIProgressBar(const String& styleName, const GUIDimensions& dimensions)
		:GUIElementContainer(dimensions, styleName), mPercent(0)
	{
		mBar = GUITexture::create(getSubStyleName(getBarStyleType()));
		mBackground = GUITexture::create(getSubStyleName(getBackgroundStyleType()));

		mBackground->_setElementDepth(mBar->_getRenderElementDepthRange());

		_registerChildElement(mBar);
		_registerChildElement(mBackground);
	}

	const String& GUIProgressBar::getBarStyleType()
	{
		static String HANDLE_STYLE_TYPE = "ProgressBarFill";
		return HANDLE_STYLE_TYPE;
	}

	const String& GUIProgressBar::getBackgroundStyleType()
	{
		static String BACKGROUND_STYLE_TYPE = "ProgressBarBackground";
		return BACKGROUND_STYLE_TYPE;
	}

	Vector2I GUIProgressBar::_getOptimalSize() const
	{
		Vector2I optimalSize = mBar->_getOptimalSize();

		Vector2I backgroundSize = mBackground->_getOptimalSize();
		optimalSize.x = std::max(optimalSize.x, backgroundSize.x);
		optimalSize.y = std::max(optimalSize.y, backgroundSize.y);

		return optimalSize;
	}

	void GUIProgressBar::_updateLayoutInternal(const GUILayoutData& data)
	{
		mBackground->_setLayoutData(data);

		const GUIElementStyle* style = _getStyle();
		
		GUILayoutData barLayoutData = data;

		barLayoutData.area.x += style->margins.left;
		barLayoutData.area.y += style->margins.top;

		UINT32 maxProgressBarWidth = std::max((UINT32)0, (UINT32)(data.area.width - style->margins.left - style->margins.right));
		UINT32 progressBarHeight = std::max((UINT32)0, (UINT32)(data.area.height - style->margins.top - style->margins.bottom));

		barLayoutData.area.width = (UINT32)Math::floorToInt(maxProgressBarWidth * mPercent);
		barLayoutData.area.height = progressBarHeight;

		mBar->_setLayoutData(barLayoutData);
	}

	void GUIProgressBar::styleUpdated()
	{
		mBar->setStyle(getSubStyleName(getBarStyleType()));
		mBackground->setStyle(getSubStyleName(getBackgroundStyleType()));
	}

	void GUIProgressBar::setPercent(float pct)
	{
		mPercent = pct;
		_markLayoutAsDirty();
	}

	void GUIProgressBar::setTint(const Color& color)
	{
		mBar->setTint(color);
		mBackground->setTint(color);
	}

	GUIProgressBar* GUIProgressBar::create(const String& styleName)
	{
		return new (bs_alloc<GUIProgressBar>()) GUIProgressBar(getStyleName<GUIProgressBar>(styleName), GUIDimensions::create());
	}

	GUIProgressBar* GUIProgressBar::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIProgressBar>()) GUIProgressBar(getStyleName<GUIProgressBar>(styleName), GUIDimensions::create(options));
	}

	const String& GUIProgressBar::getGUITypeName()
	{
		static String typeName = "ProgressBar";
		return typeName;
	}
}