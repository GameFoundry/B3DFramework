//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsShaderInclude.h"
#include "BsResources.h"
#include "BsShaderIncludeRTTI.h"

namespace bs
{
	ShaderInclude::ShaderInclude(const String& includeString)
		:Resource(false), mString(includeString)
	{

	}

	HShaderInclude ShaderInclude::create(const String& includeString)
	{
		return static_resource_cast<ShaderInclude>(gResources()._createResourceHandle(_createPtr(includeString)));
	}

	SPtr<ShaderInclude> ShaderInclude::_createPtr(const String& includeString)
	{
		SPtr<ShaderInclude> shaderIncludePtr = bs_core_ptr<ShaderInclude>(
			new (bs_alloc<ShaderInclude>()) ShaderInclude(includeString));
		shaderIncludePtr->_setThisPtr(shaderIncludePtr);
		shaderIncludePtr->initialize();

		return shaderIncludePtr;
	}

	RTTITypeBase* ShaderInclude::getRTTIStatic()
	{
		return ShaderIncludeRTTI::instance();
	}

	RTTITypeBase* ShaderInclude::getRTTI() const
	{
		return ShaderInclude::getRTTIStatic();
	}
}