//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"

namespace bs { class Font; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFont : public TScriptResource<ScriptFont, Font>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Font")

		ScriptFont(MonoObject* managedInstance, const TResourceHandle<Font>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptFont* thisPtr);

		static MonoObject* InternalGetBitmap(ScriptFont* thisPtr, float size);
		static float InternalGetClosestSize(ScriptFont* thisPtr, float size);
	};
}
