//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsDragAndDrop.h"

namespace b3d { class DragAndDropData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptDragAndDropDataWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptDragAndDropData : public TScriptReflectableWrapper<DragAndDropData, ScriptDragAndDropData, ScriptDragAndDropDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "DragAndDropData")

		ScriptDragAndDropData(const SPtr<DragAndDropData>& nativeObject);
		~ScriptDragAndDropData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalDragAndDropData(MonoObject* scriptObject);
	};
}
