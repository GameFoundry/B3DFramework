//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuProgram.h"
#include <limits>

namespace b3d
{
	namespace render
	{
		/** Shared source-generation, offline-reflection, and runtime ABI for native Metal shaders.
		 *
		 * @addtogroup MetalGpuBackend
		 *  @{
		 */

		/** Canonical resource-class order used when assigning Metal argument-buffer indices. */
		inline constexpr u64 kTypeOrderUniformBuffer = 0;
		inline constexpr u64 kTypeOrderSampledTexture = 1;
		inline constexpr u64 kTypeOrderStorageTexture = 2;
		inline constexpr u64 kTypeOrderStorageBuffer = 3;
		inline constexpr u64 kTypeOrderSampler = 4;

		/** Maximum descriptor-array length accepted by the Metal binding ABI. */
		inline constexpr u32 kMetalArgumentBufferArrayStride = 1024;

		/** Highest engine resource slot supported within a Metal parameter set. */
		inline constexpr u32 kMetalMaximumArgumentBufferSlot = 63;

		/** Highest parameter-set index; Metal buffer slots 16 and above are reserved for vertex streams. */
		inline constexpr u32 kMetalMaximumParameterSetIndex = 15;

		/** Apple7 Tier-2 per-stage texture-resource limit across argument buffers. */
		inline constexpr u32 kMetalMaximumTextureArgumentsPerStage = 1048576;

		/** Apple7 Tier-2 per-stage sampler-resource limit across argument buffers. */
		inline constexpr u32 kMetalMaximumSamplerArgumentsPerStage = 996;

		/** Prefix embedded in MSL argument-buffer member names so native reflection can recover engine bindings. */
		inline constexpr const char* kMetalResourceNamePrefix = "b3d_msl_r_";

		/** Builds the identifier emitted for one MSL argument-buffer member. */
		inline String EncodeMetalResourceName(GpuParameterType type, u32 set, u32 slot, const String& name)
		{
			return StringUtility::Format("{0}{1}_{2}_{3}_{4}", kMetalResourceNamePrefix, (u32)type, set, slot, name);
		}

		/** Decodes an engine resource binding from an MSL argument-buffer member name. */
		inline bool DecodeMetalResourceName(const char* encodedName, GpuParameterType& outType, u32& outSet, u32& outSlot, String& outName)
		{
			if(encodedName == nullptr)
				return false;

			const String encoded(encodedName);
			const String prefix(kMetalResourceNamePrefix);
			if(encoded.size() <= prefix.size() || !StringUtility::StartsWith(encoded, prefix, false))
				return false;

			const Vector<String> components = StringUtility::Split(encoded.substr(prefix.size()), "_", 2);
			if(components.size() != 3)
				return false;

			const auto fnParseValue = [](const String& value, u32& outValue)
			{
				if(value.empty())
					return false;

				const u32 parsed = ParseU32(value, std::numeric_limits<u32>::max());
				if(ToString(parsed) != value)
					return false;

				outValue = parsed;
				return true;
			};

			u32 type = 0;
			u32 set = 0;
			if(!fnParseValue(components[0], type) || !fnParseValue(components[1], set))
				return false;

			if(type >= (u32)GpuParameterType::Count)
				return false;

			const u32 slot = ParseU32(components[2], std::numeric_limits<u32>::max());
			const String bindingPrefix = StringUtility::Format("{0}{1}_{2}_{3}_", kMetalResourceNamePrefix, type, set, slot);
			if(!StringUtility::StartsWith(encoded, bindingPrefix, false) || encoded.size() == bindingPrefix.size())
				return false;

			outType = (GpuParameterType)type;
			outSet = set;
			outSlot = slot;
			outName = encoded.substr(bindingPrefix.size());

			return true;
		}

		/** @} */
	} // namespace render
} // namespace b3d
