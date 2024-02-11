//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIProgressBar.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUISizeConstraints.h"

using namespace std::placeholders;

using namespace bs;

GUIProgressBar::GUIProgressBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIElementContainer(sizeConstraints, styleName), mPercent(0)
{
	mBar = GUITexture::Create(kProgressBarFillStyleClass);
	mBackground = GUITexture::Create(kProgressBarBackgroundStyleClass);

	mBackground->SetElementDepth(mBar->GetRenderElementDepthRange());

	RegisterChildElement(mBar);
	RegisterChildElement(mBackground);
}

Vector2I GUIProgressBar::CalculateUnconstrainedOptimalSize() const
{
	Vector2I optimalSize = mBar->CalculateConstrainedSize().Optimal;

	Vector2I backgroundSize = mBackground->CalculateConstrainedSize().Optimal;
	optimalSize.X = std::max(optimalSize.X, backgroundSize.X);
	optimalSize.Y = std::max(optimalSize.Y, backgroundSize.Y);

	return optimalSize;
}

void GUIProgressBar::UpdateLayoutRecursive(const GUILayoutData& data)
{
	mBackground->SetLayoutData(data);

	const RectOffset& margins = mBackground->GetPadding();

	GUILayoutData barLayoutData = data;

	barLayoutData.Area.X += margins.Left;
	barLayoutData.Area.Y += margins.Top;

	u32 maxProgressBarWidth = std::max((u32)0, (u32)(data.Area.Width - margins.Left - margins.Right));
	u32 progressBarHeight = std::max((u32)0, (u32)(data.Area.Height - margins.Top - margins.Bottom));

	barLayoutData.Area.Width = (u32)Math::FloorToInt(maxProgressBarWidth * mPercent);
	barLayoutData.Area.Height = progressBarHeight;

	mBar->SetLayoutData(barLayoutData);
}

void GUIProgressBar::SetPercent(float percent)
{
	mPercent = percent;
	MarkLayoutAsDirty();
}

void GUIProgressBar::SetTint(const Color& color)
{
	mBar->SetTint(color);
	mBackground->SetTint(color);
}

const String& GUIProgressBar::GetGuiTypeName()
{
	static String typeName = "ProgressBar";
	return typeName;
}
