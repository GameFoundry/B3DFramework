//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCharDesc.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Text/BsFontDesc.h"
#include "BsScriptKerningPair.generated.h"

namespace bs
{
	ScriptCharDesc::ScriptCharDesc(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptCharDesc::initRuntimeData()
	{ }

	MonoObject*ScriptCharDesc::box(const __CharDescInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__CharDescInterop ScriptCharDesc::unbox(MonoObject* value)
	{
		return *(__CharDescInterop*)MonoUtil::unbox(value);
	}

	CharDesc ScriptCharDesc::fromInterop(const __CharDescInterop& value)
	{
		CharDesc output;
		output.charId = value.charId;
		output.page = value.page;
		output.uvX = value.uvX;
		output.uvY = value.uvY;
		output.uvWidth = value.uvWidth;
		output.uvHeight = value.uvHeight;
		output.width = value.width;
		output.height = value.height;
		output.xOffset = value.xOffset;
		output.yOffset = value.yOffset;
		output.xAdvance = value.xAdvance;
		output.yAdvance = value.yAdvance;
		Vector<KerningPair> veckerningPairs;
		if(value.kerningPairs != nullptr)
		{
			ScriptArray arraykerningPairs(value.kerningPairs);
			veckerningPairs.resize(arraykerningPairs.size());
			for(int i = 0; i < (int)arraykerningPairs.size(); i++)
			{
				veckerningPairs[i] = arraykerningPairs.get<KerningPair>(i);
			}
		}
		output.kerningPairs = veckerningPairs;

		return output;
	}

	__CharDescInterop ScriptCharDesc::toInterop(const CharDesc& value)
	{
		__CharDescInterop output;
		output.charId = value.charId;
		output.page = value.page;
		output.uvX = value.uvX;
		output.uvY = value.uvY;
		output.uvWidth = value.uvWidth;
		output.uvHeight = value.uvHeight;
		output.width = value.width;
		output.height = value.height;
		output.xOffset = value.xOffset;
		output.yOffset = value.yOffset;
		output.xAdvance = value.xAdvance;
		output.yAdvance = value.yAdvance;
		int arraySizekerningPairs = (int)value.kerningPairs.size();
		MonoArray* veckerningPairs;
		ScriptArray arraykerningPairs = ScriptArray::create<ScriptKerningPair>(arraySizekerningPairs);
		for(int i = 0; i < arraySizekerningPairs; i++)
		{
			arraykerningPairs.set(i, value.kerningPairs[i]);
		}
		veckerningPairs = arraykerningPairs.getInternal();
		output.kerningPairs = veckerningPairs;

		return output;
	}

}
