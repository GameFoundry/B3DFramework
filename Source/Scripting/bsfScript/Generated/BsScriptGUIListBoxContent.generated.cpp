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
	ScriptGUIListBoxContent::ScriptGUIListBoxContent()
	{ }

	MonoObject* ScriptGUIListBoxContent::Box(const __GUIListBoxContentInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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
			ScriptArray scriptArrayElements(value.Elements);
			vecElements.resize(scriptArrayElements.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayElements.Size(); elementIndex++)
			{
				ScriptLocString* scriptWrapperObjectElements;
				scriptWrapperObjectElements = ScriptLocString::GetScriptObjectWrapper(scriptArrayElements.Get<MonoObject*>(elementIndex));
				if(scriptWrapperObjectElements != nullptr)
				{
					SPtr<HString> arrayElementPointerElements = std::static_pointer_cast<HString>(scriptWrapperObjectElements->GetBaseNativeObjectAsShared());
					if(arrayElementPointerElements)
						vecElements[elementIndex] = *arrayElementPointerElements;
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
		int elementCountElements = (int)value.Elements.size();
		MonoArray* vecElements;
		ScriptArray scriptArrayElements = ScriptArray::Create<ScriptLocString>(elementCountElements);
		for(int elementIndex = 0; elementIndex < elementCountElements; elementIndex++)
		{
			SPtr<HString> arrayElementPointerElements = B3DMakeShared<HString>();
			*arrayElementPointerElements = value.Elements[elementIndex];
			MonoObject* arrayElementElements;
			arrayElementElements = ScriptLocString::GetOrCreateScriptObject(arrayElementPointerElements);
			scriptArrayElements.Set(elementIndex, arrayElementElements);
		}
		vecElements = scriptArrayElements.GetInternal();
		output.Elements = vecElements;
		output.AllowMultiselect = value.AllowMultiselect;

		return output;
	}

}
