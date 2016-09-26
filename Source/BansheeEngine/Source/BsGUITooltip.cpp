//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUITooltip.h"
#include "BsGUIPanel.h"
#include "BsCamera.h"
#include "BsViewport.h"
#include "BsGUILayoutY.h"
#include "BsGUILayoutX.h"
#include "BsGUITexture.h"
#include "BsGUILabel.h"
#include "BsGUIHelper.h"
#include "BsGUISkin.h"
#include "BsBuiltinResources.h"
#include "BsDropDownAreaPlacement.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	const UINT32 GUITooltip::TOOLTIP_WIDTH = 200;
	const UINT32 GUITooltip::CURSOR_SIZE = 16;

	String GUITooltip::getFrameStyleName()
	{
		return "TooltipFrame";
	}

	GUITooltip::GUITooltip(const HSceneObject& parent, const GUIWidget& overlaidWidget, const Vector2I& position, const WString& text)
		:CGUIWidget(parent, overlaidWidget.getCamera())
	{
		setDepth(0); // Needs to be in front of everything
		setSkin(overlaidWidget.getSkinResource());
		
		SPtr<Camera> camera = overlaidWidget.getCamera();
		SPtr<Viewport> viewport = camera->getViewport();

		Rect2I availableBounds(viewport->getX(), viewport->getY(), viewport->getWidth(), viewport->getHeight());
		
		const GUISkin& skin = getSkin();
		const GUIElementStyle* multiLineLabelStyle = skin.getStyle(BuiltinResources::MultiLineLabelStyle);
		const GUIElementStyle* backgroundStyle = skin.getStyle(getFrameStyleName());

		Vector2I size(TOOLTIP_WIDTH, 25);
		if (multiLineLabelStyle != nullptr)
		{
			GUIDimensions dimensions = GUIDimensions::create(GUIOptions(GUIOption::fixedWidth(TOOLTIP_WIDTH)));
			size = GUIHelper::calcOptimalContentsSize(text, *multiLineLabelStyle, dimensions);
		}

		INT32 contentOffsetX = 0;
		INT32 contentOffsetY = 0;
		if(backgroundStyle != nullptr)
		{
			size.x += backgroundStyle->margins.left + backgroundStyle->margins.right;
			size.y += backgroundStyle->margins.top + backgroundStyle->margins.bottom;

			contentOffsetX = backgroundStyle->margins.left;
			contentOffsetY = backgroundStyle->margins.top;
		}

		// Content area
		GUIPanel* contentPanel = getPanel()->addNewElement<GUIPanel>();
		contentPanel->setWidth((UINT32)size.x);
		contentPanel->setHeight((UINT32)size.y);
		contentPanel->setDepthRange(-1);

		// Background frame
		GUIPanel* backgroundPanel = getPanel()->addNewElement<GUIPanel>();
		backgroundPanel->setWidth((UINT32)size.x);
		backgroundPanel->setHeight((UINT32)size.y);
		backgroundPanel->setDepthRange(0);

		GUILayout* backgroundLayout = backgroundPanel->addNewElement<GUILayoutX>();

		GUITexture* backgroundFrame = GUITexture::create(TextureScaleMode::StretchToFit, getFrameStyleName());
		backgroundLayout->addElement(backgroundFrame);

		GUILayout* contentLayout = contentPanel->addNewElement<GUILayoutY>();
		contentLayout->addNewElement<GUILabel>(HString(text), 
			GUIOptions(GUIOption::fixedWidth(TOOLTIP_WIDTH), GUIOption::flexibleHeight()), 
			BuiltinResources::MultiLineLabelStyle);

		Rect2I positionBounds;
		positionBounds.x = position.x;
		positionBounds.y = position.y;
		positionBounds.width = CURSOR_SIZE;
		positionBounds.height = CURSOR_SIZE;

		DropDownAreaPlacement::HorzDir horzDir;
		DropDownAreaPlacement::VertDir vertDir;
		DropDownAreaPlacement placement = DropDownAreaPlacement::aroundBounds(positionBounds);
		Rect2I placementBounds = placement.getOptimalBounds((UINT32)size.x, (UINT32)size.y, availableBounds, horzDir, vertDir);

		backgroundPanel->setPosition(placementBounds.x, placementBounds.y);
		contentPanel->setPosition(placementBounds.x + contentOffsetX, placementBounds.y + contentOffsetY);
	}

	GUITooltip::~GUITooltip()
	{

	}
}