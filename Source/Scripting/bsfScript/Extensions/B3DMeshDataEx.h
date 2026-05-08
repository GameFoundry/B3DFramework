//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Renderer/B3DRendererMeshData.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for RendererMeshData, for adding additional functionality for the script version of the class. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(RendererMeshData)) MeshDataEx
	{
	public:
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(RendererMeshData))
		static TShared<RendererMeshData> Create(u32 numVertices, u32 numIndices, VertexLayout layout, IndexType indexType = IT_32BIT);

		/** An array of all vertex positions. Only valid if the vertex layout contains vertex positions. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(Positions))
		static Vector<Vector3> GetPositions(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(Positions))
		static void SetPositions(const TShared<RendererMeshData>& thisPtr, const Vector<Vector3>& value);

		/** An array of all vertex normals. Only valid if the vertex layout contains vertex normals. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(Normals))
		static Vector<Vector3> GetNormals(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(Normals))
		static void SetNormals(const TShared<RendererMeshData>& thisPtr, const Vector<Vector3>& value);

		/** An array of all vertex tangents. Only valid if the vertex layout contains vertex tangents. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(Tangents))
		static Vector<Vector4> GetTangents(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(Tangents))
		static void SetTangents(const TShared<RendererMeshData>& thisPtr, const Vector<Vector4>& value);

		/** An array of all vertex colors. Only valid if the vertex layout contains vertex colors. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(Colors))
		static Vector<Color> GetColors(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(Colors))
		static void SetColors(const TShared<RendererMeshData>& thisPtr, const Vector<Color>& value);

		/**
		 * An array of all vertex texture coordinates in the UV0 channel. Only valid if the vertex layout contains UV0
		 * coordinates.
		 */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(UV0))
		static Vector<Vector2> GetUV0(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(UV0))
		static void SetUV0(const TShared<RendererMeshData>& thisPtr, const Vector<Vector2>& value);

		/**
		 * An array of all vertex texture coordinates in the UV1 channel. Only valid if the vertex layout contains UV1
		 * coordinates.
		 */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(UV1))
		static Vector<Vector2> GetUV1(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(UV1))
		static void SetUV1(const TShared<RendererMeshData>& thisPtr, const Vector<Vector2>& value);

		/** An array of all vertex bone weights. Only valid if the vertex layout contains bone weights. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(BoneWeights))
		static Vector<BoneWeight> GetBoneWeights(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(BoneWeights))
		static void SetBoneWeights(const TShared<RendererMeshData>& thisPtr, const Vector<BoneWeight>& value);

		/** An array of all indices. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(Indices))
		static Vector<u32> GetIndices(const TShared<RendererMeshData>& thisPtr);
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Setter), ExportName(Indices))
		static void SetIndices(const TShared<RendererMeshData>& thisPtr, const Vector<u32>& value);

		/** Returns the number of vertices contained in the mesh. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(VertexCount))
		static int GetVertexCount(const TShared<RendererMeshData>& thisPtr);

		/** Returns the number of indices contained in the mesh. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RendererMeshData), Property(Getter), ExportName(IndexCount))
		static int GetIndexCount(const TShared<RendererMeshData>& thisPtr);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
