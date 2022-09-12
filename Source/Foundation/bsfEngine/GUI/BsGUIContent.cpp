//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIContent.h"

namespace bs
{
	const UINT32 GUIContent::IMAGE_TEXT_SPACING = 3;

	const HSpriteTexture& GUIContent::GetImage(GUIElementState state) const
	{
		switch (state)
		{
		case GUIElementState::Normal:
			return images.normal;
		case GUIElementState::Hover:
			return images.hover;
		case GUIElementState::Active:
			return images.active;
		case GUIElementState::Focused:
		case GUIElementState::FocusedHover:
			return images.focused;
		case GUIElementState::NormalOn:
			return images.normalOn;
		case GUIElementState::HoverOn:
			return images.hoverOn;
		case GUIElementState::ActiveOn:
			return images.activeOn;
		case GUIElementState::FocusedOn:
		case GUIElementState::FocusedHoverOn:
			return images.focusedOn;
		default:
			return images.normal;
		}
	}
}
