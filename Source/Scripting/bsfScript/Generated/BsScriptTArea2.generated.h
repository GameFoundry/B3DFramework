//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsArea2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsArea2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsArea2.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_int32_t__int32_t_ : public TScriptTypeDefinition<ScriptTArea2_int32_t__int32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<int,int>")

		static MonoObject* Box(const TArea2<int32_t, int32_t>& value);
		static TArea2<int32_t, int32_t> Unbox(MonoObject* value);

	private:
		ScriptTArea2_int32_t__int32_t_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_int32_t__uint32_t_ : public TScriptTypeDefinition<ScriptTArea2_int32_t__uint32_t_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<int,int>")

		static MonoObject* Box(const TArea2<int32_t, uint32_t>& value);
		static TArea2<int32_t, uint32_t> Unbox(MonoObject* value);

	private:
		ScriptTArea2_int32_t__uint32_t_();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptTArea2_float__float_ : public TScriptTypeDefinition<ScriptTArea2_float__float_>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TArea2<float,float>")

		static MonoObject* Box(const TArea2<float, float>& value);
		static TArea2<float, float> Unbox(MonoObject* value);

	private:
		ScriptTArea2_float__float_();

	};
}
