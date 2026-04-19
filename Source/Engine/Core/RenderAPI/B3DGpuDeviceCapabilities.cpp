//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/B3DGpuDeviceCapabilities.h"

using namespace b3d;

char const* const GpuDeviceCapabilities::kGpuVendorStrings[GPU_VENDOR_COUNT] = {
	"unknown",
	"nvidia"
	"amd"
	"intel"
};

GPUVendor GpuDeviceCapabilities::VendorFromString(const String& vendorString)
{
	GPUVendor ret = GPU_UNKNOWN;
	String cmpString = vendorString;
	StringUtility::ToLowerCase(cmpString);
	for(int i = 0; i < GPU_VENDOR_COUNT; ++i)
	{
		if(kGpuVendorStrings[i] == cmpString)
		{
			ret = static_cast<GPUVendor>(i);
			break;
		}
	}

	return ret;
}

String GpuDeviceCapabilities::VendorToString(GPUVendor vendor)
{
	return kGpuVendorStrings[vendor];
}
