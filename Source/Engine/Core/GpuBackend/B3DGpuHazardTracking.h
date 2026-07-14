//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuQueue.h"

#define B3D_VERIFY_BARRIERS B3D_BUILD_TYPE_DEVELOPMENT // If enabled, ensures that memory barriers are properly issued

namespace b3d
{
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

			AllShader = VertexShaderNonUniform | FragmentShaderNonUniform | ComputeShaderNonUniform | VertexShaderUniform | FragmentShaderUniform | ComputeShaderUniform,
			All = AllShader | DrawIndirect | VertexInputAttributes | VertexInputIndices | EarlyFragmentTests | LateFragmentTests | ColorAttachment | Transfer | Host
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
		struct GpuHazardStageAndAccess
		{
			GpuHazardStageAndAccess() = default;

			GpuHazardStageAndAccess(GpuStageFlags sourceStages, GpuAccessFlags sourceAccess, GpuStageFlags destinationStages, GpuAccessFlags destinationAccess)
				: SourceStages(sourceStages), SourceAccess(sourceAccess), DestinationStages(destinationStages), DestinationAccess(destinationAccess)
			{ }

			GpuStageFlags SourceStages = GpuStageFlag::None;
			GpuAccessFlags SourceAccess = GpuAccessFlag::None;
			GpuStageFlags DestinationStages = GpuStageFlag::None;
			GpuAccessFlags DestinationAccess = GpuAccessFlag::None;

			/** Returns true if the object contains a source and destination scope. */
			bool IsValid() const { return SourceStages != GpuStageFlag::None && DestinationStages != GpuStageFlag::None; }
		};

		/** Keeps track on which pipeline stages a resource was written/read, and on which stages it may be safely accessed from. */
		struct B3D_EXPORT GpuHazardStageTracking
		{
			static constexpr u32 kPipelineStageCount = 16;

			/** For each pipeline stage, stores in which stages is it safe to access the resource. */
			std::array<GpuStageFlags, kPipelineStageCount> SafeAccess;

			GpuHazardStageTracking();

			/** Clears safe access for all provided pipeline stages. */
			void ClearStageSafeAccess(GpuStageFlags stages);

			/**
			 * Adds safe access for all provided pipeline stages.
			 *
			 * @param	sourceStages		One or multiple stages to add the safe access to.
			 * @param	destinationStages	Stages to register as being safe to access from.
			 */
			void AddStageSafeAccess(GpuStageFlags sourceStages, GpuStageFlags destinationStages);

			/** Checks is it safe to access the resource in all the provided pipeline stages. */
			bool IsAccessSafe(GpuStageFlags stages) const;

			/** Returns a list of all source stages that we cannot safely access data from the provided @p stages. */
			GpuStageFlags GetUnsafeAccessStages(GpuStageFlags stages) const;

			/** Writes a descriptive error message when access is unsafe. */
			void LogUnsafeAccess(GpuStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const;
		};

		/**
		 * Outstanding read/write hazards and the destination stages that can safely access them.
		 * For each stage it maps which other stage can safely access that stage. Reads/writes have separate mappings:
		 *  - ExecutionBarrierTracking records which stages have accessed the resource for reads. Those cannot be written without issuing an execution barrier first.
		 *  - MemoryBarrierTracking records which stages have accessed the resource for writes. Those cannot be read or written without issuing a memory barrier first.
		 */
		struct B3D_EXPORT GpuHazardState
		{
			/** Read stages that require an execution dependency before a subsequent write. */
			GpuHazardStageTracking ExecutionBarrierTracking;

			/** Write stages that require a memory dependency before a subsequent read or write. */
			GpuHazardStageTracking MemoryBarrierTracking;

			/** Registers a resource access as unsafe for subsequent conflicting accesses. */
			void ClearSafeAccess(GpuStageFlags stages, GpuAccessFlags access);

			/** Marks a source access as safe for the specified destination scope. */
			void AddSafeAccess(const GpuHazardStageAndAccess& barrier);

			/** Combines hazards; a destination is safe only if it is safe in both inputs. */
			void Merge(const GpuHazardState& other);

			/** Returns stages from which reads are unsafe. */
			GpuStageFlags GetUnsafeReadStages() const { return ExecutionBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::All); }

			/** Returns stages from which writes are unsafe. */
			GpuStageFlags GetUnsafeWriteStages() const { return MemoryBarrierTracking.GetUnsafeAccessStages(GpuStageFlag::All); }

			/** Returns true if any stages still have unsafe read or write access. */
			bool HasUnsafeStages() const { return GetUnsafeReadStages() != GpuStageFlag::None || GetUnsafeWriteStages() != GpuStageFlag::None; }
		};

		/** Stores unresolved resource hazards grouped by the queue that produced them. */
		class GpuHazardStatesByQueue
		{
		public:
			/** Unresolved hazards produced by a single queue. */
			struct Entry
			{
				GpuQueueId QueueId;
				GpuHazardState Hazards;
			};

			/** Adds hazards for a queue, merging them with an existing entry. Resolved states are ignored. */
			void Add(GpuQueueId queueId, const GpuHazardState& hazards)
			{
				if(!hazards.HasUnsafeStages())
					return;

				for(Entry& entry : mEntries)
				{
					if(entry.QueueId.Id != queueId.Id)
						continue;

					entry.Hazards.Merge(hazards);
					return;
				}

				Entry entry;
				entry.QueueId = queueId;
				entry.Hazards = hazards;
				mEntries.Add(std::move(entry));
			}

			/** Returns unresolved read/write stages accumulated across all queues. */
			GpuAccessScope GetUnsafeAccessScope() const
			{
				GpuAccessScope accesses;
				for(const Entry& entry : mEntries)
				{
					accesses.Add(entry.Hazards.GetUnsafeReadStages(), GpuAccessFlag::Read);
					accesses.Add(entry.Hazards.GetUnsafeWriteStages(), GpuAccessFlag::Write);
				}

				return accesses;
			}

			/** Returns a mask containing every queue with unresolved hazards. */
			GpuQueueMask GetQueueMask() const
			{
				GpuQueueMask mask = GpuQueueMask::kNone;
				for(const Entry& entry : mEntries)
					mask |= entry.QueueId;

				return mask;
			}

			/** Returns all per-queue hazard entries. */
			const TInlineArray<Entry, 2>& GetEntries() const { return mEntries; }

		private:
			TInlineArray<Entry, 2> mEntries;
		};

		/** Tracks resource hazards and executed barriers within a command buffer. */
		struct B3D_EXPORT GpuHazardTracking
		{
			/** Accumulated stages & accesses marked unsafe in-between barriers, and the following barrier that made the accesses safe. */
			struct Epoch
			{
				GpuAccessScope AccessScope;
				GpuHazardStageAndAccess IssuedBarrier;
			};

			/** Holds information about barriers that need to be issued between two command buffers, as well as the hazards that remain. */
			struct CrossCommandBufferRecipe
			{
				GpuHazardStageAndAccess MemoryDependency; /**< Memory dependency required before the destination command buffer begins. */
				GpuHazardStageAndAccess ExecutionDependency; /** Execution dependency required before the destination command buffer begins. */
				GpuHazardState RemainingHazards; /** Remaining hazards that need to be evaluated for future command buffer submissions. */

				/** If the source command buffer has issued an internal barrier we depend on, we need wait on its queue, otherwise we risk running before the barrier is issued. */
				bool RequiresCrossQueueDependency = false;

				/** Returns destination accesses that must be covered by a boundary dependency. */
				GpuAccessScope GetDestinationAccessScope() const
				{
					GpuAccessScope scope;

					if(MemoryDependency.IsValid())
						scope.Add(MemoryDependency.DestinationStages, MemoryDependency.DestinationAccess);

					if(ExecutionDependency.IsValid())
						scope.Add(ExecutionDependency.DestinationStages, ExecutionDependency.DestinationAccess);

					return scope;
				}

				/** Returns true if destination command buffer execution must be ordered after the source submission. */
				bool HasDependency() const
				{
					return MemoryDependency.IsValid() || ExecutionDependency.IsValid() || RequiresCrossQueueDependency;
				}
			};

			GpuAccessFlags Access; /**< Has the resource been read or written so far. */
			GpuHazardState State; /**< Currently active hazards. */

			/** Notifies the tracker that a stage (or stages) is/are being accessed, making certain stage accesses unsafe. */
			void RegisterStageAccess(GpuStageFlags stages, GpuAccessFlags access)
			{
				mCurrentAccessScope.Add(stages, access);
				State.ClearSafeAccess(stages, access);
			}

			/** Notifies the tracker that a barrier was issued, making certain stage accesses safe. */
			void RegisterBarrier(const GpuHazardStageAndAccess& barrier)
			{
				// We accumulate all accesses between barriers as we need to replay them for the cross-command buffer recipe
				Epoch epoch;
				epoch.AccessScope = mCurrentAccessScope;
				epoch.IssuedBarrier = barrier;
				mCompletedEpochs.Add(epoch);

				mCurrentAccessScope = GpuAccessScope();
				State.AddSafeAccess(barrier);
			}

			/** Returns epochs terminated by an executed barrier. */
			const TInlineArray<Epoch, 2>& GetCompletedEpochs() const { return mCompletedEpochs; }

			/** Returns access scope recorded after the most recently registered barrier. */
			const GpuAccessScope& GetCurrentAccessScope() const { return mCurrentAccessScope; }

			/** Returns the conservative union of all committed access scopes. */
			GpuAccessScope GetAccumulatedAccessScope() const
			{
				GpuAccessScope accesses = mCurrentAccessScope;

				for(const Epoch& epoch : mCompletedEpochs)
					accesses.Add(epoch.AccessScope);

				return accesses;
			}

			/** Returns accesses before the first barrier was issued. Used for cross-command buffer barrier execution. */
			GpuAccessScope GetFirstAccessScope() const;

			/** Applies the recorded access/barrier history to hazards inherited from an earlier command buffer. */
			CrossCommandBufferRecipe DetermineCrossCommandBufferRecipe(const GpuHazardState& sourceCommandBufferHazards) const;

#if B3D_VERIFY_BARRIERS
			/** Verifies that the access is safe from the provided stage and access type. Logs errors if not. */
			void VerifySafeAccess(GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess) const;
#endif

			// Note - Add LayoutTransitionTracking? Tracks last use of an imagine in a specific layout, and which layouts can be safely transitioned to/from without a barrier

		private:
			TInlineArray<Epoch, 2> mCompletedEpochs;
			GpuAccessScope mCurrentAccessScope;
		};
	}
}
