//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "2D/BsSpriteMaterial.h"
#include "Math/BsVector2I.h"
#include "Math/BsRect2I.h"
#include "Image/BsColor.h"

namespace bs
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

	/** Contains information about a single sprite render element, including its geometry and material. */
	struct SpriteRenderElementData
	{
		SpriteRenderElementData() = default;

		Vector2* vertices = nullptr;
		Vector2* uvs = nullptr;
		UINT32* indexes = nullptr;
		UINT32 numQuads = 0;
		SpriteMaterialInfo matInfo;
		SpriteMaterial* material = nullptr;
	};

	/** Contains information about a single sprite render elements mesh and material */
	struct SpriteRenderElement
	{
		UINT32 numIndices = 0;
		UINT32 numVertices = 0;

		SpriteMaterialInfo* matInfo = nullptr;
		SpriteMaterial* material = nullptr;
	};

	/**	Generates geometry and contains information needed for rendering a two dimensional element. */
	class BS_EXPORT Sprite
	{
	public:
		Sprite() = default;
		virtual ~Sprite() = default;

		/**
		 * Returns clipped bounds of the sprite.
		 *
		 * @param[in]	offset		Offset that will be added to the returned bounds.
		 * @param[in]	clipRect	Local clip rect that is used for clipping the sprite bounds. (Clipping is done before
		 *							the offset is applied). If clip rect width or height is zero, no clipping is done.
		 *
		 * @return				Clipped sprite bounds.
		 */
		Rect2I GetBounds(const Vector2I& offset, const Rect2I& clipRect) const;

		/**
		 * Returns the number of separate render elements in the sprite. Normally this is 1, but some sprites may consist
		 * of multiple materials, in which case each will require its own mesh (render element)
		 * 			
		 * @return	The number render elements.
		 */
		UINT32 GetNumRenderElements() const { return (UINT32)mCachedRenderElements.Size(); }

		/**
		 * Returns information about the number of vertices and indices the required render element requires, as well
		 * as information about the material that it should be rendered with. Vertex/index counts are required
		 * when creating the buffers before calling fillBuffer().
		 *
		 * Returned data is valid until the next call to update() or until the sprite is destroyed.
		 *
		 * @param[in]		idx			Index of the render element to return the information for.
		 * @param[out]		info		Information about the render element.
		 */
		void GetRenderElementInfo(UINT32 idx, SpriteRenderElement& info) const;

		/**
		 * Fill the pre-allocated vertex, uv and index buffers with the mesh data for the specified render element.
		 *
		 * @param[out]	vertices			Previously allocated buffer where to store the vertices.
		 * @param[out]	uv					Previously allocated buffer where to store the uv coordinates.
		 * @param[out]	indices				Previously allocated buffer where to store the indices.
		 * @param[in]	vertexOffset		At which vertex should the method start filling the buffer.
		 * @param[in]	indexOffset			At which index should the method start filling the buffer.
		 * @param[in]	maxNumVerts			Total number of vertices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param[in]	maxNumIndices		Total number of indices the buffers were allocated for. Used only for memory
		 *									safety.
		 * @param[in]	vertexStride		Number of bytes between of vertices in the provided vertex and uv data.
		 * @param[in]	indexStride			Number of bytes between two indexes in the provided index data.
		 * @param[in]	renderElementIdx	Zero-based index of the render element.
		 * @param[in]	offset				Position offset to apply to all vertices, after clipping.
		 * @param[in]	clipRect			Rectangle to clip the vertices to.
		 * @param[in]	clip				Should the vertices be clipped to the provided @p clipRect.
		 *
		 * @see		getNumRenderElements()
		 * @see		getNumQuads()
		 */
		UINT32 fillBuffer(UINT8* vertices, UINT8* uv, UINT32* indices, UINT32 vertexOffset, UINT32 indexOffset,
			UINT32 maxNumVerts, UINT32 maxNumIndices, UINT32 vertexStride, UINT32 indexStride, UINT32 renderElementIdx,
			const Vector2I& offset, const Rect2I& clipRect, bool clip = true) const;

		/**
		 * Clips the provided 2D vertices to the provided clip rectangle. The vertices must form axis aligned quads.
		 *
		 * @param[in, out]	vertices	Pointer to the start of the buffer containing vertex positions.
		 * @param[in, out]	uv			Pointer to the start of the buffer containing UV coordinates.
		 * @param[in]		numQuads	Number of quads in the provided buffer pointers.
		 * @param[in]		vertStride	Number of bytes to skip when going to the next vertex. This assumes both position
		 *								and uv coordinates have the same stride (as they are likely pointing to the same
		 *								buffer).
		 * @param[in]		clipRect	Rectangle to clip the geometry to.
		 */
		static void ClipQuadsToRect(UINT8* vertices, UINT8* uv, UINT32 numQuads, UINT32 vertStride, const Rect2I& clipRect);

		/**
		 * Clips the provided 2D vertices to the provided clip rectangle. The vertices can be arbitrary triangles.
		 *
		 * @param[in]	vertices		Pointer to the start of the buffer containing vertex positions.
		 * @param[in]	uv				Pointer to the start of the buffer containing UV coordinates. Can be null if UV is
		 *								not needed.
		 * @param[in]	numTris			Number of triangles in the provided buffer pointers.
		 * @param[in]	vertStride		Number of bytes to skip when going to the next vertex. This assumes both position
		 *								and uv coordinates have the same stride (as they are likely pointing to the same
		 *								buffer).
		 * @param[in]	clipRect		Rectangle to clip the geometry to.
		 * @param[in]	writeCallback	Callback that will be triggered when clipped vertices and UV coordinates are
		 *								generated and need to be stored. Vertices are always generate in tuples of three,
		 *								forming a single triangle.
		 */
		static void clipTrianglesToRect(UINT8* vertices, UINT8* uv, UINT32 numTris, UINT32 vertStride,
			const Rect2I& clipRect, const std::function<void(Vector2*, Vector2*, UINT32)>& writeCallback);
	protected:
		/**	Returns the offset needed to move the sprite in order for it to respect the provided anchor. */
		static Vector2I GetAnchorOffset(SpriteAnchor anchor, UINT32 width, UINT32 height);

		/**	Calculates the bounds of all sprite vertices. */
		void UpdateBounds() const;

		mutable Rect2I mBounds;
		mutable Vector<SpriteRenderElementData> mCachedRenderElements;
	};

	inline void Sprite::GetRenderElementInfo(UINT32 idx, SpriteRenderElement& info) const
	{
		SpriteRenderElementData& renderElement = mCachedRenderElements[idx];
		
		info.numVertices = renderElement.numQuads * 4;
		info.numIndices = renderElement.numQuads * 6;
		info.matInfo = &renderElement.matInfo;
		info.material = renderElement.material;
	}

	/** @} */
}
