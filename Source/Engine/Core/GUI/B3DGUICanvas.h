//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "GUI/B3DGUIConstructionMethods.h"
#include "2D/B3DImageSprite.h"
#include "2D/B3DTextSprite.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**
	 * A GUI element that allows the user to draw custom graphics. All drawn elements relative to the canvas, to its origin
	 * in the top left corner.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUICanvas : public GUIInteractable, public TGUIConstructionMethodsWithoutContent<GUICanvas>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Draws a line going from @p a to @p b.
		 *
		 * @param	a		Starting point of the line, relative to the canvas origin (top-left).
		 * @param	b		Ending point of the line, relative to the canvas origin (top-left).
		 * @param	color	Color of the line.
		 * @param	depth	Depth at which to draw the element. Elements with higher depth will be drawn before others.
		 *					Additionally elements of the same type (triangle or line) will be drawn in order they are
		 *					submitted if they share the same depth.
		 */
		B3D_SCRIPT_EXPORT()
		void DrawLine(const GUILogicalPoint& a, const GUILogicalPoint& b, const Color& color, u8 depth = 128);

		/**
		 * Draws multiple lines following the path by the provided vertices. First vertex connects to the second vertex,
		 * and every following vertex connects to the previous vertex.
		 *
		 * @param	vertices	Points to use for drawing the line. Must have at least two elements. All points are
		 *						relative to the canvas origin (top-left).
		 * @param	color		Color of the line.
		 * @param	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *						others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *						they are submitted if they share the same depth.
		 */
		B3D_SCRIPT_EXPORT()
		void DrawPolyLine(const Vector<GUILogicalPoint>& vertices, const Color& color, u8 depth = 128);

		/**
		 * Draws a quad with a the provided image displayed.
		 *
		 * @param	image		Image to draw.
		 * @param	area		Position and size of the texture to draw. Position is relative to the canvas origin
		 *						(top-left). If size is zero, the default texture size will be used.
		 * @param	color		Color to tint the drawn texture with.
		 * @param	scaleMode	Scale mode to use when sizing the texture. Only relevant if the provided quad size
		 *						doesn't match the texture size.
		 * @param	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *						others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *						they are submitted if they share the same depth.
		 */
		B3D_SCRIPT_EXPORT()
		void DrawImage(const HSpriteImage& image, const GUILogicalArea& area, const Color& color, TextureScaleMode scaleMode = TextureScaleMode::StretchToFit, u8 depth = 128);

		/**
		 * Draws a triangle strip. First three vertices are used to form the initial triangle, and every next vertex will
		 * form a triangle with the previous two.
		 *
		 * @param	vertices	A set of points defining the triangles. Must have at least three elements. All points
		 *						are relative to the canvas origin (top-left).
		 * @param	color		Color of the triangles.
		 * @param	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *						others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *						they are submitted if they share the same depth.
		 */
		B3D_SCRIPT_EXPORT()
		void DrawTriangleStrip(const Vector<GUILogicalPoint>& vertices, const Color& color, u8 depth = 128);

		/**
		 * Draws a triangle list. Every three vertices in the list represent a unique triangle.
		 *
		 * @param	vertices	A set of points defining the triangles. Must have at least three elements, and its size
		 *						must be a multiple of three.
		 * @param	color		Color of the triangles.
		 * @param	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *						others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *						they are submitted if they share the same depth.
		 */
		B3D_SCRIPT_EXPORT()
		void DrawTriangleList(const Vector<GUILogicalPoint>& vertices, const Color& color, u8 depth = 128);

		/**
		 * Draws a piece of text with the wanted font. The text will be aligned to the top-left corner of the provided
		 * position, and will not be word wrapped.
		 *
		 * @param	text		Text to draw.
		 * @param	position	Position of the text to draw. This represents the top-left corner of the text. It is
		 *						relative to the canvas origin (top-left).
		 * @param	font		Font to draw the text with.
		 * @param	size		Size of the font.
		 * @param	color		Color of the text.
		 * @param	depth		Depth at which to draw the element. Elements with higher depth will be drawn before
		 *						others. Additionally elements of the same type (triangle or line) will be drawn in order
		 *						they are submitted if they share the same depth.
		 */
		B3D_SCRIPT_EXPORT()
		void DrawText(const String& text, const GUILogicalPoint& position, const HFont& font, float size, const Color& color, u8 depth = 128);

		/** Clears the canvas, removing any previously drawn elements. */
		B3D_SCRIPT_EXPORT()
		void Clear();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		struct PrivatelyConstruct { };
		GUICanvas(PrivatelyConstruct, const String& styleName, const GUISizeConstraints& sizeConstraints);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;
		u32 GetRenderElementDepthRange() const override { return mDepthRange; }
		const char* GetStyleSheetElement() const override { return "canvas"; }

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
			CanvasElementType Type;
			Color Color;
			u32 RenderElemStart;
			u32 RenderElemEnd;
			u32 DataId;
			u8 Depth;

			union
			{
				struct
				{
					u32 VertexStart;
					u32 VertexCount;
					mutable u32 ClippedVertexStart;
					mutable u32 ClippedVertexCount;
				};

				struct
				{
					ImageSprite* ImageSprite;
					TextureScaleMode ScaleMode;
				};

				struct
				{
					TextSprite* TextSprite;
					float Size;
				};
			};
		};

		/** Information required for drawing a text canvas element. */
		struct TextElementData
		{
			String String;
			HFont Font;
			GUILogicalPoint Position;
		};

		/** Information required for drawing an image canvas element. */
		struct ImageElementData
		{
			HSpriteImage Image;
			GUILogicalArea Area;
		};

		/** Information required for drawing an arbitrary triangle canvas element. */
		struct TriangleElementData
		{
			SpriteMaterialInfo MaterialInfo;
		};

		virtual ~GUICanvas();

		void FillBuffer(u8* vertices, u32* indices, u32 vertexOffset, u32 indexOffset, const Vector2I& offset, u32 maxVertexCount, u32 maxIndexCount, u32 renderElementIdx) const override;
		void UpdateRenderElements() override;

		/** Build an image sprite from the provided canvas element. */
		void BuildImageElement(const CanvasElement& element);

		/** Build a text sprite from the provided canvas element. */
		void BuildTextElement(const CanvasElement& element);

		/** Build a set of clipped triangles from the source triangles provided by the canvas element. */
		void BuildTriangleElement(const CanvasElement& element, const Vector2& offset, float scale, const Area2I& clipRect) const;

		/**
		 * Rebuilds all triangle elements on the canvas, by constructing a set of clipped and offset triangles from the
		 * triangles provided by the canvas elements.
		 */
		void BuildAllTriangleElementsIfDirty(const Vector2& offset, float scale, const Area2I& clipRect) const;

		/** Finds the canvas element that contains the render element with the specified index. */
		const CanvasElement& FindElement(u32 renderElementIdx) const;

		Vector<CanvasElement> mElements;
		u8 mDepthRange = 1;

		Vector<ImageElementData> mImageData;
		Vector<TextElementData> mTextData;
		mutable Vector<TriangleElementData> mTriangleElementData;
		Vector<GUILogicalPointF> mVertexData;

		mutable Vector<Vector2> mClippedVertices;
		mutable Vector<Vector2> mClippedLineVertices;
		mutable Vector2 mLastOffset = BsZero;
		mutable Area2I mLastClipRect;
		mutable bool mForceTriangleBuild = false;

		static const float kLineSmoothBorderWidth;
	};

	/** @} */
} // namespace b3d
