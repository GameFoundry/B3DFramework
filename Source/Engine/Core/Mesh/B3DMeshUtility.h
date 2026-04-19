//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Math/B3DVector3.h"

namespace b3d
{
	/** @addtogroup Mesh-Internal
	 *  @{
	 */

	/** Normal packed in a 32-bit structure. */
	union PackedNormal
	{
		struct
		{
			u8 X;
			u8 Y;
			u8 Z;
			u8 W;
		};

		u32 Packed;
	};

	/** Performs various operations on mesh geometry. */
	class B3D_EXPORT MeshUtility
	{
	public:
		/**
		 * Calculates per-vertex normals based on the provided vertices and indices.
		 *
		 * @param	vertices	Set of vertices containing vertex positions.
		 * @param	indices		Set of indices containing indexes into vertex array for each triangle.
		 * @param	vertexCount	Number of vertices in the @p vertices array.
		 * @param	indexCount	Number of indices in the @p indices array. Must be a multiple of three.
		 * @param	normals		Pre-allocated buffer that will contain the calculated normals. Must be the same size
		 *							as the vertex array.
		 * @param	indexSize	Size of a single index in the indices array, in bytes.
		 *
		 * @note
		 * Vertices should be split before calling this method if there are any discontinuities. (for example a vertex on a
		 * corner of a cube should be split into three vertices used by three triangles in order for the normals to be
		 * valid.)
		 */
		static void CalculateNormals(Vector3* vertices, u8* indices, u32 vertexCount, u32 indexCount, Vector3* normals, u32 indexSize = 4);

		/**
		 * Calculates per-vertex tangents and bitangents based on the provided vertices, uv coordinates and indices.
		 *
		 * @param	vertices		Set of vertices containing vertex positions.
		 * @param	normals			Set of normals to use when calculating tangents. Must the the same length as the
		 *								number of vertices.
		 * @param	uv				Set of UV coordinates to use when calculating tangents. Must the the same length as
		 *								the number of vertices.
		 * @param	indices			Set of indices containing indexes into vertex array for each triangle.
		 * @param	vertexCount		Number of vertices in the @p vertices, @p normals and @p uv arrays.
		 * @param	indexCount		Number of indices in the @p indices array. Must be a multiple of three.
		 * @param	tangents		Pre-allocated buffer that will contain the calculated tangents. Must be the same
		 *								size as the vertex array.
		 * @param	bitangents		Pre-allocated buffer that will contain the calculated bitangents. Must be the same
		 *								size as the vertex array.
		 * @param	indexSize		Size of a single index in the indices array, in bytes.
		 * @param	vertexStride	Number of bytes to advance the @p vertices, @p normals and @p uv arrays with each
		 *								vertex. If set to zero them each array is advanced according to its own size.
		 *
		 * @note
		 * Vertices should be split before calling this method if there are any discontinuities. (for example a vertex on a
		 * corner of a cube should be split into three vertices used by three triangles in order for the normals to be
		 * valid.)
		 */
		static void CalculateTangents(Vector3* vertices, Vector3* normals, Vector2* uv, u8* indices, u32 vertexCount, u32 indexCount, Vector3* tangents, Vector3* bitangents, u32 indexSize = 4, u32 vertexStride = 0);

		/**
		 * Calculates per-vertex tangent space (normal, tangent, bitangent) based on the provided vertices, uv coordinates
		 * and indices.
		 *
		 * @param	vertices	Set of vertices containing vertex positions.
		 * @param	uv			Set of UV coordinates to use when calculating tangents.
		 * @param	indices		Set of indices containing indexes into vertex array for each triangle.
		 * @param	vertexCount	Number of vertices in the "vertices" array.
		 * @param	indexCount	Number of indices in the "indices" array. Must be a multiple of three.
		 * @param	normals		Pre-allocated buffer that will contain the calculated normals. Must be the same size
		 *							as the vertex array.
		 * @param	tangents	Pre-allocated buffer that will contain the calculated tangents. Must be the same size
		 *							as the vertex array.
		 * @param	bitangents	Pre-allocated buffer that will contain the calculated bitangents. Must be the same size
		 *							as the vertex array.
		 * @param	indexSize	Size of a single index in the indices array, in bytes.
		 *
		 * @note
		 * Vertices should be split before calling this method if there are any discontinuities. (for example. a vertex on
		 * a corner of a cube should be split into three vertices used by three triangles in order for the normals to be
		 * valid.)
		 */
		static void CalculateTangentSpace(Vector3* vertices, Vector2* uv, u8* indices, u32 vertexCount, u32 indexCount, Vector3* normals, Vector3* tangents, Vector3* bitangents, u32 indexSize = 4);

		/**
		 * Clips a set of two-dimensional vertices and uv coordinates against a set of arbitrary planes.
		 *
		 * @param	vertices			A set of vertices in Vector2 format. Each vertex should be @p vertexStride bytes
		 *									from each other.
		 * @param	uvs					A set of UV coordinates in Vector2 format. Each coordinate should be
		 *									@p vertexStride bytes from each other. Can be null if UV is not needed.
		 * @param	triangleCount		Number of triangles to clip (must be number of vertices/uvs / 3).
		 * @param	vertexStride		Distance in bytes between two separate vertex or UV values in the provided
		 *									@p vertices and @p uvs buffers.
		 * @param	clipPlanes			A set of planes to clip the vertices against. Since the vertices are
		 *									two-dimensional the plane's Z coordinate should be zero.
		 * @param	writeCallback		Callback that will be triggered when clipped vertices and UV coordinates are
		 *									generated and need to be stored. Vertices are always generate in tuples of
		 *									three, forming a single triangle.
		 */
		static void Clip2D(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector2*, Vector2*, u32)>& writeCallback);

		/**
		 * Clips a set of three-dimensional vertices and uv coordinates against a set of arbitrary planes.
		 *
		 * @param	vertices			A set of vertices in Vector3 format. Each vertex should be @p vertexStride bytes
		 *									from each other.
		 * @param	uvs					A set of UV coordinates in Vector2 format. Each coordinate should be
		 *									@p vertexStride bytes from each other. Can be null if UV is not needed.
		 * @param	triangleCount		Number of triangles to clip (must be number of vertices/uvs / 3).
		 * @param	vertexStride		Distance in bytes between two separate vertex or UV values in the provided
		 *									@p vertices and @p uvs buffers.
		 * @param	clipPlanes			A set of planes to clip the vertices against.
		 * @param	writeCallback		Callback that will be triggered when clipped vertices and UV coordinates are
		 *									generated and need to be stored. Vertices are always generate in tuples of
		 *									three, forming a single triangle.
		 */
		static void Clip3D(u8* vertices, u8* uvs, u32 triangleCount, u32 vertexStride, const Vector<Plane>& clipPlanes, const std::function<void(Vector3*, Vector2*, u32)>& writeCallback);

		/**
		 * Encodes normals from 32-bit float format into 4D 8-bit packed format.
		 *
		 * @param	source			Buffer containing data to encode. Must have @p count entries.
		 * @param	destination		Buffer to output the data to. Must have @p count entries, each 32-bits.
		 * @param	count			Number of entries in the @p source and @p destination arrays.
		 * @param	inputStride		Distance between two entries in the @p source buffer, in bytes.
		 * @param	outputStride	Distance between two entries in the @p destination buffer, in bytes.
		 */
		static void PackNormals(Vector3* source, u8* destination, u32 count, u32 inputStride, u32 outputStride);

		/**
		 * Encodes normals from 32-bit float format into 4D 8-bit packed format.
		 *
		 * @param	source			Buffer containing data to encode. Must have @p count entries.
		 * @param	destination		Buffer to output the data to. Must have @p count entries, each 32-bits.
		 * @param	count			Number of entries in the @p source and @p destination arrays.
		 * @param	inputStride		Distance between two entries in the @p source buffer, in bytes.
		 * @param	outputStride	Distance between two entries in the @p destination buffer, in bytes.
		 */
		static void PackNormals(Vector4* source, u8* destination, u32 count, u32 inputStride, u32 outputStride);

		/**
		 * Decodes normals from 4D 8-bit packed format into a 32-bit float format.
		 *
		 * @param	source			Buffer containing data to encode. Must have @p count entries, each 32-bits.
		 * @param	destination		Buffer to output the data to. Must have @p count entries.
		 * @param	count			Number of entries in the @p source and @p destination arrays.
		 * @param	stride			Distance between two entries in the @p source buffer, in bytes.
		 */
		static void UnpackNormals(u8* source, Vector3* destination, u32 count, u32 stride);

		/**
		 * Decodes normals from 4D 8-bit packed format into a 32-bit float format.
		 *
		 * @param	source			Buffer containing data to encode. Must have @p count entries, each 32-bits.
		 * @param	destination		Buffer to output the data to. Must have @p count entries.
		 * @param	count			Number of entries in the @p source and @p destination arrays.
		 * @param	stride			Distance between two entries in the @p source buffer, in bytes.
		 */
		static void UnpackNormals(u8* source, Vector4* destination, u32 count, u32 stride);

		/** Decodes a normal from 4D 8-bit packed format into a 32-bit float format. */
		static Vector3 UnpackNormal(const u8* source)
		{
			const PackedNormal& packed = *(PackedNormal*)source;
			Vector3 output;

			const float inv = (1.0f / 255.0f) * 2.0f;
			output.X = (packed.X * inv - 1.0f);
			output.Y = (packed.Y * inv - 1.0f);
			output.Z = (packed.Z * inv - 1.0f);

			return output;
		}
	};

	/** @} */
} // namespace b3d
