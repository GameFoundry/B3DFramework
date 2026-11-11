//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Math/B3DMatrix4.h"

namespace b3d
{
	class IGpuResource;

	namespace render
	{
		class GpuAccelerationStructure;
	}

	/** @addtogroup GpuBackend
	 *  @{
	 */

	/** Memory an acceleration structure and its build need, as returned by GpuDevice::GetAccelerationStructureSizes(). */
	struct GpuAccelerationStructureSizes
	{
		GpuAccelerationStructureSizes() = default;
		GpuAccelerationStructureSizes(u32 accelerationStructureSize, u32 buildScratchSize)
			: AccelerationStructureSize(accelerationStructureSize), BuildScratchSize(buildScratchSize)
		{ }

		u32 AccelerationStructureSize = 0; /**< Size of the acceleration structure buffer, in bytes. */
		u32 BuildScratchSize = 0; /**< Size of the scratch memory a build needs, in bytes. */
	};

	/** Flags that control how rays interact with the geometry of a bottom level acceleration structure. */
	enum class GpuRayTracingGeometryFlag : u8
	{
		None = 0,
		Opaque = 1 << 0, /**< Hits on the geometry are accepted without running any-hit programs or callbacks. */
		/** 
		 * Any-hit programs and callbacks run at most once per primitive and ray. This is needed because a primitive may be split 
		 * across multiple acceleration structure nodes, and herefore hits may be reported more than once. Generally keeping this
		 * off results in faster ray tracing.
		 */
		NoDuplicateAnyHitInvocation = 1 << 1
	};

	using GpuRayTracingGeometryFlags = Flags<GpuRayTracingGeometryFlag, u8>;
	B3D_FLAGS_OPERATORS_EXT(GpuRayTracingGeometryFlag, u8)

	/** Flags that trade the build time of an acceleration structure against its size and the speed of rays traced against it. */
	enum class GpuAccelerationStructureBuildFlag : u8
	{
		None = 0,
		PreferFastTrace = 1 << 0, /**< Prefers faster ray tracing over a faster build. Cannot be combined with PreferFastBuild. */
		PreferFastBuild = 1 << 1, /**< Prefers a faster build over faster ray tracing. Cannot be combined with PreferFastTrace. */
		MinimizeMemory = 1 << 2 /**< Prefers a smaller acceleration structure and scratch memory, at the cost of build time and ray tracing speed. */
	};

	using GpuAccelerationStructureBuildFlags = Flags<GpuAccelerationStructureBuildFlag, u8>;
	B3D_FLAGS_OPERATORS_EXT(GpuAccelerationStructureBuildFlag, u8)

	/** Triangle list geometry of a bottom level acceleration structure. */
	struct GpuRayTracingGeometry
	{
		GpuRayTracingGeometry() = default;

		/**
		 * @param	vertexBuffer	Buffer with the vertex positions.
		 * @param	vertexStride	Distance between consecutive vertex positions, in bytes.
		 * @param	vertexCount		Number of vertices the geometry uses.
		 * @param	indexBuffer		Buffer with the triangle indices, or null for non-indexed triangles.
		 * @param	indexCount		Number of indices, three per triangle.
		 * @param	indexType		Size of a single index.
		 */
		GpuRayTracingGeometry(const TShared<render::GpuBuffer>& vertexBuffer, u32 vertexStride, u32 vertexCount, const TShared<render::GpuBuffer>& indexBuffer = nullptr,
			u32 indexCount = 0, b3d::IndexType indexType = IT_32BIT)
			: VertexBuffer(vertexBuffer), VertexStride(vertexStride), VertexCount(vertexCount), IndexBuffer(indexBuffer), IndexCount(indexCount), IndexType(indexType)
		{ }

		GpuRayTracingGeometryFlags Flags = GpuRayTracingGeometryFlag::Opaque;

		/** Buffer with the vertex positions. A vertex or structured storage buffer created with GpuBufferFlag::AccelerationStructureInput. */
		TShared<render::GpuBuffer> VertexBuffer;

		u32 VertexOffset = 0; /**< Offset of the first vertex position in the vertex buffer, in bytes. A multiple of 4. */
		u32 VertexStride = 0; /**< Distance between consecutive vertex positions, in bytes. A multiple of 4, at least the size of a position. */
		u32 VertexCount = 0; /**< Number of vertices the geometry uses. Every index is smaller than this. */
		VertexElementType VertexType = VET_FLOAT3; /**< Type of a vertex position. Must be VET_FLOAT3. */

		/**
		 * Buffer with the triangle indices, or null for non-indexed triangles, which take three consecutive vertices each. An index or
		 * structured storage buffer created with GpuBufferFlag::AccelerationStructureInput.
		 */
		TShared<render::GpuBuffer> IndexBuffer;

		u32 IndexOffset = 0; /**< Offset of the first index in the index buffer, in bytes. A multiple of the index size. */
		u32 IndexCount = 0; /**< Number of indices, three per triangle. */
		b3d::IndexType IndexType = IT_32BIT; /**< Size of a single index. */
	};

	/** Contents of an acceleration structure, used by size queries and builds. */
	struct GpuAccelerationStructureInputs
	{
		GpuAccelerationStructureInputs() = default;

		/**
		 * Describes a bottom level acceleration structure.
		 *
		 * @param	geometries	Geometries of the structure. Must stay valid until the call that receives the inputs returns.
		 * @param	flags		Flags that control the build.
		 */
		explicit GpuAccelerationStructureInputs(TArrayView<const GpuRayTracingGeometry> geometries, GpuAccelerationStructureBuildFlags flags = GpuAccelerationStructureBuildFlag::None)
			: Type(GpuAccelerationStructureType::BottomLevel), Flags(flags), Geometries(geometries)
		{ }

		/**
		 * Describes a top level acceleration structure.
		 *
		 * @param	instanceBuffer	Buffer with the instances, in the layout GpuDevice::WriteRayTracingInstances() writes.
		 * @param	instanceCount	Number of instances in the buffer.
		 * @param	flags			Flags that control the build.
		 */
		GpuAccelerationStructureInputs(const TShared<render::GpuBuffer>& instanceBuffer, u32 instanceCount, GpuAccelerationStructureBuildFlags flags = GpuAccelerationStructureBuildFlag::None)
			: Type(GpuAccelerationStructureType::TopLevel), Flags(flags), InstanceBuffer(instanceBuffer), InstanceCount(instanceCount)
		{ }

		GpuAccelerationStructureType Type = GpuAccelerationStructureType::BottomLevel;
		GpuAccelerationStructureBuildFlags Flags;

		/**
		 * Geometries of a bottom level acceleration structure. Rays report the position of the geometry they hit in this list as its
		 * geometry index. Must stay valid until the call that receives the inputs returns.
		 */
		TArrayView<const GpuRayTracingGeometry> Geometries;

		/**
		 * Instances of a top level acceleration structure, in the layout GpuDevice::WriteRayTracingInstances() writes. A structured
		 * storage buffer created with GpuBufferFlag::AccelerationStructureInput.
		 */
		TShared<render::GpuBuffer> InstanceBuffer;

		u32 InstanceOffset = 0; /**< Offset of the first instance in the instance buffer, in bytes. A multiple of GpuRayTracingCapabilities::InstanceSize. */
		u32 InstanceCount = 0; /**< Number of instances. Zero is valid, and builds a structure that rays never hit. */
	};

	/** Build of one acceleration structure, recorded by render::GpuCommandBuffer::BuildAccelerationStructures(). */
	struct GpuAccelerationStructureBuild
	{
		GpuAccelerationStructureBuild() = default;

		/**
		 * @param	inputs			Contents to build the structure from.
		 * @param	destination		Structure to build.
		 * @param	scratchBuffer	Memory the build uses while it runs.
		 * @param	scratchOffset	Offset of the build's scratch memory in @p scratchBuffer, in bytes.
		 */
		GpuAccelerationStructureBuild(const GpuAccelerationStructureInputs& inputs, const TShared<render::GpuAccelerationStructure>& destination,
			const TShared<render::GpuBuffer>& scratchBuffer, u32 scratchOffset = 0)
			: Inputs(inputs), Destination(destination), ScratchBuffer(scratchBuffer), ScratchOffset(scratchOffset)
		{ }

		/** Contents to build the structure from. Its type must match the type of the destination. */
		GpuAccelerationStructureInputs Inputs;

		/** Structure to build. Its previous contents are discarded. Its buffer must be at least as large as the size query reports. */
		TShared<render::GpuAccelerationStructure> Destination;

		/**
		 * Memory the build uses while it runs. A structured storage buffer stored on the GPU that allows unordered access. Builds recorded
		 * in one call must use separate ranges.
		 */
		TShared<render::GpuBuffer> ScratchBuffer;

		/**
		 * Offset of the build's scratch memory in the scratch buffer, in bytes. A multiple of GpuRayTracingCapabilities::ScratchOffsetAlignment,
		 * followed by at least GpuAccelerationStructureSizes::BuildScratchSize bytes.
		 */
		u32 ScratchOffset = 0;

		/**
		 * Every bottom level structure the instances of a top level structure reference. The top level structure keeps them alive until it
		 * is built again or destroyed. Must stay valid until the call that receives the build returns. Empty for a bottom level structure.
		 */
		TArrayView<const TShared<render::GpuAccelerationStructure>> BottomLevels;
	};

	/** Flags that change how rays interact with one instance of a top level acceleration structure. */
	enum class GpuRayTracingInstanceFlag : u8
	{
		None = 0,
		DisableTriangleCulling = 1 << 0, /**< Rays hit both faces of the instance's triangles, whatever their culling flags. */

		/**
		 * Swaps the faces of the instance's triangles. Without it, the face that rasterization considers the front face is the front face
		 * for rays too.
		 */
		FlipTriangleFacing = 1 << 1,

		ForceOpaque = 1 << 2, /**< Treats every geometry of the instance as opaque. Cannot be combined with ForceNonOpaque. */
		ForceNonOpaque = 1 << 3 /**< Treats every geometry of the instance as non-opaque. Cannot be combined with ForceOpaque. */
	};

	using GpuRayTracingInstanceFlags = Flags<GpuRayTracingInstanceFlag, u8>;
	B3D_FLAGS_OPERATORS_EXT(GpuRayTracingInstanceFlag, u8)

	/** Instance of a bottom level acceleration structure in a top level structure. See GpuDevice::WriteRayTracingInstances(). */
	struct GpuRayTracingInstance
	{
		GpuRayTracingInstance() = default;

		/**
		 * @param	bottomLevel		Structure the instance places in the scene.
		 * @param	transform		Transform from the space of the structure to the space of the scene.
		 * @param	instanceId		Value shaders read as the instance id.
		 */
		GpuRayTracingInstance(const render::GpuAccelerationStructure* bottomLevel, const Matrix4& transform, u32 instanceId = 0)
			: Transform(transform), InstanceId(instanceId), BottomLevel(bottomLevel)
		{ }

		Matrix4 Transform = Matrix4::kIdentity; /**< Transform from the space of the structure to the space of the scene. Affine. */
		u32 InstanceId = 0; /**< Value shaders read as the instance id. 24 bits. */
		u32 HitGroupIndexOffset = 0; /**< Added to the shader binding table record index of hits on the instance. 24 bits. */
		u8 Mask = 0xFF; /**< Rays only hit the instance if this and their instance mask have a bit in common. */
		GpuRayTracingInstanceFlags Flags;

		/** Bottom level structure the instance places in the scene. Must be in the build's GpuAccelerationStructureBuild::BottomLevels. */
		const render::GpuAccelerationStructure* BottomLevel = nullptr;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/**
		 * Bottom or top level acceleration structure, stored in a buffer of type GpuBufferType::AccelerationStructure. Barriers and frame
		 * graph declarations use the buffer. Create the buffer with the size GpuDevice::GetAccelerationStructureSizes() reports, and fill
		 * the structure with GpuCommandBuffer::BuildAccelerationStructures().
		 *
		 * @note	Render thread only.
		 */
		class B3D_EXPORT GpuAccelerationStructure
		{
		public:
			/** @param	buffer	Buffer of type GpuBufferType::AccelerationStructure that stores the structure. */
			explicit GpuAccelerationStructure(const TShared<GpuBuffer>& buffer);
			~GpuAccelerationStructure();

			GpuAccelerationStructure(const GpuAccelerationStructure&) = delete;
			GpuAccelerationStructure& operator=(const GpuAccelerationStructure&) = delete;

			/** Returns the level of the structure. */
			GpuAccelerationStructureType GetType() const { return mBuffer != nullptr ? mBuffer->GetInformation().AccelerationStructure.Type : GpuAccelerationStructureType::BottomLevel; }

			/** Returns the size of the structure's buffer, in bytes. */
			u32 GetSize() const { return mBuffer != nullptr ? mBuffer->GetInformation().AccelerationStructure.Size : 0; }

			/** Returns the buffer that stores the structure. Null if the constructor received a buffer that cannot store one. */
			const TShared<GpuBuffer>& GetBuffer() const { return mBuffer; }

			/**
			 * Returns the index shaders read the structure at in the bindless resource heap, or kGpuBindlessInvalidIndex if the buffer
			 * wasn't created with GpuBufferFlag::Bindless.
			 */
			u32 GetBindlessIndex() const { return mBuffer != nullptr ? mBuffer->GetBindlessIndex() : kGpuBindlessInvalidIndex; }

			/**
			 * @name Internal
			 * @{
			 */

			/**
			 * Keeps the native resources of @p bottomLevels alive for as long as the structure references them, and releases the ones of
			 * the previous build once the GPU completes the current frame. Called when a build of the structure is recorded.
			 */
			void SetBottomLevels(TArrayView<const TShared<GpuAccelerationStructure>> bottomLevels);

#if B3D_BUILD_TYPE_DEVELOPMENT
			/**
			 * Checks @p inputs against the rules that size queries and builds on @p device share, and logs the first rule they break.
			 * Development builds only.
			 */
			static bool ValidateInputs(const GpuDevice& device, const GpuAccelerationStructureInputs& inputs);
#endif

			/** @} */

		private:
			/** Releases the pins of the bottom level structures. */
			void UnpinBottomLevels();

			TShared<GpuBuffer> mBuffer;
			TArray<IGpuResource*> mPinnedBottomLevels;
		};

		/** @} */
	} // namespace render
} // namespace b3d
