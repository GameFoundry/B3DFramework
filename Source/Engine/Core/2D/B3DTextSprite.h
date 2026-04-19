//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "2D/B3DSprite.h"
#include "Text/B3DTextGeometry.h"
#include "Image/B3DColor.h"
#include "Math/B3DVector2.h"
#include "Allocators/B3DStaticAlloc.h"

namespace b3d
{
	struct GUIStyleSheetRules;
	/** @addtogroup 2D
	 *  @{
	 */

	/** Determines how is text horizontally aligned in a GUI element. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIHorizontalTextAlignment
	{
		Left, Center, Right
	};

	/** Determines how is text vertically aligned in a GUI element. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIVerticalTextAlignment
	{
		Top, Middle, Bottom
	};

	/**	Information for initializing or updating a text sprite. */
	struct TextSpriteInformation : SpriteInformation
	{
		TextSpriteInformation() = default;

		/** Initializes the sprite information from data provided by the style sheet. */
		void InitializeFromStyleSheetRules(const GUIStyleSheetRules& rules);

		SpriteAnchor Anchor = SA_TopLeft; /**< Determines how to anchor the text within the bounds. */

		String Text; /**< UTF-8 encoded text to generate geometry for. */
		HFont Font; /**< Font containing the data about character glyphs. */
		float FontSize = 0.0f; /**< Size of the font to use when displaying the text, in points. */
		GUIHorizontalTextAlignment HorzAlign = GUIHorizontalTextAlignment::Left; /**< Specifies how is text horizontally aligned within its bounds. */
		GUIVerticalTextAlignment VertAlign = GUIVerticalTextAlignment::Top; /**< Specifies how is text vertically aligned within its bounds. */
		bool WordWrap = false; /**< If true the text will word wrap when it doesn't fit, otherwise it will overflow. */
		bool WordBreak = true; /**< If enabled together with word wrap it will allow words to be broken if they don't fit. */
	};

	/**	A sprite consisting of a quads representing a text string. */
	class B3D_EXPORT TextSprite : public Sprite
	{
	public:
		TextSprite() = default;
		~TextSprite();

		/**
		 * Recreates internal sprite data according the specified description structure.
		 *
		 * @param	information		Describes the geometry and material of the sprite.
		 * @param	groupId			Group identifier that forces different materials to be used for different groups (for
		 *							example you don't want the sprites to share the same group if they use different world
		 *							transform matrices).
		 */
		void Update(const TextSpriteInformation& information, u64 groupId);

		/**
		 * Calculates and returns offset for each individual text line. The offsets provide information on how much to
		 * offset the lines within provided bounds.
		 *
		 * @param	textGeometry	Text geometry to generate offsets for.
		 * @param	width			Width of the text bounds into which to constrain the text, in pixels.
		 * @param	height			Height of the text bounds into which to constrain the text, in pixels.
		 * @param	horzAlign		Specifies how is text horizontally aligned within its bounds.
		 * @param	vertAlign		Specifies how is text vertically aligned within its bounds.
		 * @param	output			Pre-allocated buffer to output the results in. Buffer must have an element
		 *							for every line in @p textData.
		 */
		static void GetAlignmentOffsets(const TextGeometry& textGeometry, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, Vector2I* output);

		/**
		 * Calculates text quads you may use for text rendering, based on the specified text data. Only generates quads for
		 * the specified page.
		 *
		 * @param	page			Font page to generate the data for.
		 * @param	textGeometry	Text geometry to generate offsets for.
		 * @param	width			Width of the text bounds into which to constrain the text, in pixels.
		 * @param	height			Height of the text bounds into which to constrain the text, in pixels.
		 * @param	horzAlign		Specifies how is text horizontally aligned within its bounds.
		 * @param	vertAlign		Specifies how is text vertically aligned within its bounds.
		 * @param	anchor			Determines how to anchor the text within the bounds.
		 * @param	vertices		Output buffer containing quad positions. Must be allocated and of adequate size.
		 * @param	uv				Output buffer containing quad UV coordinates. Must be allocated and of adequate
		 *							size. Can be null.
		 * @param	indices			Output buffer containing quad indices. Must be allocated and of adequate size. Can
		 *							be null.
		 * @param	bufferSizeQuads	Size of the output buffers, in number of quads.
		 * @return					Number of generated quads.
		 */
		static u32 BuildTextQuads(u32 page, const TextGeometry& textGeometry, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads);

		/**
		 * Calculates text quads you may use for text rendering, based on the specified text data. Generates quads for all
		 * pages.
		 *
		 * @param	textGeometry	Text geometry to generate offsets for.
		 * @param	width			Width of the text bounds into which to constrain the text, in pixels.
		 * @param	height			Height of the text bounds into which to constrain the text, in pixels.
		 * @param	horzAlign		Specifies how is text horizontally aligned within its bounds.
		 * @param	vertAlign		Specifies how is text vertically aligned within its bounds.
		 * @param	anchor			Determines how to anchor the text within the bounds.
		 * @param	vertices		Output buffer containing quad positions. Must be allocated and of adequate size.
		 * @param	uv				Output buffer containing quad UV coordinates. Must be allocated and of adequate
		 *							size. Can be null.
		 * @param	indices			Output buffer containing quad indices. Must be allocated and of adequate size. Can
		 *							be null.
		 * @param	bufferSizeQuads	Size of the output buffers, in number of quads.
		 * @return					Number of generated quads.
		 */
		static u32 BuildTextQuads(const TextGeometry& textGeometry, u32 width, u32 height, GUIHorizontalTextAlignment horzAlign, GUIVerticalTextAlignment vertAlign, SpriteAnchor anchor, Vector2* vertices, Vector2* uv, u32* indices, u32 bufferSizeQuads);

	private:
		static const int kStaticCharsToBuffer = 25;
		static const int kStaticBufferSize = kStaticCharsToBuffer * (4 * (2 * sizeof(Vector2)) + (6 * sizeof(u32)));

		/**	Clears internal geometry buffers. */
		void ClearMesh();

		mutable StaticAlloc<kStaticBufferSize> mAlloc;
	};

	/** @} */
} // namespace b3d
