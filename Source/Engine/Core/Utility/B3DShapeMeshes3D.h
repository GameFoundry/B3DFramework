//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Image/B3DColor.h"
#include "Math/B3DAABox.h"
#include "Math/B3DRect3.h"

namespace b3d
{
	/** @addtogroup Mesh-Internal
	 *  @{
	 */

	/**	Helper class for easily creating common 3D shapes. */
	class B3D_EXPORT ShapeMeshes3D
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
		static void WireAaBox(const AABox& box, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		 * Optionally it may also have:
		 *  Vector2 VES_TEXCOORD
		 *  Vector4 VES_TANGENT
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void SolidAaBox(const AABox& box, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		static void WireSphere(const Sphere& sphere, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing an outline of a hemisphere.
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
		static void WireHemisphere(const Sphere& sphere, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

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
		 * Optionally it may also have:
		 *  Vector2 VES_TEXCOORD
		 *  Vector4 VES_TANGENT
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void SolidSphere(const Sphere& sphere, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 1);

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
		static void WireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

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
		 * Optionally it may also have:
		 *  Vector2 VES_TEXCOORD
		 *  Vector4 VES_TANGENT
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void SolidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

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
		static void WireDisc(const Vector3& center, float radius, const Vector3& normal, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

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
		static void SolidDisc(const Vector3& center, float radius, const Vector3& normal, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

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
		static void WireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		static void WireCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

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
		 * Optionally it may also have:
		 *  Vector2 VES_TEXCOORD
		 *  Vector4 VES_TANGENT
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void SolidCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing a wireframe cylinder.
		 *
		 * @param[in]		base			World position of the cylinder base.
		 * @param[in]		normal			Orientation of the cylinder (height gets applied in this direction).
		 * @param[in]		height			Cylinder height (distance from base to the top).
		 * @param[in]		radius			Cylinder radius (distance from base center to outer edge).
		 * @param[in]		scale			Scale to apply to the x/y axes, allowing you to create elliptical cylinders.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the cylinder will have. Higher level means
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 * 	32bit index buffer
		 * 	Enough space for ((quality + 1) * 4) * 2) vertices
		 *	Enough space for ((quality + 1) * 4) * 6) indices
		 * @note
		 * Primitives are output in the form of a line list.
		 */
		static void WireCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

		/**
		 * Fills the mesh data with vertices representing a solid cylinder.
		 *
		 * @param[in]		base			World position of the cylinder base.
		 * @param[in]		normal			Orientation of the cylinder (height gets applied in this direction).
		 * @param[in]		height			Cylinder height (distance from base to the top).
		 * @param[in]		radius			Cylinder radius (distance from base center to outer edge).
		 * @param[in]		scale			Scale to apply to the x/y axes, allowing you to create elliptical cylinders.
		 * @param[in, out]	meshData		Mesh data that will be populated.
		 * @param[in]		vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]		indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]		quality			Represents the level of tessellation the cylinder will have. Higher level means
		 *									higher quality but also more vertices and primitives.
		 *
		 * @note
		 * Provided MeshData must have some specific elements at least:
		 *  Vector3 VES_POSITION
		 *	Vector3 VES_NORMAL
		 * 	32bit index buffer
		 * 	Enough space for ((quality + 1) * 4 + 1) * 4) vertices
		 *	Enough space for ((quality + 1) * 4) * 12) indices
		 * Optionally it may also have:
		 *  Vector2 VES_TEXCOORD
		 *  Vector4 VES_TANGENT
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void SolidCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset, u32 quality = 10);

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
		 * Optionally it may also have:
		 *  Vector2 VES_TEXCOORD
		 *  Vector4 VES_TANGENT
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void SolidQuad(const Rect3& area, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		static void PixelLine(const Vector3& a, const Vector3& b, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		 *  u32  VES_COLOR
		 *  32bit index buffer
		 *	Enough space for 8 vertices and 30 indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void AntialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, const Color& color, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		static void PixelLineList(const Vector<Vector3>& linePoints, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		 *  u32  VES_COLOR
		 *  32bit index buffer
		 *	Enough space for (numLines * 8) vertices and (numLines * 30) indices
		 * @note
		 * Primitives are output in the form of a triangle list.
		 */
		static void AntialiasedLineList(const Vector<Vector3>& linePoints, const Vector3& up, float width, float borderWidth, const Color& color, const SPtr<MeshData>& meshData, u32 vertexOffset, u32 indexOffset);

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
		static void WireAaBox(const AABox& box, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

		/**
		 * Fills the provided buffers with position and index data representing a solid axis aligned box. Use
		 * getNumElementsAABox() to determine the required sizes of the output buffers.
		 *
		 * @param[in]	box				Box to create geometry for.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data.
		 * @param[out]	outUV			Pre-allocated output buffer that will store the vertex UV data. Set to null if not
		 *								required.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void SolidAaBox(const AABox& box, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

		/**
		 * Fills the provided buffers with position and index data representing a sphere. Use getNumElementsSphere() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	sphere			Sphere to create geometry for.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data. Can be null if
		 *								normals aren't needed.
		 * @param[out]	outUV			Pre-allocated output buffer that will store the vertex UV data. Set to null if not
		 *								required.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the sphere will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void SolidSphere(const Sphere& sphere, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality);

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
		static void WireArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality);

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
		 * @param[out]	outUV			Pre-allocated output buffer that will store the vertex UV data. Set to null if not
		 *								required.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the arc will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void SolidArc(const Vector3& center, float radius, const Vector3& normal, Degree startAngle, Degree amountAngle, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality);

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
		static void WireFrustum(const Vector3& position, float aspect, Degree FOV, float near, float far, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

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
		 * @param[out]	outUV			Pre-allocated output buffer that will store the vertex UV data. Set to null if not
		 *								required.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the cone will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void SolidCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality);

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
		static void WireCone(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality);

		/**
		 * Fills the provided buffers with position and index data representing a solid cylinder. Use getNumElementsCylinder() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	base			World position of the cylinder base.
		 * @param[in]	normal			Orientation of the cylinder (height gets applied in this direction).
		 * @param[in]	height			Cylinder height (distance from base to the top).
		 * @param[in]	radius			Cylinder radius (distance from base center to outer edge).
		 * @param[in]	scale			Scale to apply to the x/y axes, allowing you to create elliptical cylinders.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data. Can be null if
		 *								normals aren't needed.
		 * @param[out]	outUV			Pre-allocated output buffer that will store the vertex UV data. Set to null if not
		 *								required.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the cylinder will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void SolidCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality);

		/**
		 * Fills the provided buffers with position and index data representing a wire cylinder. Use getNumElementsWireCylinder() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	base			World position of the cylinder base.
		 * @param[in]	normal			Orientation of the cylinder (height gets applied in this direction).
		 * @param[in]	height			Cylinder height (distance from base to the top).
		 * @param[in]	radius			Cylinder radius (distance from base center to outer edge).
		 * @param[in]	scale			Scale to apply to the x/y axes, allowing you to create elliptical cylinders.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 * @param[in]	quality			Represents the level of tessellation the cylinder will have. Higher level means higher
		 *								quality but also more vertices and primitives.
		 */
		static void WireCylinder(const Vector3& base, const Vector3& normal, float height, float radius, Vector2 scale, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset, u32 quality);

		/**
		 * Fills the provided buffers with position and index data representing a solid quad. Use getNumElementsQuad() to
		 * determine the required sizes of the output buffers.
		 *
		 * @param[in]	area			Area covered by the quad.
		 * @param[out]	outVertices		Pre-allocated output buffer that will store the vertex position data.
		 * @param[out]	outNormals		Pre-allocated output buffer that will store the vertex normal data.
		 * @param[out]	outUV			Pre-allocated output buffer that will store the vertex UV data. Set to null if not
		 *								required.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffer to start writing at.
		 * @param[in]	vertexStride	Size of a single vertex, in bytes. (Same for both position and normal buffer)
		 * @param[out]	outIndices		Pre-allocated output buffer that will store the index data. Indices are 32bit.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start writing at.
		 */
		static void SolidQuad(const Rect3& area, u8* outVertices, u8* outNormals, u8* outUV, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

		/**	Calculates number of vertices and indices required for geometry of a solid axis aligned box. */
		static void GetNumElementsAaBox(u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wireframe axis aligned box. */
		static void GetNumElementsWireAaBox(u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid sphere of the specified quality. */
		static void GetNumElementsSphere(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wire sphere of the specified quality. */
		static void GetNumElementsWireSphere(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wire hemisphere of the specified quality. */
		static void GetNumElementsWireHemisphere(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid arc of the specified quality. */
		static void GetNumElementsArc(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wire arc of the specified quality. */
		static void GetNumElementsWireArc(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid desc	of the specified quality. */
		static void GetNumElementsDisc(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wire disc of the specified quality. */
		static void GetNumElementsWireDisc(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid cone of the specified quality. */
		static void GetNumElementsCone(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wireframe cone of the specified quality. */
		static void GetNumElementsWireCone(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a solid cylinder of the specified quality. */
		static void GetNumElementsCylinder(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a wireframe cylinder of the specified quality. */
		static void GetNumElementsWireCylinder(u32 quality, u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a frustum. */
		static void GetNumElementsFrustum(u32& numVertices, u32& numIndices);

		/**	Calculates number of vertices and indices required for geometry of a quadrirateral. */
		static void GetNumElementsQuad(u32& numVertices, u32& numIndices);

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
		static void PixelLine(const Vector3& a, const Vector3& b, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

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
		static void AntialiasedLine(const Vector3& a, const Vector3& b, const Vector3& up, float width, float borderWidth, const Color& color, u8* outVertices, u8* outColors, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

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
		static void PixelSolidPolygon(const Vector<Vector3>& points, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

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
		static void PixelWirePolygon(const Vector<Vector3>& points, u8* outVertices, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

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
		static void AntialiasedPolygon(const Vector<Vector3>& points, const Vector3& up, float borderWidth, const Color& color, u8* outVertices, u8* outColors, u32 vertexOffset, u32 vertexStride, u32* outIndices, u32 indexOffset);

	private:
		/**
		 * Calculates the center of the provided vertices.
		 *
		 * @param[in]	vertices		Buffer containing vertices. Vertices must be of three dimensions at least.
		 * @param[in]	numVertices		Number of vertices to calculate the center for.
		 * @param[in]	vertexStride	Number of bytes between two vertices in the buffer.
		 * @return						Center point of the vertices.
		 */
		static Vector3 CalcCenter(u8* vertices, u32 numVertices, u32 vertexStride);

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
		static u32 SubdivideTriangleOnSphere(const Vector3& center, float radius, u32 numLevels, const Vector3& a, const Vector3& b, const Vector3& c, u8* outVertices, u8* outNormals, u32 vertexOffset, u32 vertexStride);

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
		static void GenerateArcVertices(const Vector3& center, const Vector3& up, float radius, Degree startAngle, Degree angleAmount, Vector2 scale, u32 numVertices, u8* outVertices, u32 vertexOffset, u32 vertexStride);

		/**
		 * Calculates per-vertex tangents and bitangents based on the provided vertices, uv coordinates and indices.
		 *
		 * @param[in]	positions		Pointer to an array of vertex positions.
		 * @param[in]	normals			Pointer to an array of vertex normals.
		 * @param[in]	uv				Pointer to an array of vertex UV coordinates.
		 * @param[in]	indices			Set of 32-bit indices containing indexes into vertex array for each triangle.
		 * @param[in]	numVertices		Number of vertices in the @p vertices, @p normals and @p uv arrays.
		 * @param[in]	numIndices		Number of indices in the @p indices array. Must be a multiple of three.
		 * @param[in]	vertexOffset	Offset in number of vertices from the start of the buffers to start reading/writing.
		 * @param[in]	indexOffset 	Offset in number of indices from the start of the buffer to start reading from.
		 * @param[in]	vertexStride	Number of bytes to advance the @p vertices, @p normals and @p uv arrays with each
		 *								vertex. If set to zero them each array is advanced according to its own size.
		 * @param[out]	tangents		Pre-allocated buffer that will contain the calculated tangents & bitangents packed
		 *								into 4D vector where first three components are the tangent, and 4th is the sign of
		 *								the bitangent. Must be the same length as the vertex array.
		 */
		static void GenerateTangents(u8* positions, u8* normals, u8* uv, u32* indices, u32 numVertices, u32 numIndices, u32 vertexOffset, u32 indexOffset, u32 vertexStride, u8* tangents);
	};

	/** @} */
} // namespace b3d
