//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"

namespace bs { class Font; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptFont : public TScriptResourceWrapper<Font, ScriptFont>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Font")

		ScriptFont(const TResourceHandle<Font>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptFont* self);

		static MonoObject* InternalGetBitmap(ScriptFont* self, float size);
		static float InternalGetClosestSize(ScriptFont* self, float size);
	};
}
