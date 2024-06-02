//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"

namespace bs { class VectorPath; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorPath : public TScriptResource<ScriptVectorPath, VectorPath>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "VectorPath")

		ScriptVectorPath(MonoObject* managedInstance, const TResourceHandle<VectorPath>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptVectorPath* thisPtr);

	};
}
