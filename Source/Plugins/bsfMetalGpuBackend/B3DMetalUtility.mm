//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DMetalUtility.h"
#include "Image/B3DPixelUtility.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "Debug/B3DLog.h"
#include "Threading/B3DThreading.h"
#include <atomic>

// TODO(C14): migrate the warn-once pattern below (and the two matching sites in
// B3DMetalGpuCommandBuffer.mm) to a shared @c LogOnce(Level, Category, ...) helper once its home
// is decided — keeping the pattern local here avoids pulling a new utility header into every
// Metal backend TU for a three-site cleanup.

namespace b3d
{
	namespace render
	{
		namespace
		{
			// File-local helper so @c GetPixelFormat's default / TARGET_OS_IPHONE paths can drop a
			// warn-once log line before returning @c MTLPixelFormatInvalid. Per-format dedup means
			// unmapped formats are surfaced one-by-one as they're first encountered, instead of a
			// single overall log hiding the rest. The set is guarded by a mutex so worker fibers on
			// different threads that first hit the same format can't race on the insert — without the
			// lock two fibers could both @c find-miss, both call @c insert, and either corrupt the
			// set internals or log the same format twice.
			MTLPixelFormat InvalidFormatWarn(PixelFormat format)
			{
				static Mutex sWarnedFormatsMutex;
				static UnorderedSet<u32> sWarnedFormats;
				bool isFirst = false;
				{
					Lock lock(sWarnedFormatsMutex);
					isFirst = sWarnedFormats.insert((u32)format).second;
				}
				if (isFirst)
				{
					B3D_LOG(Warning, LogRenderBackend,
						"No MTLPixelFormat mapping for PixelFormat={0}; returning MTLPixelFormatInvalid. "
						"Texture / render-target creation will fail for this format until a mapping is added in B3DMetalUtility.mm.",
						(u32)format);
				}
				return MTLPixelFormatInvalid;
			}
		}

		MTLPixelFormat MetalUtility::GetPixelFormat(PixelFormat format, bool gamma)
		{
			switch (format)
			{
			case PF_R8:			return MTLPixelFormatR8Unorm;
			case PF_RG8:		return MTLPixelFormatRG8Unorm;
			case PF_BGRA8:		return gamma ? MTLPixelFormatBGRA8Unorm_sRGB : MTLPixelFormatBGRA8Unorm;
			case PF_RGBA8:		return gamma ? MTLPixelFormatRGBA8Unorm_sRGB : MTLPixelFormatRGBA8Unorm;
			case PF_R8I:		return MTLPixelFormatR8Sint;
			case PF_RG8I:		return MTLPixelFormatRG8Sint;
			case PF_RGBA8I:		return MTLPixelFormatRGBA8Sint;
			case PF_R8U:		return MTLPixelFormatR8Uint;
			case PF_RG8U:		return MTLPixelFormatRG8Uint;
			case PF_RGBA8U:		return MTLPixelFormatRGBA8Uint;
			case PF_R8S:		return MTLPixelFormatR8Snorm;
			case PF_RG8S:		return MTLPixelFormatRG8Snorm;
			case PF_RGBA8S:		return MTLPixelFormatRGBA8Snorm;
			case PF_R16F:		return MTLPixelFormatR16Float;
			case PF_RG16F:		return MTLPixelFormatRG16Float;
			case PF_RGBA16F:	return MTLPixelFormatRGBA16Float;
			case PF_R32F:		return MTLPixelFormatR32Float;
			case PF_RG32F:		return MTLPixelFormatRG32Float;
			case PF_RGBA32F:	return MTLPixelFormatRGBA32Float;
			case PF_R16I:		return MTLPixelFormatR16Sint;
			case PF_RG16I:		return MTLPixelFormatRG16Sint;
			case PF_RGBA16I:	return MTLPixelFormatRGBA16Sint;
			case PF_R16U:		return MTLPixelFormatR16Uint;
			case PF_RG16U:		return MTLPixelFormatRG16Uint;
			case PF_RGBA16U:	return MTLPixelFormatRGBA16Uint;
			case PF_R32I:		return MTLPixelFormatR32Sint;
			case PF_RG32I:		return MTLPixelFormatRG32Sint;
			case PF_RGBA32I:	return MTLPixelFormatRGBA32Sint;
			case PF_R32U:		return MTLPixelFormatR32Uint;
			case PF_RG32U:		return MTLPixelFormatRG32Uint;
			case PF_RGBA32U:	return MTLPixelFormatRGBA32Uint;
			case PF_R16S:		return MTLPixelFormatR16Snorm;
			case PF_RG16S:		return MTLPixelFormatRG16Snorm;
			case PF_RGBA16S:	return MTLPixelFormatRGBA16Snorm;
			case PF_R16:		return MTLPixelFormatR16Unorm;
			case PF_RG16:		return MTLPixelFormatRG16Unorm;
			case PF_RGBA16:		return MTLPixelFormatRGBA16Unorm;
			// Packed float/unorm HDR formats — widely used for bloom / tone-mapped intermediate targets.
			case PF_RG11B10F:	return MTLPixelFormatRG11B10Float;
			case PF_RGB10A2:	return MTLPixelFormatRGB10A2Unorm;
			case PF_D16:		return MTLPixelFormatDepth16Unorm;
			case PF_D32:		return MTLPixelFormatDepth32Float;
			case PF_D32_S8X24:	return MTLPixelFormatDepth32Float_Stencil8;
			// BC1 with 1-bit alpha uses the same Metal format as BC1 without — Metal's BC1_RGBA honors
			// the 1-bit alpha encoding, and there is no separate BC1_RGB format on any GPU family.
			case PF_BC1a:
#if !TARGET_OS_IPHONE
			case PF_BC1:		return gamma ? MTLPixelFormatBC1_RGBA_sRGB : MTLPixelFormatBC1_RGBA;
			case PF_BC2:		return gamma ? MTLPixelFormatBC2_RGBA_sRGB : MTLPixelFormatBC2_RGBA;
			case PF_BC3:		return gamma ? MTLPixelFormatBC3_RGBA_sRGB : MTLPixelFormatBC3_RGBA;
			case PF_BC4:		return MTLPixelFormatBC4_RUnorm;
			case PF_BC5:		return MTLPixelFormatBC5_RGUnorm;
			case PF_BC6H:		return MTLPixelFormatBC6H_RGBUfloat;
			case PF_BC7:		return gamma ? MTLPixelFormatBC7_RGBAUnorm_sRGB : MTLPixelFormatBC7_RGBAUnorm;
#else
			// Fall through on iOS where BC* is unsupported — same warn-once path as any other
			// unmapped format below.
				return InvalidFormatWarn(format);
#endif
			default:
				return InvalidFormatWarn(format);
			}
		}

		MTLTextureType MetalUtility::GetTextureType(TextureType type, u32 sampleCount, u32 arraySliceCount)
		{
			const bool msaa = sampleCount > 1;
			const bool array = arraySliceCount > 1;

			switch (type)
			{
			case TEX_TYPE_1D:
				return array ? MTLTextureType1DArray : MTLTextureType1D;

			case TEX_TYPE_2D:
				if (msaa)
					return array ? MTLTextureType2DMultisampleArray : MTLTextureType2DMultisample;
				return array ? MTLTextureType2DArray : MTLTextureType2D;

			case TEX_TYPE_3D:
				return MTLTextureType3D;

			case TEX_TYPE_CUBE_MAP:
				return array ? MTLTextureTypeCubeArray : MTLTextureTypeCube;
			}

			return MTLTextureType2D;
		}

		MTLSamplerMinMagFilter MetalUtility::GetMinMagFilter(FilterOptions filter)
		{
			switch (filter)
			{
			case FO_NONE:			return MTLSamplerMinMagFilterNearest;
			case FO_POINT:			return MTLSamplerMinMagFilterNearest;
			case FO_LINEAR:			return MTLSamplerMinMagFilterLinear;
			case FO_ANISOTROPIC:	return MTLSamplerMinMagFilterLinear;
			}
			return MTLSamplerMinMagFilterNearest;
		}

		MTLSamplerMipFilter MetalUtility::GetMipFilter(FilterOptions filter)
		{
			switch (filter)
			{
			case FO_NONE:			return MTLSamplerMipFilterNotMipmapped;
			case FO_POINT:			return MTLSamplerMipFilterNearest;
			case FO_LINEAR:			return MTLSamplerMipFilterLinear;
			case FO_ANISOTROPIC:	return MTLSamplerMipFilterLinear;
			}
			return MTLSamplerMipFilterNotMipmapped;
		}

		MTLSamplerAddressMode MetalUtility::GetAddressMode(TextureAddressingMode mode)
		{
			switch (mode)
			{
			case TAM_WRAP:		return MTLSamplerAddressModeRepeat;
			case TAM_MIRROR:	return MTLSamplerAddressModeMirrorRepeat;
			case TAM_CLAMP:		return MTLSamplerAddressModeClampToEdge;
			case TAM_BORDER:	return MTLSamplerAddressModeClampToBorderColor;
			}
			return MTLSamplerAddressModeClampToEdge;
		}

		MTLCompareFunction MetalUtility::GetCompareFunction(CompareFunction func)
		{
			switch (func)
			{
			case CMPF_ALWAYS_FAIL:		return MTLCompareFunctionNever;
			case CMPF_ALWAYS_PASS:		return MTLCompareFunctionAlways;
			case CMPF_LESS:				return MTLCompareFunctionLess;
			case CMPF_LESS_EQUAL:		return MTLCompareFunctionLessEqual;
			case CMPF_EQUAL:			return MTLCompareFunctionEqual;
			case CMPF_NOT_EQUAL:		return MTLCompareFunctionNotEqual;
			case CMPF_GREATER_EQUAL:	return MTLCompareFunctionGreaterEqual;
			case CMPF_GREATER:			return MTLCompareFunctionGreater;
			}
			return MTLCompareFunctionAlways;
		}

		MTLBlendFactor MetalUtility::GetBlendFactor(BlendFactor factor)
		{
			switch (factor)
			{
			case BF_ONE:				return MTLBlendFactorOne;
			case BF_ZERO:				return MTLBlendFactorZero;
			case BF_DEST_COLOR:			return MTLBlendFactorDestinationColor;
			case BF_SOURCE_COLOR:		return MTLBlendFactorSourceColor;
			case BF_INV_DEST_COLOR:		return MTLBlendFactorOneMinusDestinationColor;
			case BF_INV_SOURCE_COLOR:	return MTLBlendFactorOneMinusSourceColor;
			case BF_DEST_ALPHA:			return MTLBlendFactorDestinationAlpha;
			case BF_SOURCE_ALPHA:		return MTLBlendFactorSourceAlpha;
			case BF_INV_DEST_ALPHA:		return MTLBlendFactorOneMinusDestinationAlpha;
			case BF_INV_SOURCE_ALPHA:	return MTLBlendFactorOneMinusSourceAlpha;
			}
			return MTLBlendFactorOne;
		}

		MTLBlendOperation MetalUtility::GetBlendOperation(BlendOperation op)
		{
			switch (op)
			{
			case BO_ADD:				return MTLBlendOperationAdd;
			case BO_SUBTRACT:			return MTLBlendOperationSubtract;
			case BO_REVERSE_SUBTRACT:	return MTLBlendOperationReverseSubtract;
			case BO_MIN:				return MTLBlendOperationMin;
			case BO_MAX:				return MTLBlendOperationMax;
			}
			return MTLBlendOperationAdd;
		}

		MTLCullMode MetalUtility::GetCullMode(CullingMode mode)
		{
			switch (mode)
			{
			case CULL_NONE:				return MTLCullModeNone;
			case CULL_CLOCKWISE:		return MTLCullModeFront;
			case CULL_COUNTERCLOCKWISE:	return MTLCullModeBack;
			}
			return MTLCullModeNone;
		}

		MTLWinding MetalUtility::GetFrontFaceWinding(CullingMode mode)
		{
			// Engine convention: CULL_CLOCKWISE means "front-facing polygons are wound CW" (so CW triangles
			// get culled when used as the cull mode). Metal's MTLFrontFacingWinding describes the winding of
			// the *front* face, so CULL_CLOCKWISE => front=CW, which in Metal's left-handed-viewport terms
			// becomes MTLWindingCounterClockwise due to the y-axis direction of clip space. For all other
			// modes (including CULL_COUNTERCLOCKWISE and CULL_NONE) the front face is CCW in engine space,
			// which maps to MTLWindingClockwise on the Metal side.
			return mode == CULL_CLOCKWISE ? MTLWindingCounterClockwise : MTLWindingClockwise;
		}

		MTLTriangleFillMode MetalUtility::GetFillMode(PolygonMode mode)
		{
			switch (mode)
			{
			case PM_WIREFRAME:	return MTLTriangleFillModeLines;
			case PM_SOLID:		return MTLTriangleFillModeFill;
			}
			return MTLTriangleFillModeFill;
		}

		MTLStencilOperation MetalUtility::GetStencilOperation(StencilOperation op)
		{
			switch (op)
			{
			case SOP_KEEP:				return MTLStencilOperationKeep;
			case SOP_ZERO:				return MTLStencilOperationZero;
			case SOP_REPLACE:			return MTLStencilOperationReplace;
			case SOP_INCREMENT:			return MTLStencilOperationIncrementClamp;
			case SOP_DECREMENT:			return MTLStencilOperationDecrementClamp;
			case SOP_INCREMENT_WRAP:	return MTLStencilOperationIncrementWrap;
			case SOP_DECREMENT_WRAP:	return MTLStencilOperationDecrementWrap;
			case SOP_INVERT:			return MTLStencilOperationInvert;
			}
			return MTLStencilOperationKeep;
		}

		MTLPrimitiveType MetalUtility::GetPrimitiveType(DrawOperationType op)
		{
			switch (op)
			{
			case DOT_POINT_LIST:		return MTLPrimitiveTypePoint;
			case DOT_LINE_LIST:			return MTLPrimitiveTypeLine;
			case DOT_LINE_STRIP:		return MTLPrimitiveTypeLineStrip;
			case DOT_TRIANGLE_LIST:		return MTLPrimitiveTypeTriangle;
			case DOT_TRIANGLE_STRIP:	return MTLPrimitiveTypeTriangleStrip;
			case DOT_TRIANGLE_FAN:		return MTLPrimitiveTypeTriangle; // Metal has no triangle-fan; caller must expand
			}
			return MTLPrimitiveTypeTriangle;
		}

		MTLPrimitiveTopologyClass MetalUtility::GetPrimitiveTopologyClass(DrawOperationType op)
		{
			switch (op)
			{
			case DOT_POINT_LIST:		return MTLPrimitiveTopologyClassPoint;
			case DOT_LINE_LIST:
			case DOT_LINE_STRIP:		return MTLPrimitiveTopologyClassLine;
			case DOT_TRIANGLE_LIST:
			case DOT_TRIANGLE_STRIP:
			case DOT_TRIANGLE_FAN:		return MTLPrimitiveTopologyClassTriangle;
			}
			return MTLPrimitiveTopologyClassTriangle;
		}

		MTLVertexFormat MetalUtility::GetVertexFormat(VertexElementType type)
		{
			switch (type)
			{
			case VET_FLOAT1:		return MTLVertexFormatFloat;
			case VET_FLOAT2:		return MTLVertexFormatFloat2;
			case VET_FLOAT3:		return MTLVertexFormatFloat3;
			case VET_FLOAT4:		return MTLVertexFormatFloat4;
			// VET_COLOR_ABGR encodes bytes in memory as {r, g, b, a}, which is exactly what Metal's
			// MTLVertexFormatUChar4Normalized reads per-lane, so the vertex-shader sees .rgba correctly.
			// VET_COLOR is the engine's platform-preferred alias — on Metal the capability defaults to
			// VertexColorType = VET_COLOR_ABGR, so meshes authored against VET_COLOR land in ABGR byte
			// order and this fast path is correct for both.
			case VET_COLOR:
			case VET_COLOR_ABGR:
			case VET_UBYTE4_NORM:	return MTLVertexFormatUChar4Normalized;
			// VET_COLOR_ARGB encodes bytes as {a, r, g, b} — the D3D authoring order. Metal can't
			// swizzle at vertex-fetch, so a mesh with VET_COLOR_ARGB read via UChar4Normalized would
			// deliver (a, r, g, b) to the shader's .rgba lane and render with shifted channels. Warn
			// once so the authoring side is visible, then fall back to UChar4Normalized — callers must
			// either pre-swizzle the mesh to ABGR or target the D3D12 backend. The engine's own mesh
			// generators already honor the capability's VertexColorType, so this case fires only for
			// meshes that were hard-coded to ARGB.
			case VET_COLOR_ARGB:
			{
				// @c std::atomic<bool>::exchange is the lightest safe primitive here: only the first
				// caller across every worker fiber to flip false->true wins the race and emits the
				// log; any concurrent callers see true and skip.
				static std::atomic<bool> sWarnedArgb{false};
				if (!sWarnedArgb.exchange(true, std::memory_order_relaxed))
				{
					B3D_LOG(Error, LogRenderBackend,
						"VET_COLOR_ARGB vertex elements are not supported on the Metal backend — Metal cannot swizzle at fetch. "
						"Pre-swizzle the mesh to VET_COLOR_ABGR byte order or author against VET_COLOR "
						"(which resolves to the capability's VertexColorType, VET_COLOR_ABGR on Metal). "
						"Falling back to UChar4Normalized; rendered colors will have channels shifted.");
				}
				return MTLVertexFormatUChar4Normalized;
			}
			case VET_SHORT1:		return MTLVertexFormatShort;
			case VET_SHORT2:		return MTLVertexFormatShort2;
			case VET_SHORT4:		return MTLVertexFormatShort4;
			case VET_USHORT1:		return MTLVertexFormatUShort;
			case VET_USHORT2:		return MTLVertexFormatUShort2;
			case VET_USHORT4:		return MTLVertexFormatUShort4;
			case VET_UBYTE4:		return MTLVertexFormatUChar4;
			case VET_INT1:			return MTLVertexFormatInt;
			case VET_INT2:			return MTLVertexFormatInt2;
			case VET_INT3:			return MTLVertexFormatInt3;
			case VET_INT4:			return MTLVertexFormatInt4;
			case VET_UINT1:			return MTLVertexFormatUInt;
			case VET_UINT2:			return MTLVertexFormatUInt2;
			case VET_UINT3:			return MTLVertexFormatUInt3;
			case VET_UINT4:			return MTLVertexFormatUInt4;
			case VET_HALF1:			return MTLVertexFormatHalf;
			case VET_HALF2:			return MTLVertexFormatHalf2;
			case VET_HALF3:			return MTLVertexFormatHalf3;
			case VET_HALF4:			return MTLVertexFormatHalf4;
			default:				return MTLVertexFormatInvalid;
			}
		}

		u32 MetalUtility::GetTextureRowPitch(PixelFormat format, u32 width)
		{
			const u32 blockSize = PixelUtility::GetBlockSize(format);
			if (blockSize == 0)
				return 0;

			u64 rowPitch;
			if (PixelUtility::IsCompressed(format))
			{
				const Vector2I blockDim = PixelUtility::GetBlockDimensions(format);
				if (blockDim.X <= 0)
					return 0;

				const u64 blockCols = ((u64)std::max(1u, width) + (u32)blockDim.X - 1) / (u32)blockDim.X;
				rowPitch = blockCols * blockSize;
			}
			else
				rowPitch = (u64)std::max(1u, width) * blockSize;

			return rowPitch <= (u64)~0u ? (u32)rowPitch : 0;
		}

		u32 MetalUtility::GetTextureSlicePitch(PixelFormat format, u32 width, u32 height)
		{
			const u32 rowPitch = GetTextureRowPitch(format, width);
			if (rowPitch == 0)
				return 0;

			u64 slicePitch;
			if (PixelUtility::IsCompressed(format))
			{
				const Vector2I blockDim = PixelUtility::GetBlockDimensions(format);
				if (blockDim.Y <= 0)
					return 0;

				const u64 blockRows = ((u64)std::max(1u, height) + (u32)blockDim.Y - 1) / (u32)blockDim.Y;
				slicePitch = (u64)rowPitch * blockRows;
			}
			else
				slicePitch = (u64)rowPitch * std::max(1u, height);

			return slicePitch <= (u64)~0u ? (u32)slicePitch : 0;
		}

		MTLStorageMode MetalUtility::GetBufferStorageMode(const GpuBufferInformation& information)
		{
			// CPU-visible buffers use shared storage so Map can expose [buffer contents] directly
			// and the engine's GpuBufferUtility can memcpy in place. Everything else is GPU-private,
			// which makes GpuBufferUtility fall back to its staging-buffer + CopyBufferToBuffer
			// path. Managed storage (for discrete Macs) would need explicit didModifyRange on every
			// write; shared works on all Apple platforms and is coherent on Apple Silicon.
			const bool cpuVisible = information.Flags.IsSet(GpuBufferFlag::StoreOnCPUWithGPUAccess)
				|| information.Type == GpuBufferType::StagingRead
				|| information.Type == GpuBufferType::StagingWrite;

			return cpuVisible ? MTLStorageModeShared : MTLStorageModePrivate;
		}

		MTLResourceOptions MetalUtility::GetResourceOptions(MTLStorageMode storageMode)
		{
			// Storage/cache/hazard modes share this bit mask. Keep direct resources consistent
			// with placement heaps by selecting the configured hazard policy here.
			MTLResourceOptions options = (MTLResourceOptions)((NSUInteger)storageMode << MTLResourceStorageModeShift);
#if B3D_METAL_USE_EXPLICIT_RESOURCE_SYNCHRONIZATION
			options |= MTLResourceHazardTrackingModeUntracked;
#else
			options |= MTLResourceHazardTrackingModeTracked;
#endif

			return options;
		}

		bool IsMetalPixelFormatSupported(PixelFormat format, bool gamma)
		{
			return MetalUtility::GetPixelFormat(format, gamma) != MTLPixelFormatInvalid;
		}
	} // namespace render
} // namespace b3d
