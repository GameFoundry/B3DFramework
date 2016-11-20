//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsSpriteMaterial.h"
#include "BsVector2I.h"
#include "BsRect2I.h"
#include "BsColor.h"

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
	struct SpriteRenderElement
	{
		SpriteRenderElement()
			:vertices(nullptr), uvs(nullptr), indexes(nullptr), numQuads(0), material(nullptr)
		{ }

		Vector2* vertices;
		Vector2* uvs;
		UINT32* indexes;
		UINT32 numQuads;
		SpriteMaterialInfo matInfo;
		SpriteMaterial* material;
	};

	/**	Generates geometry and contains information needed for rendering a two dimensional element. */
	class BS_EXPORT Sprite
	{
	public:
		Sprite();
		virtual ~Sprite();

		/**
		 * Returns clipped bounds of the sprite.
		 *
		 * @param[in]	offset		Offset that will be added to the returned bounds.
		 * @param[in]	clipRect	Local clip rect that is used for clipping the sprite bounds. (Clipping is done before 
		 *							the offset is applied). If clip rect width or height is zero, no clipping is done.
		 *
		 * @return				Clipped sprite bounds.
		 */
		Rect2I getBounds(const Vector2I& offset, const Rect2I& clipRect) const;

		/**
		 * Returns the number of separate render elements in the sprite. Normally this is 1, but some sprites may consist 
		 * of multiple materials, in which case each will require its own mesh (render element)
		 * 			
		 * @return	The number render elements.
		 */
		UINT32 getNumRenderElements() const;

		/**
		 * Gets material information required for rendering the element at the specified index.
		 *
		 * @see		getNumRenderElements()
		 */
		const SpriteMaterialInfo& getMaterialInfo(UINT32 renderElementIdx) const;

		/**
		 * Gets the material that will be used for rendering the element at the specified index.
		 *
		 * @see		getNumRenderElements()
		 */
		SpriteMaterial* getMaterial(UINT32 renderElementIdx) const;

		/**
		 * Returns the number of quads that the specified render element will use. You will need this value when creating
		 * the buffers before calling fillBuffer().
		 * 			
		 * @return	Number of quads for the specified render element. 
		 *	
		 * @note	Number of vertices = Number of quads * 4
		 *			Number of indices = Number of quads * 6
		 *			
		 * @see		getNumRenderElements()
		 * @see		fillBuffer()
		 */
		UINT32 getNumQuads(UINT32 renderElementIdx) const;

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
		static void clipQuadsToRect(UINT8* vertices, UINT8* uv, UINT32 numQuads, UINT32 vertStride, const Rect2I& clipRect);

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
		static Vector2I getAnchorOffset(SpriteAnchor anchor, UINT32 width, UINT32 height);

		/**	Calculates the bounds of all sprite vertices. */
		void updateBounds() const;

		mutable Rect2I mBounds;
		mutable Vector<SpriteRenderElement> mCachedRenderElements;
	};

	/** @} */
}