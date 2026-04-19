//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "2D/B3DImageSprite.h"
#include "2D/B3DTextSprite.h"
#include "GUI/B3DGUIContent.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Structure used for initializing GUIBackgroundSprite. */
	struct GUIBackgroundSpriteCreateInformation
	{
		GUIBackgroundSpriteCreateInformation(const Size2UI& size, const GUIStyleSheetRules& rules, const Color& tint, u64 batchId)
			: Size(size), Rules(rules), Tint(tint), BatchId(batchId)
		{ }

		Vector2I Offset = Vector2I::kZero; /**< Offset relative to the parent GUI element's content area to place the sprite at. */
		Size2UI Size; /**< Size of the GUI element as determined by the layouting pass. */
		u32 Depth = 1; /**< Depth at which to render the sprite. Higher depth means a sprite is rendered behind sprites with lower depth. */
		Color Tint; /**< Runtime color tint to apply to the sprite. */
		u64 BatchId = 0; /**< ID that specifies if the sprite is allowed to be batched with other sprites. Only sprites with the same batch ID can be batched. */

		const GUIStyleSheetRules& Rules; /**< Style sheet rules that determine how to style the sprite. */
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

		/** Updates the animation start time (in seconds since application start), in case the background contains an animated sprite. */
		void SetAnimationStartTime(float time);

		/** Sets an interface that constructs the vector path used for drawing the background. */
		void SetBackgroundPathBuilder(const IGUIVectorPathBuilder* pathBuilder) { mBackgroundPathBuilder = pathBuilder; }
	private:
		ImageSprite mBackgroundSprite;
		ImageSpriteInformation mBackgroundSpriteInformation;

		const IGUIVectorPathBuilder* mBackgroundPathBuilder;
	};

	/** Structure used for initializing GUIContentSprites. */
	struct GUIContentSpriteCreateInformation
	{
		GUIContentSpriteCreateInformation(const Size2UI& size, const GUIContent& content, const GUIStyleSheetRules& rules, const Color& tint, float fontScale, u64 batchId)
			: Content(content), ContentArea(0, 0, size.Width, size.Height), Rules(rules), Tint(tint), BatchId(batchId), FontScale(fontScale)
		{ }

		GUIContent Content; /**< Image and/or text content to display. */
		Vector2I Offset = Vector2I::kZero; /**< Offset relative to the parent GUI element's content area to place the sprite at. */
		Area2I ContentArea = Area2I::kEmpty; /**< Area relative to parent GUI element, in which to place the contents. Any sprite elements outside of this area will be clipped. */
		u32 Depth = 0; /**< Depth at which to render the sprites. Higher depth means a sprite is rendered behind sprites with lower depth. */
		Color Tint; /**< Runtime color tint to apply to the sprite. */
		u64 BatchId = 0; /**< ID that specifies if the sprite is allowed to be batched with other sprites. Only sprites with the same batch ID can be batched. */
		float FontScale = 1.0f; /**< Scale to apply to font size. */
		bool WordWrap = false; /**< If true, text will wrap to a new line if it exceeds the content area width. If false, the text will be clipped. */

		const GUIStyleSheetRules& Rules; /**< Style sheet rules that determine how to style the sprites. */
	};

	/** Wrapper around Sprite that helps construct a sprite for drawing a GUI element with text and/or image contents. */
	class GUIContentSprites
	{
	public:
		/**
		 * Builds the background render elements and appends them to the render elements array.
		 *
		 * @param	createInformation	Information about the sprites to build render elements for.
		 * @param	outRenderElements	Array to which the generated render element will be appended to.
		 */
		void BuildRenderElements(const GUIContentSpriteCreateInformation& createInformation, TInlineArray<GUIRenderElement, 4>& outRenderElements);

		/** Builds a struct used for initializing the text sprite required for rendering the provided contents within the provided bounds. */
		static TextSpriteInformation BuildTextSpriteInformation(const Area2I& contentArea, const String& text, const GUIStyleSheetRules& rules, const Color& tint, float fontScale = 1.0f, bool wordWrap = false);

		/** Updates the animation start time (in seconds since application start), in case the content image contains an animated sprite. */
		void SetAnimationStartTime(float time);

		/** Returns the image sprite. Note the sprite is only initialized/updated after a call to BuildRenderElements(). */
		const ImageSprite& GetImageSprite() const { return mContentImageSprite; }

		/** Returns the text sprite. Note the sprite is only initialized/updated after a call to BuildRenderElements(). */
		const TextSprite& GetTextSprite() const { return mContentTextSprite; }

	private:
		/** Calculates the size of the provided image so it fits in the provided @p size, while preserving aspect ratio of the image. */
		static Size2I CalculateScaledImageSize(const HSpriteImage& image, const Size2I& size);

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
		static void CalculateContentBounds(const Area2I& contentArea, const Size2UI& imageSize, const Size2UI& textSize, GUIImagePosition imagePosition, Area2& outTextBounds, Area2& outImageBounds);

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
		static void BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, GUIBackgroundSprite& sprite, const Vector2I& offset = Vector2I::kZero, u32 depth = 1);

		/** Builds sprite elements for GUIContentSprites. */
		static void BuildSpriteRenderElements(GUIInteractable& element, GUIElementState state, const GUIContent& content, GUIContentSprites& sprites, const Vector2I& offset = Vector2I::kZero, u32 depth = 0, bool wordWrap = false);

		/** Builds a struct used for initializing a TextSprite, required for rendering the provided contents from the provided GUI element. */
		static TextSpriteInformation BuildTextSpriteInformation(const GUIInteractable& element, GUIElementState state, const String& text, float fontScale = 1.0f, bool wordWrap = false);
	};

	/** @} */
} // namespace b3d
