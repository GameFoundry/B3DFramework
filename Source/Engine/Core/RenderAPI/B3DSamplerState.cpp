//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DSamplerState.h"
#include "RTTI/B3DSamplerStateRTTI.h"

using namespace b3d;

bool SamplerStateInformation::operator==(const SamplerStateInformation& rhs) const
{
	return AddressMode == rhs.AddressMode &&
		MinFilter == rhs.MinFilter &&
		MagFilter == rhs.MagFilter &&
		MipFilter == rhs.MipFilter &&
		MaxAniso == rhs.MaxAniso &&
		MipmapBias == rhs.MipmapBias &&
		MipMin == rhs.MipMin &&
		MipMax == rhs.MipMax &&
		BorderColor == rhs.BorderColor &&
		ComparisonFunc == rhs.ComparisonFunc;
}

SamplerState::SamplerState(const SamplerStateCreateInformation& createInformation)
	: mInformation(createInformation)
{
}

u64 SamplerState::GenerateHash(const SamplerStateInformation& information)
{
	size_t hash = 0;
	B3DCombineHash(hash, (u32)information.AddressMode.U);
	B3DCombineHash(hash, (u32)information.AddressMode.V);
	B3DCombineHash(hash, (u32)information.AddressMode.W);
	B3DCombineHash(hash, (u32)information.MinFilter);
	B3DCombineHash(hash, (u32)information.MagFilter);
	B3DCombineHash(hash, (u32)information.MipFilter);
	B3DCombineHash(hash, information.MaxAniso);
	B3DCombineHash(hash, information.MipmapBias);
	B3DCombineHash(hash, information.MipMin);
	B3DCombineHash(hash, information.MipMax);
	B3DCombineHash(hash, information.BorderColor);
	B3DCombineHash(hash, (u32)information.ComparisonFunc);

	return (u64)hash;
}

/************************************************************************/
/* 								RTTI		                     		*/
/************************************************************************/

RTTIType* SamplerState::GetRttiStatic()
{
	return SamplerStateRTTI::Instance();
}

RTTIType* SamplerState::GetRtti() const
{
	return SamplerState::GetRttiStatic();
}
