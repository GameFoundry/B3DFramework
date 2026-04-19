//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "2D/B3DSpriteMaterial.h"
#include "Math/B3DArea2.h"
#include "Image/B3DColor.h"
#include "Math/B3DArea2.h"

namespace b3d
{
	/** @addtogroup 2D-Internal
	 *  @{
	 */

	/** Determines position of the sprite in its bounds. */
	enum SpriteAnchor
	{
		SA_TopLeft,
		SA_TopCenter,
		SA_TopRight,
		SA_MiddleLeft,
		SA_MiddleCenter,
		SA_MiddleRight,
		SA_BottomLeft,
		SA_BottomCenter,
		SA_BottomRight
	};

	/** Contains information about a single sprite render elements mesh and material */
	struct SpriteRenderElement
	{
		u32 IndexCount = 0;
		u32 VertexCount = 0;

		Vector2* VertexPositions = nullptr;
		Vector2* VertexUVs = nullptr;
		u32* Indices = nullptr;

		SpriteMaterialInfo* MaterialInformation = nullptr;
		SpriteMaterial* Material = nullptr;

		/**
		 * Retrieves vertex and index data from the sprite render element and outputs them to the provided buffers.
		 *
		 * @param	vertexOffset		At which vertex should the method start writing to the output position/uv buffer.
		 * @param	indexOffset			At which index should the method start writing to the output index buffer.
		 * @param	offset				Offset that should be applied to all output vertex positions.
		 * @param	clipRectangle		Rectangle to clip the vertices to, if clipping is enabled.
		 * @param	performClipping		Should the vertices be clipped to the provided @p clipRectangle.
		 * @param	outPositions		Previously allocated buffer where to store the vertex positions. Caller must ensure
		 *								size matches the vertex count.
		 * @param	outUVs				Previously allocated buffer where to store the vertex UVs. Caller must ensure
		 *								size matches the vertex count.
		 * @param	outIndices			Previously allocated buffer where to store the indices. Caller must ensure
		 *								size matches the index count.
		 * @return						Number of quads that were written.
		 */
		u32 GetVertexAndIndexData(u32 vertexOffset, u32 indexOffset, const Vector2& offset, const Area2& clipRectangle, bool performClipping, DataRange& outPositions, DataRange& outUVs, DataRange& outIndices) const;
	};

	/** Common information for all sprite types. */
	struct SpriteInformation
	{
		Size2I Size{kZeroTag}; /**< Size of the sprite in pixels. */
		bool Transparent = true; /**< Should the sprite be rendered with transparency. */
		Color Color; /**< Color tint to apply to the sprite. */
	};

	/**	Generates geometry and contains information needed for rendering a two dimensional element. */
	class B3D_EXPORT Sprite
	{
	public:
		Sprite() = default;
		virtual ~Sprite() = default;

		/**
		 * Returns clipped bounds of the sprite.
		 *
		 * @param	offset		Offset that will be added to the returned bounds.
		 * @param	clipRect	Local clip rect that is used for clipping the sprite bounds. (Clipping is done before
		 *						the offset is applied). If clip rect width or height is zero, no clipping is done.
		 *
		 * @return				Clipped sprite bounds.
		 */
		Area2I GetBounds(const Vector2I& offset, const Area2I& clipRect) const;

		/**
		 * Returns the number of separate render elements in the sprite. Normally this is 1, but some sprites may consist
		 * of multiple materials, in which case each will require its own mesh (render element)
		 *
		 * @return	The number render elements.
		 */
		u32 GetRenderElementCount() const { return (u32)mCachedRenderElements.size(); }

		/**
		 * Copies the internal render element information into the provided @p info object.
		 *
		 * Note the pointers to vertex/index buffers continue to be owned by the Sprite. You must not manually free them.
		 * They will be valid until the sprite is destroyed, or until sprite is updated with new data or its mesh data is
		 * explicitly cleared, at which point you must no longer use them. 
		 */
		void GetRenderElement(u32 index, SpriteRenderElement& info) const { info = mCachedRenderElements[index].RenderElement; }

		/**
		 * Fill the pre-allocated vertex, uv and index buffers with the mesh data for the specified render element.
		 *
		 * @param	outVertices				Previously allocated buffer where to store the vertices.
		 * @param	outUv						Previously allocated buffer where to store the uv coordinates.
		 * @param	outIndices				Previously allocated buffer where to store the indices.
		 * @param	vertexOffset			At which vertex should the method start filling the buffer.
		 * @param	indexOffset				At which index should the method start filling the buffer.
		 * @param	maxVertexCount			Total number of vertices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param	maxIndexCount			Total number of indices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param	vertexStride			Number of bytes between of vertices in the provided vertex and uv data.
		 * @param	indexStride				Number of bytes between two indexes in the provided index data.
		 * @param	renderElementIndex		Zero-based index of the render element.
		 * @param	offset					Position offset to apply to all vertices, after clipping.
		 * @param	clipRect				Rectangle to clip the vertices to.
		 * @param	clip					Should the vertices be clipped to the provided @p clipRect.
		 *
		 * @see		GetRenderElementCount()
		 * @see		GetNumQuads()
		 */
		u32 FillBuffer(u8* outVertices, u8* outUv, u32* outIndices, u32 vertexOffset, u32 indexOffset, u32 maxVertexCount, u32 maxIndexCount, u32 vertexStride, u32 indexStride, u32 renderElementIndex, const Vector2I& offset, const Area2I& clipRect, bool clip = true) const; // DEPRECATED

		/**
		 * Clips the provided 2D vertices to the provided clip rectangle. The vertices must form axis aligned quads.
		 *
		 * @param	outVertices			Pointer to the start of the buffer containing vertex positions.
		 * @param	outUv					Pointer to the start of the buffer containing UV coordinates.
		 * @param	quadCount			Number of quads in the provided buffer pointers.
		 * @param	vertexStride		Number of bytes to skip when going to the next vertex. This assumes both position
		 *								and uv coordinates have the same stride (as they are likely pointing to the same
		 *								buffer).
		 * @param	clipRect			Rectangle to clip the geometry to.
		 */
		static void ClipQuadsToRect(u8* outVertices, u8* outUv, u32 quadCount, u32 vertexStride, const Area2I& clipRect);

		/**
		 * Clips the provided 2D vertices to the provided clip rectangle. The vertices must form axis aligned quads.
		 *
		 * @param	vertices			Buffer containing vertex positions to clip.
		 * @param	uv					Buffer containing UV positions to clip.
		 * @param	quadCount			Number of quads to clip.
		 * @param	startVertexIndex	Offset into vertex/uv buffers at which to start clipping.
		 * @param	clipRectangle		Rectangle to clip the geometry to.
		 */
		static void ClipQuadsToRectangle(DataRange& vertices, DataRange& uv, u32 quadCount, u32 startVertexIndex, const Area2& clipRectangle);

		/**
		 * Clips the provided 2D vertices to the provided clip rectangle. The vertices can be arbitrary triangles.
		 *
		 * @param	vertices			Pointer to the start of the buffer containing vertex positions.
		 * @param	uv					Pointer to the start of the buffer containing UV coordinates. Can be null if UV is
		 *								not needed.
		 * @param	triangleCount		Number of triangles in the provided buffer pointers.
		 * @param	vertexStride		Number of bytes to skip when going to the next vertex. This assumes both position
		 *								and uv coordinates have the same stride (as they are likely pointing to the same
		 *								buffer).
		 * @param	clipRect			Rectangle to clip the geometry to.
		 * @param	writeCallback		Callback that will be triggered when clipped vertices and UV coordinates are
		 *								generated and need to be stored. Vertices are always generate in tuples of three,
		 *								forming a single triangle.
		 */
		static void ClipTrianglesToRect(u8* vertices, u8* uv, u32 triangleCount, u32 vertexStride, const Area2I& clipRect, const std::function<void(Vector2*, Vector2*, u32)>& writeCallback);

	protected:
		/**	Returns the offset needed to move the sprite in order for it to respect the provided anchor. */
		static Vector2I GetAnchorOffset(SpriteAnchor anchor, u32 width, u32 height);

		/**	Calculates the bounds of all sprite vertices. */
		void UpdateBounds() const;

		/** Information about a sprite render element. */
		struct RenderElementData
		{
			SpriteRenderElement RenderElement;
			SpriteMaterialInfo MaterialInformation;
		};

		mutable Area2I mBounds;
		mutable TInlineArray<RenderElementData, 2> mCachedRenderElements;
	};

	/** @} */
} // namespace b3d
