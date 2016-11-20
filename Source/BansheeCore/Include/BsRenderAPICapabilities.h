//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsGpuProgram.h"

#include <cstdint>
#define CAPS_CATEGORY_SIZE INT64_C(8)
#define BS_CAPS_BITSHIFT (INT64_C(64) - CAPS_CATEGORY_SIZE)
#define CAPS_CATEGORY_MASK (((INT64_C(1) << CAPS_CATEGORY_SIZE) - INT64_C(1)) << BS_CAPS_BITSHIFT)
#define BS_CAPS_VALUE(cat, val) ((cat << BS_CAPS_BITSHIFT) | (INT64_C(1) << val))

#define BS_MAX_BOUND_VERTEX_BUFFERS 16

namespace bs 
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Categories of render API capabilities. */
	enum CapabilitiesCategory : UINT64
	{
		CAPS_CATEGORY_COMMON = 0,
		CAPS_CATEGORY_GL = 1,
		CAPS_CATEGORY_D3D11 = 2,
		CAPS_CATEGORY_VULKAN = 3,
		CAPS_CATEGORY_COUNT = 32 /**< Maximum number of categories. */
	};

	/** Enum describing the different hardware capabilities we can check for. */
	enum Capabilities : UINT64
	{
		RSC_TEXTURE_COMPRESSION_BC		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 0), /**< Supports compressed textures in the BC formats. */
		RSC_TEXTURE_COMPRESSION_ETC2	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 1), /**< Supports compressed textures in the ETC2 and EAC format. */
		RSC_TEXTURE_COMPRESSION_ASTC	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 2), /**< Supports compressed textures in the ASTC format. */
		RSC_GEOMETRY_PROGRAM			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 3), /**< Supports hardware geometry programs. */
		RSC_TESSELLATION_PROGRAM		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 4), /**< Supports hardware tessellation programs. */
		RSC_COMPUTE_PROGRAM				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 5), /**< Supports hardware compute programs. */
	};

	/** Holds data about render system driver version. */
	struct BS_CORE_EXPORT DriverVersion 
	{
		DriverVersion() { }

		/**	Returns the driver version as a single string. */
		String toString() const 
		{
			StringStream str;
			str << major << "." << minor << "." << release << "." << build;
			return str.str();
		}

		/** Parses a string in the major.minor.release.build format and stores the version numbers. */
		void fromString(const String& versionString)
		{
			Vector<bs::String> tokens = StringUtil::split(versionString, ".");
			if(!tokens.empty())
			{
				major = parseINT32(tokens[0]);
				if (tokens.size() > 1)
					minor = parseINT32(tokens[1]);
				if (tokens.size() > 2)
					release = parseINT32(tokens[2]);
				if (tokens.size() > 3)
					build = parseINT32(tokens[3]);
			}

		}

		INT32 major = 0;
		INT32 minor = 0;
		INT32 release = 0;
		INT32 build = 0;
	};

	/** Types of GPU vendors. */
	enum GPUVendor
	{
		GPU_UNKNOWN = 0,
		GPU_NVIDIA = 1,
		GPU_AMD = 2, 
		GPU_INTEL = 3,
		GPU_VENDOR_COUNT = 4
	};

	/**
	 * Holds information about render hardware and driver capabilities and allows you to easily set and query those 
	 * capabilities.
	 */
	class BS_CORE_EXPORT RenderAPICapabilities
	{
	public:	
		RenderAPICapabilities ();
		virtual ~RenderAPICapabilities ();

		/** Sets the current driver version. */
		void setDriverVersion(const DriverVersion& version)
		{
			mDriverVersion = version;
		}
		
		/**	Returns current driver version. */
		DriverVersion getDriverVersion() const
		{
			return mDriverVersion;
		}

		/**	Returns vendor of the currently used GPU. */
		GPUVendor getVendor() const
		{
			return mVendor;
		}

		/**	Sets the GPU vendor. */
		void setVendor(GPUVendor v)
		{
			mVendor = v;
		}

		/** Parses a vendor string and returns an enum with the vendor if parsed succesfully. */
		static GPUVendor vendorFromString(const String& vendorString);

		/** Converts a vendor enum to a string. */
		static String vendorToString(GPUVendor v);

		/**	Sets the maximum number of texture units per pipeline stage. */
		void setNumTextureUnits(GpuProgramType type, UINT16 num)
		{
			mNumTextureUnitsPerStage[type] = num;
		}

		/** Sets the maximum number of texture units in all pipeline stages. */
		void setNumCombinedTextureUnits(UINT16 num)
		{
			mNumCombinedTextureUnits = num;
		}

		/**	Sets the maximum number of load-store texture units per pipeline stage. */
		void setNumLoadStoreTextureUnits(GpuProgramType type, UINT16 num)
		{
			mNumLoadStoreTextureUnitsPerStage[type] = num;
		}

		/** Sets the maximum number of load-store texture units in all pipeline stages. */
		void setNumCombinedLoadStoreTextureUnits(UINT16 num)
		{
			mNumCombinedLoadStoreTextureUnits = num;
		}

		/**	Sets the maximum number of GPU param block buffers per pipeline stage. */
		void setNumGpuParamBlockBuffers(GpuProgramType type, UINT16 num)
		{
			mNumGpuParamBlocksPerStage[type] = num;
		}

		/**	Sets the maximum number of GPU param block buffers in all pipeline stages. */
		void setNumCombinedGpuParamBlockBuffers(UINT16 num)
		{
			mNumCombinedUniformBlocks = num;
		}

		/**	Sets maximum number of bound vertex buffers. */
		void setMaxBoundVertexBuffers(UINT32 num)
		{
			mMaxBoundVertexBuffers = num;
		}

		/**	Sets maximum number of simultaneously set render targets. */
		void setNumMultiRenderTargets(UINT16 num)
		{
			mNumMultiRenderTargets = num;
		}

		/**	Returns the number of texture units supported per pipeline stage. */
		UINT16 getNumTextureUnits(GpuProgramType type) const
		{
			auto iterFind = mNumTextureUnitsPerStage.find(type);
			if(iterFind != mNumTextureUnitsPerStage.end())
				return iterFind->second;
			else
				return 0;
		}

		/**	Returns the number of texture units supported in all pipeline stages. */
		UINT16 getNumCombinedTextureUnits() const
		{
			return mNumCombinedTextureUnits;
		}

		/**	Returns the number of load-store texture units supported per pipeline stage. */
		UINT16 getNumLoadStoreTextureUnits(GpuProgramType type) const
		{
			auto iterFind = mNumLoadStoreTextureUnitsPerStage.find(type);
			if (iterFind != mNumLoadStoreTextureUnitsPerStage.end())
				return iterFind->second;
			else
				return 0;
		}

		/**	Returns the number of load-store texture units supported in all pipeline stages. */
		UINT16 getNumCombinedLoadStoreTextureUnits() const
		{
			return mNumCombinedLoadStoreTextureUnits;
		}

		/**	Returns the maximum number of bound GPU program param block buffers per pipeline stage. */
		UINT16 getNumGpuParamBlockBuffers(GpuProgramType type) const
		{
			auto iterFind = mNumGpuParamBlocksPerStage.find(type);
			if(iterFind != mNumGpuParamBlocksPerStage.end())
				return iterFind->second;
			else
				return 0;
		}

		/** Returns the maximum number of bound GPU program param block buffers in all pipeline stages. */
		UINT16 getNumCombinedGpuParamBlockBuffers() const
		{
			return mNumCombinedUniformBlocks;
		}

		/** Returns the maximum number of vertex buffers that can be bound at once. */
		UINT32 getMaxBoundVertexBuffers() const
		{
			return mMaxBoundVertexBuffers;
		}

		/**	Returns the maximum number of render targets we can render to simultaneously. */
		UINT16 getNumMultiRenderTargets() const
		{
			return mNumMultiRenderTargets;
		}

		/**	Sets a capability flag indicating this capability is supported. */
		void setCapability(const Capabilities c) 
		{ 
			UINT64 index = (CAPS_CATEGORY_MASK & c) >> BS_CAPS_BITSHIFT;
			mCapabilities[index] |= (c & ~CAPS_CATEGORY_MASK);
		}

		/**	Remove a capability flag indicating this capability is not supported (default). */
		void unsetCapability(const Capabilities c) 
		{ 
			UINT64 index = (CAPS_CATEGORY_MASK & c) >> BS_CAPS_BITSHIFT;
			mCapabilities[index] &= (~c | CAPS_CATEGORY_MASK);
		}

		/**	Checks is the specified capability supported. */
		bool hasCapability(const Capabilities c) const
		{
			UINT64 index = (CAPS_CATEGORY_MASK & c) >> BS_CAPS_BITSHIFT;

			return (mCapabilities[index] & (c & ~CAPS_CATEGORY_MASK)) != 0;
		}

		/**	Adds a shader profile to the list of render-system specific supported profiles. */
		void addShaderProfile(const String& profile)
		{
			mSupportedShaderProfiles.insert(profile);
		}

		/**	Adds a mapping between GPU program profile enum and render-system specific profile name. */
		void addGpuProgramProfile(GpuProgramProfile gpuProgProfile, const String& rsSpecificProfile)
		{
			mGenericToSpecificShaderProfileMap[gpuProgProfile] = rsSpecificProfile;
		}

		/**	Returns true if the provided profile is supported. */
		bool isShaderProfileSupported(const String& profile) const
		{
			return (mSupportedShaderProfiles.end() != mSupportedShaderProfiles.find(profile));
		}

		/**	Returns a set of all supported shader profiles. */
		const Set<String>& getSupportedShaderProfiles() const
		{
			return mSupportedShaderProfiles;
		}

		/**
		 * Converts a generic GpuProgramProfile identifier into a render-system specific one. Returns an empty string if 
		 * conversion cannot be done.
		 */
		String gpuProgProfileToRSSpecificProfile(GpuProgramProfile gpuProgProfile) const
		{
			auto iterFind = mGenericToSpecificShaderProfileMap.find(gpuProgProfile);
			if(mGenericToSpecificShaderProfileMap.end() != iterFind)
			{
				return iterFind->second;
			}

			return "";
		}

		/**	Sets the current GPU device name. */
		void setDeviceName(const String& name)
		{
			mDeviceName = name;
		}

		/**	Gets the current GPU device name. */
		String getDeviceName() const
		{
			return mDeviceName;
		}

		/**	Sets the number of vertices a single geometry program run can emit. */
		void setGeometryProgramNumOutputVertices(int numOutputVertices)
		{
			mGeometryProgramNumOutputVertices = numOutputVertices;
		}

		/**	Gets the number of vertices a single geometry program run can emit. */
		int getGeometryProgramNumOutputVertices(void) const
		{
			return mGeometryProgramNumOutputVertices;
		}

		/**	Get the identifier of the render system from which these capabilities were generated. */
		StringID getRenderAPIName() const
		{
			return mRenderAPIName;
		}

		/**	Set the identifier of the render system from which these capabilities were generated. */
		void setRenderAPIName(const StringID& rs)
		{
			mRenderAPIName = rs;
		}

	private:
		/**	Initializes vendor enum -> vendor name mappings. */
		static void initVendorStrings();

	private:
		static Vector<String> msGPUVendorStrings;

		DriverVersion mDriverVersion;
		GPUVendor mVendor = GPU_UNKNOWN;

		// The number of texture units available per stage
		Map<GpuProgramType, UINT16> mNumTextureUnitsPerStage;
		// Total number of texture units available
		UINT16 mNumCombinedTextureUnits = 0;
		// The number of uniform blocks available per stage
		Map<GpuProgramType, UINT16> mNumGpuParamBlocksPerStage;
		// Total number of uniform blocks available
		UINT16 mNumCombinedUniformBlocks = 0;
		// The number of load-store texture unitss available per stage
		Map<GpuProgramType, UINT16> mNumLoadStoreTextureUnitsPerStage;
		// Total number of load-store texture units available
		UINT16 mNumCombinedLoadStoreTextureUnits = 0;
		// Maximum number of vertex buffers we can bind at once
		UINT32 mMaxBoundVertexBuffers = 0;
		// Stores the capabilities flags.
		UINT32 mCapabilities[CAPS_CATEGORY_COUNT];
		// The name of the device as reported by the render system
		String mDeviceName;
		// The identifier associated with the render API for which these capabilities are valid
		StringID mRenderAPIName;

		// The number of simultaneous render targets supported
		UINT16 mNumMultiRenderTargets = 0;
		// The number of vertices a geometry program can emit in a single run
		UINT32 mGeometryProgramNumOutputVertices = 0;

		// The list of supported shader profiles
		Set<String> mSupportedShaderProfiles;

		// Allows us to convert a generic shader profile to a render-system specific one
		UnorderedMap<GpuProgramProfile, String> mGenericToSpecificShaderProfileMap;
	};

	/** @} */
}
