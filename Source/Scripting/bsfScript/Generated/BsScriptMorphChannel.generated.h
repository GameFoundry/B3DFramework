//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"

namespace bs { class MorphChannel; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMorphChannel : public TScriptReflectable<ScriptMorphChannel, MorphChannel>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MorphChannel")

		ScriptMorphChannel(MonoObject* managedInstance, const SPtr<MorphChannel>& value);

		static MonoObject* Create(const SPtr<MorphChannel>& value);

	private:
		static MonoString* InternalGetName(ScriptMorphChannel* self);
		static MonoArray* InternalGetShapes(ScriptMorphChannel* self);
	};
}
