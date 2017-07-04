//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsStandardDeferredLighting.h"
#include "BsRendererUtility.h"
#include "BsRendererView.h"
#include "BsGpuParamsSet.h"
#include "BsRenderTargets.h"
#include "BsMesh.h"

namespace bs { namespace ct {
	PerLightParamDef gPerLightParamDef;

	template<bool MSAA>
	DirectionalLightMat<MSAA>::DirectionalLightMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		const GpuParams& gpuParams = *mParamsSet->getGpuParams();
		gpuParams.getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	template<bool MSAA>
	void DirectionalLightMat<MSAA>::_initDefines(ShaderDefines& defines)
	{
		if (MSAA)
			defines.set("MSAA_COUNT", 2); // Actual count doesn't matter, as long as it's greater than one
		else
			defines.set("MSAA_COUNT", 1);
	}

	template<bool MSAA>
	void DirectionalLightMat<MSAA>::bind(const RenderTargets& renderTargets, const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		RendererUtility::instance().setPass(mMaterial, 0);

		mGBufferParams.bind(renderTargets);
		mLightOcclusionTexParam.set(renderTargets.get(RTT_LightOcclusion));
		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);
	}

	template<bool MSAA>
	void DirectionalLightMat<MSAA>::setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mParamsSet->setParamBlockBuffer("PerLight", perLight, true);
		
		gRendererUtility().setPassParams(mParamsSet);
	}

	template class DirectionalLightMat<true>;
	template class DirectionalLightMat<false>;

	template<bool MSAA, bool InsideGeometry>
	PointLightMat<MSAA, InsideGeometry>::PointLightMat()
		:mGBufferParams(mMaterial, mParamsSet)
	{
		const GpuParams& gpuParams = *mParamsSet->getGpuParams();
		gpuParams.getTextureParam(GPT_FRAGMENT_PROGRAM, "gLightOcclusionTex", mLightOcclusionTexParam);
	}

	template<bool MSAA, bool InsideGeometry>
	void PointLightMat<MSAA, InsideGeometry>::_initDefines(ShaderDefines& defines)
	{
		if (MSAA)
			defines.set("MSAA_COUNT", 2); // Actual count doesn't matter, as long as it's greater than one
		else
			defines.set("MSAA_COUNT", 1);

		if (InsideGeometry)
			defines.set("INSIDE_GEOMETRY", 1);
	}

	template<bool MSAA, bool InsideGeometry>
	void PointLightMat<MSAA, InsideGeometry>::bind(const RenderTargets& renderTargets, 
		const SPtr<GpuParamBlockBuffer>& perCamera)
	{
		RendererUtility::instance().setPass(mMaterial, 0);

		mGBufferParams.bind(renderTargets);
		mLightOcclusionTexParam.set(renderTargets.get(RTT_LightOcclusion));
		mParamsSet->setParamBlockBuffer("PerCamera", perCamera, true);
	}

	template<bool MSAA, bool InsideGeometry>
	void PointLightMat<MSAA, InsideGeometry>::setPerLightParams(const SPtr<GpuParamBlockBuffer>& perLight)
	{
		mParamsSet->setParamBlockBuffer("PerLight", perLight, true);
		
		gRendererUtility().setPassParams(mParamsSet);
	}

	template class PointLightMat<false, false>;
	template class PointLightMat<false, true>;
	template class PointLightMat<true, false>;
	template class PointLightMat<true, true>;

	StandardDeferred::StandardDeferred()
	{
		mPerLightBuffer = gPerLightParamDef.createBuffer();
	}

	void StandardDeferred::renderLight(LightType lightType, const RendererLight& light, const RendererView& view, 
		const RenderTargets& renderTargets)
	{
		const auto& viewProps = view.getProperties();

		bool isMSAA = view.getProperties().numSamples > 1;
		SPtr<GpuParamBlockBuffer> perViewBuffer = view.getPerViewBuffer();

		light.getParameters(mPerLightBuffer);

		if (lightType == LightType::Directional)
		{
			if(isMSAA)
			{
				mDirLightMat_T.bind(renderTargets, perViewBuffer);
				mDirLightMat_T.setPerLightParams(mPerLightBuffer);
			}
			else
			{
				mDirLightMat_F.bind(renderTargets, perViewBuffer);
				mDirLightMat_F.setPerLightParams(mPerLightBuffer);
			}

			gRendererUtility().drawScreenQuad();
		}
		else // Radial or spot
		{
			// Check if viewer is inside the light volume
			float distSqrd = (light.internal->getBounds().getCenter() - viewProps.viewOrigin).squaredLength();

			// Extend the bounds slighty to cover the case when the viewer is outside, but the near plane is intersecting
			// the light bounds. We need to be conservative since the material for rendering outside will not properly
			// render the inside of the light volume.
			float boundRadius = light.internal->getBounds().getRadius() + viewProps.nearPlane * 3.0f;

			bool isInside = distSqrd < (boundRadius * boundRadius);
			if(isMSAA)
			{
				if(isInside)
				{
					mPointLightMat_TT.bind(renderTargets, perViewBuffer);
					mPointLightMat_TT.setPerLightParams(mPerLightBuffer);
				}
				else
				{
					mPointLightMat_TF.bind(renderTargets, perViewBuffer);
					mPointLightMat_TF.setPerLightParams(mPerLightBuffer);
				}
			}
			else
			{
				if(isInside)
				{
					mPointLightMat_FT.bind(renderTargets, perViewBuffer);
					mPointLightMat_FT.setPerLightParams(mPerLightBuffer);
				}
				else
				{
					mPointLightMat_FF.bind(renderTargets, perViewBuffer);
					mPointLightMat_TF.setPerLightParams(mPerLightBuffer);
				}
			}

			SPtr<Mesh> stencilMesh;
			if(lightType == LightType::Radial)
				stencilMesh = RendererUtility::instance().getRadialLightStencil();
			else // Spot
				stencilMesh = RendererUtility::instance().getSpotLightStencil();

			gRendererUtility().draw(stencilMesh);
		}
	}
}}
