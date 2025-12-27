//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIProgressBar.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUITexture.h"
#include "GUI/B3DGUISizeConstraints.h"

using namespace b3d;

GUIProgressBar::GUIProgressBar(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints)
	: GUIElementContainer(sizeConstraints, styleName), mPercent(0)
{
	mBar = GUITexture::Create(kProgressBarFillStyleClass);
	mBackground = GUITexture::Create(kProgressBarBackgroundStyleClass);

	mBackground->SetElementDepth(mBar->GetRenderElementDepthRange());

	RegisterChildElement(mBar);
	RegisterChildElement(mBackground);
}

GUILogicalSize GUIProgressBar::CalculateUnconstrainedOptimalSize() const
{
	GUILogicalSize optimalSize = mBar->CalculateConstrainedOptimalSize();

	GUILogicalSize backgroundSize = mBackground->CalculateConstrainedOptimalSize();
	optimalSize.Width = Math::Max(optimalSize.Width, backgroundSize.Width);
	optimalSize.Height = Math::Max(optimalSize.Height, backgroundSize.Height);

	return optimalSize;
}

void GUIProgressBar::UpdateLayoutForChildren()
{
	GUILayoutData backgroundLayoutData = mLayoutData;
	backgroundLayoutData.RelativePosition = GUILogicalPoint::kZero;

	mBackground->SetLayoutData(backgroundLayoutData);

	const RectOffset& margins = mBackground->GetPadding();

	GUILayoutData barLayoutData = mLayoutData;
	barLayoutData.RelativePosition = GUILogicalPoint(margins.Left, margins.Top);

	GUILogicalUnit maxProgressBarWidth = Math::Max(mLayoutData.Size.Width - margins.Left - margins.Right, 0);
	GUILogicalUnit progressBarHeight = Math::Max(mLayoutData.Size.Height - margins.Top - margins.Bottom, 0);

	barLayoutData.Size.Width = GUILogicalUnit(Math::FloorToInt((float)maxProgressBarWidth * mPercent));
	barLayoutData.Size.Height = progressBarHeight;

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
