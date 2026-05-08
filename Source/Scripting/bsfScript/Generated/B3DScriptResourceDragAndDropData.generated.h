//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptDragAndDropData.generated.h"
#include "../../../Engine/Core/GUI/B3DDragAndDrop.h"

namespace b3d { class ResourceDragAndDropData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptResourceDragAndDropData : public TScriptReflectableWrapper<ResourceDragAndDropData, ScriptResourceDragAndDropData, ScriptDragAndDropDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ResourceDragAndDropData")

		ScriptResourceDragAndDropData(const TShared<ResourceDragAndDropData>& nativeObject);
		~ScriptResourceDragAndDropData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalResourceDragAndDropData(MonoObject* scriptObject);
		static void InternalResourceDragAndDropData0(MonoObject* scriptObject, MonoString* relativeResourcePath);
		static void InternalResourceDragAndDropData1(MonoObject* scriptObject, MonoArray* relativeResourcePaths);
		static MonoArray* InternalGetRelativeResourcePaths(ScriptResourceDragAndDropData* self);
		static void InternalSetRelativeResourcePaths(ScriptResourceDragAndDropData* self, MonoArray* value);
	};
}
