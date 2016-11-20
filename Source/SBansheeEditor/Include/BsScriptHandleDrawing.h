//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEditorPrerequisites.h"
#include "BsScriptObject.h"
#include "BsVector3.h"
#include "BsMatrix4.h"
#include "BsRect3.h"
#include "BsColor.h"

namespace bs
{
	/** @addtogroup ScriptInteropEditor
	 *  @{
	 */

	/**	Interop class between C++ & CLR for HandleDrawManager. */
	class BS_SCR_BED_EXPORT ScriptHandleDrawing : public ScriptObject <ScriptHandleDrawing>
	{
	public:
		SCRIPT_OBJ(EDITOR_ASSEMBLY, "BansheeEditor", "HandleDrawing")

	private:
		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_SetColor(Color* color);
		static void internal_SetTransform(Matrix4* transform);
		static void internal_SetLayer(UINT64 layer);

		static void internal_DrawCube(Vector3* position, Vector3* extents, float size);
		static void internal_DrawSphere(Vector3* position, float radius, float size);
		static void internal_DrawWireCube(Vector3* position, Vector3* extents, float size);
		static void internal_DrawWireSphere(Vector3* position, float radius, float size);
		static void internal_DrawLine(Vector3* start, Vector3* end, float size);
		static void internal_DrawCone(Vector3* coneBase, Vector3* normal, float height, float radius, float size);
		static void internal_DrawDisc(Vector3* position, Vector3* normal, float radius, float size);
		static void internal_DrawWireDisc(Vector3* position, Vector3* normal, float radius, float size);
		static void internal_DrawArc(Vector3* position, Vector3* normal, float radius, Degree* startAngle, Degree* amountAngle, float size);
		static void internal_DrawWireArc(Vector3* position, Vector3* normal, float radius, Degree* startAngle, Degree* amountAngle, float size);
		static void internal_DrawRect(Vector3* center, Vector3* horzAxis, Vector3* vertAxis, float extentH, float extentV, float size);
		static void internal_DrawText(Vector3* position, MonoString* text, ScriptFont* font, int fontSize, float size);
	};

	/** @} */
}