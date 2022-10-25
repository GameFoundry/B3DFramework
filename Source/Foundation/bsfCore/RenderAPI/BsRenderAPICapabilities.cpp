//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsRenderAPICapabilities.h"

using namespace bs;

char const* const RenderAPICapabilities::GPU_VENDOR_STRINGS[GPU_VENDOR_COUNT] = {
	"unknown",
	"nvidia"
	"amd"
	"intel"
};

GPUVendor RenderAPICapabilities::VendorFromString(const String& vendorString)
{
	GPUVendor ret = GPU_UNKNOWN;
	String cmpString = vendorString;
	StringUtil::ToLowerCase(cmpString);
	for(int i = 0; i < GPU_VENDOR_COUNT; ++i)
	{
		if(GPU_VENDOR_STRINGS[i] == cmpString)
		{
			ret = static_cast<GPUVendor>(i);
			break;
		}
	}

	return ret;
}

String RenderAPICapabilities::VendorToString(GPUVendor vendor)
{
	return GPU_VENDOR_STRINGS[vendor];
}
