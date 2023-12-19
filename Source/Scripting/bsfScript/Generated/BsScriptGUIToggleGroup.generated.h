//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs { class GUIToggleGroup; }
namespace bs { class GUIToggle; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleGroup : public ScriptObject<ScriptGUIToggleGroup>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIToggleGroup")

		ScriptGUIToggleGroup(MonoObject* managedInstance, const SPtr<GUIToggleGroup>& value);

		SPtr<GUIToggleGroup> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<GUIToggleGroup>& value);

	private:
		SPtr<GUIToggleGroup> mInternal;

		static void InternalCreateToggleGroup(MonoObject* managedInstance, bool allowAllOff);
	};
}
