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

#define MAX_BOUND_VERTEX_BUFFERS 32

namespace BansheeEngine 
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Categories of render API capabilities. */
	enum CapabilitiesCategory : UINT64
	{
		CAPS_CATEGORY_COMMON = 0,
		CAPS_CATEGORY_D3D9 = 1,
		CAPS_CATEGORY_GL = 2,
		CAPS_CATEGORY_D3D11 = 3,
		CAPS_CATEGORY_COUNT = 32 /**< Maximum number of categories. */
	};

	/** Enum describing the different hardware capabilities we can check for. */
	enum Capabilities : UINT64
	{
		RSC_AUTOMIPMAP				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 0), /**< Supports generating mipmaps in hardware. */
		RSC_ANISOTROPY				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 1), /**< Supports anisotropic texture filtering. */
		RSC_CUBEMAPPING				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 2), /**< Supports cube mapping. */
		RSC_TWO_SIDED_STENCIL		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 3), /**< Supports separate stencil updates for both front and back faces. */
		RSC_STENCIL_WRAP			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 4), /**< Supports wrapping the stencil value at the range extremes. */
		RSC_HWOCCLUSION				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 5), /**< Supports hardware occlusion queries. */
		RSC_USER_CLIP_PLANES		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 6), /**< Supports user clipping planes. */
		RSC_VERTEX_FORMAT_UBYTE4	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 7), /**< Supports the VET_UBYTE4 vertex element type. */
		RSC_INFINITE_FAR_PLANE		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 8), /**< Supports infinite far plane projection. */
		RSC_HWRENDER_TO_TEXTURE		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 9), /**< Supports hardware render-to-texture. */
		RSC_TEXTURE_FLOAT			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 10), /**< Supports float textures and render targets. */
		RSC_NON_POWER_OF_2_TEXTURES = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 11), /**< Supports non-power of two textures. */
		RSC_TEXTURE_3D				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 12), /**< Supports 3d (volume) textures. */
		RSC_POINT_SPRITES			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 13), /**< Supports basic point sprite rendering. */

		RSC_POINT_EXTENDED_PARAMETERS	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 14), /**< Supports extra point parameters (minsize, maxsize, attenuation). */
		RSC_VERTEX_TEXTURE_FETCH		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 15), /**< Supports vertex texture fetch. */
		RSC_MIPMAP_LOD_BIAS				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 16), /**< Supports mipmap LOD biasing. */
		RSC_GEOMETRY_PROGRAM			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 17), /**< Supports hardware geometry programs. */

		RSC_TEXTURE_COMPRESSION			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 18), /**< Supports compressed textures. */
		RSC_TEXTURE_COMPRESSION_DXT		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 19), /**< Supports compressed textures in the DXT/ST3C formats. */
		RSC_TEXTURE_COMPRESSION_VTC		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 20), /**< Supports compressed textures in the VTC format. */
		RSC_TEXTURE_COMPRESSION_PVRTC	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 21), /**< Supports compressed textures in the PVRTC format. */
		RSC_MRT_DIFFERENT_BIT_DEPTHS	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 22), /**< Supports multiple render targets with different bit depths. */
		RSC_ALPHA_TO_COVERAGE			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 23), /**< Supports Alpha to Coverage. */
		RSC_ADVANCED_BLEND_OPERATIONS	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 24), /**< Supports blend operations like subtract, min, max. */
		RSC_SHADER_SUBROUTINE			= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 25), /**< Supports dynamic shader linking. */
		RSC_HWOCCLUSION_ASYNCHRONOUS	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 26), /**< Support for async occlusion queries. */
		RSC_HWRENDER_TO_VERTEX_BUFFER	= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 27), /**< Supports rendering to vertex buffers. */
		RSC_TESSELLATION_PROGRAM		= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 28), /**< Supports hardware tessellation programs. */
		RSC_COMPUTE_PROGRAM				= BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 29), /**< Supports hardware compute programs. */

		// ***** DirectX 9 specific caps *****
		RSC_PERSTAGECONSTANT = BS_CAPS_VALUE(CAPS_CATEGORY_D3D9, 0), /**< Are per stage constants supported. */

		// ***** GL Specific caps *****
		RSC_FBO              = BS_CAPS_VALUE(CAPS_CATEGORY_GL, 0), /**< Support for Frame Buffer Objects. */
		RSC_PBUFFER			 = BS_CAPS_VALUE(CAPS_CATEGORY_GL, 1), /**< Support for PBuffers. */
	};

	/** Holds data about render system driver version. */
	struct BS_CORE_EXPORT DriverVersion 
	{
		int major;
		int minor;
		int release;
		int build;

		DriverVersion() 
		{
			major = minor = release = build = 0;
		}

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
			Vector<BansheeEngine::String> tokens = StringUtil::split(versionString, ".");
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

		/**	Sets maximum stencil buffer depth in bits. */
		void setStencilBufferBitDepth(UINT16 num)
		{
			mStencilBufferBitDepth = num;
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

		/** Returns the maximum number of bits available for the stencil buffer. */
		UINT16 getStencilBufferBitDepth() const
		{
			return mStencilBufferBitDepth;
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

		/** Gets the number of floating-point constants vertex programs support. */
		UINT16 getVertexProgramConstantFloatCount() const
		{
			return mVertexProgramConstantFloatCount;           
		}

		/**	Gets the number of integer constants vertex programs support. */
		UINT16 getVertexProgramConstantIntCount() const
		{
			return mVertexProgramConstantIntCount;           
		}

		/**	Gets the number of boolean constants vertex programs support. */
		UINT16 getVertexProgramConstantBoolCount() const
		{
			return mVertexProgramConstantBoolCount;           
		}

		/**	Gets the number of floating-point constants geometry programs support. */
		UINT16 getGeometryProgramConstantFloatCount() const
		{
			return mGeometryProgramConstantFloatCount;           
		}

		/**	 Gets the number of integer constants geometry programs support. */
		UINT16 getGeometryProgramConstantIntCount() const
		{
			return mGeometryProgramConstantIntCount;           
		}

		/**	 Gets the number of boolean constants geometry programs support. */
		UINT16 getGeometryProgramConstantBoolCount() const
		{
			return mGeometryProgramConstantBoolCount;           
		}

		/**	Gets the number of floating-point constants fragment programs support. */
		UINT16 getFragmentProgramConstantFloatCount() const
		{
			return mFragmentProgramConstantFloatCount;           
		}

		/**	Gets the number of integer constants fragment programs support. */
		UINT16 getFragmentProgramConstantIntCount() const
		{
			return mFragmentProgramConstantIntCount;           
		}
		
		/** Gets the number of boolean constants fragment programs support. */
		UINT16 getFragmentProgramConstantBoolCount() const
		{
			return mFragmentProgramConstantBoolCount;           
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

		/**	Sets the number of floating-point constants vertex programs support. */
		void setVertexProgramConstantFloatCount(UINT16 c)
		{
			mVertexProgramConstantFloatCount = c;           
		}

		/**	Sets the number of integer constants vertex programs support. */
		void setVertexProgramConstantIntCount(UINT16 c)
		{
			mVertexProgramConstantIntCount = c;           
		}

		/**	Sets the number of boolean constants vertex programs support. */
		void setVertexProgramConstantBoolCount(UINT16 c)
		{
			mVertexProgramConstantBoolCount = c;           
		}

		/**	Sets the number of floating-point constants geometry programs support. */
		void setGeometryProgramConstantFloatCount(UINT16 c)
		{
			mGeometryProgramConstantFloatCount = c;           
		}

		/**	Sets the number of integer constants geometry programs support. */
		void setGeometryProgramConstantIntCount(UINT16 c)
		{
			mGeometryProgramConstantIntCount = c;           
		}

		/**	Sets the number of boolean constants geometry programs support. */
		void setGeometryProgramConstantBoolCount(UINT16 c)
		{
			mGeometryProgramConstantBoolCount = c;           
		}

		/**	Sets the number of floating-point constants fragment programs support. */
		void setFragmentProgramConstantFloatCount(UINT16 c)
		{
			mFragmentProgramConstantFloatCount = c;           
		}

		/**	Sets the number of integer constants fragment programs support. */
		void setFragmentProgramConstantIntCount(UINT16 c)
		{
			mFragmentProgramConstantIntCount = c;           
		}

		/**	Sets the number of boolean constants fragment programs support. */
		void setFragmentProgramConstantBoolCount(UINT16 c)
		{
			mFragmentProgramConstantBoolCount = c;           
		}

		/**	Sets the maximum point screen size in pixels. */
		void setMaxPointSize(float s)
		{
			mMaxPointSize = s;
		}

		/**	Gets the maximum point screen size in pixels. */
		float getMaxPointSize(void) const
		{
			return mMaxPointSize;
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
		// The stencil buffer bit depth
		UINT16 mStencilBufferBitDepth = 0;
		// Maximum number of vertex buffers we can bind at once
		UINT32 mMaxBoundVertexBuffers = 0;
		// Stores the capabilities flags.
		UINT32 mCapabilities[CAPS_CATEGORY_COUNT];
		// The name of the device as reported by the render system
		String mDeviceName;
		// The identifier associated with the render API for which these capabilities are valid
		StringID mRenderAPIName;

		// The number of floating-point constants vertex programs support
		UINT16 mVertexProgramConstantFloatCount = 0;
		// The number of integer constants vertex programs support
		UINT16 mVertexProgramConstantIntCount = 0;
		// The number of boolean constants vertex programs support
		UINT16 mVertexProgramConstantBoolCount = 0;
		// The number of floating-point constants geometry programs support
		UINT16 mGeometryProgramConstantFloatCount = 0;
		// The number of integer constants vertex geometry support
		UINT16 mGeometryProgramConstantIntCount = 0;
		// The number of boolean constants vertex geometry support
		UINT16 mGeometryProgramConstantBoolCount = 0;
		// The number of floating-point constants fragment programs support
		UINT16 mFragmentProgramConstantFloatCount = 0;
		// The number of integer constants fragment programs support
		UINT16 mFragmentProgramConstantIntCount = 0;
		// The number of boolean constants fragment programs support
		UINT16 mFragmentProgramConstantBoolCount = 0;
		// The number of simultaneous render targets supported
		UINT16 mNumMultiRenderTargets = 0;
		// The maximum point size in pixels
		float mMaxPointSize = 0.0f;
		// The number of vertices a geometry program can emit in a single run
		UINT32 mGeometryProgramNumOutputVertices = 0;

		// The list of supported shader profiles
		Set<String> mSupportedShaderProfiles;

		// Allows us to convert a generic shader profile to a render-system specific one
		UnorderedMap<GpuProgramProfile, String> mGenericToSpecificShaderProfileMap;
	};

	/** @} */
}
