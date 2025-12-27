//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Material/B3DShaderInclude.h"
#include "Resources/B3DResources.h"
#include "RTTI/B3DShaderIncludeRTTI.h"

using namespace b3d;

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
	SPtr<ShaderInclude> shaderIncludePtr = B3DMakeSharedFromExisting<ShaderInclude>(
		new(B3DAllocate<ShaderInclude>()) ShaderInclude(includeString));
	shaderIncludePtr->SetShared(shaderIncludePtr);
	shaderIncludePtr->Initialize();

	return shaderIncludePtr;
}

RTTIType* ShaderInclude::GetRttiStatic()
{
	return ShaderIncludeRTTI::Instance();
}

RTTIType* ShaderInclude::GetRtti() const
{
	return ShaderInclude::GetRttiStatic();
}
