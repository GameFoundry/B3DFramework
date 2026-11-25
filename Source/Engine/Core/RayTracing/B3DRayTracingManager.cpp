//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RayTracing/B3DRayTracingManager.h"

#include "CoreObject/B3DRenderThread.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuAccelerationStructure.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "RayTracing/B3DRayTracingGeometry.h"

namespace b3d
{
	TConfigVariable<bool> gRenderRayTracingCompaction("render.RayTracingCompaction",
		"If enabled, the structures of static ray tracing geometry are compacted once they are built, so they use less memory. Applies to geometry created after the change.", true);
} // namespace b3d

namespace b3d::render
{
	const TShared<GpuBuffer>& RayTracingManager::GetDefaultAttributes(GpuDevice& device)
	{
		ASSERT_IF_NOT_RENDER_THREAD;

		TShared<GpuBuffer>& defaultAttributes = mDefaultAttributes[&device];
		if(defaultAttributes != nullptr || device.GetBindlessHeaps() == nullptr)
			return defaultAttributes;

		// TODO: Device memory might be a better choice, but probably not worth the extra complexity
		const TShared<GpuBuffer> buffer = device.CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(sizeof(RayTracingDefaultAttributes), 1,
			GpuBufferFlag::Bindless, GpuBufferMemory::Mapped));
		if(buffer == nullptr)
			return defaultAttributes;

		buffer->SetName("Ray tracing default attributes");

		{
			const GpuBufferMappedScope mapping = buffer->Map(GpuMapOption::Write);
			if(!B3D_ENSURE_LOG(mapping.IsValid(), "Couldn't map the default attributes of ray tracing geometry."))
				return defaultAttributes;

			const RayTracingDefaultAttributes values;
			std::memcpy(mapping.GetMappedMemory(), &values, sizeof(values));
		}

		defaultAttributes = buffer;
		return defaultAttributes;
	}

	void RayTracingManager::Update(FrameGraph& graph)
	{
		ASSERT_IF_NOT_RENDER_THREAD;

		GpuDevice& device = graph.GetWorkContext().GetDevice();

		// Results of geometries that were destroyed, rebuilt or written since their query, or that compaction wouldn't shrink, are dropped
		FrameAllocatorScope frameScope;
		FrameVector<std::pair<TShared<RayTracingGeometry>, u32>> geometries;
		for(auto batchIterator = mCompactedSizeQueryBatches.begin(); batchIterator != mCompactedSizeQueryBatches.end();)
		{
			CompactedSizeQueryBatch& batch = **batchIterator;
			if(batch.Device != &device || !batch.IsRecorded.load(std::memory_order_acquire))
			{
				++batchIterator;
				continue;
			}

			// A batch whose queries failed to record has no queries, and its pool would never resolve
			if(!batch.Queries.empty())
			{
				if(!batch.QueryPool->TryResolve(false))
				{
					++batchIterator;
					continue;
				}

				for(const CompactedSizeQuery& query : batch.Queries)
				{
					const TShared<RayTracingGeometry> geometry = query.Geometry.lock();
					if(geometry == nullptr || geometry->GetVersion() != query.Version || !geometry->IsBuilt() || !gRenderRayTracingCompaction)
						continue;

					const u64 compactedSize = batch.QueryPool->GetQueryResult(query.Query);
					if(compactedSize > 0 && compactedSize < geometry->GetAccelerationStructure()->GetSize())
						geometries.push_back(std::make_pair(geometry, (u32)compactedSize));
				}
			}

			// Later batches reset the pool before they use it
			mFreeCompactedSizeQueryPools[&device].push_back(batch.QueryPool);
			batchIterator = mCompactedSizeQueryBatches.erase(batchIterator);
		}

		if(geometries.empty())
			return;

		FrameGraphAccelerationStructurePass pass;
		Vector<GpuAccelerationStructureCopy> copies;
		u64 originalSize = 0;
		u64 compactedSize = 0;
		for(const auto& [geometry, size] : geometries)
		{
			const TShared<GpuAccelerationStructure> source = geometry->GetAccelerationStructure();
			const TShared<GpuAccelerationStructure> compacted = device.CreateGpuAccelerationStructure(GpuAccelerationStructureCreateInformation(
				GpuAccelerationStructureType::BottomLevel, size, source->GetFlags(), source->GetName()));
			if(compacted == nullptr)
				continue;

			graph.ImportAccelerationStructure(source);
			graph.ImportAccelerationStructure(compacted);

			const GpuAccelerationStructureCopy copy(source, compacted, GpuAccelerationStructureCopyMode::Compact);
			pass.AddCopy(copy);
			pass.Export(compacted);
			copies.push_back(copy);

			originalSize += source->GetSize();
			compactedSize += size;

			// The copy holds the source until the pass executes
			geometry->SetAccelerationStructure(compacted);
		}

		if(copies.empty())
			return;

		B3D_LOG(Verbose, LogRenderer, "Compacting {0} ray tracing geometries from {1} to {2} bytes.", copies.size(), originalSize, compactedSize);

		graph.AddAccelerationStructurePass("Compact ray tracing geometries", std::move(pass), [copies = std::move(copies)](FrameGraphPassContext& passContext)
		{
			passContext.GetCommandBuffer().CopyAccelerationStructures(copies);
		});
	}

	void RayTracingManager::RequestCompaction(FrameGraph& graph, TArrayView<RayTracingGeometry* const> geometries)
	{
		ASSERT_IF_NOT_RENDER_THREAD;

		// Dynamic geometry is built without compaction, and a geometry whose build couldn't be added isn't built
		FrameAllocatorScope frameScope;
		FrameVector<RayTracingGeometry*> compactableGeometries;
		for(RayTracingGeometry* geometry : geometries)
		{
			const RayTracingGeometryFlags flags = geometry->GetFlags();
			if(flags.IsSet(RayTracingGeometryFlag::AllowCompaction) && !flags.IsSet(RayTracingGeometryFlag::Dynamic) && geometry->IsBuilt())
				compactableGeometries.push_back(geometry);
		}

		if(compactableGeometries.empty())
			return;

		GpuDevice& device = graph.GetWorkContext().GetDevice();
		Vector<TShared<GpuQueryPool>>& freeQueryPools = mFreeCompactedSizeQueryPools[&device];

		// Each batch fills one pool, of those Update() freed if there are any
		FrameGraphAccelerationStructurePass pass;
		Vector<TShared<CompactedSizeQueryBatch>> batches;
		Vector<TShared<GpuAccelerationStructure>> structures;
		structures.reserve(compactableGeometries.size());
		for(RayTracingGeometry* geometry : compactableGeometries)
		{
			if(batches.empty() || batches.back()->Queries.size() == kCompactedSizeQueryPoolSize)
			{
				TShared<GpuQueryPool> queryPool;
				if(!freeQueryPools.empty())
				{
					queryPool = freeQueryPools.back();
					freeQueryPools.pop_back();
				}
				else
				{
					GpuQueryPoolCreateInformation queryPoolInformation;
					queryPoolInformation.Type = GpuQueryType::AccelerationStructureCompactedSize;
					queryPoolInformation.PoolSize = kCompactedSizeQueryPoolSize;

					queryPool = device.CreateQueryPool(queryPoolInformation);
					if(queryPool == nullptr)
						break;
				}

				const TShared<CompactedSizeQueryBatch> batch = B3DMakeShared<CompactedSizeQueryBatch>();
				batch->Device = &device;
				batch->QueryPool = std::move(queryPool);
				batches.push_back(batch);
			}

			batches.back()->Queries.emplace_back(geometry->weak_from_this(), geometry->GetVersion());
			structures.push_back(geometry->GetAccelerationStructure());
			pass.AddQuery(geometry->GetAccelerationStructure());
		}

		if(batches.empty())
			return;

		mCompactedSizeQueryBatches.insert(mCompactedSizeQueryBatches.end(), batches.begin(), batches.end());

		graph.AddAccelerationStructurePass("Query ray tracing geometry sizes", std::move(pass), [batches = std::move(batches), structures = std::move(structures)](FrameGraphPassContext& passContext)
		{
			GpuCommandBuffer& commandBuffer = passContext.GetCommandBuffer();

			// The structures are in the order of the batches' queries
			u32 structureIndex = 0;
			for(const TShared<CompactedSizeQueryBatch>& batch : batches)
			{
				commandBuffer.ResetQueries(batch->QueryPool);

				// Resetting the pool restarts its allocation, so the queries are allocated as they are recorded
				Vector<GpuQueryId> queries;
				queries.reserve(batch->Queries.size());
				for(CompactedSizeQuery& query : batch->Queries)
				{
					query.Query = batch->QueryPool->AllocateQuery();
					queries.push_back(query.Query);
				}

				const TArrayView<const TShared<GpuAccelerationStructure>> batchStructures(structures.data() + structureIndex, queries.size());
				structureIndex += (u32)queries.size();

				if(!commandBuffer.WriteAccelerationStructureQueries(batchStructures, queries, batch->QueryPool))
					batch->Queries.clear();

				batch->IsRecorded.store(true, std::memory_order_release);
			}
		});
	}

	void RayTracingManager::OnShutDown()
	{
		// The buffers and query pools are only used on the render thread, so they are released there
		GetRenderThread().PostCommand([this]()
		{
			mDefaultAttributes.clear();
			mCompactedSizeQueryBatches.clear();
			mFreeCompactedSizeQueryPools.clear();
		}, "Release ray tracing resources", true);
	}
} // namespace b3d::render
