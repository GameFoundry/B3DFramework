//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCharacterInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "BsScriptKerningPair.generated.h"

namespace bs
{
	ScriptCharacterInformation::ScriptCharacterInformation()
	{ }

	MonoObject* ScriptCharacterInformation::Box(const __CharacterInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__CharacterInformationInterop ScriptCharacterInformation::Unbox(MonoObject* value)
	{
		return *(__CharacterInformationInterop*)MonoUtil::Unbox(value);
	}

	CharacterInformation ScriptCharacterInformation::FromInterop(const __CharacterInformationInterop& value)
	{
		CharacterInformation output;
		output.CharId = value.CharId;
		output.Page = value.Page;
		output.UvX = value.UvX;
		output.UvY = value.UvY;
		output.UvWidth = value.UvWidth;
		output.UvHeight = value.UvHeight;
		output.Width = value.Width;
		output.Height = value.Height;
		output.XOffset = value.XOffset;
		output.YOffset = value.YOffset;
		output.XAdvance = value.XAdvance;
		output.YAdvance = value.YAdvance;
		output.PointSize = value.PointSize;
		Vector<KerningPair> vecKerningPairs;
		if(value.KerningPairs != nullptr)
		{
			ScriptArray scriptArrayKerningPairs(value.KerningPairs);
			vecKerningPairs.resize(scriptArrayKerningPairs.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayKerningPairs.Size(); elementIndex++)
			{
				vecKerningPairs[elementIndex] = scriptArrayKerningPairs.Get<KerningPair>(elementIndex);
			}
		}
		output.KerningPairs = vecKerningPairs;

		return output;
	}

	__CharacterInformationInterop ScriptCharacterInformation::ToInterop(const CharacterInformation& value)
	{
		__CharacterInformationInterop output;
		output.CharId = value.CharId;
		output.Page = value.Page;
		output.UvX = value.UvX;
		output.UvY = value.UvY;
		output.UvWidth = value.UvWidth;
		output.UvHeight = value.UvHeight;
		output.Width = value.Width;
		output.Height = value.Height;
		output.XOffset = value.XOffset;
		output.YOffset = value.YOffset;
		output.XAdvance = value.XAdvance;
		output.YAdvance = value.YAdvance;
		output.PointSize = value.PointSize;
		int elementCountKerningPairs = (int)value.KerningPairs.size();
		MonoArray* vecKerningPairs;
		ScriptArray scriptArrayKerningPairs = ScriptArray::Create<ScriptKerningPair>(elementCountKerningPairs);
		for(int elementIndex = 0; elementIndex < elementCountKerningPairs; elementIndex++)
		{
			scriptArrayKerningPairs.Set(elementIndex, value.KerningPairs[elementIndex]);
		}
		vecKerningPairs = scriptArrayKerningPairs.GetInternal();
		output.KerningPairs = vecKerningPairs;

		return output;
	}

}
