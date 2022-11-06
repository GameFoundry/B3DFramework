//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Text/BsFontDesc.h"
#include "../../../Foundation/bsfCore/Text/BsFontDesc.h"

namespace bs
{
	struct __CharDescInterop
	{
		uint32_t CharId;
		uint32_t Page;
		float UvX;
		float UvY;
		float UvWidth;
		float UvHeight;
		uint32_t Width;
		uint32_t Height;
		int32_t XOffset;
		int32_t YOffset;
		int32_t XAdvance;
		int32_t YAdvance;
		MonoArray* KerningPairs;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCharDesc : public ScriptObject<ScriptCharDesc>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "CharDesc")

		static MonoObject* Box(const __CharDescInterop& value);
		static __CharDescInterop Unbox(MonoObject* value);
		static CharDesc FromInterop(const __CharDescInterop& value);
		static __CharDescInterop ToInterop(const CharDesc& value);

	private:
		ScriptCharDesc(MonoObject* managedInstance);

	};
}
