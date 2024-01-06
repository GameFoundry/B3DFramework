//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUITooltip.h"
#include "GUI/BsGUIPanel.h"
#include "Renderer/BsCamera.h"
#include "RenderAPI/BsViewport.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUILayoutX.h"
#include "GUI/BsGUITexture.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIHelper.h"
#include "GUI/BsGUISkin.h"
#include "Resources/BsBuiltinResources.h"
#include "GUI/BsDropDownAreaPlacement.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace std::placeholders;

using namespace bs;

const u32 GUITooltip::kTooltipWidth = 200;
const u32 GUITooltip::kCursorSize = 16;

GUITooltip::GUITooltip(const HSceneObject& parent, const GUIWidget& overlaidWidget, const Vector2I& position, const String& text)
	: CGUIWidget(parent, overlaidWidget.GetCamera())
{
	SetDepth(0); // Needs to be in front of everything
	SetSkin(overlaidWidget.GetSkinResource());

	SPtr<Camera> camera = overlaidWidget.GetCamera();
	SPtr<Viewport> viewport = camera->GetViewport();

	Rect2I availableBounds = viewport->GetPixelArea();

	const GUISkin& skin = GetSkin();
	const GUIElementStyle* multiLineLabelStyle = skin.GetStyle(BuiltinResources::kMultiLineLabelStyle);

	const SPtr<const GUIStyleSheetRuleset> backgroundStyleSheetRuleset = GetStyleSheet()->BuildRuleset(GUITexture::kElementType, kBackgroundStyleClass);

	Vector2I size(kTooltipWidth, 25);
	if(multiLineLabelStyle != nullptr)
	{
		GUISizeConstraints dimensions = GUISizeConstraints::Create(GUIOptions(GUIOption::FixedWidth(kTooltipWidth)));
		size = GUIHelper::CalculateOptimalContentSize(text, *multiLineLabelStyle, dimensions);
	}

	i32 contentOffsetX = 0;
	i32 contentOffsetY = 0;
	if(backgroundStyleSheetRuleset != nullptr)
	{
		const GUIStyleSheetRules& backgroundStyleSheetRules = backgroundStyleSheetRuleset->Rules;

		size.X += backgroundStyleSheetRules.Padding.Left + backgroundStyleSheetRules.Padding.Right;
		size.Y += backgroundStyleSheetRules.Padding.Top + backgroundStyleSheetRules.Padding.Bottom;

		contentOffsetX = backgroundStyleSheetRules.Padding.Left;
		contentOffsetY = backgroundStyleSheetRules.Padding.Top;
	}

	// Content area
	GUIPanel* contentPanel = GetPanel()->AddNewElement<GUIPanel>();
	contentPanel->SetWidth((u32)size.X);
	contentPanel->SetHeight((u32)size.Y);
	contentPanel->SetDepthRange(-1);

	// Background frame
	GUIPanel* backgroundPanel = GetPanel()->AddNewElement<GUIPanel>();
	backgroundPanel->SetWidth((u32)size.X);
	backgroundPanel->SetHeight((u32)size.Y);
	backgroundPanel->SetDepthRange(0);

	GUILayout* backgroundLayout = backgroundPanel->AddNewElement<GUILayoutX>();

	GUITexture* backgroundFrame = GUITexture::Create(GUITextureContents(nullptr, TextureScaleMode::StretchToFit), kBackgroundStyleClass);
	backgroundLayout->AddElement(backgroundFrame);

	GUILayout* contentLayout = contentPanel->AddNewElement<GUILayoutY>();
	contentLayout->AddNewElement<GUILabel>(HString(text), BuiltinResources::kMultiLineLabelStyle, GUIOptions(GUIOption::FixedWidth(kTooltipWidth), GUIOption::FlexibleHeight()));

	Rect2I positionBounds;
	positionBounds.X = position.X;
	positionBounds.Y = position.Y;
	positionBounds.Width = kCursorSize;
	positionBounds.Height = kCursorSize;

	DropDownAreaPlacement::HorzDir horzDir;
	DropDownAreaPlacement::VertDir vertDir;
	DropDownAreaPlacement placement = DropDownAreaPlacement::AroundBounds(positionBounds);
	Rect2I placementBounds = placement.GetOptimalBounds((u32)size.X, (u32)size.Y, availableBounds, horzDir, vertDir);

	backgroundPanel->SetPosition(placementBounds.X, placementBounds.Y);
	contentPanel->SetPosition(placementBounds.X + contentOffsetX, placementBounds.Y + contentOffsetY);
}
