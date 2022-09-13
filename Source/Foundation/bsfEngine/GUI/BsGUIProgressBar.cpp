//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIProgressBar.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUIDimensions.h"

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

	const String& GUIProgressBar::GetBarStyleType()
	{
		static String HANDLE_STYLE_TYPE = "ProgressBarFill";
		return HANDLE_STYLE_TYPE;
	}

	const String& GUIProgressBar::GetBackgroundStyleType()
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

		barLayoutData.area.width = (UINT32)Math::FloorToInt(maxProgressBarWidth * mPercent);
		barLayoutData.area.height = progressBarHeight;

		mBar->_setLayoutData(barLayoutData);
	}

	void GUIProgressBar::StyleUpdated()
	{
		mBar->SetStyle(getSubStyleName(getBarStyleType()));
		mBackground->SetStyle(getSubStyleName(getBackgroundStyleType()));
	}

	void GUIProgressBar::SetPercent(float pct)
	{
		mPercent = pct;
		_markLayoutAsDirty();
	}

	void GUIProgressBar::SetTint(const Color& color)
	{
		mBar->SetTint(color);
		mBackground->SetTint(color);
	}

	GUIProgressBar* GUIProgressBar::create(const String& styleName)
	{
		return new (bs_alloc<GUIProgressBar>()) GUIProgressBar(getStyleName<GUIProgressBar>(styleName), GUIDimensions::create());
	}

	GUIProgressBar* GUIProgressBar::create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIProgressBar>()) GUIProgressBar(getStyleName<GUIProgressBar>(styleName), GUIDimensions::create(options));
	}

	const String& GUIProgressBar::GetGUITypeName()
	{
		static String typeName = "ProgressBar";
		return typeName;
	}
}
