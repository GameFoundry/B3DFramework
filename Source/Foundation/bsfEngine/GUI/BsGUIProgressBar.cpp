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
		mBar = GUITexture::Create(GetSubStyleName(GetBarStyleType()));
		mBackground = GUITexture::Create(GetSubStyleName(GetBackgroundStyleType()));

		mBackground->SetElementDepthInternal(mBar->GetRenderElementDepthRangeInternal());

		RegisterChildElementInternal(mBar);
		RegisterChildElementInternal(mBackground);
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

	Vector2I GUIProgressBar::GetOptimalSizeInternal() const
	{
		Vector2I optimalSize = mBar->GetOptimalSizeInternal();

		Vector2I backgroundSize = mBackground->GetOptimalSizeInternal();
		optimalSize.x = std::max(optimalSize.x, backgroundSize.x);
		optimalSize.y = std::max(optimalSize.y, backgroundSize.y);

		return optimalSize;
	}

	void GUIProgressBar::UpdateLayoutInternalInternal(const GUILayoutData& data)
	{
		mBackground->SetLayoutDataInternal(data);

		const GUIElementStyle* style = GetStyleInternal();
		
		GUILayoutData barLayoutData = data;

		barLayoutData.area.x += style->margins.left;
		barLayoutData.area.y += style->margins.top;

		UINT32 maxProgressBarWidth = std::max((UINT32)0, (UINT32)(data.area.width - style->margins.left - style->margins.right));
		UINT32 progressBarHeight = std::max((UINT32)0, (UINT32)(data.area.height - style->margins.top - style->margins.bottom));

		barLayoutData.area.width = (UINT32)Math::FloorToInt(maxProgressBarWidth * mPercent);
		barLayoutData.area.height = progressBarHeight;

		mBar->SetLayoutDataInternal(barLayoutData);
	}

	void GUIProgressBar::StyleUpdated()
	{
		mBar->SetStyle(GetSubStyleName(GetBarStyleType()));
		mBackground->SetStyle(GetSubStyleName(GetBackgroundStyleType()));
	}

	void GUIProgressBar::SetPercent(float pct)
	{
		mPercent = pct;
		MarkLayoutAsDirtyInternal();
	}

	void GUIProgressBar::SetTint(const Color& color)
	{
		mBar->SetTint(color);
		mBackground->SetTint(color);
	}

	GUIProgressBar* GUIProgressBar::Create(const String& styleName)
	{
		return new (bs_alloc<GUIProgressBar>()) GUIProgressBar(GetStyleName<GUIProgressBar>(styleName), GUIDimensions::Create());
	}

	GUIProgressBar* GUIProgressBar::Create(const GUIOptions& options, const String& styleName)
	{
		return new (bs_alloc<GUIProgressBar>()) GUIProgressBar(GetStyleName<GUIProgressBar>(styleName), GUIDimensions::Create(options));
	}

	const String& GUIProgressBar::GetGuiTypeName()
	{
		static String typeName = "ProgressBar";
		return typeName;
	}
}
