//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/GUI/B3DDragAndDrop.h"

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

		ScriptDragAndDropData(const TShared<DragAndDropData>& nativeObject);
		~ScriptDragAndDropData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalDragAndDropData(MonoObject* scriptObject);
	};
}
