//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"

namespace bs { class VectorPath; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVectorPath : public TScriptResourceWrapper<VectorPath, ScriptVectorPath>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "VectorPath")

		ScriptVectorPath(const TResourceHandle<VectorPath>& nativeObject);
		~ScriptVectorPath();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptVectorPath* self);

	};
}
