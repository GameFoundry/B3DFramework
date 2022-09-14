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

	HShaderInclude ShaderInclude::create(const String& includeString)
	{
		return static_resource_cast<ShaderInclude>(gResources().CreateResourceHandleInternal(CreatePtrInternal(includeString)));
	}

	SPtr<ShaderInclude> ShaderInclude::CreatePtrInternal(const String& includeString)
	{
		SPtr<ShaderInclude> shaderIncludePtr = bs_core_ptr<ShaderInclude>(
			new (bs_alloc<ShaderInclude>()) ShaderInclude(includeString));
		shaderIncludePtr->SetThisPtrInternal(shaderIncludePtr);
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
