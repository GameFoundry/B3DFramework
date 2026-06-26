//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuProgram.h"
#include "Utility/B3DDataBlob.h"
#include "Math/B3DMath.h"
#include <cstring>

namespace b3d
{
	namespace render
	{
		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Authoritative layout contract for the Metal backend's compiled-bytecode blob.
		 *
		 * The Metal backend does not persist a native object-code form — @c MTLLibrary is always built
		 * from MSL source text. The bytecode blob stored by @c GpuProgramBytecode::Instructions is
		 * therefore the MSL source the device emitter produced, wrapped in a small self-describing
		 * envelope so the reader can verify the blob it was handed is actually the backend's payload
		 * and extract the compute workgroup size without re-parsing the MSL.
		 *
		 * Byte layout, laid out contiguously:
		 *
		 *   [ u32 workgroupSize[3] ] (compute programs only)
		 *   [ u32 magic == kMetalMslSourceMagic ]
		 *   [ char[] MSL source, length = totalSize - headerSize - paddingSize ]
		 *   [ u8[] '\0' padding rounding total size up to a 4-byte boundary ]
		 *
		 * The magic doubles as a MoltenVK-compatibility marker so a single reader can consume either
		 * producer's output. @c GpuProgramBytecode::CompilerId and @c ::CompilerVersion travel in
		 * parallel fields on @c GpuProgramBytecode — not inside @c Instructions — and are validated
		 * against @c kMetalCompilerId / @c kMetalCompilerVersion by the reader before it consults the
		 * payload.
		 *
		 * All values are little-endian because every Metal-capable platform Banshee supports (macOS,
		 * iOS, iPadOS, visionOS — all Apple Silicon or x86-64) is little-endian; no byte-swap is
		 * performed on read.
		 */

		/** Identifier of the compiler used for compiling Metal GPU programs. */
		inline constexpr const char* kMetalCompilerId = "Metal";

		/**
		 * Version of the compiler used for compiling Metal GPU programs. Tick this whenever the compiler
		 * updates in order to force bytecode to rebuild.
		 */
		inline constexpr u32 kMetalCompilerVersion = 1;

		/** Magic header in the MSL bytecode blob identifying it as MSL source text (matches MoltenVK's magic). */
		inline constexpr u32 kMetalMslSourceMagic = 0x19960412;

		/** Sentinel default for the compute workgroup size when no value has been read. */
		inline constexpr u32 kMetalDefaultWorkgroupSize[3] = { 1, 1, 1 };

		/**
		 * Canonical per-type ordering used when assigning argument-buffer indices inside a Metal
		 * argument buffer. Two sites must agree on this order byte-for-byte:
		 *
		 *   1. @c MetalGpuPipelineParameterSetLayout's ctor, which walks the base-class per-type
		 *      uniform lists and appends bindings in this exact sequence before assigning
		 *      @c ArgIndex values.
		 *   2. The native Metal bytecode compiler's SPIRV-Cross hook (BytecodeCompilerMSL), which folds
		 *      the same five lists into a single packed u64 sort key (Set:24 | TypeOrder:8 | Slot:32) so
		 *      the emitted MSL uses identical @c msl_buffer / @c msl_texture / @c msl_sampler values as
		 *      the layout's MTLArgumentEncoder.
		 *
		 * They live here, in the shared bytecode-layout contract, so the offline compiler (bsfBytecodeMSL)
		 * and the runtime backend (bsfMetalGpuBackend) consume the single authoritative copy — an earlier
		 * bug (phase-2 review S2) hard-coded the values independently in each site and any edit to one
		 * would silently corrupt shader bindings until the other was updated in lockstep.
		 */
		static constexpr u64 kTypeOrderUniformBuffer  = 0;
		static constexpr u64 kTypeOrderSampledTexture = 1;
		static constexpr u64 kTypeOrderStorageTexture = 2;
		static constexpr u64 kTypeOrderStorageBuffer  = 3;
		static constexpr u64 kTypeOrderSampler        = 4;

		/**
		 * Result of parsing the bytecode envelope. @c MslSource and @c MslSize describe a read-only window
		 * into the caller's @c GpuProgramBytecode — lifetime is tied to the source buffer.
		 */
		struct MetalBytecodePayload
		{
			const u8* MslSource = nullptr;  /**< Pointer to the first byte of MSL source inside the blob. */
			u32 MslSize = 0;                /**< Number of valid MSL source bytes (trailing zero padding stripped). */
			u32 WorkgroupSize[3] = { 1, 1, 1 }; /**< Compute workgroup size; all ones for non-compute programs. */
			bool IsValid = false;           /**< False if the blob failed the magic check or was too small. */
		};

		/**
		 * Parses a @c GpuProgramBytecode::Instructions blob according to the Metal bytecode layout
		 * documented above. Returns @c MetalBytecodePayload::IsValid == false if the blob is truncated
		 * or the magic does not match.
		 *
		 * @param	programType		Used to determine whether a leading workgroup-size triple is present.
		 * @param	bytecode		Raw blob bytes — must remain valid for the lifetime of the returned
		 *							@c MetalBytecodePayload::MslSource.
		 * @param	bytecodeSize	Number of bytes in @p bytecode.
		 */
		inline MetalBytecodePayload ReadMetalBytecode(GpuProgramType programType, const u8* bytecode, u32 bytecodeSize)
		{
			MetalBytecodePayload payload;
			payload.WorkgroupSize[0] = kMetalDefaultWorkgroupSize[0];
			payload.WorkgroupSize[1] = kMetalDefaultWorkgroupSize[1];
			payload.WorkgroupSize[2] = kMetalDefaultWorkgroupSize[2];

			if (bytecode == nullptr || bytecodeSize == 0)
				return payload;

			const u8* cursor = bytecode;
			u32 remaining = bytecodeSize;

			// Compute programs prepend a 3-u32 workgroup size to the MSL payload.
			if (programType == GPT_COMPUTE_PROGRAM)
			{
				if (remaining <= sizeof(payload.WorkgroupSize))
					return payload;

				std::memcpy(payload.WorkgroupSize, cursor, sizeof(payload.WorkgroupSize));
				cursor += sizeof(payload.WorkgroupSize);
				remaining -= (u32)sizeof(payload.WorkgroupSize);
			}

			// Expect the magic tag that marks the remaining bytes as MSL source text.
			if (remaining < sizeof(u32))
				return payload;

			u32 magic = 0;
			std::memcpy(&magic, cursor, sizeof(magic));
			if (magic != kMetalMslSourceMagic)
				return payload;

			cursor += sizeof(u32);
			remaining -= (u32)sizeof(u32);

			// Strip any trailing padding (the bytecode is rounded up to a 4-byte boundary).
			while (remaining > 0 && cursor[remaining - 1] == '\0')
				remaining--;

			payload.MslSource = cursor;
			payload.MslSize = remaining;
			payload.IsValid = true;
			return payload;
		}

		/**
		 * Serializes an MSL source payload into a newly allocated @c DataBlob matching the Metal
		 * bytecode layout contract. Allocates via @c B3DAllocate — the caller assumes ownership of
		 * @c DataBlob::Data and must free it with @c B3DFree (typically via
		 * @c GpuProgramBytecode::~GpuProgramBytecode).
		 *
		 * @param	programType			When equal to @c GPT_COMPUTE_PROGRAM, @p workgroupSize is written
		 *								in front of the magic.
		 * @param	workgroupSize		Compute workgroup size; ignored for non-compute program types.
		 * @param	mslSource			Pointer to the MSL source bytes. May be null only if @p mslSize
		 *								is zero, which fails the write.
		 * @param	mslSize				Number of MSL source bytes to copy.
		 * @return						A @c DataBlob owning the packed bytes, or a zero-filled blob if
		 *								@p mslSize is zero.
		 */
		inline DataBlob WriteMetalBytecode(GpuProgramType programType, const u32 workgroupSize[3], const char* mslSource, u32 mslSize)
		{
			DataBlob blob;
			if (mslSource == nullptr || mslSize == 0)
				return blob;

			const bool isCompute = programType == GPT_COMPUTE_PROGRAM;
			u32 payloadSize = mslSize + (u32)sizeof(u32); // source + magic
			if (isCompute)
				payloadSize += (u32)sizeof(u32) * 3;

			const u32 wordCount = Math::DivideAndRoundUp(payloadSize, 4u);
			const u32 totalSize = wordCount * 4;
			u8* buffer = (u8*)B3DAllocate(totalSize);
			u8* dst = buffer;

			if (isCompute)
			{
				std::memcpy(dst, workgroupSize, sizeof(u32) * 3);
				dst += sizeof(u32) * 3;
			}

			const u32 magic = kMetalMslSourceMagic;
			std::memcpy(dst, &magic, sizeof(magic));
			dst += sizeof(magic);

			std::memcpy(dst, mslSource, mslSize);

			for (u32 byteIndex = payloadSize; byteIndex < totalSize; byteIndex++)
				buffer[byteIndex] = '\0';

			blob.Size = totalSize;
			blob.Data = buffer;
			return blob;
		}

		// TODO(C10): round-trip unit test — construct a payload with WriteMetalBytecode, feed the
		// resulting bytes through ReadMetalBytecode, assert magic / MslSize / WorkgroupSize round-trip
		// exactly. Lands with the test-harness batch.

		/** @} */
	} // namespace render
} // namespace b3d
