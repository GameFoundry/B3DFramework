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

using namespace std::placeholders;

namespace bs
{
	const u32 GUITooltip::TOOLTIP_WIDTH = 200;
	const u32 GUITooltip::CURSOR_SIZE = 16;

	String GUITooltip::GetFrameStyleName()
	{
		return "TooltipFrame";
	}

	GUITooltip::GUITooltip(const HSceneObject& parent, const GUIWidget& overlaidWidget, const Vector2I& position, const String& text)
		: CGUIWidget(parent, overlaidWidget.GetCamera())
	{
		SetDepth(0); // Needs to be in front of everything
		SetSkin(overlaidWidget.GetSkinResource());

		SPtr<Camera> camera = overlaidWidget.GetCamera();
		SPtr<Viewport> viewport = camera->GetViewport();

		Rect2I availableBounds = viewport->GetPixelArea();

		const GUISkin& skin = GetSkin();
		const GUIElementStyle* multiLineLabelStyle = skin.GetStyle(BuiltinResources::MultiLineLabelStyle);
		const GUIElementStyle* backgroundStyle = skin.GetStyle(GetFrameStyleName());

		Vector2I size(TOOLTIP_WIDTH, 25);
		if(multiLineLabelStyle != nullptr)
		{
			GUIDimensions dimensions = GUIDimensions::Create(GUIOptions(GUIOption::FixedWidth(TOOLTIP_WIDTH)));
			size = GUIHelper::CalcOptimalContentsSize(text, *multiLineLabelStyle, dimensions);
		}

		i32 contentOffsetX = 0;
		i32 contentOffsetY = 0;
		if(backgroundStyle != nullptr)
		{
			size.X += backgroundStyle->Margins.Left + backgroundStyle->Margins.Right;
			size.Y += backgroundStyle->Margins.Top + backgroundStyle->Margins.Bottom;

			contentOffsetX = backgroundStyle->Margins.Left;
			contentOffsetY = backgroundStyle->Margins.Top;
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

		GUITexture* backgroundFrame = GUITexture::Create(TextureScaleMode::StretchToFit, GetFrameStyleName());
		backgroundLayout->AddElement(backgroundFrame);

		GUILayout* contentLayout = contentPanel->AddNewElement<GUILayoutY>();
		contentLayout->AddNewElement<GUILabel>(HString(text), GUIOptions(GUIOption::FixedWidth(TOOLTIP_WIDTH), GUIOption::FlexibleHeight()), BuiltinResources::MultiLineLabelStyle);

		Rect2I positionBounds;
		positionBounds.X = position.X;
		positionBounds.Y = position.Y;
		positionBounds.Width = CURSOR_SIZE;
		positionBounds.Height = CURSOR_SIZE;

		DropDownAreaPlacement::HorzDir horzDir;
		DropDownAreaPlacement::VertDir vertDir;
		DropDownAreaPlacement placement = DropDownAreaPlacement::AroundBounds(positionBounds);
		Rect2I placementBounds = placement.GetOptimalBounds((u32)size.X, (u32)size.Y, availableBounds, horzDir, vertDir);

		backgroundPanel->SetPosition(placementBounds.X, placementBounds.Y);
		contentPanel->SetPosition(placementBounds.X + contentOffsetX, placementBounds.Y + contentOffsetY);
	}
} // namespace bs
