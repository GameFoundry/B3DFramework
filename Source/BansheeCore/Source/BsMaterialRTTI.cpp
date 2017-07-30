//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMaterialRTTI.h"
#include "BsMaterialManager.h"
#include "BsMaterialParams.h"

namespace bs
{
	void MaterialRTTI::onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params)
	{
		Material* material = static_cast<Material*>(obj);
		material->initialize();

		if(material->mRTTIData.empty())
			return;

		material->initializeTechniques();

		if (material->getNumTechniques() > 0)
		{
			SPtr<MaterialParams> matParams = any_cast<SPtr<MaterialParams>>(material->mRTTIData);
			material->setParams(matParams);
		}

		material->mRTTIData = nullptr; // Delete temporary data
	}

	SPtr<IReflectable> MaterialRTTI::newRTTIObject()
	{
		return MaterialManager::instance().createEmpty();
	}
}