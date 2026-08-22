//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuQueue.h"
#include "GpuBackend/B3DGpuTextureSubresource.h"

#define B3D_VERIFY_BARRIERS B3D_BUILD_TYPE_DEVELOPMENT // If enabled, ensures that memory barriers are properly issued

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

		/**
		 * Contains hazards that determine what kind of barrier (if any) needs to be issued on the next access of the resource. This information
		 * is all that is required to determine barriers within a single command buffer recording scope.
		 *
		 * Each write waits on all prior reads & writes, and then resets the hazard state as all hazards are guaranteed to be resolved.
		 * Each read waits on prior writes (except those that have been waited on by prior reads - represented as visible stages).
		 */
		struct B3D_EXPORT GpuResourceWriteEpochHazardState
		{
			/** Stages that produced the write starting the current write epoch. */
			GpuStageFlags WriteStages = GpuStageFlag::None;

			/** Stages that read the resource after the most recent write. */
			GpuStageFlags ReaderStages = GpuStageFlag::None;

			/** Stages where the most recent write has been made visible for reads. */
			GpuStageFlags VisibleStages = GpuStageFlag::None;

			/**
			 * Determines the barrier required before an access without changing the tracked state.
			 *
			 * @p broadenedReadStages may optionally be used to specify additional stages beyond what's currently required. Usually does not increase the cost
			 * of the barrier by much, and saves cost by skipping future barriers to be issued for those stages (e.g. whenever reading a resource you
			 * may want to issue a barrier with all shader stages).
			 */
			GpuBarrierScope GetRequiredBarrier(GpuStageFlags stages, GpuAccessFlags access, GpuStageFlags broadenedReadStages = GpuStageFlag::None) const;

			/** Records that a resource was accessed on a particular stage. This either starts a new write epoch or appends the list of readers. */
			void RecordAccess(GpuStageFlags stages, GpuAccessFlags access);

			/** Marks stages that a write was made visible to after the provided barrier was issued. */
			void RecordBarrier(const GpuBarrierScope& barrier);
		};

		/**
		 * Tracks accesses, barriers and write-epoch hazards for one resource over a single command buffer recording scope. 
		 *
		 * Barriers can be placed into two categories depending where they are issued:
		 *  - During command buffer recording - Barriers issued during recording depend exclusively on LastWriteEpochHazardState (and LastBarrier, for backends that require barrier chaining).  All other
		 *	  state we keep is for the purpose of determining the submission barrier.
		 *  - During command buffer submission - These barriers are issued in a specialized 'prelude' command buffer that will execute before the command buffer that recorded the resource access. We call these 'submission barriers'.
		 *    We cannot issue these barriers during recording as at that time we do not know what command buffer will be submitted before it, and how will it use the same resources. The state tracking here generally
		 *    boils down to 'how was the resource first used in this command buffer'.
		 * 
		 */
		struct B3D_EXPORT GpuResourceHazardState
		{
			GpuAccessScope AccessScopeBeforeFirstBarrier; /**< Accesses recorded before the first barrier. */
			bool HasLeadingBarrier = false; /**< Whether an explicit barrier was recorded before the first access. */
			GpuAccessScope AllAccessScope; /**< Accumulation of all accesses recorded in the tracking scope. Generally only used for cross-queue synchronization (answers the question does this command buffer read and/or write?). */
			GpuResourceWriteEpochHazardState LastWriteEpochHazardState; /**< Last write-epoch hazard state in the recording scope. Updated during recording. After recording stores the last state. */
			GpuBarrierScope LastBarrier; /**< Most recently recorded barrier in the command-buffer recording scope. Some backends require this for barrier chaining, for others it's unused. */

			/** Determines the resource hazard barrier required before an access. */
			GpuBarrierScope GetRequiredBarrier(GpuStageFlags stages, GpuAccessFlags access, GpuStageFlags broadenedReadStages = GpuStageFlag::None) const;

			/** Records that a resource was accessed on a particular stage. */
			void RecordAccess(GpuStageFlags stages, GpuAccessFlags access);

			/** Records a barrier and credits any visibility it establishes. */
			void RecordBarrier(const GpuBarrierScope& barrier);

			/** Returns true if the tracking scope accesses the resource. */
			bool HasAccess() const { return AllAccessScope.IsValid(); }

			/** Returns true if the tracking scope can change the resource's carried hazard state. */
			bool HasSubmissionEffect() const { return HasAccess() || HasLeadingBarrier; }

			/** Returns true if the tracking scope writes the resource. */
			bool HasWrite() const { return AllAccessScope.WriteStages != GpuStageFlag::None; }

			/** Returns the destination access scope required by the submission barrier. */
			const GpuAccessScope& GetSubmissionBarrierAccessScope() const { return AccessScopeBeforeFirstBarrier; }
		};

		/**
		 * Submission state for a GPU resource. Updated after a command buffer using a resource is submitted - it describes its usage
		 * on the last submitted command buffer and its queue, as well as any other queues it was submitted on. Used for determining
		 * what memory barriers, execution barriers or queue waits are required when a new command buffer using this resource is 
		 * submitted (represented by GpuSubmissionTransition).
		 */
		struct B3D_EXPORT GpuResourceSubmissionState
		{
			GpuResourceWriteEpochHazardState WriterHazards; /**< Write-epoch hazards on the queue containing the latest write. */
			GpuQueueId WriterQueueId; /**< Queue containing the latest write. Valid only when HasWriter is true. */
			GpuQueueMask AcquiredQueues = GpuQueueMask::kNone; /**< Queues that already acquired the latest write. */
			GpuQueueMask ReaderQueues = GpuQueueMask::kNone; /**< Queues with read submissions after the latest write. */
			GpuStageFlags ReaderStages = GpuStageFlag::None; /**< Conservative stage union for ReaderQueues. */
			bool HasWriter = false; /**< Whether the current epoch was created by a write. */

			/** Returns unresolved accesses accumulated by the writer and all parallel readers. */
			GpuAccessScope GetUnsafeAccessScope() const
			{
				GpuAccessScope scope;
				scope.Add(WriterHazards.ReaderStages | ReaderStages, GpuAccessFlag::Read);
				scope.Add(WriterHazards.WriteStages, GpuAccessFlag::Write);
				return scope;
			}
		};

		/** 
		 * Contains required barriers and waits for a GPU resource during a command buffer submission. The exact transitions depend on 
		 * the GPU backend to interpret them, but generally falls into the following categories: 
		 * - Command buffer submitted on the same queue that the resource was used on previously. In this case the backend may issue a memory barrier and/or 
		 *   execution barrier to ensure proper ordering of resource accesses.
		 * - Command buffer submitted on a different queue than the resource was used on previously. In that case the backend may issue a semaphore wait to 
		 *   ensure that the previous queue has completed its work before the current queue starts using the resource.
		 *
		 * In either case, the transition also contains the post-submission state of the resource, which will be used to update the resource's submission state 
		 * after the command buffer is submitted, so the next submitted command buffer can calculate its required transition.
		 */
		struct B3D_EXPORT GpuSubmissionTransition
		{
			IGpuResource* StateResource = nullptr;
			GpuAccessScope SubmissionBarrierAccessScope; /**< Destination access scope required by the submission barrier. */
			GpuAccessScope DestinationAllAccessScope;
			GpuAccessScope SourceAccessScope; /**< Conservative source scope for backend ownership/layout/state transitions. */
			GpuBarrierScope MemoryBarrier; /**< Same-queue memory dependency recorded before destination execution. */
			GpuBarrierScope ExecutionBarrier; /**< Same-queue execution dependency recorded before destination execution. */
			GpuResourceSubmissionState PostTransitionSubmissionState; /**< State committed after the visitor constructs native synchronization. */

			/**
			 * Complete cross-queue wait set required to make the command buffer's recorded resource accesses safe while
			 * preserving parallelism where legal. Reads wait for the latest writer but remain parallel with other readers;
			 * writes wait for active readers and therefore naturally become exclusive.
			 */
			GpuQueueMask ParallelAccessWaitMask = GpuQueueMask::kNone;

			/**
			 * Alternative complete wait set for an operation that requires exclusive resource access, such as mutating
			 * image layout, queue ownership, native resource state or compression metadata. Use this instead of
			 * ParallelAccessWaitMask for that operation, not in addition to it.
			 */
			GpuQueueMask ExclusiveAccessWaitMask = GpuQueueMask::kNone;

			GpuSubmissionTransition(IGpuResource& stateResource, const GpuAccessScope& submissionBarrierAccessScope, const GpuAccessScope& destinationAllAccessScope);

			/** Returns true if a barrier must be recorded on the destination queue. */
			bool HasSameQueueDependency() const { return MemoryBarrier.IsValid() || ExecutionBarrier.IsValid(); }

			/** Builds the complete synchronization and post-submission state for one resource on @p destinationQueueId. */
			static GpuSubmissionTransition Build(IGpuResource& stateResource, GpuQueueId destinationQueueId, const GpuResourceHazardState& destinationHazardState);
		};

		/** Submission-boundary description for a buffer. */
		struct B3D_EXPORT GpuSubmissionBufferTransition : GpuSubmissionTransition
		{
			GpuSubmissionBufferTransition(IGpuBufferResource& buffer, GpuSubmissionTransition&& transition)
				: GpuSubmissionTransition(std::move(transition)), Buffer(&buffer)
			{ }

			IGpuBufferResource* Buffer = nullptr;
		};

		/** Submission-boundary description for one image face/mip. */
		struct B3D_EXPORT GpuSubmissionImageTransition : GpuSubmissionTransition
		{
			GpuSubmissionImageTransition(IGpuImageResource& image, const GpuTextureSubresourceRange& imageRange, GpuImageLayout initialLayout, GpuImageLayout finalLayout, GpuSubmissionTransition&& transition)
				: GpuSubmissionTransition(std::move(transition)), Image(&image), ImageRange(imageRange), InitialLayout(initialLayout), FinalLayout(finalLayout)
			{ }

			IGpuImageResource* Image = nullptr;
			GpuTextureSubresourceRange ImageRange;
			GpuImageLayout InitialLayout{};
			GpuImageLayout FinalLayout{};
		};

		/** Receives submission-boundary transitions generated by a resource tracker. */
		class B3D_EXPORT GpuSubmissionTransitionVisitor
		{
		public:
			virtual ~GpuSubmissionTransitionVisitor() = default;
			virtual void VisitBuffer(const GpuSubmissionBufferTransition& transition) = 0;
			virtual void VisitImage(const GpuSubmissionImageTransition& transition) = 0;
		};

	}
}
