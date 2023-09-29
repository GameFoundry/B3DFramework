//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/BsShaderInclude.h"
#include "Resources/BsResources.h"
#include "Private/RTTI/BsShaderIncludeRTTI.h"

using namespace bs;

ShaderInclude::ShaderInclude(const String& includeString)
	: Resource(false), mString(includeString)
{
}

HShaderInclude ShaderInclude::Create(const String& includeString)
{
	return B3DStaticResourceCast<ShaderInclude>(GetResources().CreateResourceHandle(CreatePtrInternal(includeString)));
}

SPtr<ShaderInclude> ShaderInclude::CreatePtrInternal(const String& includeString)
{
	SPtr<ShaderInclude> shaderIncludePtr = B3DMakeCoreFromExisting<ShaderInclude>(
		new(B3DAllocate<ShaderInclude>()) ShaderInclude(includeString));
	shaderIncludePtr->SetShared(shaderIncludePtr);
	shaderIncludePtr->Initialize();

	return shaderIncludePtr;
}

RTTITypeBase* ShaderInclude::GetRttiStatic()
{
	return ShaderIncludeRTTI::Instance();
}

RTTITypeBase* ShaderInclude::GetRtti() const
{
	return ShaderInclude::GetRttiStatic();
}
