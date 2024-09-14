//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector4I.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector4I.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector4I : public TScriptTypeDefinition<ScriptVector4I>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector4I")

		static MonoObject* Box(const TVector4I<int32_t>& value);
		static TVector4I<int32_t> Unbox(MonoObject* value);

	private:
		ScriptVector4I();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector4UI : public TScriptTypeDefinition<ScriptVector4UI>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector4UI")

		static MonoObject* Box(const TVector4I<uint32_t>& value);
		static TVector4I<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptVector4UI();

	};
}
