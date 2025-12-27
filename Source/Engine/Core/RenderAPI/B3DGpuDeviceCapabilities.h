//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "RenderAPI/B3DGpuProgram.h"

#include <cstdint>
#define CAPS_CATEGORY_SIZE INT64_C(8)
#define BS_CAPS_BITSHIFT (INT64_C(64) - CAPS_CATEGORY_SIZE)
#define CAPS_CATEGORY_MASK (((INT64_C(1) << CAPS_CATEGORY_SIZE) - INT64_C(1)) << BS_CAPS_BITSHIFT)
#define BS_CAPS_VALUE(cat, val) ((cat << BS_CAPS_BITSHIFT) | (INT64_C(1) << val))

#define B3D_MAX_BOUND_VERTEX_BUFFERS 16

namespace b3d
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Categories of render API capabilities. */
	enum GpuCapabilityCategory : u64
	{
		CAPS_CATEGORY_COMMON = 0,
		CAPS_CATEGORY_GL = 1,
		CAPS_CATEGORY_D3D11 = 2,
		CAPS_CATEGORY_VULKAN = 3,
		CAPS_CATEGORY_COUNT = 32 /**< Maximum number of categories. */
	};

	/** Enum describing the different hardware capabilities we can check for. */
	enum GpuCapabilityBits : u64
	{
		/** Supports compressed textures in the BC formats. */
		RSC_TEXTURE_COMPRESSION_BC = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 0),
		/** Supports compressed textures in the ETC2 and EAC format. */
		RSC_TEXTURE_COMPRESSION_ETC2 = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 1),
		/** Supports compressed textures in the ASTC format. */
		RSC_TEXTURE_COMPRESSION_ASTC = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 2),
		/** Supports GPU geometry programs. */
		RSC_GEOMETRY_PROGRAM = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 3),
		/** Supports GPU tessellation programs. */
		RSC_TESSELLATION_PROGRAM = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 4),
		/** Supports GPU compute programs. */
		RSC_COMPUTE_PROGRAM = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 5),
		/** Supports load-store (unordered access) writes to textures/buffers in GPU programs. */
		RSC_LOAD_STORE = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 6),
		/** Supports load-store (unordered access) writes to textures with multiple samples. */
		RSC_LOAD_STORE_MSAA = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 7),
		/**
		 * Supports views that allow a sub-set of a texture to be bound to a GPU program. (i.e. specific mip level or mip
		 * range, and/or specific array slice or array slice range)
		 */
		RSC_TEXTURE_VIEWS = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 8),
		/** GPU programs are allowed to cache their bytecode for faster compilation. */
		RSC_BYTECODE_CACHING = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 9),
		/** Supports rendering to multiple layers of a render texture at once. */
		RSC_RENDER_TARGET_LAYERS = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 10),
		/** Has native support for command buffers that can be populated from secondary threads. */
		RSC_MULTI_THREADED_CB = BS_CAPS_VALUE(CAPS_CATEGORY_COMMON, 11),
	};

	/** Conventions used for a specific render backend. */
	struct B3D_EXPORT GpuBackendConventions
	{
		enum class Axis : u8
		{
			Up,
			Down
		};

		enum class MatrixOrder : u8
		{
			ColumnMajor,
			RowMajor
		};

		/** Determines the direction of the Y axis in UV (texture mapping) space. */
		Axis UvYAxis = Axis::Down;

		/** Determines the direction of the Y axis in normalized device coordinate (NDC) space. */
		Axis NdcYAxis = Axis::Up;

		/** Determines the order in which matrices are laid out in GPU programs. */
		MatrixOrder MatrixOrder = MatrixOrder::RowMajor;
	};

	/** Holds data about render system driver version. */
	struct B3D_EXPORT GpuDriverVersion
	{
		GpuDriverVersion() = default;

		/**	Returns the driver version as a single string. */
		String ToString() const
		{
			StringStream str;
			str << Major << "." << Minor << "." << Release << "." << Build;
			return str.str();
		}

		/** Parses a string in the major.minor.release.build format and stores the version numbers. */
		void FromString(const String& versionString)
		{
			Vector<b3d::String> tokens = StringUtil::Split(versionString, ".");
			if(!tokens.empty())
			{
				Major = ParseI32(tokens[0]);
				if(tokens.size() > 1)
					Minor = ParseI32(tokens[1]);
				if(tokens.size() > 2)
					Release = ParseI32(tokens[2]);
				if(tokens.size() > 3)
					Build = ParseI32(tokens[3]);
			}
		}

		i32 Major = 0;
		i32 Minor = 0;
		i32 Release = 0;
		i32 Build = 0;
	};

	/** Types of GPU vendors. */
	enum GPUVendor
	{
		GPU_UNKNOWN = 0,
		GPU_NVIDIA = 1,
		GPU_AMD = 2,
		GPU_INTEL = 3,
		GPU_APPLE = 4,
		GPU_VENDOR_COUNT = 5
	};

	/** Information about hardware (GPU) and driver capabilities, such as supported features, limits and conventions. */
	class B3D_EXPORT GpuDeviceCapabilities final
	{
	public:
		/** The identifier associated with the GPU backend. */
		StringID BackendName; // TODO - Move outside of capabilities into a separate structure

		/** Current version of the GPU driver. */
		GpuDriverVersion DriverVersion; // TODO - Move outside of capabilities into a separate structure

		/** The name of the GPU device as reported by the GPU backend. */
		String DeviceName; // TODO - Move outside of capabilities into a separate structure

		/** Vendor of the GPU device. */
		GPUVendor DeviceVendor = GPU_UNKNOWN; // TODO - Move outside of capabilities into a separate structure

		/** The number of texture units available per stage. */
		u16 NumTextureUnitsPerStage[GPT_COUNT]{ 0 };

		/** Total number of texture units available. */
		u16 NumCombinedTextureUnits = 0;

		/** The number of parameter block buffers available per stage. */
		u16 NumGpuParamBlockBuffersPerStage[GPT_COUNT]{ 0 };

		/** Total number of parameter block buffers available. */
		u16 NumCombinedParamBlockBuffers = 0;

		/** The number of load-store texture unitss available per stage. */
		u16 NumLoadStoreTextureUnitsPerStage[GPT_COUNT]{ 0 };

		/** Total number of load-store texture units available. */
		u16 NumCombinedLoadStoreTextureUnits = 0;

		/** Maximum number of vertex buffers we can bind at once. */
		u32 MaxBoundVertexBuffers = 0;

		/** The number of simultaneous render targets supported. */
		u16 NumMultiRenderTargets = 0;

		/** The number of vertices a geometry program can emit in a single run. */
		u32 GeometryProgramNumOutputVertices = 0;

		/** Minimum alignment required for uniform buffers that are sub-allocated within a larger buffer. */
		u32 MinimumUniformBufferOffsetAlignment = 16;

		/** Optimal alignment for offsets used in buffers for copy source or destination operations, when copying from a buffer to another buffer. */
		u32 OptimalBufferToBufferCopyOffsetAlignment = 16;

		/** Optimal alignment for offsets used in buffers for copy source or destination operations, when copying from a buffer to an image, or vice versa. */
		u32 OptimalBufferToImageCopyOffsetAlignment = 16;

		/** Horizontal texel offset used for mapping texels to pixels. */
		float HorizontalTexelOffset = 0.0f;

		/** Vertical texel offset used for mapping texels to pixels. */
		float VerticalTexelOffset = 0.0f;

		/** Minimum (closest) depth value used by this render backend */
		float MinDepth = 0.0f;

		/** Maximum (farthest) depth value used by this render backend. */
		float MaxDepth = 1.0f;

		/** Returns various conventions expected by the render backend. */
		GpuBackendConventions Conventions;

		/** Native type used for vertex colors. */
		VertexElementType VertexColorType = VET_COLOR_ABGR;

		/**	Sets a capability flag indicating this capability is supported. */
		void SetCapability(const GpuCapabilityBits c)
		{
			u64 index = (CAPS_CATEGORY_MASK & c) >> BS_CAPS_BITSHIFT;
			mCapabilities[index] |= (c & ~CAPS_CATEGORY_MASK);
		}

		/**	Remove a capability flag indicating this capability is not supported (default). */
		void UnsetCapability(const GpuCapabilityBits c)
		{
			u64 index = (CAPS_CATEGORY_MASK & c) >> BS_CAPS_BITSHIFT;
			mCapabilities[index] &= (~c | CAPS_CATEGORY_MASK);
		}

		/**	Checks is the specified capability supported. */
		bool HasCapability(const GpuCapabilityBits c) const
		{
			u64 index = (CAPS_CATEGORY_MASK & c) >> BS_CAPS_BITSHIFT;

			return (mCapabilities[index] & (c & ~CAPS_CATEGORY_MASK)) != 0;
		}

		/**	Adds a shader profile to the list of render-system specific supported profiles. */
		void AddShaderProfile(const String& profile)
		{
			mSupportedShaderProfiles.insert(profile);
		}

		/**	Returns true if the provided profile is supported. */
		bool IsShaderProfileSupported(const String& profile) const
		{
			return (mSupportedShaderProfiles.end() != mSupportedShaderProfiles.find(profile));
		}

		/**	Returns a set of all supported shader profiles. */
		const UnorderedSet<String>& GetSupportedShaderProfiles() const
		{
			return mSupportedShaderProfiles;
		}

		/** Parses a vendor string and returns an enum with the vendor if parsed succesfully. */
		static GPUVendor VendorFromString(const String& vendorString);

		/** Converts a vendor enum to a string. */
		static String VendorToString(GPUVendor vendor);

	private:
		static char const* const kGpuVendorStrings[GPU_VENDOR_COUNT];

		/** Stores the capabilities flags. */
		u32 mCapabilities[CAPS_CATEGORY_COUNT]{ 0 };

		/** The list of supported shader profiles. */
		UnorderedSet<String> mSupportedShaderProfiles;
	};

	/** @} */
} // namespace b3d
