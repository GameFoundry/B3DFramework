//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace bs
{
	struct __ManagedMemberStyleInterop
	{
		float RangeMin;
		float RangeMax;
		float StepIncrement;
		bool DisplayAsSlider;
		MonoString* CategoryName;
		int32_t Order;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedMemberStyle : public TScriptTypeDefinition<ScriptManagedMemberStyle>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedMemberStyle")

		static MonoObject* Box(const __ManagedMemberStyleInterop& value);
		static __ManagedMemberStyleInterop Unbox(MonoObject* value);
		static ManagedMemberStyle FromInterop(const __ManagedMemberStyleInterop& value);
		static __ManagedMemberStyleInterop ToInterop(const ManagedMemberStyle& value);

	private:
		ScriptManagedMemberStyle();

	};
}
