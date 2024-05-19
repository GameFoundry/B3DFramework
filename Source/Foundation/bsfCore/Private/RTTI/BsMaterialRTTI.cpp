//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/RTTI/BsMaterialRTTI.h"
#include "Material/BsMaterialParams.h"

using namespace bs;

void MaterialRTTI::OnDeserializationEnded(IReflectable* obj, RTTIOperationContext* context)
{
	Material* material = static_cast<Material*>(obj);
	material->Initialize();

	if(!mMatParams)
		return;

	material->InitializeTechniques();

	if(material->GetNumTechniques() > 0)
		material->SetParams(mMatParams);
}

SPtr<IReflectable> MaterialRTTI::NewRttiObject()
{
	return Material::CreateEmpty();
}
