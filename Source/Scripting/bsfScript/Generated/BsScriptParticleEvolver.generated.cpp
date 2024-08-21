//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptParticleEvolver.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptParticleEvolver::ScriptParticleEvolver(const SPtr<ParticleEvolver>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
	}

	void ScriptParticleEvolver::SetupScriptBindings()
	{

	}

	MonoObject* ScriptParticleEvolver::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
}
