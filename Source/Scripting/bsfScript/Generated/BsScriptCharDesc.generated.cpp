//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCharDesc.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Text/BsFontDesc.h"
#include "BsScriptKerningPair.generated.h"

namespace bs
{
	ScriptCharDesc::ScriptCharDesc(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptCharDesc::InitRuntimeData()
	{ }

	MonoObject*ScriptCharDesc::Box(const __CharDescInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__CharDescInterop ScriptCharDesc::Unbox(MonoObject* value)
	{
		return *(__CharDescInterop*)MonoUtil::Unbox(value);
	}

	CharDesc ScriptCharDesc::FromInterop(const __CharDescInterop& value)
	{
		CharDesc output;
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
		Vector<KerningPair> vecKerningPairs;
		if(value.KerningPairs != nullptr)
		{
			ScriptArray arrayKerningPairs(value.KerningPairs);
			vecKerningPairs.resize(arrayKerningPairs.Size());
			for(int i = 0; i < (int)arrayKerningPairs.Size(); i++)
			{
				vecKerningPairs[i] = arrayKerningPairs.Get<KerningPair>(i);
			}
		}
		output.KerningPairs = vecKerningPairs;

		return output;
	}

	__CharDescInterop ScriptCharDesc::ToInterop(const CharDesc& value)
	{
		__CharDescInterop output;
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
		int arraySizeKerningPairs = (int)value.KerningPairs.size();
		MonoArray* vecKerningPairs;
		ScriptArray arrayKerningPairs = ScriptArray::Create<ScriptKerningPair>(arraySizeKerningPairs);
		for(int i = 0; i < arraySizeKerningPairs; i++)
		{
			arrayKerningPairs.Set(i, value.KerningPairs[i]);
		}
		vecKerningPairs = arrayKerningPairs.GetInternal();
		output.KerningPairs = vecKerningPairs;

		return output;
	}

}
