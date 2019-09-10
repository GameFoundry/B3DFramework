//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsImageSprite.h"
#include "2D/BsTextSprite.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	
	 * A GUI element that allows the user to draw custom graphics. All drawn elements relative to the canvas, to its origin
	 * in the top left corner.
	 */
	class BS_EXPORT GUICanvas : public GUIElement
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI canvas element.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUICanvas* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI canvas element.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUICanvas* create(const String& styleName = StringUtil::BLANK);
		
		/**
		 * Draws a line going from @p a to @p b.
		 *
		 * @param[in]	a		Starting point of the line, relative to the canvas origin (top-left).
		 * @param[in]	b		Ending point of the line, relative to the canvas origin (top-left).
		 * @param[in]	color	Color of the line.
		 * @param[in]	depth	Depth at which to draw the element. Elements with higher depth will be drawn before others.
		 *						Additionally elements of the same type (triangle or line) will be drawn in order they are
		 *						submitted if they share the same depth.
		 */
		void drawLine(const Vector2I& a, const Vector2I& b, const Color& color = Color::White, UINT8 depth = 128);

		/**
		 * Draws multiple lines following the path by the provided vertices. First vertex connects to the second vertex,
		 * and every following vertex connects to the previous vertex.
		 *
		 * @param[in]	vertices	Points to use for drawing the line. Must have at least two elements. All points are
		 *							relative to the canvas origin (top-left).
		 * @param[in]	color		Color of the line.
		 * @param[in]	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *							others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *							they are submitted if they share the same depth.
		 */
		void drawPolyLine(const Vector<Vector2I>& vertices, const Color& color = Color::White, UINT8 depth = 128);

		/**
		 * Draws a quad with a the provided texture displayed.
		 *
		 * @param[in]	texture		Texture to draw.
		 * @param[in]	area		Position and size of the texture to draw. Position is relative to the canvas origin
		 *							(top-left). If size is zero, the default texture size will be used.
		 * @param[in]	scaleMode	Scale mode to use when sizing the texture. Only relevant if the provided quad size
		 *							doesn't match the texture size.
		 * @param[in]	color		Color to tint the drawn texture with.
		 * @param[in]	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *							others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *							they are submitted if they share the same depth.
		 */
		void drawTexture(const HSpriteTexture& texture, const Rect2I& area,
			TextureScaleMode scaleMode = TextureScaleMode::StretchToFit, const Color& color = Color::White, UINT8 depth = 128);

		/**
		 * Draws a triangle strip. First three vertices are used to form the initial triangle, and every next vertex will
		 * form a triangle with the previous two.
		 *
		 * @param[in]	vertices	A set of points defining the triangles. Must have at least three elements. All points
		 *							are relative to the canvas origin (top-left).
		 * @param[in]	color		Color of the triangles.
		 * @param[in]	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *							others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *							they are submitted if they share the same depth.
		 */
		void drawTriangleStrip(const Vector<Vector2I>& vertices, const Color& color = Color::White, UINT8 depth = 128);

		/**
		 * Draws a triangle list. Every three vertices in the list represent a unique triangle.
		 *
		 * @param[in]	vertices	A set of points defining the triangles. Must have at least three elements, and its size
		 *							must be a multiple of three.
		 * @param[in]	color		Color of the triangles.
		 * @param[in]	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *							others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *							they are submitted if they share the same depth.
		 */
		void drawTriangleList(const Vector<Vector2I>& vertices, const Color& color = Color::White, UINT8 depth = 128);

		/**
		 * Draws a piece of text with the wanted font. The text will be aligned to the top-left corner of the provided
		 * position, and will not be word wrapped.
		 *
		 * @param[in]	text		Text to draw.
		 * @param[in]	position	Position of the text to draw. This represents the top-left corner of the text. It is
		 *							relative to the canvas origin (top-left).
		 * @param[in]	font		Font to draw the text with.
		 * @param[in]	size		Size of the font.
		 * @param[in]	color		Color of the text.
		 * @param[in]	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *							others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *							they are submitted if they share the same depth.
		 */
		void drawText(const String& text, const Vector2I& position, const HFont& font, UINT32 size = 10,
			const Color& color = Color::White, UINT8 depth = 128);

		/** Clears the canvas, removing any previously drawn elements. */
		void clear();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @copydoc GUIElement::_getRenderElementDepthRange */
		UINT32 _getRenderElementDepthRange() const override { return mDepthRange; }

		/** @} */
	protected:
		/** Type of elements that may be drawn on the canvas. */
		enum class CanvasElementType
		{
			Line,
			Triangle,
			Image,
			Text
		};

		/** Represents a single element drawn by the canvas. */
		struct CanvasElement
		{
			CanvasElementType type;
			Color color;
			UINT32 renderElemStart;
			UINT32 renderElemEnd;
			UINT32 dataId;
			UINT8 depth;

			union
			{
				struct
				{
					UINT32 vertexStart;
					UINT32 numVertices;
					mutable UINT32 clippedVertexStart;
					mutable UINT32 clippedNumVertices;
				};

				struct
				{
					ImageSprite* imageSprite;
					TextureScaleMode scaleMode;
				};

				struct
				{
					TextSprite* textSprite;
					UINT32 size;
				};
			};
		};

		/** Information required for drawing a text canvas element. */
		struct TextElementData
		{
			String string;
			HFont font;
			Vector2I position;
		};

		/** Information required for drawing an image canvas element. */
		struct ImageElementData
		{
			HSpriteTexture texture;
			Rect2I area;
		};

		/** Information required for drawing an arbitrary triangle canvas element. */
		struct TriangleElementData
		{
			SpriteMaterialInfo matInfo;
		};

		GUICanvas(const String& styleName, const GUIDimensions& dimensions);
		virtual ~GUICanvas();

		/** @copydoc GUIElement::_fillBuffer */
		void _fillBuffer(
			UINT8* vertices,
			UINT32* indices,
			UINT32 vertexOffset,
			UINT32 indexOffset,
			const Vector2I& offset,
			UINT32 maxNumVerts,
			UINT32 maxNumIndices,
			UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal */
		void updateRenderElementsInternal() override;

		/** Build an image sprite from the provided canvas element. */
		void buildImageElement(const CanvasElement& element);

		/** Build a text sprite from the provided canvas element. */
		void buildTextElement(const CanvasElement& element);

		/** Build a set of clipped triangles from the source triangles provided by the canvas element. */
		void buildTriangleElement(const CanvasElement& element, const Vector2& offset, const Rect2I& clipRect) const;

		/**
		 * Rebuilds all triangle elements on the canvas, by constructing a set of clipped and offset triangles from the
		 * triangles provided by the canvas elements.
		 */
		void buildAllTriangleElementsIfDirty(const Vector2& offset, const Rect2I& clipRect) const;

		/** Finds the canvas element that contains the render element with the specified index. */
		const CanvasElement& findElement(UINT32 renderElementIdx) const;

		Vector<CanvasElement> mElements;
		UINT8 mDepthRange = 1;

		Vector<ImageElementData> mImageData;
		Vector<TextElementData> mTextData;
		mutable Vector<TriangleElementData> mTriangleElementData;
		Vector<Vector2> mVertexData;

		mutable Vector<Vector2> mClippedVertices;
		mutable Vector<Vector2> mClippedLineVertices;
		mutable Vector2 mLastOffset = BsZero;
		mutable Rect2I mLastClipRect;
		mutable bool mForceTriangleBuild = false;

		static const float LINE_SMOOTH_BORDER_WIDTH;
	};

	/** @} */
}
