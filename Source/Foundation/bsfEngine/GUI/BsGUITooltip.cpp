//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITooltip.h"

#include "BsGUIUtility.h"
#include "GUI/BsGUIPanel.h"
#include "Components/BsCamera.h"
#include "RenderAPI/BsViewport.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUILabel.h"
#include "Resources/BsBuiltinResources.h"
#include "GUI/BsDropDownAreaPlacement.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace std::placeholders;

using namespace b3d;

const GUILogicalUnit GUITooltip::kTooltipWidth = 200;
const GUILogicalUnit GUITooltip::kCursorSize = 16;

GUITooltip::GUITooltip(const HSceneObject& parent, const GUIWidget& overlaidWidget, const GUIPhysicalPoint& position, const String& text)
	: GUIWidget(parent, overlaidWidget.GetCamera())
{
	SetDepth(0); // Needs to be in front of everything
	SetStyleSheetCascade(overlaidWidget.GetStyleSheetCascadeAsShared());

	HCamera camera = overlaidWidget.GetCamera();
	SPtr<Viewport> viewport = camera->GetViewport();

	const GUIPhysicalArea availableBounds = viewport->GetPixelArea().To<GUIPhysicalUnit>();

	const GUIStyleSheetCascade& styleSheetCascade = GetStyleSheetCascade();

	GUIStyleSheetRules multiLineLabelStyleSheetRules = styleSheetCascade.BuildRules(GUILabel::kStyleSheetElementType, BuiltinResources::kMultiLineLabelStyle);
	const GUIStyleSheetRules backgroundStyleSheetRules = styleSheetCascade.BuildRules(GUITexture::kStyleSheetElementType, kBackgroundStyleClass);

	GUISizeConstraints dimensions = GUISizeConstraints::Create(GUIOptions(GUIOption::FixedWidth(kTooltipWidth)));
	GUILogicalSize size = GUIUtility::CalculateOptimalContentSizeWithPaddingAndBorder(text, multiLineLabelStyleSheetRules, dimensions.MaximumWidth);

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
	contentLayout->AddNewElement<GUILabel>(HString(text), BuiltinResources::kMultiLineLabelStyle, GUIOptions(GUIOption::FixedWidth(kTooltipWidth), GUIOption::FlexibleHeight()));

	const GUILogicalPoint logicalPosition = GUIUtility::PhysicalToLogical(position, GetDPIScale());
	const GUILogicalArea logicalAvailableBounds = GUIUtility::PhysicalToLogical(availableBounds, GetDPIScale());

	GUILogicalArea positionBounds;
	positionBounds.X = logicalPosition.X;
	positionBounds.Y = logicalPosition.Y;
	positionBounds.Width = kCursorSize;
	positionBounds.Height = kCursorSize;

	TDropDownAreaPlacement<GUILogicalUnit>::HorizontalDirection horzDir;
	TDropDownAreaPlacement<GUILogicalUnit>::VerticalDirection vertDir;
	TDropDownAreaPlacement<GUILogicalUnit> placement = TDropDownAreaPlacement<GUILogicalUnit>::AroundBounds(positionBounds);
	GUILogicalArea placementBounds = placement.GetOptimalBounds(size, logicalAvailableBounds, horzDir, vertDir);

	backgroundPanel->SetPosition(placementBounds.X, placementBounds.Y);
	contentPanel->SetPosition(placementBounds.X + contentOffsetX, placementBounds.Y + contentOffsetY);
}
