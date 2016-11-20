//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsScriptMeshData.h"
#include "BsMesh.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**
	 * Possible mesh toplogy values. 
	 *
	 * @note	Must match C# enum MeshTopology.
	 */
	enum class MeshTopology
	{
		PointList = 1,
		LineList = 2,
		LineStrip = 3,
		TriangleList = 4,
		TriangleStrip = 5,
		TriangleFan = 6
	};

	/**	Contains data about a portion of a mesh inside MeshData. */
	struct BS_SCR_BE_EXPORT SubMeshData
	{
		UINT32 indexOffset;
		UINT32 indexCount;
		MeshTopology topology;
	};

	/**	Interop class between C++ & CLR for SubMesh. */
	class BS_SCR_BE_EXPORT ScriptSubMesh : public ScriptObject < ScriptSubMesh >
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "SubMesh")

		/** Unboxes a boxed managed SubMesh struct and returns the native version of the structure. */
		static SubMeshData unbox(MonoObject* obj);

		/** Boxes a native SubMesh struct and returns a managed object containing it. */
		static MonoObject* box(const SubMeshData& value);

	private:
		ScriptSubMesh(MonoObject* instance);
	};

	/**	Interop class between C++ & CLR for Mesh. */
	class BS_SCR_BE_EXPORT ScriptMesh : public TScriptResource<ScriptMesh, Mesh>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Mesh")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

	private:
		friend class ScriptResourceManager;

		ScriptMesh(MonoObject* instance, const HMesh& mesh);

		/** Converts the C# MeshTopology enum to DrawOperationType enum used by engine internals. */
		static DrawOperationType meshTopologyToDrawOp(MeshTopology topology);

		/** Converts the DrawOperationType enum used by engine internals to C# MeshTopology enum. */
		static MeshTopology drawOpToMeshTopology(DrawOperationType drawOp);
		
		/** Converts a managed array of SubMeshData%es into an array of SubMesh%es used by engine internals. */
		static Vector<SubMesh> monoToNativeSubMeshes(MonoArray* subMeshes);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance, int numVertices,
			int numIndices, MonoArray* subMeshes, MeshUsage usage, VertexLayout vertex, ScriptIndexType index);
		static void internal_CreateInstanceMeshData(MonoObject* instance, ScriptMeshData* data, MonoArray* subMeshes,
			MeshUsage usage);
		static MonoArray* internal_GetSubMeshes(ScriptMesh* thisPtr);
		static UINT32 internal_GetSubMeshCount(ScriptMesh* thisPtr);
		static MonoObject* internal_GetSkeleton(ScriptMesh* thisPtr);
		static MonoObject* internal_GetMorphShapes(ScriptMesh* thisPtr);
		static void internal_GetBounds(ScriptMesh* thisPtr, AABox* box, Sphere* sphere);
		static MonoObject* internal_GetMeshData(ScriptMesh* thisPtr);
		static void internal_SetMeshData(ScriptMesh* thisPtr, ScriptMeshData* value);
	};

	/** @} */
}