//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsVector3.h"
#include "BsMatrix4.h"
#include "BsColor.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for GizmoManager. */
	class BS_SCR_BED_EXPORT ScriptGizmos : public ScriptObject<ScriptGizmos>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "Gizmos")

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_SetColor(Color* color);
		static void internal_GetColor(Color* color);
		static void internal_SetTransform(Matrix4* transform);
		static void internal_GetTransform(Matrix4* transform);
		static void internal_DrawCube(Vector3* position, Vector3* extents);
		static void internal_DrawSphere(Vector3* position, float radius);
		static void internal_DrawCone(Vector3* base, Vector3* normal, float height, float radius, Vector2* scale);
		static void internal_DrawWireCube(Vector3* position, Vector3* extents);
		static void internal_DrawWireSphere(Vector3* position, float radius);
		static void internal_DrawWireCapsule(Vector3* position, float height, float radius);
		static void internal_DrawWireCone(Vector3* base, Vector3* normal, float height, float radius, Vector2* scale);
		static void internal_DrawLine(Vector3* start, Vector3* end);
		static void internal_DrawLineList(MonoArray* linePoints);
		static void internal_DrawWireDisc(Vector3* position, Vector3* normal, float radius);
		static void internal_DrawWireArc(Vector3* position, Vector3* normal, float radius, float startAngle, float amountAngle);
		static void internal_DrawWireMesh(ScriptMeshData* meshData);
		static void internal_DrawFrustum(Vector3* position, float aspect, Degree* FOV, float near, float far);
		static void internal_DrawIcon(Vector3* position, MonoObject* image, bool fixedScale);
		static void internal_DrawText(Vector3* position, MonoString* text, ScriptFont* font, int size);
	};

	/** @} */
}