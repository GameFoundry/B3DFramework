//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGUIElementStyle.h"
#include "BsPrerequisites.h"
#include "GUI/BsGUIInteractable.h"
#include "2D/BsImageSprite.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUIContent.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** Structure used for initializing GUIBackgroundSprite. */
	struct GUIBackgroundSpriteCreateInformation
	{
		GUIBackgroundSpriteCreateInformation(const Size2UI& size, const GUIStyleSheetRules& rules, const Color& tint, u64 batchId)
			: Size(size), Rules(rules), Tint(tint), BatchId(batchId)
		{ }

		Vector2I Offset = Vector2I::kZero;
		RectOffset Border;
		Size2UI Size;
		u32 Depth = 1;
		Color Tint;
		u64 BatchId = 0;

		const GUIStyleSheetRules& Rules;
	};

	/** Wrapper around Sprite that helps construct a sprite for drawing a GUI element background controlled by style sheet rules. */
	class GUIBackgroundSprite
	{
	public:
		GUIBackgroundSprite();

		/**
		 * Builds the background render elements and appends them to the render elements array.
		 *
		 * @param	createInformation	Information about the sprite to build render elements for.
		 * @param	outRenderElements	Array to which the generated render element will be appended to.
		 */
		void BuildRenderElements(const GUIBackgroundSpriteCreateInformation& createInformation, TInlineArray<GUIRenderElement, 4>& outRenderElements);

		/** Same as the other overload, but for the old deprecated GUIElementStyle type, instead of style-sheets. */
		void BuildRenderElements(const Size2UI& size, const GUIElementStyle& style, GUIElementState state, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements, const Vector2I& offset = Vector2I::kZero, u32 depth = 1);

		/** Updates the animation start time (in seconds since application start), in case the background contains an animated sprite. */
		void SetAnimationStartTime(float time);

		/** Sets an interface that constructs the vector path used for drawing the background. */
		void SetBackgroundPathBuilder(const IGUIVectorPathBuilder* pathBuilder) { mBackgroundPathBuilder = pathBuilder; }
	private:
		ImageSprite mBackgroundSprite;
		ImageSpriteInformation mBackgroundSpriteInformation;

		const IGUIVectorPathBuilder* mBackgroundPathBuilder;
	};

	/** Wrapper around Sprite that helps construct a sprite for drawing a GUI element with text and/or image contents. */
	class GUIContentSprites
	{
	public:
		/**
		 * Builds the background render elements and appends them to the render elements array.
		 *
		 * @param	size				Size of the GUI element as determined by the layouting pass.
		 * @param	content				Content (text and/or image) to display.
		 * @param	rules				Active style-sheet rules for the GUI element.
		 * @param	tint				Runtime color tint to apply to the sprite.
		 * @param	batchId				ID that specifies if the sprite is allowed to be batched with other sprites. Only sprites with the same batch ID can be batched.
		 * @param	outRenderElements	Array to which the generated render element will be appended to.
		 */
		void BuildRenderElements(const Size2UI& size, const GUIContent& content, const GUIStyleSheetRules& rules, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements);

		/** Same as the other overload, but for the old deprecated GUIElementStyle type, instead of style-sheets. */
		void BuildRenderElements(const Size2UI& size, const GUIContent& content, const GUIElementStyle& style, GUIElementState state, const Color& tint, u64 batchId, TInlineArray<GUIRenderElement, 4>& outRenderElements);

		/** Updates the animation start time (in seconds since application start), in case the content image contains an animated sprite. */
		void SetAnimationStartTime(float time);

	private:
		/** Calculates the size of the provided image so it fits in the provided @p size, while preserving aspect ratio of the image. */
		static Size2UI CalculateScaledImageSize(const HSpriteImage& image, const Size2UI& size);

		/**
		 * Calculates the bounds at which to place text and/or image sprites.
		 *
		 * @param	contentArea		Content area of the GUI element. Both text and image must fit in this area.
		 * @param	imageSize		Size of the image sprite.
		 * @param	textSize		Size of the text sprite.
		 * @param	imagePosition	Position of the image relative to the text.
		 * @param	outTextBounds	Position of the text sprite, relative to the GUI element.
		 * @param	outImageBounds	Position of the image sprite, relative to the GUI element.
		 */
		static void CalculateContentBounds(const Rect2I& contentArea, const Size2UI& imageSize, const Size2UI& textSize, GUIImagePosition imagePosition, Rect2& outTextBounds, Rect2& outImageBounds);

		ImageSprite mContentImageSprite;
		TextSprite mContentTextSprite;

		ImageSpriteInformation mContentImageSpriteInformation;
		TextSpriteInformation mContentTextSpriteInformation;
	};

	/** Provides helper functionality that automatically extracts necessary data from a GUIElement and builds render elements for one of the GUI*Sprites types. */
	class GUISpriteHelper
	{
	public:
		/** Builds sprite elements for GUIBackgroundSprites. */
		static void BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, GUIBackgroundSprite& sprite, u32 depth = 1);

		/** Builds sprite elements for GUIContentSprites. */
		static void BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, const GUIContent& content, GUIContentSprites& sprites);
	};

	/** @} */
} // namespace bs
