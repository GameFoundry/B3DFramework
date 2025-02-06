//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2 : public TScriptTypeDefinition<ScriptVector2>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2")

		static MonoObject* Box(const TVector2<float>& value);
		static TVector2<float> Unbox(MonoObject* value);

	private:
		ScriptVector2();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2D : public TScriptTypeDefinition<ScriptVector2D>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2D")

		static MonoObject* Box(const TVector2<double>& value);
		static TVector2<double> Unbox(MonoObject* value);

	private:
		ScriptVector2D();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2I : public TScriptTypeDefinition<ScriptVector2I>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2I")

		static MonoObject* Box(const TVector2<int32_t>& value);
		static TVector2<int32_t> Unbox(MonoObject* value);

	private:
		ScriptVector2I();

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2UI : public TScriptTypeDefinition<ScriptVector2UI>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2UI")

		static MonoObject* Box(const TVector2<uint32_t>& value);
		static TVector2<uint32_t> Unbox(MonoObject* value);

	private:
		ScriptVector2UI();

	};
}
