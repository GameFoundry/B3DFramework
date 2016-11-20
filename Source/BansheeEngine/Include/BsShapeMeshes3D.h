//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsColor.h"
#include "BsAABox.h"
#include "BsRect3.h"

namespace bs
{
	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/**	Helper class for easily creating common 3D shapes. */
	class BS_EXPORT ShapeMeshes3D
	{
	public:
		/**
		 * Fills the mesh data with vertices representing an outline of axis aligned box.
		 *
		 * @param[in]		box				Axis aligned box to create the mesh data for
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *  32bit index buffer
		 *  Enough space for 8 vertices and 24 indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void wireAABox(const AABox& box, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing a solid axis aligned box.
		 *
		 * @param[in]		box				Axis aligned box to create the mesh data for
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *	Vector3 VES_NORMAL
		 * 	32bit index buffer
		 * 	Enough space for 24 vertices and 36 indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void solidAABox(const AABox& box, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing an outline of a sphere.
		 *
		 * @param[in]		sphere			Sphere to get the mesh data for
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the sphere will have. Higher level means 
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *  32bit index buffer
		 *  Enough space for 3 * ((quality + 1) * 5) vertices 
		 *	Enough space for 6 * ((quality + 1) * 5 - 1) indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void wireSphere(const Sphere& sphere, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset, 
			UINT32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing a sphere.
		 *
		 * @param[in]		sphere			Sphere to get the mesh data for
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the sphere will have. Higher level means 
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *	Vector3 VES_NORMAL
		 * 	32bit index buffer
		 * 	Enough space for 20 * (4 * (3 ^ quality)) vertices 
		 *	Enough space for 20 * (4 * (3 ^ quality)) indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void solidSphere(const Sphere& sphere, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset, 
			UINT32 quality = 1);

		/**
		 * Fills the mesh data with vertices representing an outline of an arc.
		 *
		 * @param[in]		center			Center of the arc to generate geometry for.
		 * @param[in]		radius			Radius of the arc to generate geometry for.
		 * @param[in]		normal			Normal around which the arc is generated. Arc geometry will be perpendicular
		 *									to the normal.
		 * @param[in]		startAngle		Angle at which the arc starts.
		 * @param[in]		amountAngle		Angle that the arc spans.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the arc will have. Higher level means
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *  32bit index buffer
		 *  Enough space for ((quality + 1) * 5) vertices 
		 *	Enough space for (((quality + 1) * 5 - 1) * 2) indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void wireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle,
			const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing a solid double-sided arc.
		 *
		 * @param[in]		center			Center of the arc to generate geometry for.
		 * @param[in]		radius			Radius of the arc to generate geometry for.
		 * @param[in]		normal			Normal around which the arc is generated. Arc geometry will be perpendicular to
		 *									the normal.
		 * @param[in]		startAngle		Angle at which the arc starts.
		 * @param[in]		amountAngle		Angle that the arc spans.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the arc will have. Higher level means higher 
		 *									quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *	Vector3 VES_NORMAL
		 * 	32bit index buffer
		 * 	Enough space for ((quality + 1) * 5 + 1) * 2 vertices 
		 *	Enough space for (((quality + 1) * 5 - 1) * 6) indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void solidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle,
			const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing an outline of a disc.
		 *
		 * @param[in]		center			Center of the disc to generate geometry for.
		 * @param[in]		radius			Radius of the disc to generate geometry for.
		 * @param[in]		normal			Normal around which the disc is generated. Disc geometry will be perpendicular
		 *									to the normal.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the disc will have. Higher level means
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 * 	Vector3 VES_POSITION
		 * 	32bit index buffer
		 * 	Enough space for ((quality + 1) * 5) vertices 
		 *	Enough space for (((quality + 1) * 5 - 1) * 2) indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void wireDisc(const Vector3& center, float radius, const Vector3& normal, const SPtr<MeshData>& meshData,
			UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing a solid double-sided disc.
		 *
		 * @param[in]		center			Center of the disc to generate geometry for.
		 * @param[in]		radius			Radius of the disc to generate geometry for.
		 * @param[in]		normal			Normal around which the disc is generated. Disc geometry will be perpendicular 
		 *									to the normal.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the disc will have. Higher level means higher
		 *									quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *	Vector3 VES_NORMAL
		 * 	32bit index buffer
		 * 	Enough space for ((quality + 1) * 5 + 1) * 2 vertices 
		 *	Enough space for (((quality + 1) * 5 - 1) * 6) indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void solidDisc(const Vector3& center, float radius, const Vector3& normal, const SPtr<MeshData>& meshData,
			UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing an outline of an camera frustum. Frustum will be facing -z and be
		 * positioned at world origin.
		 *
		 * @param[in]		position		Starting point for the frustum.
		 * @param[in]		aspect			Aspect ratio (width / height).
		 * @param[in]		FOV				Horizontal field of view angle.
		 * @param[in]		near			Distance to near clipping plane.
		 * @param[in]		far				Distance to far clipping plane.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *  32bit index buffer
		 *  Enough space for 8 vertices and 24 indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void wireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far,
			const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing a wireframe cone.
		 *
		 * @param[in]		base			World position of the cone base.
		 * @param[in]		normal			Direction of the pointed part of the cone.
		 * @param[in]		height			Cone height (distance from base to the top).
		 * @param[in]		radius			Cone radius (distance from base center to outer edge).
		 * @param[in]		scale			Scale to apply to the x/y axes, allowing you to create elliptical cones.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the cone will have. Higher level means 
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 * 	32bit index buffer
		 * 	Enough space for ((quality + 1) * 4 + 5) vertices
		 *	Enough space for (((quality + 1) * 4 + 4) * 2) indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void wireCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale,
			const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing a solid cone.
		 *
		 * @param[in]		base			World position of the cone base.
		 * @param[in]		normal			Direction of the pointed part of the cone.
		 * @param[in]		height			Cone height (distance from base to the top).
		 * @param[in]		radius			Cone radius (distance from base center to outer edge).
		 * @param[in]		scale			Scale to apply to the x/y axes, allowing you to create elliptical cones.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the cone will have. Higher level means 
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *	Vector3 VES_NORMAL
		 * 	32bit index buffer
		 * 	Enough space for ((quality + 1) * 4) * 3 + 1 vertices 
		 *	Enough space for (((quality + 1) * 4) * 6) indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void solidCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale,
			const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset, UINT32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing a quad (4 triangles, two sided).
		 *
		 * @param[in]		area			Area in which to draw the quad.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *	Vector3 VES_NORMAL
		 * 	32bit index buffer
		 * 	Enough space for 8 vertices and 12 indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void solidQuad(const Rect3& area, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing a per-pixel line.
		 *
		 * @param[in]		a				Start point of the line.
		 * @param[in]		b				End point of the line.
		 * @param[in, out]	meshData	Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3	  VES_POSITION
		 *  32bit index buffer
		 *  Enough space for 2 vertices and 2 indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void pixelLine(const Vector3& a, const Vector3& b, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing an anti-aliased line of specific width. Antialiasing is done 
		 * using alpha blending.
		 *
		 * @param[in]		a				Start point of the line.
		 * @param[in]		b				End point of the line.
		 * @param[in]		up				Up direction to which the line will run perpendicular to.
		 * @param[in]		width			Width of the line.
		 * @param[in]		borderWidth		Width of the anti-aliased border.
		 * @param[in]		color			Color of the line.
		 * @param[in, out]	meshData	Mesh data that will be populated by this method.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *  UINT32  VES_COLOR
		 *  32bit index buffer
		 *	Enough space for 8 vertices and 30 indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void antialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, const Color& color,
			const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing per-pixel lines.
		 *
		 * @param[in]		linePoints		A list of start and end points for the lines. Must be a multiple of 2.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3  VES_POSITION
		 *  32bit index buffer
		 *  Enough space for (numLines * 2) vertices and (numLines * 2) indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void pixelLineList(const Vector<Vector3>& linePoints, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the mesh data with vertices representing anti-aliased lines of specific width. Antialiasing is done using
		 * alpha blending.
		 *
		 * @param[in]		linePoints		A list of start and end points for the lines. Must be a multiple of 2.
		 * @param[in]		up				Up direction to which the line will run perpendicular to.
		 * @param[in]		width			Width of the line.
		 * @param[in]		borderWidth		Width of the anti-aliased border.
		 * @param[in]		color			Color of the line.
		 * @param[in, out]	meshData		Mesh data that will be populated by this method.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * 							
		 * @note	
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *  UINT32  VES_COLOR
		 *  32bit index buffer
		 *	Enough space for (numLines * 8) vertices and (numLines * 30) indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void antialiasedLineList(const Vector<Vector3>& linePoints, const Vector3& up, float width, float borderWidth,
			const Color& color, const SPtr<MeshData>& meshData, UINT32 vertexOffset, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with position and index data representing an outline of an axis aligned box. Use 
		 * getNumElementsWireAABox() to determine the required sizes of the output buffers.
		 *
		 * @param[in]	box				Box to create geometry for.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes.
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void wireAABox(const AABox& box, UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride,
			UINT32* outIndices, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with position and index data representing a solid axis aligned box. Use 
		 * getNumElementsAABox() to determine the required sizes of the output buffers.
		 *
		 * @param[in]	box				Box to create geometry for.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void solidAABox(const AABox& box, UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride,
			UINT32* outIndices, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with position and index data representing a sphere. Use getNumElementsSphere() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	sphere			Sphere to create geometry for.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data. Can be null if
		 *								normals aren't needed.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the sphere will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void solidSphere(const Sphere& sphere, UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride,
			UINT32* outIndices, UINT32 indexOffset, UINT32 quality);

		/**
		 * Fills the provided buffers with position and index data representing an outline of an arc. Use
		 * getNumElementWiresArc() to determine the required sizes of the output buffers.
		 *
		 * @param[in]	center			Center of the arc to generate geometry for.
		 * @param[in]	radius			Radius of the arc to generate geometry for.
		 * @param[in]	normal			Normal around which the arc is generated. Arc geometry will be perpendicular to the
		 *								normal.
		 * @param[in]	startAngle		Angle at which the arc starts.
		 * @param[in]	amountAngle		Angle that the arc spans.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes.
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the arc will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void wireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle,
			UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset, UINT32 quality);

		/**
		 * Fills the provided buffers with position and index data representing a solid arc. Use getNumElementsArc() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	center			Center of the arc to generate geometry for.
		 * @param[in]	radius			Radius of the arc to generate geometry for.
		 * @param[in]	normal			Normal around which the arc is generated. Arc geometry will be perpendicular to the
		 *								normal.
		 * @param[in]	startAngle		Angle at which the arc starts.
		 * @param[in]	amountAngle		Angle that the arc spans.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the arc will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void solidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle,
			UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset, UINT32 quality);

		/**
		 * Fills the provided buffers with position and index data representing an outline of a camera frustum. Use
		 * getNumElementsFrustum() to determine the required sizes of the output buffers.
		 *
		 * @param[in]	position		Starting point for the frustum.
		 * @param[in]	aspect			Aspect ratio (width / height).
		 * @param[in]	FOV				Horizontal field of view angle.
		 * @param[in]	near			Distance to near clipping plane.
		 * @param[in]	far				Distance to far clipping plane.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes.
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void wireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far,
			UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with position and index data representing a solid cone. Use getNumElementsCone() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	base			World position of the cone base.
		 * @param[in]	normal			Direction of the pointed part of the cone.
		 * @param[in]	height			Cone height (distance from base to the top).
		 * @param[in]	radius			Cone radius (distance from base center to outer edge).
		 * @param[in]	scale			Scale to apply to the x/y axes, allowing you to create elliptical cones.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data. Can be null if
		 *								normals aren't needed.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the cone will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void solidCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale,
			UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, 
			UINT32 indexOffset, UINT32 quality);

		/**
		 * Fills the provided buffers with position and index data representing a wire cone. Use getNumElementsWireCone() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	base			World position of the cone base.
		 * @param[in]	normal			Direction of the pointed part of the cone.
		 * @param[in]	height			Cone height (distance from base to the top).
		 * @param[in]	radius			Cone radius (distance from base center to outer edge).
		 * @param[in]	scale			Scale to apply to the x/y axes, allowing you to create elliptical cones.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the cone will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void wireCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale,
			UINT8* outVertices, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset, 
			UINT32 quality);

		/**
		 * Fills the provided buffers with position and index data representing a solid quad. Use getNumElementsCone() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	area			Area covered by the quad.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void solidQuad(const Rect3& area, UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**	Calculates number of vertices and indices required for geometry of a solid axis aligned box. */
		static void getNumElementsAABox(UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wireframe axis aligned box. */
		static void getNumElementsWireAABox(UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid sphere of the specified quality. */
		static void getNumElementsSphere(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wire sphere of the specified quality. */
		static void getNumElementsWireSphere(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid arc of the specified quality. */
		static void getNumElementsArc(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wire arc of the specified quality. */
		static void getNumElementsWireArc(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid desc	of the specified quality. */
		static void getNumElementsDisc(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wire disc of the specified quality. */
		static void getNumElementsWireDisc(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid cone of the specified quality. */
		static void getNumElementsCone(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wireframe cone of the specified quality. */
		static void getNumElementsWireCone(UINT32 quality, UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a frustum. */
		static void getNumElementsFrustum(UINT32& numVertices, UINT32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a quadrirateral. */
		static void getNumElementsQuad(UINT32& numVertices, UINT32& numIndices);

		static const UINT32 NUM_VERTICES_AA_LINE;
		static const UINT32 NUM_INDICES_AA_LINE;

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
		static void pixelLine(const Vector3& a, const Vector3& b, UINT8* outVertices,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with vertices representing an antialiased line with a custom width.
		 *
		 * @param[in]	a				Start point of the line.
		 * @param[in]	b				End point of the line.
		 * @param[in]	up				Up direction to which the line will run perpendicular to.
		 * @param[in]	width			Width of the line.
		 * @param[in]	borderWidth		Width of the anti-aliased border.
		 * @param[in]	color			Color of the line.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[out]	outColors		Output buffer that will store the vertex color data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param[out]	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void antialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, 
			const Color& color, UINT8* outVertices, UINT8* outColors, UINT32 vertexOffset, UINT32 vertexStride, 
			UINT32* outIndices, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with position data and indices representing an inner area of a polygon (basically a
		 * normal non-antialiased polygon).
		 *
		 * @param[in]	points			Points defining the polygon. First point is assumed to be the start and end point.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param[out]	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void pixelSolidPolygon(const Vector<Vector3>& points, UINT8* outVertices,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with vertices representing a pixel-wide polygon border.
		 *
		 * @param[in]	points			Points defining the polygon. First point is assumed to be the start and end point.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param[out]	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void pixelWirePolygon(const Vector<Vector3>& points, UINT8* outVertices,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

		/**
		 * Fills the provided buffers with vertices representing an antialiased polygon.
		 *
		 * @param[in]	points			Points defining the polygon. First point is assumed to be the start and end point.
		 * @param[in]	up				Up direction to which the polygon will run perpendicular to.
		 * @param[in]	borderWidth		Width of the anti-aliased border.
		 * @param[in]	color			Color of the polygon.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[out]	outColors		Output buffer that will store the vertex color data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @param[out]	outIndices		Output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void antialiasedPolygon(const Vector<Vector3>& points, const Vector3& up, float borderWidth, const Color& color, UINT8* outVertices, UINT8* outColors,
			UINT32 vertexOffset, UINT32 vertexStride, UINT32* outIndices, UINT32 indexOffset);

	private:
		/**
		 * Calculates the center of the provided vertices.
		 * 
		 * @param[in]	vertices		Buffer containing vertices. Vertices must be of three dimensions at least.
		 * @param[in]	numVertices		Number of vertices to calculate the center for.
		 * @param[in]	vertexStride	Number of bytes between two vertices in the buffer.
		 * @return						Center point of the vertices.
		 */
		static Vector3 calcCenter(UINT8* vertices, UINT32 numVertices, UINT32 vertexStride);

		/**
		 * Subdivides the provided triangle so it approximates a curved surface of a sphere.
		 *
		 * @param[in]	center			Center of the sphere to approximate.
		 * @param[in]	radius			Radius of the sphere to approximate.
		 * @param[in]	numLevels		Number of times to subdivide the triangle. Higher number means better approximation.
		 * @param[in]	a				First corner of the triangle. Must be normalized.
		 * @param[in]	b				Second corner of the triangle. Must be normalized.
		 * @param[in]	c				Third corner of the triangle. Must be normalized.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Output buffer that will store the vertex normal data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 * @return						Number of vertices generated (3 * (4 ^ numLevels)).
		 */
		static UINT32 subdivideTriangleOnSphere(const Vector3& center, float radius, UINT32 numLevels,
			const Vector3& a, const Vector3& b, const Vector3& c,
			UINT8* outVertices, UINT8* outNormals, UINT32 vertexOffset, UINT32 vertexStride);

		/**
		 * Generates vertices that are part of an arc approximation.
		 *
		 * @param[in]	center			Determines world position of the arc.
		 * @param[in]	up				Determines rotation of the arc. Arc vertices will be perpendicular to this 
		 *								direction.
		 * @param[in]	radius			Distance of arc vertices from the center.
		 * @param[in]	startAngle		Angle in degrees to start the arc at.
		 * @param[in]	angleAmount		Angle in degrees to extend the arc from the start angle.
		 * @param[in]	scale			Scale to apply to the x/y axes, allowing you to create elliptical arcs.
		 * @param[in]	numVertices		Number of vertices to generate for the arc. Higher number means better arc 
		 *								approximation. Must be 2 or higher.
		 * @param[out]	outVertices		Output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and color buffer)
		 */
		static void generateArcVertices(const Vector3& center, const Vector3& up, float radius, Degree startAngle, 
			Degree angleAmount, Vector2 scale, UINT32 numVertices, UINT8* outvertices, UINT32 vertexOffset, UINT32 vertexStride);
	};

	/** @} */
}