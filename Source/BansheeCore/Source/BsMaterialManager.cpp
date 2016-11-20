//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMaterialManager.h"
#include "BsMaterial.h"

namespace bs
{
	SPtr<Material> MaterialManager::create() const
	{
		SPtr<Material> newMat = bs_core_ptr<Material>(new (bs_alloc<Material>()) Material());
		newMat->_setThisPtr(newMat);
		newMat->initialize();

		return newMat;
	}

	SPtr<Material> MaterialManager::create(const HShader& shader) const
	{
		SPtr<Material> newMat = bs_core_ptr<Material>(new (bs_alloc<Material>()) Material(shader));
		newMat->_setThisPtr(newMat);
		newMat->initialize();

		return newMat;
	}

	SPtr<Material> MaterialManager::createEmpty() const
	{
		SPtr<Material> newMat = bs_core_ptr<Material>(new (bs_alloc<Material>()) Material());
		newMat->_setThisPtr(newMat);

		return newMat;
	}
}