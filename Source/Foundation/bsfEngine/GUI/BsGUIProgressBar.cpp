//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIProgressBar.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUISizeConstraints.h"

using namespace std::placeholders;

using namespace bs;

GUIProgressBar::GUIProgressBar(const String& styleName, const GUISizeConstraints& dimensions)
	: GUIElementContainer(dimensions, styleName), mPercent(0)
{
	mBar = GUITexture::Create(GetSubStyleName(GetBarStyleType()));
	mBackground = GUITexture::Create(GetSubStyleName(GetBackgroundStyleType()));

	mBackground->SetElementDepth(mBar->GetRenderElementDepthRange());

	RegisterChildElement(mBar);
	RegisterChildElement(mBackground);
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

Vector2I GUIProgressBar::CalculateUnconstrainedOptimalSize() const
{
	Vector2I optimalSize = mBar->CalculateUnconstrainedOptimalSize();

	Vector2I backgroundSize = mBackground->CalculateUnconstrainedOptimalSize();
	optimalSize.X = std::max(optimalSize.X, backgroundSize.X);
	optimalSize.Y = std::max(optimalSize.Y, backgroundSize.Y);

	return optimalSize;
}

void GUIProgressBar::UpdateLayoutRecursive(const GUILayoutData& data)
{
	mBackground->SetLayoutData(data);

	const RectOffset& margins = GetPadding();

	GUILayoutData barLayoutData = data;

	barLayoutData.Area.X += margins.Left;
	barLayoutData.Area.Y += margins.Top;

	u32 maxProgressBarWidth = std::max((u32)0, (u32)(data.Area.Width - margins.Left - margins.Right));
	u32 progressBarHeight = std::max((u32)0, (u32)(data.Area.Height - margins.Top - margins.Bottom));

	barLayoutData.Area.Width = (u32)Math::FloorToInt(maxProgressBarWidth * mPercent);
	barLayoutData.Area.Height = progressBarHeight;

	mBar->SetLayoutData(barLayoutData);
}

void GUIProgressBar::NotifyStyleChanged()
{
	mBar->SetStyleSheetClass(GetSubStyleName(GetBarStyleType()));
	mBackground->SetStyleSheetClass(GetSubStyleName(GetBackgroundStyleType()));
}

void GUIProgressBar::SetPercent(float pct)
{
	mPercent = pct;
	MarkLayoutAsDirty();
}

void GUIProgressBar::SetTint(const Color& color)
{
	mBar->SetTint(color);
	mBackground->SetTint(color);
}

GUIProgressBar* GUIProgressBar::Create(const String& styleName)
{
	return new(B3DAllocate<GUIProgressBar>()) GUIProgressBar(GetStyleName<GUIProgressBar>(styleName), GUISizeConstraints::Create());
}

GUIProgressBar* GUIProgressBar::Create(const GUIOptions& options, const String& styleName)
{
	return new(B3DAllocate<GUIProgressBar>()) GUIProgressBar(GetStyleName<GUIProgressBar>(styleName), GUISizeConstraints::Create(options));
}

const String& GUIProgressBar::GetGuiTypeName()
{
	static String typeName = "ProgressBar";
	return typeName;
}
