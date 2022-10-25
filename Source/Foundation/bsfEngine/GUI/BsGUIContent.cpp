//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIContent.h"

using namespace bs;

const u32 GUIContent::IMAGE_TEXT_SPACING = 3;

const HSpriteTexture& GUIContent::GetImage(GUIElementState state) const
{
	switch(state)
	{
	case GUIElementState::Normal:
		return Images.Normal;
	case GUIElementState::Hover:
		return Images.Hover;
	case GUIElementState::Active:
		return Images.Active;
	case GUIElementState::Focused:
	case GUIElementState::FocusedHover:
		return Images.Focused;
	case GUIElementState::NormalOn:
		return Images.NormalOn;
	case GUIElementState::HoverOn:
		return Images.HoverOn;
	case GUIElementState::ActiveOn:
		return Images.ActiveOn;
	case GUIElementState::FocusedOn:
	case GUIElementState::FocusedHoverOn:
		return Images.FocusedOn;
	default:
		return Images.Normal;
	}
}
