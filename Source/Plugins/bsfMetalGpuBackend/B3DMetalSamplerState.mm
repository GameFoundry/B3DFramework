//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalSamplerState.h"
#include "B3DMetalGpuDevice.h"
#include "B3DMetalUtility.h"
#include "Debug/B3DLog.h"

namespace b3d
{
	namespace render
	{
		struct MetalSamplerState::Impl
		{
			id<MTLSamplerState> Sampler = nil;
		};

		namespace
		{
			/**
			 * Picks the closest @c MTLSamplerBorderColor to the engine's requested RGBA. Metal only
			 * exposes three fixed border colors; anything other than (0,0,0,0), (0,0,0,1) or (1,1,1,1)
			 * is rounded to the nearest opaque value and logged so the caller can see their border
			 * color won't match exactly.
			 */
			MTLSamplerBorderColor PickBorderColor(const Color& requested, bool usesBorderAddressing)
			{
				// Within one 8-bit step of the predefined color on every channel
				constexpr float kTolerance = 1.0f / 255.0f;

				if (requested.ApproxEquals(Color::kZero, kTolerance))
					return MTLSamplerBorderColorTransparentBlack;

				if (requested.ApproxEquals(Color::kBlack, kTolerance))
					return MTLSamplerBorderColorOpaqueBlack;

				if (requested.ApproxEquals(Color::kWhite, kTolerance))
					return MTLSamplerBorderColorOpaqueWhite;

				if (usesBorderAddressing)
				{
					B3D_LOG(Warning, LogRenderBackend,
						"Metal supports only predefined sampler border colors; requested ({0}, {1}, {2}, {3}) rounded to opaque white. "
						"Switch to TAM_CLAMP or perform the border comparison in shader code to get an exact match.",
						requested.R, requested.G, requested.B, requested.A);
				}
				return MTLSamplerBorderColorOpaqueWhite;
			}
		} // namespace

		MetalSamplerState::MetalSamplerState(MetalGpuDevice& gpuDevice, const SamplerStateCreateInformation& createInformation)
			: SamplerState(createInformation), mGpuDevice(gpuDevice), mImpl(B3DMakeUnique<Impl>())
		{
		}

		MetalSamplerState::~MetalSamplerState()
		{
			if (mImpl)
			{
#if !__has_feature(objc_arc)
				[mImpl->Sampler release];
#endif
				mImpl->Sampler = nil;
			}
		}

		id<MTLSamplerState> MetalSamplerState::GetMetalSampler() const
		{
			return mImpl->Sampler;
		}

		void MetalSamplerState::Initialize()
		{
			@autoreleasepool
			{
			id<MTLDevice> device = mGpuDevice.GetMetalDevice();
			if (device == nil)
			{
				B3D_LOG(Error, LogRenderBackend, "Cannot create Metal sampler state: device is null.");
				SamplerState::Initialize();

				return;
			}

			const float minimumLod = std::max(0.0f, mInformation.MipMin);

			MTLSamplerDescriptor* descriptor = [[MTLSamplerDescriptor alloc] init];
			descriptor.minFilter = MetalUtility::GetMinMagFilter(mInformation.MinFilter);
			descriptor.magFilter = MetalUtility::GetMinMagFilter(mInformation.MagFilter);
			descriptor.mipFilter = MetalUtility::GetMipFilter(mInformation.MipFilter);
			descriptor.sAddressMode = MetalUtility::GetAddressMode(mInformation.AddressMode.U);
			descriptor.tAddressMode = MetalUtility::GetAddressMode(mInformation.AddressMode.V);
			descriptor.rAddressMode = MetalUtility::GetAddressMode(mInformation.AddressMode.W);
			descriptor.lodMinClamp = minimumLod;
			descriptor.lodMaxClamp = std::max(minimumLod, mInformation.MipMax);

			if (mInformation.MipmapBias != 0.0f)
				B3D_LOG(Warning, LogRenderBackend, "Metal sampler objects do not expose a mip LOD bias; requested bias {0} is ignored.", mInformation.MipmapBias);

			const u32 hardwareMaxAniso = std::max(1u, mGpuDevice.GetMaxSamplerAnisotropy());
			const bool anisotropic = mInformation.MinFilter == FO_ANISOTROPIC || mInformation.MagFilter == FO_ANISOTROPIC || mInformation.MipFilter == FO_ANISOTROPIC;
			descriptor.maxAnisotropy = anisotropic ? std::max(1u, std::min(hardwareMaxAniso, mInformation.MaxAniso)) : 1;
			descriptor.compareFunction = MetalUtility::GetCompareFunction(mInformation.ComparisonFunc);

			const bool usesBorderAddressing = mInformation.AddressMode.U == TAM_BORDER
				|| mInformation.AddressMode.V == TAM_BORDER
				|| mInformation.AddressMode.W == TAM_BORDER;

			descriptor.borderColor = PickBorderColor(mInformation.BorderColor, usesBorderAddressing);
			descriptor.supportArgumentBuffers = YES;

			mImpl->Sampler = [device newSamplerStateWithDescriptor:descriptor];
#if !__has_feature(objc_arc)
			[descriptor release];
#endif
			if (mImpl->Sampler == nil)
				B3D_LOG(Error, LogRenderBackend, "Failed to create Metal sampler state.");

			SamplerState::Initialize();
			} // @autoreleasepool
		}
	} // namespace render
} // namespace b3d
