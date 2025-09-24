//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfEngine/GUI/BsDragAndDrop.h"
#include "BsScriptTypeDefinition.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDragAndDrop : public TScriptTypeDefinition<ScriptDragAndDrop>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "DragAndDrop")

		ScriptDragAndDrop();

		static void SetupScriptBindings();

	private:
		static void InternalStartDrag(MonoObject* data);
		static bool InternalIsDragInProgress();
		static bool InternalIsDropInProgress();
		static MonoObject* InternalGetDragData();
		static MonoObject* InternalGetDropData();
	};
}
