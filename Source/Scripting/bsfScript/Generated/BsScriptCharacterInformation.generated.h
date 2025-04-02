//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"

namespace bs
{
	struct __CharacterInformationInterop
	{
		uint32_t CharId;
		uint32_t Page;
		float UvX;
		float UvY;
		float UvWidth;
		float UvHeight;
		float Width;
		float Height;
		float XOffset;
		float YOffset;
		float XAdvance;
		float YAdvance;
		float PointSize;
		MonoArray* KerningPairs;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptCharacterInformation : public TScriptTypeDefinition<ScriptCharacterInformation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "CharacterInformation")

		static MonoObject* Box(const __CharacterInformationInterop& value);
		static __CharacterInformationInterop Unbox(MonoObject* value);
		static CharacterInformation FromInterop(const __CharacterInformationInterop& value);
		static __CharacterInformationInterop ToInterop(const CharacterInformation& value);

	private:
		ScriptCharacterInformation();

	};
}
