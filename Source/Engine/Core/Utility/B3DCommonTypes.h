//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	struct RenderProxySyncPacket;
	// Undefine defines from other libs, that conflict with enums below
#undef None
#undef Convex

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/**	Factors used when blending new pixels with existing pixels. */
	enum BlendFactor
	{
		BF_ONE, /**< Use a value of one for all pixel components. */
		BF_ZERO, /**< Use a value of zero for all pixel components. */
		BF_DEST_COLOR, /**< Use the existing pixel value. */
		BF_SOURCE_COLOR, /**< Use the newly generated pixel value. */
		BF_INV_DEST_COLOR, /**< Use the inverse of the existing value. */
		BF_INV_SOURCE_COLOR, /**< Use the inverse of the newly generated pixel value. */
		BF_DEST_ALPHA, /**< Use the existing alpha value. */
		BF_SOURCE_ALPHA, /**< Use the newly generated alpha value. */
		BF_INV_DEST_ALPHA, /**< Use the inverse of the existing alpha value. */
		BF_INV_SOURCE_ALPHA /**< Use the inverse of the newly generated alpha value. */
	};

	/**	Operations that determines how are blending factors combined. */
	enum BlendOperation
	{
		BO_ADD, /**< Blend factors are added together. */
		BO_SUBTRACT, /**< Blend factors are subtracted in "srcFactor - dstFactor" order. */
		BO_REVERSE_SUBTRACT, /**< Blend factors are subtracted in "dstFactor - srcFactor" order. */
		BO_MIN, /**< Minimum of the two factors is chosen. */
		BO_MAX /**< Maximum of the two factors is chosen. */
	};

	/**	Comparison functions used for the depth/stencil buffer. */
	enum CompareFunction
	{
		CMPF_ALWAYS_FAIL, /**< Operation will always fail. */
		CMPF_ALWAYS_PASS, /**< Operation will always pass. */
		CMPF_LESS, /**< Operation will pass if the new value is less than existing value. */
		CMPF_LESS_EQUAL, /**< Operation will pass if the new value is less or equal than existing value. */
		CMPF_EQUAL, /**< Operation will pass if the new value is equal to the existing value. */
		CMPF_NOT_EQUAL, /**< Operation will pass if the new value is not equal to the existing value. */
		CMPF_GREATER_EQUAL, /**< Operation will pass if the new value greater or equal than the existing value. */
		CMPF_GREATER /**< Operation will pass if the new value greater than the existing value. */
	};

	/**
	 * Types of texture addressing modes that determine what happens when texture coordinates are outside of the valid range.
	 */
	enum TextureAddressingMode
	{
		TAM_WRAP, /**< Coordinates wrap back to the valid range. */
		TAM_MIRROR, /**< Coordinates flip every time the size of the valid range is passed. */
		TAM_CLAMP, /**< Coordinates are clamped within the valid range. */
		TAM_BORDER /**< Coordinates outside of the valid range will return a separately set border color. */
	};

	/**	Types of available filtering situations. */
	enum FilterType
	{
		FT_MIN, /**< The filter used when shrinking a texture. */
		FT_MAG, /**< The filter used when magnifying a texture. */
		FT_MIP /**< The filter used when filtering between mipmaps. */
	};

	/**	Filtering options for textures. */
	enum FilterOptions
	{
		FO_NONE = 0, /**< Use no filtering. Only relevant for mipmap filtering. */
		FO_POINT = 1, /**< Filter using the nearest found pixel. Most basic filtering. */
		FO_LINEAR = 2, /**< Average a 2x2 pixel area, signifies bilinear filtering for texture, trilinear for mipmaps. */
		FO_ANISOTROPIC = 3, /**< More advanced filtering that improves quality when viewing textures at a steep angle */
	};

	/**	Types of frame buffers. */
	enum FrameBufferType
	{
		FBT_COLOR = 0x1, /**< Color surface. */
		FBT_DEPTH = 0x2, /**< Depth surface. */
		FBT_STENCIL = 0x4 /**< Stencil surface. */
	};

	/**
	 * Types of culling that determine how (and if) hardware discards faces with certain winding order. Winding order can
	 * be used for determining front or back facing polygons by checking the order of its vertices from the render
	 * perspective.
	 */
	enum CullingMode
	{
		CULL_NONE = 0, /**< Hardware performs no culling and renders both sides. */
		CULL_CLOCKWISE = 1, /**< Hardware culls faces that have a clockwise vertex ordering. */
		CULL_COUNTERCLOCKWISE = 2 /**< Hardware culls faces that have a counter-clockwise vertex ordering. */
	};

	/**	Polygon mode to use when rasterizing. */
	enum PolygonMode
	{
		PM_WIREFRAME = 1, /**< Render as wireframe showing only polygon outlines. */
		PM_SOLID = 2 /**< Render as solid showing whole polygons. */
	};

	/**	Types of action that can happen on the stencil buffer. */
	enum StencilOperation
	{
		SOP_KEEP, /**< Leave the stencil buffer unchanged. */
		SOP_ZERO, /**< Set the stencil value to zero. */
		SOP_REPLACE, /**< Replace the stencil value with the reference value. */
		SOP_INCREMENT, /**< Increase the stencil value by 1, clamping at the maximum value. */
		SOP_DECREMENT, /**< Decrease the stencil value by 1, clamping at 0. */
		SOP_INCREMENT_WRAP, /**< Increase the stencil value by 1, wrapping back to 0 when incrementing past the maximum value. */
		SOP_DECREMENT_WRAP, /**< Decrease the stencil value by 1, wrapping when decrementing 0. */
		SOP_INVERT /**< Invert the bits of the stencil buffer. */
	};

	/** Describes operation that will be used for rendering a certain set of vertices. */
	enum B3D_SCRIPT_EXPORT(ExportName(MeshTopology), DocumentationGroup(Rendering)) DrawOperationType
	{
		/** Each vertex represents a point. */
		DOT_POINT_LIST B3D_SCRIPT_EXPORT(ExportName(PointList)) = 1,
		/** Each sequential pair of vertices represent a line. */
		DOT_LINE_LIST B3D_SCRIPT_EXPORT(ExportName(LineList)) = 2,
		/** Each vertex (except the first) forms a line with the previous vertex. */
		DOT_LINE_STRIP B3D_SCRIPT_EXPORT(ExportName(LineStrip)) = 3,
		/** Each sequential 3-tuple of vertices represent a triangle. */
		DOT_TRIANGLE_LIST B3D_SCRIPT_EXPORT(ExportName(TriangleList)) = 4,
		/** Each vertex (except the first two) form a triangle with the previous two vertices. */
		DOT_TRIANGLE_STRIP B3D_SCRIPT_EXPORT(ExportName(TriangleStrip)) = 5,
		/** Each vertex (except the first two) form a triangle with the first vertex and previous vertex. */
		DOT_TRIANGLE_FAN B3D_SCRIPT_EXPORT(ExportName(TriangleFan)) = 6,
		/** Total number of draw operation types. */
		DOT_COUNT B3D_SCRIPT_EXPORT(ExportName(TriangleFan)) = 6
	};

	/**	Type of mesh indices used, used for determining maximum number of vertices in a mesh. */
	enum B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) IndexType
	{
		IT_16BIT B3D_SCRIPT_EXPORT(ExportName(Index16)), /**< 16-bit indices. */
		IT_32BIT B3D_SCRIPT_EXPORT(ExportName(Index32)) /**< 32-bit indices. */
	};

	/** Types of programs that may run on GPU. */
	enum GpuProgramType
	{
		GPT_VERTEX_PROGRAM, /**< Vertex program. */
		GPT_FRAGMENT_PROGRAM, /**< Fragment(pixel) program. */
		GPT_GEOMETRY_PROGRAM, /**< Geometry program. */
		GPT_DOMAIN_PROGRAM, /**< Domain (tesselation evaluation) program. */
		GPT_HULL_PROGRAM, /**< Hull (tesselation control) program. */
		GPT_COMPUTE_PROGRAM, /**< Compute program. */
		GPT_COUNT // Keep at end
	};

	/** All possible GPU program stages as a bit mask. */
	enum class GpuProgramStageBit
	{
		None = 0,
		Vertex = 1 << GPT_VERTEX_PROGRAM,
		Fragment = 1 << GPT_FRAGMENT_PROGRAM,
		Hull = 1 << GPT_HULL_PROGRAM,
		Domain = 1 << GPT_DOMAIN_PROGRAM,
		Geometry = 1 << GPT_GEOMETRY_PROGRAM,
		Compute = 1 << GPT_COMPUTE_PROGRAM
	};

	using GpuProgramStageBits = Flags<GpuProgramStageBit>;
	B3D_FLAGS_OPERATORS(GpuProgramStageBit)
	/** Types of valid formats used for standard GPU buffers. */
	enum GpuBufferFormat
	{
		BF_16X1F, /**< 1D 16-bit floating-point format. */
		BF_16X2F, /**< 2D 16-bit floating-point format. */
		BF_16X4F, /**< 4D 16-bit floating-point format. */
		BF_32X1F, /**< 1D 32-bit floating-point format. */
		BF_32X2F, /**< 2D 32-bit floating-point format. */
		BF_32X3F, /**< 3D 32-bit floating-point format. */
		BF_32X4F, /**< 4D 32-bit floating-point format. */
		BF_8X1, /**< 1D 8-bit normalized format. */
		BF_8X2, /**< 2D 8-bit normalized format. */
		BF_8X4, /**< 4D 8-bit normalized format. */
		BF_16X1, /**< 1D 16-bit normalized format. */
		BF_16X2, /**< 2D 16-bit normalized format. */
		BF_16X4, /**< 4D 16-bit normalized format. */
		BF_8X1S, /**< 1D 8-bit signed integer format. */
		BF_8X2S, /**< 2D 8-bit signed integer format. */
		BF_8X4S, /**< 4D 8-bit signed integer format. */
		BF_16X1S, /**< 1D 16-bit signed integer format. */
		BF_16X2S, /**< 2D 16-bit signed integer format. */
		BF_16X4S, /**< 4D 16-bit signed integer format. */
		BF_32X1S, /**< 1D 32-bit signed integer format. */
		BF_32X2S, /**< 2D 32-bit signed integer format. */
		BF_32X3S, /**< 3D 32-bit signed integer format. */
		BF_32X4S, /**< 4D 32-bit signed integer format. */
		BF_8X1U, /**< 1D 8-bit unsigned integer format. */
		BF_8X2U, /**< 2D 8-bit unsigned integer format. */
		BF_8X4U, /**< 4D 8-bit unsigned integer format. */
		BF_16X1U, /**< 1D 16-bit unsigned integer format. */
		BF_16X2U, /**< 2D 16-bit unsigned integer format. */
		BF_16X4U, /**< 4D 16-bit unsigned integer format. */
		BF_32X1U, /**< 1D 32-bit unsigned integer format. */
		BF_32X2U, /**< 2D 32-bit unsigned integer format. */
		BF_32X3U, /**< 3D 32-bit unsigned integer format. */
		BF_32X4U, /**< 4D 32-bit unsigned integer format. */
		BF_64X1F, /**< 1D 64-bit floating-point format. */
		BF_64X2F, /**< 2D 64-bit floating-point format. */
		BF_64X3F, /**< 3D 64-bit floating-point format. */
		BF_64X4F, /**< 4D 64-bit floating-point format. */
		BF_64X1S, /**< 1D 64-bit signed integer format. */
		BF_64X2S, /**< 2D 64-bit signed integer format. */
		BF_64X3S, /**< 3D 64-bit signed integer format. */
		BF_64X4S, /**< 4D 64-bit signed integer format. */
		BF_64X1U, /**< 1D 64-bit unsigned integer format. */
		BF_64X2U, /**< 2D 64-bit unsigned integer format. */
		BF_64X3U, /**< 3D 64-bit unsigned integer format. */
		BF_64X4U, /**< 4D 64-bit unsigned integer format. */
		BF_COUNT, /**< Not a valid format. Keep just before BS_UNKNOWN. */
		BF_UNKNOWN = 0xffff /**< Unknown format (used for non-standard buffers, like structured or raw. */
	};

	/** Different types of GPU views that control how GPU sees a hardware buffer. */
	enum GpuViewUsage
	{
		/** Buffer is seen as a default shader resource, used primarily for reading. (for example a texture for sampling) */
		GVU_DEFAULT = 0x01,
		/** Buffer is seen as a render target that color pixels will be written to after pixel shader stage. */
		GVU_RENDERTARGET = 0x02,
		/** Buffer is seen as a depth stencil target that depth and stencil information is written to. */
		GVU_DEPTHSTENCIL = 0x04,
		/** Buffer that allows you to write to any part of it from within a GPU program. */
		GVU_RANDOMWRITE = 0x08
	};

	/** Types of GPU parameters (i.e. uniform type). */
	enum class GpuParameterType
	{
		UniformBuffer,
		SampledTexture,
		StorageTexture,
		StorageBuffer,
		Sampler,
		Count,
		Unknown,
	};

	/**	Type of GPU data parameters that can be used as inputs to a GPU program. */
	enum GpuDataParameterType
	{
		GPDT_FLOAT1 = 1, /**< 1D 32-bit floating point value. */
		GPDT_FLOAT2 = 2, /**< 2D 32-bit floating point value. */
		GPDT_FLOAT3 = 3, /**< 3D 32-bit floating point value. */
		GPDT_FLOAT4 = 4, /**< 4D 32-bit floating point value. */
		GPDT_MATRIX_2X2 = 11, /**< 2x2 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_2X3 = 12, /**< 2x3 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_2X4 = 13, /**< 2x4 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_3X2 = 14, /**< 3x2 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_3X3 = 15, /**< 3x3 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_3X4 = 16, /**< 3x4 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_4X2 = 17, /**< 4x2 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_4X3 = 18, /**< 4x3 matrix with 32-bit floating point elements. */
		GPDT_MATRIX_4X4 = 19, /**< 4x4 matrix with 32-bit floating point elements. */
		GPDT_INT1 = 20, /**< 1D signed integer value. */
		GPDT_INT2 = 21, /**< 2D signed integer value. */
		GPDT_INT3 = 22, /**< 3D signed integer value. */
		GPDT_INT4 = 23, /**< 4D signed integer value. */
		GPDT_BOOL = 24, /**< Boolean value. */
		GPDT_STRUCT = 25, /**< Variable size structure. */
		GPDT_COLOR = 26, /**< Same as GPDT_FLOAT4, but can be used to better deduce usage. */
		GPDT_UINT1 = 27, /**< 1D unsigned integer value. */
		GPDT_UINT2 = 28, /**< 2D unsigned integer value. */
		GPDT_UINT3 = 29, /**< 3D unsigned integer value. */
		GPDT_UINT4 = 30, /**< 4D unsigned integer value. */
		GPDT_DOUBLE1 = 31, /**< 1D 64-bit floating point value. */
		GPDT_DOUBLE2 = 32, /**< 2D 64-bit floating point value. */
		GPDT_DOUBLE3 = 33, /**< 3D 64-bit floating point value. */
		GPDT_DOUBLE4 = 34, /**< 4D 64-bit floating point value. */
		GPDT_HALF1 = 35, /**< 1D 16-bit floating point value. */
		GPDT_HALF2 = 36, /**< 2D 16-bit floating point value. */
		GPDT_HALF3 = 37, /**< 3D 16-bit floating point value. */
		GPDT_HALF4 = 38, /**< 4D 16-bit floating point value. */
		GPDT_DOUBLE_MATRIX_2X2 = 39, /**< 2x2 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_2X3 = 40, /**< 2x3 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_2X4 = 41, /**< 2x4 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_3X2 = 42, /**< 3x2 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_3X3 = 43, /**< 3x3 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_3X4 = 44, /**< 3x4 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_4X2 = 45, /**< 4x2 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_4X3 = 46, /**< 4x3 matrix with 64-bit floating point elements. */
		GPDT_DOUBLE_MATRIX_4X4 = 47, /**< 4x4 matrix with 64-bit floating point elements. */
		GPDT_HALF_MATRIX_2X2 = 48, /**< 2x2 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_2X3 = 49, /**< 2x3 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_2X4 = 50, /**< 2x4 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_3X2 = 51, /**< 3x2 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_3X3 = 52, /**< 3x3 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_3X4 = 53, /**< 3x4 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_4X2 = 54, /**< 4x2 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_4X3 = 55, /**< 4x3 matrix with 16-bit floating point elements. */
		GPDT_HALF_MATRIX_4X4 = 67, /**< 4x4 matrix with 16-bit floating point elements. */
		GPDT_COUNT = 68, /**< Keep at end before GPDT_UNKNOWN. */
		GPDT_UNKNOWN = 0xffff
	};

	/**	Contains data about a type used for GPU data parameters. */
	struct GpuDataParameterTypeInformation
	{
		u32 BaseTypeSize;
		u32 Size;
		u32 Alignment;
		u32 NumRows;
		u32 NumColumns;
	};

	/**	Contains a lookup table for various information of all types used for data GPU parameters. Sizes are in bytes. */
	struct GpuDataParameterTypeInformationLookup
	{
		GpuDataParameterTypeInformationLookup()
		{
			memset(Lookup, 0, sizeof(Lookup));

			Lookup[(u32)GPDT_FLOAT1] = { 4, 4, 4, 1, 1 };
			Lookup[(u32)GPDT_FLOAT2] = { 4, 8, 8, 1, 2 };
			Lookup[(u32)GPDT_FLOAT3] = { 4, 16, 16, 1, 3 };
			Lookup[(u32)GPDT_FLOAT4] = { 4, 16, 16, 1, 4 };
			Lookup[(u32)GPDT_COLOR] = { 4, 16, 16, 1, 4 };
			Lookup[(u32)GPDT_MATRIX_2X2] = { 4, 16, 8, 2, 2 };
			Lookup[(u32)GPDT_MATRIX_2X3] = { 4, 32, 16, 2, 3 };
			Lookup[(u32)GPDT_MATRIX_2X4] = { 4, 32, 16, 2, 4 };
			Lookup[(u32)GPDT_MATRIX_3X2] = { 4, 24, 8, 3, 2 };
			Lookup[(u32)GPDT_MATRIX_3X3] = { 4, 48, 16, 3, 3 };
			Lookup[(u32)GPDT_MATRIX_3X4] = { 4, 48, 16, 3, 4 };
			Lookup[(u32)GPDT_MATRIX_4X2] = { 4, 32, 8, 4, 2 };
			Lookup[(u32)GPDT_MATRIX_4X3] = { 4, 64, 16, 4, 3 };
			Lookup[(u32)GPDT_MATRIX_4X4] = { 4, 64, 16, 4, 4 };
			Lookup[(u32)GPDT_INT1] = { 4, 4, 4, 1, 1 };
			Lookup[(u32)GPDT_INT2] = { 4, 8, 8, 1, 2 };
			Lookup[(u32)GPDT_INT3] = { 4, 12, 16, 1, 3 };
			Lookup[(u32)GPDT_INT4] = { 4, 16, 16, 1, 4 };
			Lookup[(u32)GPDT_UINT1] = { 4, 4, 4, 1, 1 };
			Lookup[(u32)GPDT_UINT2] = { 4, 8, 8, 1, 2 };
			Lookup[(u32)GPDT_UINT3] = { 4, 12, 16, 1, 3 };
			Lookup[(u32)GPDT_UINT4] = { 4, 16, 16, 1, 4 };
			Lookup[(u32)GPDT_DOUBLE1] = { 8, 8, 8, 1, 1 };
			Lookup[(u32)GPDT_DOUBLE2] = { 8, 16, 16, 1, 2 };
			Lookup[(u32)GPDT_DOUBLE3] = { 8, 24, 32, 1, 3 };
			Lookup[(u32)GPDT_DOUBLE4] = { 8, 32, 32, 1, 4 };
			Lookup[(u32)GPDT_HALF1] = { 2, 2, 4, 1, 1 };
			Lookup[(u32)GPDT_HALF2] = { 2, 4, 4, 1, 2 };
			Lookup[(u32)GPDT_HALF3] = { 2, 6, 8, 1, 3 };
			Lookup[(u32)GPDT_HALF4] = { 2, 8, 8, 1, 4 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_2X2] = { 8, 32, 16, 2, 2 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_2X3] = { 8, 64, 32, 2, 3 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_2X4] = { 8, 64, 32, 2, 4 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_3X2] = { 8, 48, 16, 3, 2 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_3X3] = { 8, 96, 32, 3, 3 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_3X4] = { 8, 96, 32, 3, 4 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_4X2] = { 8, 64, 16, 4, 2 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_4X3] = { 8, 128, 32, 4, 3 };
			Lookup[(u32)GPDT_DOUBLE_MATRIX_4X4] = { 8, 128, 32, 4, 4 };
			Lookup[(u32)GPDT_HALF_MATRIX_2X2] = { 2, 8, 4, 2, 2 };
			Lookup[(u32)GPDT_HALF_MATRIX_2X3] = { 2, 16, 8, 2, 3 };
			Lookup[(u32)GPDT_HALF_MATRIX_2X4] = { 2, 16, 8, 2, 4 };
			Lookup[(u32)GPDT_HALF_MATRIX_3X2] = { 2, 12, 4, 3, 2 };
			Lookup[(u32)GPDT_HALF_MATRIX_3X3] = { 2, 24, 8, 3, 3 };
			Lookup[(u32)GPDT_HALF_MATRIX_3X4] = { 2, 24, 8, 3, 4 };
			Lookup[(u32)GPDT_HALF_MATRIX_4X2] = { 2, 16, 4, 4, 2 };
			Lookup[(u32)GPDT_HALF_MATRIX_4X3] = { 2, 32, 8, 4, 3 };
			Lookup[(u32)GPDT_HALF_MATRIX_4X4] = { 2, 32, 8, 4, 4 };
			Lookup[(u32)GPDT_COLOR] = { 4, 16, 16, 1, 4 };
			Lookup[(u32)GPDT_BOOL] = { 4, 4, 4, 1, 1 };
			Lookup[(u32)GPDT_STRUCT] = { 4, 0, 16, 1, 1 };
		}

		GpuDataParameterTypeInformation Lookup[GPDT_COUNT];
	};

	/**	Type of GPU object parameters that can be used as inputs to a GPU program. */
	enum GpuParameterObjectType
	{
		GPOT_SAMPLER1D = 1, /**< Sampler state for a 1D texture. */
		GPOT_SAMPLER2D = 2, /**< Sampler state for a 2D texture. */
		GPOT_SAMPLER3D = 3, /**< Sampler state for a 3D texture. */
		GPOT_SAMPLERCUBE = 4, /**< Sampler state for a cube texture. */
		GPOT_SAMPLER2DMS = 5, /**< Sampler state for a 2D texture with multiple samples. */
		GPOT_TEXTURE1D = 11, /**< 1D texture. */
		GPOT_TEXTURE2D = 12, /**< 2D texture. */
		GPOT_TEXTURE3D = 13, /**< 3D texture. */
		GPOT_TEXTURECUBE = 14, /**< Cube texture. */
		GPOT_TEXTURE2DMS = 15, /**< 2D texture with multiple samples. */
		GPOT_BYTE_BUFFER = 32, /**< Buffer containing raw bytes (no interpretation). */
		GPOT_STRUCTURED_BUFFER = 33, /**< Buffer containing a set of structures. */
		GPOT_RWTYPED_BUFFER = 41, /**< Read-write buffer containing a set of primitives. */
		GPOT_RWBYTE_BUFFER = 42, /**< Read-write buffer containing raw bytes (no interpretation). */
		GPOT_RWSTRUCTURED_BUFFER = 43, /**< Read-write buffer containing a set of structures. */
		GPOT_RWSTRUCTURED_BUFFER_WITH_COUNTER = 44, /**< Read-write buffer containing a set of structures, with a counter. */
		GPOT_RWAPPEND_BUFFER = 45, /**< Buffer that can be used for appending data in a stack-like fashion. */
		GPOT_RWCONSUME_BUFFER = 46, /**< Buffer that can be used for consuming data in a stack-like fashion. */
		GPOT_RWTEXTURE1D = 50, /**< 1D texture with unordered read/writes. */
		GPOT_RWTEXTURE2D = 51, /**< 2D texture with unordered read/writes. */
		GPOT_RWTEXTURE3D = 52, /**< 3D texture with unordered read/writes. */
		GPOT_RWTEXTURE2DMS = 53, /**< 2D texture with multiple samples and unordered read/writes. */
		GPOT_TEXTURE1DARRAY = 54, /**< 1D texture with multiple array entries. */
		GPOT_TEXTURE2DARRAY = 55, /**< 2D texture with multiple array entries. */
		GPOT_TEXTURECUBEARRAY = 56, /**< Cubemap texture with multiple array entries. */
		GPOT_TEXTURE2DMSARRAY = 57, /**< 2D texture with multiple samples and array entries. */
		GPOT_RWTEXTURE1DARRAY = 58, /**< 1D texture with multiple array entries and unordered read/writes. */
		GPOT_RWTEXTURE2DARRAY = 59, /**< 2D texture with multiple array entries and unordered read/writes. */
		GPOT_RWTEXTURE2DMSARRAY = 60, /**< 2D texture with multiple array entries, samples and unordered read/writes. */
		GPOT_UNKNOWN = 0xffff
	};

	/** Returns information about GPU parameter types. */
	struct GpuObjectParameterTypeInformation 
	{
		static bool Is1DTexture(const GpuParameterObjectType type)
		{
			return type == GPOT_RWTEXTURE1D || type == GPOT_TEXTURE1D;
		}

		static bool Is1DTextureArray(const GpuParameterObjectType type)
		{
			return type == GPOT_RWTEXTURE1DARRAY || type == GPOT_TEXTURE1DARRAY;
		}

		static bool Is2DTexture(GpuParameterObjectType type)
		{
			return type == GPOT_RWTEXTURE2D || type == GPOT_RWTEXTURE2DMS || type == GPOT_TEXTURE2D || type == GPOT_TEXTURE2DMS;
		}

		static bool Is2DTextureArray(GpuParameterObjectType type)
		{
			return type == GPOT_RWTEXTURE2DARRAY || type == GPOT_RWTEXTURE2DMSARRAY || type == GPOT_TEXTURE2DARRAY || type == GPOT_TEXTURE2DMSARRAY;
		}

		static bool Is3DTexture(GpuParameterObjectType type)
		{
			return type == GPOT_RWTEXTURE3D || type == GPOT_TEXTURE3D;
		}

		static bool IsCubeTexture(GpuParameterObjectType type)
		{
			return type == GPOT_TEXTURECUBE;
		}

		static bool IsCubeTextureArray(GpuParameterObjectType type)
		{
			return type == GPOT_TEXTURECUBEARRAY;
		}

		static bool IsTexture(GpuParameterObjectType type)
		{
			return Is1DTexture(type) || Is1DTextureArray(type) || Is2DTexture(type) || Is2DTextureArray(type) || Is3DTexture(type) || IsCubeTexture(type) || IsCubeTextureArray(type);
		}

		static bool IsSampler(GpuParameterObjectType type)
		{
			return type == GPOT_SAMPLER1D || type == GPOT_SAMPLER2D || type == GPOT_SAMPLER2DMS || type == GPOT_SAMPLER3D || type == GPOT_SAMPLERCUBE;
		}

		static bool IsBuffer(GpuParameterObjectType type)
		{
			return !IsTexture(type) && !IsSampler(type) && type != GPOT_UNKNOWN;
		}
	};

	/** Specifies the type of GPU queue. This controls which command buffers are allowed to be submitted on the queue. */
	enum GpuQueueType
	{
		/** Queue used for rendering. Allows the use of draw, compute and transfer commands. This is the default all-purpose usage. */
		GQT_GRAPHICS,
		/** Queue used for compute operations. Allows the use of compute and transfer commands, but no draw commands allowed. */
		GQT_COMPUTE,
		/** Queue used for memory transfer operations only. No draw or compute commands allowed. */
		GQT_TRANSFER,
		GQT_COUNT, // Keep in front of GQT_UNKNOWN
		GQT_UNKNOWN
	};

	/**
	 * Bits that map to a specific surface of a render target. Combine the bits to generate a mask that references
	 * only specific render target surfaces.
	 */
	enum RenderSurfaceMaskBits
	{
		RT_NONE = 0,
		RT_COLOR0 = 1 << 0,
		RT_COLOR1 = 1 << 1,
		RT_COLOR2 = 1 << 2,
		RT_COLOR3 = 1 << 3,
		RT_COLOR4 = 1 << 4,
		RT_COLOR5 = 1 << 5,
		RT_COLOR6 = 1 << 6,
		RT_COLOR7 = 1 << 7,
		RT_DEPTH = 1 << 30,
		RT_STENCIL = 1 << 31,
		RT_DEPTH_STENCIL = RT_DEPTH | RT_STENCIL,
		RT_COLOR_ALL = RT_COLOR0 | RT_COLOR1 | RT_COLOR2 | RT_COLOR3 | RT_COLOR4 | RT_COLOR5 | RT_COLOR6 | RT_COLOR7,
		RT_ALL = 0xFFFFFFFF
	};

	typedef Flags<RenderSurfaceMaskBits> RenderSurfaceMask;
	B3D_FLAGS_OPERATORS(RenderSurfaceMaskBits);

	/**	Texture addressing mode, per component. */
	struct UVWAddressingMode
	{
		UVWAddressingMode()
			: U(TAM_WRAP), V(TAM_WRAP), W(TAM_WRAP)
		{}

		bool operator==(const UVWAddressingMode& rhs) const
		{
			return U == rhs.U && V == rhs.V && W == rhs.W;
		}

		TextureAddressingMode U, V, W;
	};

	/**	References a subset of surfaces within a texture. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportAsStruct(true)) TextureSurface
	{
		TextureSurface(u32 mipLevel = 0, u32 mipLevelCount = 1, u32 face = 0, u32 faceCount = 1, bool isBoundAs2DArray = false)
			: MipLevel(mipLevel), MipLevelCount(mipLevelCount), Face(face), FaceCount(faceCount), IsBoundAs2DArray(isBoundAs2DArray)
		{}

		bool operator==(const TextureSurface& rhs) const
		{
			return MipLevel == rhs.MipLevel && MipLevelCount == rhs.MipLevelCount && Face == rhs.Face && FaceCount == rhs.FaceCount && IsBoundAs2DArray == rhs.IsBoundAs2DArray;
		}

		bool operator!=(const TextureSurface& rhs) const { return !operator==(rhs); }

		/** First mip level to reference. */
		u32 MipLevel;

		/** Number of mip levels to reference. Must be greater than zero. */
		u32 MipLevelCount;

		/**
		 * First face to reference. Face can represent a single cubemap face, or a single array entry in a
		 * texture array. If cubemaps are laid out in a texture array then every six sequential faces represent a single
		 * array entry.
		 */
		u32 Face;

		/** Number of faces to reference, if the texture has more than one. */
		u32 FaceCount;

		/** Forces a cubemap or a 3D texture to be bound as a 2D texture array. */
		bool IsBoundAs2DArray;

		/** Surface that covers all texture sub-resources. */
		static B3D_EXPORT const TextureSurface kComplete;
	};

	/** @} */

	/** @addtogroup Image
	 *  @{
	 */

	/**	Available texture types. */
	enum B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) TextureType
	{
		/** One dimensional texture. Just a row of pixels. */
		TEX_TYPE_1D B3D_SCRIPT_EXPORT(ExportName(Texture1D)) = 1,
		/** Two dimensional texture. */
		TEX_TYPE_2D B3D_SCRIPT_EXPORT(ExportName(Texture2D)) = 2,
		/** Three dimensional texture. */
		TEX_TYPE_3D B3D_SCRIPT_EXPORT(ExportName(Texture3D)) = 3,
		/** Texture consisting out of six 2D textures describing an inside of a cube. Allows special sampling. */
		TEX_TYPE_CUBE_MAP B3D_SCRIPT_EXPORT(ExportName(TextureCube)) = 4
	};

	/** @} */

	/** @addtogroup Rendering
	 *  @{
	 */

	/**	Projection type to use by the camera. */
	enum B3D_SCRIPT_EXPORT() ProjectionType
	{
		/** Projection type where object size remains constant and parallel lines remain parallel. */
		PT_ORTHOGRAPHIC B3D_SCRIPT_EXPORT(ExportName(Orthographic)),
		/** Projection type that emulates human vision. Objects farther away appear smaller. */
		PT_PERSPECTIVE B3D_SCRIPT_EXPORT(ExportName(Perspective))
	};

	/** @} */

	/** @addtogroup Renderer
	 *  @{
	 */

	/**
	 * Suggested queue priority numbers used for sorting objects in the render queue. Objects with higher priority will
	 * be renderer sooner.
	 */
	enum class QueuePriority
	{
		Opaque = 100000,
		Transparent = 90000,
		Skybox = 80000,
		Overlay = 70000
	};

	/** Type of sorting to perform on an object when added to a render queue. */
	enum class QueueSortType
	{
		FrontToBack, /**< All objects with the same priority will be rendered front to back based on their center. */
		BackToFront, /**< All objects with the same priority will be rendered back to front based on their center. */
		None /**< Objects will not be sorted and will be processed in the order they were added to the queue. */
	};

	/** Determines the type of the source image for generating cubemaps. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Utility), Assembly(Engine), Assembly(Editor), API(FrameworkOnly)) CubemapSourceType
	{
		/** Source is a single image that will be replicated on all cubemap faces. */
		Single,

		/**
		 * Source is a list of 6 images, either sequentially next to each other or in a cross format. The system will
		 * automatically guess the layout and orientation based on the aspect ratio.
		 */
		Faces,

		/** Source is a single spherical panoramic image. */
		Spherical,

		/** Source is a single cylindrical panoramic image. */
		Cylindrical
	};

	/** @} */

	/** @addtogroup Material
	 *  @{
	 */

	/**	Flags that may be assigned to a shader that let the renderer know how to interpret the shader. */
	enum class ShaderFlag
	{
		Transparent = 0x1, /**< Signifies that the shader is rendering a transparent object. */
		Forward = 0x2 /**< Signifies the shader should use the forward rendering pipeline, if relevant. */
	};

	typedef Flags<ShaderFlag> ShaderFlags;
	B3D_FLAGS_OPERATORS(ShaderFlag)

	/** @} */

	/** @addtogroup Physics
	 *  @{
	 */

	/** Valid types of a mesh used for physics. */
	enum class B3D_SCRIPT_EXPORT() PhysicsMeshType
	{
		/**
		 * A regular triangle mesh. Mesh can be of arbitrary size but cannot be used for triggers and non-kinematic
		 * objects. Incurrs a significantly larger performance impact than convex meshes.
		 */
		Triangle,
		/**
		 * Mesh representing a convex shape. Mesh will not have more than 256 vertices. Incurrs a significantly lower
		 * performance impact than triangle meshes.
		 */
		Convex
	};

	/** @} */

	/** @addtogroup Utility-Engine
	 *  @{
	 */

	/** Names of individual components of a vector. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Utility)) VectorComponent
	{
		X,
		Y,
		Z,
		W
	};

	/** Names of individual components of a color. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Utility)) ColorComponent
	{
		R,
		G,
		B,
		A
	};

	/** Identifiers representing a range of values. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Utility)) RangeComponent
	{
		Min,
		Max
	};

	/** Names of individual components of an area. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Utility)) AreaComponent
	{
		X,
		Y,
		Width,
		Height
	};

	/** Names of individual components of a size. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Utility)) SizeComponent
	{
		Width,
		Height
	};

	/** @} */

	/** @addtogroup Scene
	 *  @{
	 */

	/**
	 * Controls what kind of mobility restrictions a scene object has. This is used primarily as a performance hint to
	 * other systems. Generally the more restricted the mobility the higher performance can be achieved.
	 */
	enum class B3D_SCRIPT_EXPORT() ObjectMobility
	{
		/** Scene object can be moved and has no mobility restrictions. */
		Movable,
		/**
		 * Scene object isn't allowed to be moved but is allowed to be visually changed in other ways (e.g. changing the
		 * displayed mesh or light intensity (depends on attached components).
		 */
		Immovable,
		/** Scene object isn't allowed to be moved nor is it allowed to be visually changed. Object must be fully static. */
		Static
	};

	/** @} */

	/** @addtogroup Audio
	 *  @{
	 */

	/** Meta-data describing a chunk of audio. */
	struct AudioDataInfo
	{
		u32 SampleCount; /**< Total number of audio samples in the audio data (includes all channels). */
		u32 SampleRate; /**< Number of audio samples per second, per channel. */
		u32 ChannelCount; /**< Number of channels. Each channel has its own set of samples. */
		u32 BitDepth; /**< Number of bits per sample. */
	};

	/** @} */

	/** @addtogroup RenderThread
	 *  @{
	 */

	/** Helper class for syncing dirty data from CoreObject to RenderProxy. */
	class CoreSyncData
	{
	public:
		CoreSyncData(RenderProxySyncPacket* syncPacket = nullptr)
			: mSyncPacket(syncPacket)
		{}

		template<class T = RenderProxySyncPacket>
		T* GetSyncPacket() const
		{
			return (T*)mSyncPacket;
		}

	private:
		RenderProxySyncPacket* mSyncPacket = nullptr;
	};

	/** @} */
} // namespace b3d

/** @cond STDLIB */

namespace std {

template <>
struct hash<b3d::TextureSurface>
{
	size_t operator()(const b3d::TextureSurface& value) const
	{
		size_t hash = 0;
		b3d::B3DCombineHash(hash, value.MipLevel);
		b3d::B3DCombineHash(hash, value.MipLevelCount);
		b3d::B3DCombineHash(hash, value.Face);
		b3d::B3DCombineHash(hash, value.FaceCount);

		return hash;
	}
};
} // namespace std

/** @endcond */
