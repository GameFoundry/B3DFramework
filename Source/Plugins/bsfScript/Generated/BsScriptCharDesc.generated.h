//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Text/BsFontDesc.h"
#include "../../../../../Foundation/bsfCore/Text/BsFontDesc.h"

namespace bs
{
	struct __CharDescInterop
	{
		uint32_t charId;
		uint32_t page;
		float uvX;
		float uvY;
		float uvWidth;
		float uvHeight;
		uint32_t width;
		uint32_t height;
		int32_t xOffset;
		int32_t yOffset;
		int32_t xAdvance;
		int32_t yAdvance;
		MonoArray* kerningPairs;
	};

	class BS_SCR_BE_EXPORT ScriptCharDesc : public ScriptObject<ScriptCharDesc>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "CharDesc")

		static MonoObject* box(const __CharDescInterop& value);
		static __CharDescInterop unbox(MonoObject* value);
		static CharDesc fromInterop(const __CharDescInterop& value);
		static __CharDescInterop toInterop(const CharDesc& value);

	private:
		ScriptCharDesc(MonoObject* managedInstance);

	};
}
