//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUITooltip.h"

#include "B3DGUIUtility.h"
#include "GUI/B3DGUIPanel.h"
#include "Components/B3DCamera.h"
#include "GpuBackend/B3DViewport.h"
#include "GUI/B3DGUILayoutY.h"
#include "GUI/B3DGUILayoutX.h"
#include "GUI/B3DGUITexture.h"
#include "GUI/B3DGUILabel.h"
#include "Resources/B3DBuiltinResources.h"
#include "GUI/B3DDropDownAreaPlacement.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

const GUILogicalUnit GUITooltip::kTooltipWidth = 200;
const GUILogicalUnit GUITooltip::kCursorSize = 16;

GUITooltip::GUITooltip(const HSceneObject& parent, const GUIWidget& overlaidWidget, const GUIPhysicalPoint& position, const String& text)
	: GUIWidget(parent, overlaidWidget.GetCamera()), mPosition(position), mText(text)
{
	SetStyleSheetCascade(overlaidWidget.GetStyleSheetCascadeAsShared());
}

void GUITooltip::OnCreated()
{
	GUIWidget::OnCreated();

	SetDepth(0); // Needs to be in front of everything

	TShared<Viewport> viewport = GetCamera()->GetViewport();

	const GUIPhysicalArea availableBounds = viewport->GetPixelArea().To<GUIPhysicalUnit>();

	const GUIStyleSheetCascade& styleSheetCascade = GetStyleSheetCascade();

	GUIStyleSheetRules multiLineLabelStyleSheetRules = styleSheetCascade.BuildRules(GUILabel::kStyleSheetElementType, BuiltinResources::kMultiLineLabelStyle);
	const GUIStyleSheetRules backgroundStyleSheetRules = styleSheetCascade.BuildRules(GUITexture::kStyleSheetElementType, kBackgroundStyleClass);

	GUISizeConstraints dimensions = GUISizeConstraints::Create(GUIOptions(GUIOption::FixedWidth(kTooltipWidth)));
	GUILogicalSize size = GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(mText, multiLineLabelStyleSheetRules, dimensions.MaximumWidth);

	GUILogicalUnit contentOffsetX = backgroundStyleSheetRules.Padding.Left;
	GUILogicalUnit contentOffsetY = backgroundStyleSheetRules.Padding.Top;

	// Content area
	GUIPanel* contentPanel = GetPanel()->AddNewElement<GUIPanel>();
	contentPanel->SetWidth(size.Width);
	contentPanel->SetHeight(size.Height);
	contentPanel->SetDepthRange(-1);

	// Background frame
	size.Width += backgroundStyleSheetRules.Padding.Left + backgroundStyleSheetRules.Padding.Right;
	size.Height += backgroundStyleSheetRules.Padding.Top + backgroundStyleSheetRules.Padding.Bottom;

	GUIPanel* backgroundPanel = GetPanel()->AddNewElement<GUIPanel>();
	backgroundPanel->SetWidth(size.Width);
	backgroundPanel->SetHeight(size.Height);
	backgroundPanel->SetDepthRange(0);

	GUILayout* backgroundLayout = backgroundPanel->AddNewElement<GUILayoutX>();

	GUITexture* backgroundFrame = GUITexture::Create(GUITextureContents(nullptr, TextureScaleMode::StretchToFit), kBackgroundStyleClass);
	backgroundLayout->AddElement(backgroundFrame);

	GUILayout* contentLayout = contentPanel->AddNewElement<GUILayoutY>();
	contentLayout->AddNewElement<GUILabel>(HString(mText), BuiltinResources::kMultiLineLabelStyle, GUIOptions(GUIOption::FixedWidth(kTooltipWidth), GUIOption::FlexibleHeight()));

	const GUILogicalPoint logicalPosition = GUIUtility::PhysicalToLogical(mPosition, GetDPIScale());
	const GUILogicalArea logicalAvailableBounds = GUIUtility::PhysicalToLogical(availableBounds, GetDPIScale());

	GUILogicalArea positionBounds;
	positionBounds.X = logicalPosition.X;
	positionBounds.Y = logicalPosition.Y;
	positionBounds.Width = kCursorSize;
	positionBounds.Height = kCursorSize;

	TDropDownAreaPlacement<GUILogicalUnit>::HorizontalDirection horizontalDirection;
	TDropDownAreaPlacement<GUILogicalUnit>::VerticalDirection verticalDirection;
	TDropDownAreaPlacement<GUILogicalUnit> placement = TDropDownAreaPlacement<GUILogicalUnit>::AroundBounds(positionBounds);
	GUILogicalArea placementBounds = placement.GetOptimalBounds(size, logicalAvailableBounds, horizontalDirection, verticalDirection);

	backgroundPanel->SetPosition(placementBounds.X, placementBounds.Y);
	contentPanel->SetPosition(placementBounds.X + contentOffsetX, placementBounds.Y + contentOffsetY);
}
