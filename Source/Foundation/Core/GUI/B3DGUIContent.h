//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DGUIUnits.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Possible positions used for positioning content image within a GUI element. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIImagePosition
	{
		Left,
		Right
	};

	/**	Type of GUI element states. */
	enum class B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIElementState // TODO - Replace this with bitmask, add disabled state, handle focused+active, hover+active, etc.
	{
		Normal = 0x01, /**< Normal state when element is not being interacted with. */
		Hover = 0x02, /**< State when pointer is hovering over the element. */
		Active = 0x04, /**< State when element is being clicked. */
		Focused = 0x08, /**< State when the element has input focus and pointer is not hovering over the element. */
		FocusedHover = 0x10, /**< State when the element has input focus and pointer is hovering over the element. */
		NormalOn = 0x101, /**< Same as Normal, if the element is also in the "on" state. */
		HoverOn = 0x102, /**< Same as Hover, if the element is also in the "on" state. */
		ActiveOn = 0x104, /**< Same as Active, if the element is also in the "on" state. */
		FocusedOn = 0x108, /**< Same as Focused, if the element is also in the "on" state. */
		FocusedHoverOn = 0x110, /**< Same as FocusedHover, if the element is also in the "on" state. */

		// Helpers
		TypeMask B3D_SCRIPT_EXPORT(Exclude(true)) = 0xFF, /**< Mask for determining the state type (ignoring the on state). */
		OnFlag B3D_SCRIPT_EXPORT(Exclude(true)) = 0x100 /**< Flag that differentiates between on and off states. */
	};

	/**	Contains separate GUI content images for every possible GUI element state. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIContentImages
	{
		GUIContentImages() = default;

		GUIContentImages(const HSpriteImage& image)
			: Normal(image), Hover(image), Active(image), Focused(image), NormalOn(image), HoverOn(image), ActiveOn(image), FocusedOn(image)
		{}

		B3D_NO_RREF HSpriteImage Normal;
		B3D_NO_RREF HSpriteImage Hover;
		B3D_NO_RREF HSpriteImage Active;
		B3D_NO_RREF HSpriteImage Focused;
		B3D_NO_RREF HSpriteImage NormalOn;
		B3D_NO_RREF HSpriteImage HoverOn;
		B3D_NO_RREF HSpriteImage ActiveOn;
		B3D_NO_RREF HSpriteImage FocusedOn;
	};

	/**
	 * Holds data used for displaying content in a GUIElement. Content can consist of a string, image, a tooltip or none
	 * of those.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIContent
	{
	public:
		/**	Constructs an empty content. */
		GUIContent() = default;

		/**	Constructs content with just a string. */
		GUIContent(const HString& text)
			: Text(text)
		{}

		/**	Constructs content with a string and a tooltip. */
		GUIContent(const HString& text, const HString& tooltip)
			: Text(text), Tooltip(tooltip)
		{}

		/**	Constructs content with just an image. */
		GUIContent(const GUIContentImages& image)
			: Images(image)
		{}

		/**	Constructs content with an image and a tooltip. */
		GUIContent(const GUIContentImages& image, const HString& tooltip)
			: Images(image), Tooltip(tooltip)
		{}

		/**	Constructs content with a string and an image. */
		GUIContent(const HString& text, const GUIContentImages& image)
			: Text(text), Images(image)
		{}

		/**	Constructs content with a string, an image and a tooltip. */
		GUIContent(const HString& text, const GUIContentImages& image, const HString& tooltip)
			: Text(text), Images(image), Tooltip(tooltip)
		{}

		/**	Returns image content (if any). */
		const HSpriteImage& GetImage(GUIElementState state = GUIElementState::Normal) const;

		/**	Determines the spacing between text and image content in pixels. */
		static const GUILogicalUnit kImageTextSpacing;

		HString Text;
		GUIContentImages Images;
		HString Tooltip;
	};

	/** @} */
} // namespace b3d
