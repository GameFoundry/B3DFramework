//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsShaderInclude.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsShaderIncludeRTTI.h"

namespace bs
{
	ShaderInclude::ShaderInclude(const String& includeString)
		:Resource(false), mString(includeString)
	{

	}

	HShaderInclude ShaderInclude::Create(const String& includeString)
	{
		return static_resource_cast<ShaderInclude>(gResources()._createResourceHandle(_createPtr(includeString)));
	}

	SPtr<ShaderInclude> ShaderInclude::_createPtr(const String& includeString)
	{
		SPtr<ShaderInclude> shaderIncludePtr = bs_core_ptr<ShaderInclude>(
			new (bs_alloc<ShaderInclude>()) ShaderInclude(includeString));
		shaderIncludePtr->_setThisPtr(shaderIncludePtr);
		shaderIncludePtr->Initialize();

		return shaderIncludePtr;
	}

	RTTITypeBase* ShaderInclude::getRTTIStatic()
	{
		return ShaderIncludeRTTI::Instance();
	}

	RTTITypeBase* ShaderInclude::getRTTI() const
	{
		return ShaderInclude::GetRTTIStatic();
	}
}
