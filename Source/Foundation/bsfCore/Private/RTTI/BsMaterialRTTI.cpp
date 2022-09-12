//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/RTTI/BsMaterialRTTI.h"
#include "Material/BsMaterialParams.h"

namespace bs
{
	void MaterialRTTI::OnDeserializationEnded(IReflectable* obj, SerializationContext* context)
	{
		Material* material = static_cast<Material*>(obj);
		material->initialize();

		if(!mMatParams)
			return;

		material->initializeTechniques();

		if (material->getNumTechniques() > 0)
			material->setParams(mMatParams);
	}

	SPtr<IReflectable> MaterialRTTI::NewRTTIObject()
	{
		return Material::CreateEmpty();
	}
}
