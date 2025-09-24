//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "Math/B3DAABox.h"

namespace b3d
{
	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/**	Helper class for easily creating common 2D shapes. */
	class B3D_EXPORT ShapeMeshes2D
	{
	public:
		/**
		 * Fills the mesh data with vertices representing a quad (2 triangles).
		 *
		 * @param[in]	area			Area in which to draw the quad.
		 * @param[out]	meshData		Mesh data that will be populated.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 *
		 * @note
		 * Provided MeshData must have some specific elements at least:
		 * 	Vector2 VES_POSITION
		 * 	32bit index buffer
		 * 	Enough space for 4 vertices and 6 indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void SolidQuad(const Area2& area, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing a per-pixel line.
		 *
		 * @param[in]	a				Start point of the line.
		 * @param[in]	b				End point of the line.
		 * @param[out]	meshData		Mesh data that will be populated.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 *
		 * @note
		 * Provided MeshData must have some specific elements at least:
		 *	Vector2 VES_POSITION
		 * 	32bit index buffer
		 * 	Enough space for 2 vertices and 2 indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void PixelLine(const Vector2& a, const Vector2& b, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing a line of specific width as a quad.
		 *
		 * @param[in]	a				Start point of the line.
		 * @param[in]	b				End point of the line.
		 * @param[in]	width			Width of the line.
		 * @param[in]	border			Optional border that will increase the width and the length at both end-points.
		 *								Useful if you are using some kind of filtering for the line rendering, as the
		 *								filtered pixels can belong to the border region.
		 * @param[in]	color			Color of the line.
		 * @param[out]	meshData		Mesh data that will be populated by this method.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 *
		 * @note
		 * Provided MeshData must have some specific elements at least:
		 *  Vector2 VES_POSITION
		 *  u32  VES_COLOR
		 *  32bit index buffer
		 *	Enough space for 4 vertices and 6 indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void QuadLine(const Vector2& a, const Vector2& b, float width, float border, const Color& color, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing per-pixel lines.
		 *
		 * @param[in]	linePoints		A list of start and end points for the lines. Must be a multiple of 2.
		 * @param[out]	meshData		Mesh data that will be populated.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 *
		 * @note
		 * Provided MeshData must have some specific elements at least:
		 *  Vector2  VES_POSITION
		 *  32bit index buffer
		 *  Enough space for (numLines * 2) vertices and (numLines * 2) indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void PixelLineList(const Vector<Vector2>& linePoints, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing a polyline of specific width as a set of quads.
		 *
		 * @param[in]	linePoints		A list of start and end points for the lines.
		 * @param[in]	width			Width of the line.
		 * @param[in]	border			Optional border that will increase the width and the length at both end-points.
		 *								Useful if you are using some kind of filtering for the line rendering, as the
		 *								filtered pixels can belong to the border region.
		 * @param[in]	color			Color of the line.
		 * @param[out]	meshData		Mesh data that will be populated by this method.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 *
		 * @note
		 * Provided MeshData must have some specific elements at least:
		 *  Vector2 VES_POSITION
		 *  u32  VES_COLOR
		 *  32bit index buffer
		 *	Enough space for (numLines * 2) + 2 vertices and numLines * 6 indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void QuadLineList(const Vector<Vector2>& linePoints, float width, float border, const Color& color, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

		/**
		 * Fills the provided buffers with vertices representing a polyline of specific width as a set of quads
		 * (triangle list).
		 *
		 * @param[in]	linePoints		A list of start and end points for the lines.
		 * @param[in]	numPoints		Number of points in the @p linePoints buffer.
		 * @param[in]	width			Width of the line.
		 * @param[in]	border			Optional border that will increase the width and the length at both end-points.
		 *								Useful if you are using some kind of filtering for the line rendering, as the
		 *								filtered pixels can belong to the border region.
		 * @param[out]	outVertices		Pre-allocated buffer for the vertices, of size ((numLines * 2) + 2) * @p vertexStride
		 *								if @p indexed is true, or (numLines * 6) * @p vertexStride if false.
		 * @param[in]	vertexStride	Distance between two vertices in the output buffer. Must be at least sizeof(Vector2).
		 * @param[in]	indexed			If true there will be ((numLines * 2) + 2) vertices generated, assuming an index
		 *								buffer will be used for rendering. If false then (numLines * 6) vertices will be
		 *								generated.
		 */
		static void QuadLineList(const Vector2* linePoints, u32 numPoints, float width, float border, u8* outVertices, u32 vertexStride, bool indexed);

		static const u32 kNumVerticesAaLine;
		static const u32 kNumIndicesAaLine;

	protected:
		/**
		 * Fills the provided buffers with vertices representing a per-pixel line.
		 *
		 * @param[in]	a				Start point of the line.
		 * @param[in]	b				End point of the line.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param[out]	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void PixelLine(const Vector2& a, const Vector2& b, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

		/**
		 * Fills the provided buffers with position data and indices representing an inner
		 *			area of a polygon (basically a normal non-antialiased polygon).
		 *
		 * @param[in]	points			Points defining the polygon. First point is assumed to be the start and end point.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param[out]	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void PixelSolidPolygon(const Vector<Vector2>& points, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);
	};

	/** @} */
} // namespace b3d
