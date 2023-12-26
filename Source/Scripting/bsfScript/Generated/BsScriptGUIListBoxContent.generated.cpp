//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIListBoxContent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"
#include "BsScriptHString.generated.h"

namespace bs
{
	ScriptGUIListBoxContent::ScriptGUIListBoxContent(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUIListBoxContent::InitRuntimeData()
	{ }

	MonoObject*ScriptGUIListBoxContent::Box(const __GUIListBoxContentInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__GUIListBoxContentInterop ScriptGUIListBoxContent::Unbox(MonoObject* value)
	{
		return *(__GUIListBoxContentInterop*)MonoUtil::Unbox(value);
	}

	GUIListBoxContent ScriptGUIListBoxContent::FromInterop(const __GUIListBoxContentInterop& value)
	{
		GUIListBoxContent output;
		Vector<HString> vecElements;
		if(value.Elements != nullptr)
		{
			ScriptArray arrayElements(value.Elements);
			vecElements.resize(arrayElements.Size());
			for(int i = 0; i < (int)arrayElements.Size(); i++)
			{
				ScriptLocString* scriptElements;
				scriptElements = ScriptLocString::ToNative(arrayElements.Get<MonoObject*>(i));
				if(scriptElements != nullptr)
				{
					SPtr<HString> arrayElemPtrElements = scriptElements->GetInternal();
					if(arrayElemPtrElements)
						vecElements[i] = *arrayElemPtrElements;
				}
			}
		}
		output.Elements = vecElements;
		output.AllowMultiselect = value.AllowMultiselect;

		return output;
	}

	__GUIListBoxContentInterop ScriptGUIListBoxContent::ToInterop(const GUIListBoxContent& value)
	{
		__GUIListBoxContentInterop output;
		int arraySizeElements = (int)value.Elements.size();
		MonoArray* vecElements;
		ScriptArray arrayElements = ScriptArray::Create<ScriptLocString>(arraySizeElements);
		for(int i = 0; i < arraySizeElements; i++)
		{
			SPtr<HString> arrayElemPtrElements = B3DMakeShared<HString>();
			*arrayElemPtrElements = value.Elements[i];
			MonoObject* arrayElemElements;
			arrayElemElements = ScriptLocString::Create(arrayElemPtrElements);
			arrayElements.Set(i, arrayElemElements);
		}
		vecElements = arrayElements.GetInternal();
		output.Elements = vecElements;
		output.AllowMultiselect = value.AllowMultiselect;

		return output;
	}

}
