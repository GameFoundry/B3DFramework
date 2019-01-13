//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderAPICapabilities.h"
#include "Error/BsException.h"

namespace bs 
{
	Vector<bs::String> RenderAPICapabilities::msGPUVendorStrings;

	RenderAPICapabilities::RenderAPICapabilities()
	{
		for(UINT32 i = 0; i < CAPS_CATEGORY_COUNT; i++)
		{
			mCapabilities[i] = 0;
		}

		mNumTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = 0;
		mNumTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = 0;
		mNumTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = 0;
		mNumTextureUnitsPerStage[GPT_HULL_PROGRAM] = 0;
		mNumTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] = 0;
		mNumTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = 0;

		mNumGpuParamBlocksPerStage[GPT_VERTEX_PROGRAM] = 0;
		mNumGpuParamBlocksPerStage[GPT_FRAGMENT_PROGRAM] = 0;
		mNumGpuParamBlocksPerStage[GPT_GEOMETRY_PROGRAM] = 0;
		mNumGpuParamBlocksPerStage[GPT_HULL_PROGRAM] = 0;
		mNumGpuParamBlocksPerStage[GPT_DOMAIN_PROGRAM] = 0;
		mNumGpuParamBlocksPerStage[GPT_COMPUTE_PROGRAM] = 0;

		mNumLoadStoreTextureUnitsPerStage[GPT_VERTEX_PROGRAM] = 0;
		mNumLoadStoreTextureUnitsPerStage[GPT_FRAGMENT_PROGRAM] = 0;
		mNumLoadStoreTextureUnitsPerStage[GPT_GEOMETRY_PROGRAM] = 0;
		mNumLoadStoreTextureUnitsPerStage[GPT_HULL_PROGRAM] = 0;
		mNumLoadStoreTextureUnitsPerStage[GPT_DOMAIN_PROGRAM] = 0;
		mNumLoadStoreTextureUnitsPerStage[GPT_COMPUTE_PROGRAM] = 0;
	}

	GPUVendor RenderAPICapabilities::vendorFromString(const String& vendorString)
	{
		initVendorStrings();
		GPUVendor ret = GPU_UNKNOWN;
		String cmpString = vendorString;
		StringUtil::toLowerCase(cmpString);
		for (int i = 0; i < GPU_VENDOR_COUNT; ++i)
		{
			// case insensitive (lower case)
			if (msGPUVendorStrings[i] == cmpString)
			{
				ret = static_cast<GPUVendor>(i);
				break;
			}
		}

		return ret;
	}

	String RenderAPICapabilities::vendorToString(GPUVendor v)
	{
		initVendorStrings();
		return msGPUVendorStrings[v];
	}

	void RenderAPICapabilities::initVendorStrings()
	{
		if (msGPUVendorStrings.empty())
		{
			// Always lower case
			msGPUVendorStrings.resize(GPU_VENDOR_COUNT);
			msGPUVendorStrings[GPU_UNKNOWN] = "unknown";
			msGPUVendorStrings[GPU_NVIDIA] = "nvidia";
			msGPUVendorStrings[GPU_AMD] = "amd";
			msGPUVendorStrings[GPU_INTEL] = "intel";
		}
	}
}
