//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Renderer/BsRendererMeshData.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for RendererMeshData, for adding additional functionality for the script version of the class. */
	class BS_SCRIPT_EXPORT(e:RendererMeshData) MeshDataEx
	{
	public:
		BS_SCRIPT_EXPORT(ec:RendererMeshData)
		static SPtr<RendererMeshData> create(UINT32 numVertices, UINT32 numIndices, VertexLayout layout, 
			IndexType indexType = IT_32BIT);

		/** An array of all vertex positions. Only valid if the vertex layout contains vertex positions. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:Positions)
		static Vector<Vector3> getPositions(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:Positions)
		static void setPositions(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector3>& value);

		/** An array of all vertex normals. Only valid if the vertex layout contains vertex normals. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:Normals)
		static Vector<Vector3> getNormals(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:Normals)
		static void setNormals(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector3>& value);

		/** An array of all vertex tangents. Only valid if the vertex layout contains vertex tangents. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:Tangents)
		static Vector<Vector4> getTangents(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:Tangents)
		static void setTangents(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector4>& value);

		/** An array of all vertex colors. Only valid if the vertex layout contains vertex colors. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:Colors)
		static Vector<Color> getColors(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:Colors)
		static void setColors(const SPtr<RendererMeshData>& thisPtr, const Vector<Color>& value);

		/** 
		 * An array of all vertex texture coordinates in the UV0 channel. Only valid if the vertex layout contains UV0
		 * coordinates.
		 */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:UV0)
		static Vector<Vector2> getUV0(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:UV0)
		static void setUV0(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector2>& value);

		/** 
		 * An array of all vertex texture coordinates in the UV1 channel. Only valid if the vertex layout contains UV1
		 * coordinates.
		 */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:UV1)
		static Vector<Vector2> getUV1(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:UV1)
		static void setUV1(const SPtr<RendererMeshData>& thisPtr, const Vector<Vector2>& value);

		/** An array of all vertex bone weights. Only valid if the vertex layout contains bone weights. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:BoneWeights)
		static Vector<BoneWeight> getBoneWeights(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:BoneWeights)
		static void setBoneWeights(const SPtr<RendererMeshData>& thisPtr, const Vector<BoneWeight>& value);

		/** An array of all indices. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:Indices)
		static Vector<UINT32> getIndices(const SPtr<RendererMeshData>& thisPtr);
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:setter,n:Indices)
		static void setIndices(const SPtr<RendererMeshData>& thisPtr, const Vector<UINT32>& value);

		/** Returns the number of vertices contained in the mesh. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:VertexCount)
		static int getVertexCount(const SPtr<RendererMeshData>& thisPtr);

		/** Returns the number of indices contained in the mesh. */
		BS_SCRIPT_EXPORT(e:RendererMeshData,pr:getter,n:IndexCount)
		static int getIndexCount(const SPtr<RendererMeshData>& thisPtr);
	};

	/** @endcond */
	/** @} */
}
