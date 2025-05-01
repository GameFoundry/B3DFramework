//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVectorPath.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/VectorGraphics/BsVectorGraphics.h"

namespace bs
{
	ScriptVectorPath::ScriptVectorPath(const TResourceHandle<VectorPath>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptVectorPath::~ScriptVectorPath()
	{
		UnregisterEvents();
	}

	void ScriptVectorPath::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptVectorPath::InternalGetRef);

	}

	MonoObject* ScriptVectorPath::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptVectorPath::InternalGetRef(ScriptVectorPath* self)
	{
		return self->GetOrCreateResourceReference();
	}

}
