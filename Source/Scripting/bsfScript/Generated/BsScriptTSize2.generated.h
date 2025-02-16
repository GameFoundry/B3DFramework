//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsSize2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsSize2.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_float_ : public TScriptTypeDefinition<ScriptTSize2_float_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<float>")

		static MonoObject* Box(const TSize2<float>& value);
		static TSize2<float> Unbox(MonoObject* value);

	private:
		ScriptTSize2_float_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTSize2_uint32_t_ : public TScriptTypeDefinition<ScriptTSize2_uint32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TSize2<int>")

		static MonoObject* Box(const TSize2<uint32_t>& value);
		static TSize2<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptTSize2_uint32_t_();

	};
}
