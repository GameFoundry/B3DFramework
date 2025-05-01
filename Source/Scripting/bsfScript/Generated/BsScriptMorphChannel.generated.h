//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"

namespace bs { class MorphChannel; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMorphChannel : public TScriptReflectableWrapper<MorphChannel, ScriptMorphChannel>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MorphChannel")

		ScriptMorphChannel(const SPtr<MorphChannel>& nativeObject);
		~ScriptMorphChannel();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetName(ScriptMorphChannel* self);
		static MonoArray* InternalGetShapes(ScriptMorphChannel* self);
	};
}
