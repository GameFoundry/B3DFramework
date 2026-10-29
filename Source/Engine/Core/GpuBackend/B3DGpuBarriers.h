//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuQueue.h"
#include "GpuBackend/B3DGpuTextureSubresource.h"
#include "Utility/B3DTArrayView.h"

namespace b3d
{
	class IGpuResource;
	class IGpuBufferResource;
	class IGpuImageResource;

	/** Flags that determine how is a resource being accessed by the GPU. */
	enum class GpuAccessFlag
	{
		None = 0,
		Read = 0x1,
		Write = 0x2
	};

	typedef Flags<GpuAccessFlag> GpuAccessFlags;
	B3D_FLAGS_OPERATORS(GpuAccessFlag);

	namespace render
	{
		enum class GpuImageLayout;

		/**
		 * Determines on which pipeline stage and in what manner a resource is being accessed. Combined with read/write
		 * access flags this uniquely determines the synchronization (pipeline/cache barriers) a backend must issue.
		 * Each backend maps these logical stages to its native pipeline stage and access/cache masks.
		 */
		enum class GpuStageFlag : u16
		{
			None							= 0,
			DrawIndirect					= 1 << 0,	/**< Indirect draw/dispatch argument fetch. */
			VertexInputAttributes			= 1 << 1,	/**< Vertex attribute fetch (vertex buffers). */
			VertexInputIndices				= 1 << 2,	/**< Index buffer fetch. */
			VertexShaderNonUniform			= 1 << 3,	/**< Non-uniform (sampled/storage) access in the vertex shader stage. */
			FragmentShaderNonUniform		= 1 << 4,	/**< Non-uniform (sampled/storage) access in the fragment shader stage. */
			ComputeShaderNonUniform			= 1 << 5,	/**< Non-uniform (sampled/storage) access in the compute shader stage. */
			VertexShaderUniform				= 1 << 6,	/**< Uniform/constant buffer access in the vertex shader stage. */
			FragmentShaderUniform			= 1 << 7,	/**< Uniform/constant buffer access in the fragment shader stage. */
			ComputeShaderUniform			= 1 << 8,	/**< Uniform/constant buffer access in the compute shader stage. */
			EarlyFragmentTests				= 1 << 9,	/**< Depth/stencil access before the fragment shader. */
			LateFragmentTests				= 1 << 10,	/**< Depth/stencil access after the fragment shader. */
			ColorAttachment					= 1 << 11,	/**< Color attachment (render target) read/write. */
			Transfer						= 1 << 12,	/**< Copy/blit/clear transfer operations. */
			Host							= 1 << 13,	/**< Access by the host (CPU). */
			Resolve							= 1 << 14,	/**< Multisample resolve operations. */

			AllShader = VertexShaderNonUniform | FragmentShaderNonUniform | ComputeShaderNonUniform | VertexShaderUniform | FragmentShaderUniform | ComputeShaderUniform,
			All = AllShader | DrawIndirect | VertexInputAttributes | VertexInputIndices | EarlyFragmentTests | LateFragmentTests | ColorAttachment | Transfer | Host | Resolve
		};

		typedef Flags<GpuStageFlag, u16> GpuStageFlags;
		B3D_FLAGS_OPERATORS_EXT(GpuStageFlag, u16)

		/** Additional behavior requested when issuing a buffer or an image barrier. */
		enum class GpuBarrierFlag : u8
		{
			None = 0,

			/**
			 * The barrier starts a new lifetime of the resource on memory that earlier resources used (see GpuAliasAcquire). On an image it
			 * starts from GpuImageLayout::Undefined.
			 */
			AliasAcquire = 1 << 0,

			/**
			 * The barrier ends the lifetime of a resource whose memory an alias acquire of another resource takes over (see
			 * GpuAliasAcquire::Predecessors). Its destination has GpuImageLayout::Undefined and no access, and its destination stages are
			 * those of the acquiring resource's first use. It is the last barrier of the resource, recorded together with the alias acquire
			 * or before it. Backends whose alias acquires flush the earlier resources' writes record nothing for it.
			 */
			AliasRelease = 1 << 1
		};

		typedef Flags<GpuBarrierFlag, u8> GpuBarrierFlags;
		B3D_FLAGS_OPERATORS_EXT(GpuBarrierFlag, u8)

		/** Records all stages that a resource was read or written from. */
		struct GpuAccessScope
		{
			GpuStageFlags ReadStages = GpuStageFlag::None;
			GpuStageFlags WriteStages = GpuStageFlag::None;

			/** Adds a read/write access without losing its association with the provided stages. */
			void Add(GpuStageFlags stages, GpuAccessFlags access)
			{
				if(access.IsSet(GpuAccessFlag::Read))
					ReadStages |= stages;

				if(access.IsSet(GpuAccessFlag::Write))
					WriteStages |= stages;
			}

			/** Adds all accesses from another scope. */
			void Add(const GpuAccessScope& scope)
			{
				ReadStages |= scope.ReadStages;
				WriteStages |= scope.WriteStages;
			}

			/** Returns all stages that perform a read or write access. */
			GpuStageFlags GetStages() const { return ReadStages | WriteStages; }

			/** Returns the access types used by any stage in the scope. */
			GpuAccessFlags GetAccess() const
			{
				GpuAccessFlags access = GpuAccessFlag::None;
				if(ReadStages != GpuStageFlag::None)
					access |= GpuAccessFlag::Read;

				if(WriteStages != GpuStageFlag::None)
					access |= GpuAccessFlag::Write;

				return access;
			}

			/** Returns the access types used by the requested stages. */
			GpuAccessFlags GetAccess(GpuStageFlags stages) const
			{
				GpuAccessFlags access = GpuAccessFlag::None;
				if(ReadStages.IsSetAny(stages))
					access |= GpuAccessFlag::Read;

				if(WriteStages.IsSetAny(stages))
					access |= GpuAccessFlag::Write;

				return access;
			}

			/** Returns true if the scope contains a read or write access. */
			bool IsValid() const { return GetStages() != GpuStageFlag::None; }
		};

		/**
		 * Describes how the first access of a resource placed on memory that earlier resources used must be synchronized. The resource starts a
		 * new lifetime on that memory, and its contents are undefined. See GpuBarrierFlag::AliasAcquire.
		 */
		struct GpuAliasAcquire
		{
			/**
			 * Last accesses of the earlier resources of the scope on the memory (Predecessors), on the queue that records the acquire. The new
			 * resource's first access is ordered after them, and through them after every access they were ordered after. Accesses not ordered
			 * before one another, such as reads in different stages, must all be included. Empty if there are none. Accesses on other queues
			 * must already be ordered by queue waits. The predecessors may cover only part of the memory, see HasUnknownSourceAccess.
			 */
			GpuAccessScope Source;

			/**
			 * Earlier resources on the memory. Accessing them after the acquire is invalid until they are acquired again. Their lifetimes
			 * have to be ended using a GpuBarrierFlag::AliasRelease barrier before the memory is used on a new resource.
			 */
			TArrayView<IGpuResource* const> Predecessors;

			/**
			 * True if earlier accesses to part of the memory are unknown: no earlier resource of the scope used it, but resources of earlier
			 * scopes may have, and Source does not include their accesses. The new resource's first access must then also be ordered after all
			 * earlier work on its queue. 
			 */
			bool HasUnknownSourceAccess = false;
		};

		/** Describes a source and destination stage & access participating in a hazard barrier. */
		struct GpuBarrierScope
		{
			GpuBarrierScope() = default;

			GpuBarrierScope(GpuStageFlags sourceStages, GpuAccessFlags sourceAccess, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess)
				: SourceStages(sourceStages), SourceAccess(sourceAccess), DestinationStages(destinationStages), DestinationAccess(destinationAccess)
			{ }

			GpuStageFlags SourceStages = GpuStageFlag::None;
			GpuAccessFlags SourceAccess = GpuAccessFlag::None;
			GpuStageFlags DestinationStages = GpuStageFlag::None;
			GpuAccessFlags DestinationAccess = GpuAccessFlag::None;

			/** Returns true if the object contains a source and destination scope. */
			bool IsValid() const { return SourceStages != GpuStageFlag::None && DestinationStages != GpuStageFlag::None; }
		};

	}
}
