//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/SkinCache/B3DSkinCache.h"

#include "Animation/B3DMorphShapes.h"
#include "Animation/B3DSkeleton.h"
#include "FrameGraph/B3DFrameGraph.h"
#include "GpuBackend/B3DGpuBuffer.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuDevice.h"
#include "GpuBackend/B3DGpuDeviceCapabilities.h"
#include "GpuBackend/B3DGpuParameterSet.h"
#include "GpuBackend/B3DGpuPipelineParameterLayout.h"
#include "GpuBackend/B3DGpuWorkContext.h"
#include "GpuBackend/B3DVertexData.h"
#include "GpuBackend/B3DVertexDescription.h"
#include "Mesh/B3DMesh.h"
#include "RayTracing/B3DRayTracingGeometry.h"
#include "Renderer/B3DGpuUniformBuffer.h"
#include "Renderer/B3DRendererMaterial.h"
#include "Animation/SkinCache/B3DMorphShapeBuffers.h"
#include "ThirdParty/CityHash/city.h"
#include "Utility/B3DBitwise.h"

namespace b3d::render
{
	namespace
	{
		B3D_UNIFORM_BUFFER_BEGIN(SkinCacheDeformationUniformDefinition)
			B3D_UNIFORM_BUFFER_MEMBER(u32, gVertexCount)
			B3D_UNIFORM_BUFFER_MEMBER(u32, gPositionStride) // in bytes
			B3D_UNIFORM_BUFFER_MEMBER(u32, gPositionOffset) // in bytes
			B3D_UNIFORM_BUFFER_MEMBER(u32, gTangentFrameStride) // in bytes
			B3D_UNIFORM_BUFFER_MEMBER(u32, gNormalOffset) // in bytes
			B3D_UNIFORM_BUFFER_MEMBER(u32, gTangentOffset) // in bytes
			B3D_UNIFORM_BUFFER_MEMBER(u32, gBlendIndicesOffset) // in bytes
			B3D_UNIFORM_BUFFER_MEMBER(u32, gBlendWeightsOffset) // in bytes
			B3D_UNIFORM_BUFFER_MEMBER(u32, gBoneOffset) // in bytes, in the pose buffer
			B3D_UNIFORM_BUFFER_MEMBER(u32, gMorphShapeWeightOffset) // in bytes, in the pose buffer
		B3D_UNIFORM_BUFFER_END

		SkinCacheDeformationUniformDefinition gSkinCacheDeformationUniformDefinition;

		/** Number of threads in a group of the deformation shader, one per vertex. */
		constexpr u32 kDeformationGroupSize = 64;

		/** Size of a deformed position, in bytes. */
		constexpr u32 kPositionSize = 12;

		/** Size of a deformed normal and tangent, in bytes. */
		constexpr u32 kNormalTangentSize = 8;

		/** Size of the transform of one bone in the pose buffer, in bytes: three rows of four floats. */
		constexpr u32 kBoneTransformSize = 48;

		class SkinCacheDeformationMaterial;

		/** Variation and parameter set of an entry's dispatch, and the number of vertices it deforms. */
		struct SkinCacheDeformationDispatch
		{
			SkinCacheDeformationDispatch(SkinCacheDeformationMaterial* material, const TShared<GpuParameterSet>& set, u32 vertexCount)
				: Material(material), Set(set), VertexCount(vertexCount)
			{ }

			SkinCacheDeformationMaterial* Material = nullptr;
			TShared<GpuParameterSet> Set;
			u32 VertexCount = 0;
		};

		/** Deforms the vertices of skin cache entries, one dispatch per entry. */
		class SkinCacheDeformationMaterial : public RendererMaterial<SkinCacheDeformationMaterial>
		{
			RMAT_DEF("SkinCacheDeformation.bsl")

		public:
			SkinCacheDeformationMaterial() = default;

			void Initialize() override
			{
				mParameterLayout->GetStorageBufferParameter("gSourcePositions", mSourcePositionsParameter);
				mParameterLayout->GetStorageBufferParameter("gSourceTangentFrames", mSourceTangentFramesParameter);
				mParameterLayout->GetStorageBufferParameter("gPoses", mPosesParameter);
				mParameterLayout->GetStorageBufferParameter("gPositions", mPositionsParameter);
				mParameterLayout->GetStorageBufferParameter("gNormalsTangents", mNormalsTangentsParameter);
				mParameterLayout->TryGetUniformBufferParameter("Parameters", mParametersParameter);

				// Only the variation that deforms morph shapes reads them
				mHasMorphShapes = mVariationParameters.GetBool("MORPH");
				if(mHasMorphShapes)
				{
					mParameterLayout->GetStorageBufferParameter("gMorphVertexRanges", mMorphVertexRangesParameter);
					mParameterLayout->GetStorageBufferParameter("gMorphDeltas", mMorphDeltasParameter);
				}
			}

			/**
			 * Creates the parameter set of @p pass, which deforms the vertices in @p sourcePositions and @p sourceTangentFrames by a pose
			 * in @p poses, and by @p morphShapeBuffers in the variation that deforms morph shapes, into @p positions and @p normalsTangents.
			 * Returns the set to pass to Execute().
			 */
			TShared<GpuParameterSet> Prepare(FrameGraphPass& pass, const TShared<GpuBuffer>& sourcePositions, const TShared<GpuBuffer>& sourceTangentFrames,
				const TShared<MorphShapeBuffers>& morphShapeBuffers, const TShared<GpuBuffer>& poses, const TShared<GpuBuffer>& positions,
				const TShared<GpuBuffer>& normalsTangents, const TShared<GpuBuffer>& parameters)
			{
				const TShared<GpuParameterSet> set = CreateGpuParameterSet(pass);
				mSourcePositionsParameter.Set(set, sourcePositions);
				mSourceTangentFramesParameter.Set(set, sourceTangentFrames);
				mPosesParameter.Set(set, poses);
				mPositionsParameter.Set(set, positions);
				mNormalsTangentsParameter.Set(set, normalsTangents);
				mParametersParameter.Set(set, parameters);

				if(mHasMorphShapes)
				{
					mMorphVertexRangesParameter.Set(set, morphShapeBuffers->GetVertexRanges());
					mMorphDeltasParameter.Set(set, morphShapeBuffers->GetDeltas());
				}

				return set;
			}

			/**
			 * Binds the material and the set of each of @p dispatches, from Prepare(), with a dispatch over its vertices. A material is
			 * bound only if the dispatch before used another one, so @p dispatches should be sorted by material.
			 */
			static void Execute(FrameGraphPassContext& context, TArrayView<const SkinCacheDeformationDispatch> dispatches)
			{
				GpuCommandBuffer& commandBuffer = context.GetCommandBuffer();

				const SkinCacheDeformationMaterial* boundMaterial = nullptr;
				for(const SkinCacheDeformationDispatch& dispatch : dispatches)
				{
					if(dispatch.Material != boundMaterial)
					{
						dispatch.Material->Bind(commandBuffer);
						boundMaterial = dispatch.Material;
					}

					commandBuffer.SetGpuParameterSet(dispatch.Set);
					commandBuffer.DispatchCompute(Math::DivideAndRoundUp(dispatch.VertexCount, kDeformationGroupSize));
				}
			}

			/** Returns the variation that applies bone transforms if @p skinned is true, and morph shapes if @p morph is true. */
			static SkinCacheDeformationMaterial* GetVariation(GpuWorkContext& context, bool skinned, bool morph)
			{
				static ShaderVariationParameters variations[2][2] =
				{
					{
						ShaderVariationParameters({ ShaderVariationParameter("SKINNED", false), ShaderVariationParameter("MORPH", false) }),
						ShaderVariationParameters({ ShaderVariationParameter("SKINNED", false), ShaderVariationParameter("MORPH", true) })
					},
					{
						ShaderVariationParameters({ ShaderVariationParameter("SKINNED", true), ShaderVariationParameter("MORPH", false) }),
						ShaderVariationParameters({ ShaderVariationParameter("SKINNED", true), ShaderVariationParameter("MORPH", true) })
					}
				};

				return Get(context, variations[skinned][morph]);
			}

		private:
			GpuParameterStorageBuffer mSourcePositionsParameter;
			GpuParameterStorageBuffer mSourceTangentFramesParameter;
			GpuParameterStorageBuffer mPosesParameter;
			GpuParameterStorageBuffer mPositionsParameter;
			GpuParameterStorageBuffer mNormalsTangentsParameter;
			GpuParameterStorageBuffer mMorphVertexRangesParameter;
			GpuParameterStorageBuffer mMorphDeltasParameter;
			GpuParameterUniformBuffer mParametersParameter;
			bool mHasMorphShapes = false;
		};

		/** Returns the hash of a pose of @p boneTransforms and @p morphShapeWeights. */
		u64 GeneratePoseHash(TArrayView<const Matrix4> boneTransforms, TArrayView<const float> morphShapeWeights)
		{
			const u64 boneHash = CityHash64((const char*)boneTransforms.Data(), boneTransforms.Size() * sizeof(Matrix4));
			return CityHash64WithSeed((const char*)morphShapeWeights.Data(), morphShapeWeights.Size() * sizeof(float), boneHash);
		}
	} // namespace

	SkinCacheEntry::SkinCacheEntry(SkinCache& cache, const SkinCacheEntryCreateInformation& createInformation)
		: mCache(cache), mMesh(createInformation.Mesh), mFlags(createInformation.Flags), mName(createInformation.Name)
	{
		const TShared<VertexData> meshVertexData = mMesh->GetVertexData();
		const VertexDescription& meshDescription = *meshVertexData->VertexDescription;

		const VertexElement* position = meshDescription.FindElement(VES_POSITION);
		const VertexElement* normal = meshDescription.FindElement(VES_NORMAL);

		mVertexCount = meshVertexData->VertexCount;
		mSupportsRayTracingGeometry = mFlags.IsSet(SkinCacheEntryFlag::RayTracingGeometry) && mCache.mDevice.GetCapabilities().HasCapability(RSC_ACCELERATION_STRUCTURES) && mMesh->SupportsRayTracing();
		mMorphShapeCount = mMesh->GetMorphShapeBuffers() != nullptr ? mMesh->GetMorphShapeBuffers()->GetShapeCount() : 0;
		mPositionStreamIndex = position->GetStreamIndex();
		mPositionOffset = position->GetOffset();
		mTangentFrameStreamIndex = normal->GetStreamIndex();
		mNormalOffset = normal->GetOffset();
		mTangentOffset = meshDescription.FindElement(VES_TANGENT)->GetOffset();

		const TShared<Skeleton> skeleton = mMesh->GetSkeleton();
		if(skeleton != nullptr)
		{
			mBoneCount = skeleton->GetBoneCount();
			mBlendIndicesOffset = meshDescription.FindElement(VES_BLEND_INDICES)->GetOffset();
			mBlendWeightsOffset = meshDescription.FindElement(VES_BLEND_WEIGHTS)->GetOffset();
		}

		// The mesh's streams keep their offsets and strides, with the attributes the entry deforms turned into padding, and the entry's
		// buffers follow as streams of their own
		TInlineArray<VertexElement, 8> vertexElements;
		u32 meshStreamCount = 0;
		for(const VertexElement& element : meshDescription.GetElements())
		{
			const bool isDeformed = element.GetSemanticIndex() == 0 && (element.GetSemantic() == VES_POSITION || element.GetSemantic() == VES_NORMAL || element.GetSemantic() == VES_TANGENT);
			const VertexElementSemantic semantic = isDeformed ? VES_PADDING : element.GetSemantic();
			vertexElements.Add(VertexElement(element.GetType(), semantic, element.GetSemanticIndex(), element.GetStreamIndex(), element.GetInstanceStepRate(), element.GetOffset()));

			meshStreamCount = std::max(meshStreamCount, element.GetStreamIndex() + 1u);
		}

		mFirstEntryStreamIndex = meshStreamCount;

		vertexElements.Add(VertexElement(RayTracingGeometry::kPositionType, VES_POSITION, 0, meshStreamCount, 0, 0));
		vertexElements.Add(VertexElement(RayTracingGeometry::kNormalType, VES_NORMAL, 0, meshStreamCount + 1, 0, 0));
		vertexElements.Add(VertexElement(RayTracingGeometry::kTangentType, VES_TANGENT, 0, meshStreamCount + 1, 0, VertexElement::GetSizeForType(RayTracingGeometry::kNormalType)));

		if(mFlags.IsSet(SkinCacheEntryFlag::PreviousPositions))
			vertexElements.Add(VertexElement(RayTracingGeometry::kPositionType, VES_POSITION, 1, meshStreamCount + 2, 0, 0));

		mVertexDescription = B3DMakeShared<VertexDescription>(vertexElements, false);

		// The entry starts in the bind pose, which the next update deforms its vertices to
		const Vector<Matrix4> bindPose(mBoneCount, Matrix4::kIdentity);
		const Vector<float> bindPoseWeights(mMorphShapeCount, 0.0f);
		mPose.resize(mBoneCount * kBoneTransformSize / sizeof(float) + mMorphShapeCount);
		SetPose(bindPose, bindPoseWeights);

		SkinCache::AddToList(mCache.mEntries, *this, &SkinCacheEntry::mEntriesIndex);
	}

	bool SkinCacheEntry::IsSupportedFormat(const render::Mesh& mesh)
	{
		// Normals, tangents and blend data are read from one stream, positions from the same or another one
		const VertexData& vertexData = *mesh.GetVertexData();
		const VertexDescription& description = *vertexData.VertexDescription;
		const VertexElement* position = description.FindElement(VES_POSITION);
		const VertexElement* normal = description.FindElement(VES_NORMAL);
		const VertexElement* tangent = description.FindElement(VES_TANGENT);
		if(position == nullptr || position->GetType() != VET_FLOAT3 || normal == nullptr || normal->GetType() != VET_UBYTE4_NORM || tangent == nullptr ||
			tangent->GetType() != VET_UBYTE4_NORM)
		{
			return false;
		}

		const u32 tangentFrameStreamIndex = normal->GetStreamIndex();
		if(tangent->GetStreamIndex() != tangentFrameStreamIndex || !vertexData.IsBufferBound(position->GetStreamIndex()) || !vertexData.IsBufferBound(tangentFrameStreamIndex))
			return false;

		// Only skinning reads the blend data
		if(mesh.GetSkeleton() == nullptr)
			return true;

		const VertexElement* blendIndices = description.FindElement(VES_BLEND_INDICES);
		const VertexElement* blendWeights = description.FindElement(VES_BLEND_WEIGHTS);
		return blendIndices != nullptr && blendIndices->GetType() == VET_UBYTE4 && blendIndices->GetStreamIndex() == tangentFrameStreamIndex &&
			blendWeights != nullptr && blendWeights->GetType() == VET_FLOAT4 && blendWeights->GetStreamIndex() == tangentFrameStreamIndex;
	}

	SkinCacheEntry::~SkinCacheEntry()
	{
		SkinCache::RemoveFromList(mCache.mEntries, *this, &SkinCacheEntry::mEntriesIndex);
		SkinCache::RemoveFromList(mCache.mEntriesQueuedForUpdate, *this, &SkinCacheEntry::mEntriesQueuedForUpdateIndex);
		SkinCache::RemoveFromList(mCache.mEntriesMovedByLastUpdate, *this, &SkinCacheEntry::mEntriesMovedByLastUpdateIndex);
	}

	void SkinCacheEntry::SetPose(TArrayView<const Matrix4> boneTransforms, TArrayView<const float> morphShapeWeights)
	{
		if(!B3D_ENSURE_LOG(boneTransforms.Size() == mBoneCount && morphShapeWeights.Size() == mMorphShapeCount,
			"Skin cache entry '{0}' needs a pose of {1} bone transforms and {2} morph shape weights, not {3} and {4}.", mName, mBoneCount, mMorphShapeCount,
			boneTransforms.Size(), morphShapeWeights.Size()))
		{
			return;
		}

		const u64 poseHash = GeneratePoseHash(boneTransforms, morphShapeWeights);
		// Until the first deformation every pose is taken, so the entry gets vertices even in the pose it was created in
		if(mHasDeformedVertices && poseHash == mPoseHash)
			return;

		float* poseValue = mPose.data();
		for(const Matrix4& boneTransform : boneTransforms)
		{
			// The deformation only reads the three rows of the affine transform
			std::memcpy(poseValue, &boneTransform, kBoneTransformSize);
			poseValue += kBoneTransformSize / sizeof(float);
		}

		if(!morphShapeWeights.IsEmpty())
			std::memcpy(poseValue, morphShapeWeights.Data(), morphShapeWeights.Size() * sizeof(float));

		mPoseHash = poseHash;
		if(mEntriesQueuedForUpdateIndex == kInvalidIndex)
			SkinCache::AddToList(mCache.mEntriesQueuedForUpdate, *this, &SkinCacheEntry::mEntriesQueuedForUpdateIndex);
	}

	TShared<VertexData> SkinCacheEntry::CreateVertexData(u32 positionBufferIndex, u32 previousPositionBufferIndex) const
	{
		const TShared<VertexData> vertexData = B3DMakeShared<VertexData>();
		vertexData->VertexDescription = mVertexDescription;
		vertexData->VertexCount = mVertexCount;

		for(const auto& meshBuffer : mMesh->GetVertexData()->GetBuffers())
			vertexData->SetBuffer(meshBuffer.first, meshBuffer.second);

		vertexData->SetBuffer(mFirstEntryStreamIndex, mPositionBuffers[positionBufferIndex]);
		vertexData->SetBuffer(mFirstEntryStreamIndex + 1, mNormalTangentBuffer);

		if(mFlags.IsSet(SkinCacheEntryFlag::PreviousPositions))
			vertexData->SetBuffer(mFirstEntryStreamIndex + 2, mPositionBuffers[previousPositionBufferIndex]);

		return vertexData;
	}

	u32 SkinCacheEntry::GetMemorySize() const
	{
		u32 memorySize = 0;
		for(const TShared<GpuBuffer>& buffer : { mPositionBuffers[0], mPositionBuffers[1], mNormalTangentBuffer })
		{
			if(buffer != nullptr)
				memorySize += buffer->GetTotalSize();
		}

		if(mRayTracingGeometry != nullptr)
			memorySize += mRayTracingGeometry->GetAccelerationStructure()->GetSize();

		return memorySize;
	}

	SkinCache::SkinCache(GpuDevice& device, const StringView& name)
		: mDevice(device), mName(name)
	{ }

	SkinCache::~SkinCache()
	{
		B3D_ENSURE_LOG(mEntries.empty(), "Skin cache '{0}' is destroyed before its {1} entries.", mName, (u32)mEntries.size());
	}

	void SkinCache::AddToList(Vector<SkinCacheEntry*>& list, SkinCacheEntry& entry, u32 SkinCacheEntry::* listIndex)
	{
		entry.*listIndex = (u32)list.size();
		list.push_back(&entry);
	}

	void SkinCache::RemoveFromList(Vector<SkinCacheEntry*>& list, SkinCacheEntry& entry, u32 SkinCacheEntry::* listIndex)
	{
		const u32 index = entry.*listIndex;
		if(index == SkinCacheEntry::kInvalidIndex)
			return;

		SkinCacheEntry* const lastEntry = list.back();
		list[index] = lastEntry;
		lastEntry->*listIndex = index;

		list.pop_back();
		entry.*listIndex = SkinCacheEntry::kInvalidIndex;
	}

	TShared<SkinCacheEntry> SkinCache::CreateEntry(const SkinCacheEntryCreateInformation& createInformation)
	{
		const TShared<render::Mesh>& mesh = createInformation.Mesh;
		if(mesh == nullptr)
			return nullptr;

		const TShared<MorphShapes> morphShapes = mesh->GetMorphShapes();
		const u32 morphShapeCount = morphShapes != nullptr ? morphShapes->GetShapeCount() : 0;
		if(mesh->GetSkeleton() == nullptr && morphShapeCount == 0)
			return nullptr;

		if(!SkinCacheEntry::IsSupportedFormat(*mesh))
		{
			B3D_LOG(Warning, LogRenderer, "Skin cache '{0}' can't deform mesh '{1}': its vertices need positions of three floats, and normals, tangents and, if it has a "
				"skeleton, blend indices and blend weights in the formats of RendererMeshData::VertexLayoutVertexDesc(), in one stream.", mName, createInformation.Name);
			return nullptr;
		}

		if(morphShapeCount > 0 && mesh->GetMorphShapeBuffers() == nullptr)
		{
			B3D_LOG(Error, LogRenderer, "Skin cache '{0}' can't deform mesh '{1}': the mesh has no morph shape buffers.", mName, createInformation.Name);
			return nullptr;
		}

		return B3DMakeSharedFromExisting(new(B3DAllocate<SkinCacheEntry>()) SkinCacheEntry(*this, createInformation));
	}

	bool SkinCache::Update(FrameGraph& graph)
	{
		// Vertices the last update moved don't move further unless this update deforms them again
		for(SkinCacheEntry* entry : mEntriesMovedByLastUpdate)
			entry->mEntriesMovedByLastUpdateIndex = SkinCacheEntry::kInvalidIndex;

		mEntriesMovedByLastUpdate.clear();
		if(mEntriesQueuedForUpdate.empty())
			return false;

		GpuWorkContext& context = graph.GetWorkContext();
		const bool hasBindlessHeaps = mDevice.GetBindlessHeaps() != nullptr;

		// Each deformation writes the position buffer that doesn't hold the current positions, if the entry keeps the previous ones.
		// Buffers are created on first use. Entries whose buffers can't be created stay queued for the next update.
		FrameAllocatorScope frameScope;
		FrameVector<SkinCacheEntry*> entriesToDeform;
		u32 stillQueuedEntryCount = 0;
		for(SkinCacheEntry* entry : mEntriesQueuedForUpdate)
		{
			const bool keepsPreviousPositions = entry->mFlags.IsSet(SkinCacheEntryFlag::PreviousPositions);
			const u32 targetIndex = keepsPreviousPositions && entry->mHasDeformedVertices ? 1 - entry->mCurrentPositionBufferIndex : 0;

			// Ray tracing builds the structure from the positions, and hit shaders read every buffer through the bindless heap
			GpuBufferFlags normalTangentFlags = GpuBufferFlag::GpuWrite;
			GpuBufferFlags positionFlags = GpuBufferFlag::GpuWrite;
			if(entry->mSupportsRayTracingGeometry)
			{
				positionFlags |= GpuBufferFlag::AccelerationStructureInput;
				if(hasBindlessHeaps)
				{
					normalTangentFlags |= GpuBufferFlag::Bindless;
					positionFlags |= GpuBufferFlag::Bindless;
				}
			}

			if(entry->mNormalTangentBuffer == nullptr)
			{
				entry->mNormalTangentBuffer = mDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(kNormalTangentSize, entry->mVertexCount, normalTangentFlags));
				if(entry->mNormalTangentBuffer != nullptr)
					entry->mNormalTangentBuffer->SetName(entry->mName + " normals and tangents");
			}

			if(entry->mPositionBuffers[targetIndex] == nullptr)
			{
				entry->mPositionBuffers[targetIndex] = mDevice.CreateGpuBuffer(GpuBufferCreateInformation::CreateVertex(kPositionSize, entry->mVertexCount, positionFlags));
				if(entry->mPositionBuffers[targetIndex] != nullptr)
					entry->mPositionBuffers[targetIndex]->SetName(entry->mName + " positions");
			}

			if(entry->mNormalTangentBuffer == nullptr || entry->mPositionBuffers[targetIndex] == nullptr)
			{
				B3D_LOG(Error, LogRenderer, "Skin cache '{0}' couldn't create the buffers of entry '{1}'.", mName, entry->mName);
				entry->mEntriesQueuedForUpdateIndex = stillQueuedEntryCount;
				mEntriesQueuedForUpdate[stillQueuedEntryCount++] = entry;
				continue;
			}

			entry->mEntriesQueuedForUpdateIndex = SkinCacheEntry::kInvalidIndex;
			entriesToDeform.push_back(entry);
		}

		mEntriesQueuedForUpdate.resize(stillQueuedEntryCount);
		if(entriesToDeform.empty())
			return false;

		// The poses of all entries go into one buffer, each at its own offset
		FrameVector<u32> poseOffsets;
		u32 poseBufferSize = 0;
		for(const SkinCacheEntry* entry : entriesToDeform)
		{
			poseOffsets.push_back(poseBufferSize);
			poseBufferSize += Bitwise::AlignUp((u32)(entry->mPose.size() * sizeof(float)), 16u);
		}

		const TShared<GpuBuffer> poseBuffer = context.CreateScratchGpuBuffer(GpuBufferCreateInformation::CreateStructuredStorage(sizeof(float),
			std::max(1u, poseBufferSize / (u32)sizeof(float)), GpuBufferFlag::None, GpuBufferMemory::Mapped));
		// If the pose buffer can't be written, the entries stay queued for the next update
		const auto fnRequeueEntriesToDeform = [this, &entriesToDeform]()
		{
			for(SkinCacheEntry* entry : entriesToDeform)
				AddToList(mEntriesQueuedForUpdate, *entry, &SkinCacheEntry::mEntriesQueuedForUpdateIndex);
		};

		if(!B3D_ENSURE_LOG(poseBuffer != nullptr, "Skin cache '{0}' couldn't create its pose buffer.", mName))
		{
			fnRequeueEntriesToDeform();
			return false;
		}

		poseBuffer->SetName("Skin cache poses");
		{
			const GpuBufferMappedScope mapping = poseBuffer->Map(GpuMapOption::Write);
			if(!B3D_ENSURE_LOG(mapping.IsValid(), "Skin cache '{0}' couldn't map its pose buffer.", mName))
			{
				fnRequeueEntriesToDeform();
				return false;
			}

			u8* const poses = static_cast<u8*>(mapping.GetMappedMemory());
			for(u32 entryIndex = 0; entryIndex < (u32)entriesToDeform.size(); entryIndex++)
			{
				const Vector<float>& pose = entriesToDeform[entryIndex]->mPose;
				std::memcpy(poses + poseOffsets[entryIndex], pose.data(), pose.size() * sizeof(float));
			}
		}

		FrameGraphComputePass pass;
		Vector<SkinCacheDeformationDispatch> dispatches;
		dispatches.reserve(entriesToDeform.size());
		FrameVector<RayTracingGeometry*> geometriesToBuild;
		FrameGraphAccelerationStructurePass geometryPass;
		for(u32 entryIndex = 0; entryIndex < (u32)entriesToDeform.size(); entryIndex++)
		{
			SkinCacheEntry& entry = *entriesToDeform[entryIndex];
			const bool keepsPreviousPositions = entry.mFlags.IsSet(SkinCacheEntryFlag::PreviousPositions);
			const u32 targetIndex = keepsPreviousPositions && entry.mHasDeformedVertices ? 1 - entry.mCurrentPositionBufferIndex : 0;
			const TShared<GpuBuffer>& positionBuffer = entry.mPositionBuffers[targetIndex];

			const TShared<VertexData> meshVertexData = entry.mMesh->GetVertexData();
			const VertexDescription& meshDescription = *meshVertexData->VertexDescription;

			SkinCacheDeformationMaterial* const material = SkinCacheDeformationMaterial::GetVariation(context, entry.mBoneCount > 0, entry.mMorphShapeCount > 0);
			if(!B3D_ENSURE_LOG(material != nullptr, "Skin cache '{0}' can't deform entry '{1}', its deformation shader isn't available.", mName, entry.mName))
			{
				AddToList(mEntriesQueuedForUpdate, entry, &SkinCacheEntry::mEntriesQueuedForUpdateIndex);
				continue;
			}

			const TShared<GpuBuffer> parameterBuffer = gSkinCacheDeformationUniformDefinition.CreateScratchBuffer(context);
			if(!B3D_ENSURE_LOG(parameterBuffer != nullptr, "Skin cache '{0}' couldn't create the parameters of entry '{1}'.", mName, entry.mName))
			{
				AddToList(mEntriesQueuedForUpdate, entry, &SkinCacheEntry::mEntriesQueuedForUpdateIndex);
				continue;
			}

			{
				const GpuBufferMappedScope parameters = parameterBuffer->Map(GpuMapOption::Write);
				gSkinCacheDeformationUniformDefinition.gVertexCount.Set(parameters, entry.mVertexCount);
				gSkinCacheDeformationUniformDefinition.gPositionStride.Set(parameters, meshDescription.GetVertexStride(entry.mPositionStreamIndex));
				gSkinCacheDeformationUniformDefinition.gPositionOffset.Set(parameters, entry.mPositionOffset);
				gSkinCacheDeformationUniformDefinition.gTangentFrameStride.Set(parameters, meshDescription.GetVertexStride(entry.mTangentFrameStreamIndex));
				gSkinCacheDeformationUniformDefinition.gNormalOffset.Set(parameters, entry.mNormalOffset);
				gSkinCacheDeformationUniformDefinition.gTangentOffset.Set(parameters, entry.mTangentOffset);
				gSkinCacheDeformationUniformDefinition.gBlendIndicesOffset.Set(parameters, entry.mBlendIndicesOffset);
				gSkinCacheDeformationUniformDefinition.gBlendWeightsOffset.Set(parameters, entry.mBlendWeightsOffset);
				gSkinCacheDeformationUniformDefinition.gBoneOffset.Set(parameters, poseOffsets[entryIndex]);
				gSkinCacheDeformationUniformDefinition.gMorphShapeWeightOffset.Set(parameters, poseOffsets[entryIndex] + entry.mBoneCount * kBoneTransformSize);
			}

			// The buffers are written by the pass and returned to rest right after it, for every later reader
			graph.ImportBuffer(positionBuffer);
			graph.ImportBuffer(entry.mNormalTangentBuffer);

			const TShared<GpuParameterSet> set = material->Prepare(pass, meshVertexData->GetBuffer(entry.mPositionStreamIndex), meshVertexData->GetBuffer(entry.mTangentFrameStreamIndex),
				entry.mMesh->GetMorphShapeBuffers(), poseBuffer, positionBuffer, entry.mNormalTangentBuffer, parameterBuffer);
			dispatches.push_back(SkinCacheDeformationDispatch(material, set, entry.mVertexCount));

			// The positions of an entry with ray tracing geometry are exported after the build that reads them
			if(!entry.mSupportsRayTracingGeometry)
				pass.Export(positionBuffer);

			pass.Export(entry.mNormalTangentBuffer);

			// The vertices move if the entry keeps the positions they had before. The vertex data without motion is created too, for the
			// updates after which they don't move.
			const bool hasMotion = keepsPreviousPositions && entry.mHasDeformedVertices;
			if(hasMotion && entry.mVertexDataWithMotion[targetIndex] == nullptr)
				entry.mVertexDataWithMotion[targetIndex] = entry.CreateVertexData(targetIndex, 1 - targetIndex);

			if(entry.mVertexDataWithoutMotion[targetIndex] == nullptr)
				entry.mVertexDataWithoutMotion[targetIndex] = entry.CreateVertexData(targetIndex, targetIndex);

			entry.mCurrentPositionBufferIndex = targetIndex;
			entry.mHasDeformedVertices = true;

			if(hasMotion)
				AddToList(mEntriesMovedByLastUpdate, entry, &SkinCacheEntry::mEntriesMovedByLastUpdateIndex);

			// The structure is built from the position buffer just written. An entry that keeps previous positions writes the other
			// buffer each time, so its geometry is pointed at it.
			if(entry.mSupportsRayTracingGeometry)
			{
				const VertexData& vertexData = *entry.mVertexDataWithoutMotion[targetIndex];
				if(entry.mRayTracingGeometry == nullptr)
				{
					entry.mRayTracingGeometry = RayTracingGeometry::Create(mDevice, vertexData, entry.mMesh->GetIndexBuffer(), entry.mMesh->GetProperties().SubMeshes,
						RayTracingGeometryFlag::Dynamic);
				}
				else if(keepsPreviousPositions)
					entry.mRayTracingGeometry->SetVertexData(vertexData);

				if(entry.mRayTracingGeometry != nullptr)
				{
					geometriesToBuild.push_back(entry.mRayTracingGeometry.get());
					geometryPass.Export(positionBuffer);
				}
				else
				{
					// The positions are then exported by the deformation pass, as for an entry without the flag
					B3D_LOG(Warning, LogRenderer, "Skin cache '{0}' couldn't create the ray tracing geometry of entry '{1}'. The entry won't be ray traced.", mName, entry.mName);
					entry.mSupportsRayTracingGeometry = false;
					pass.Export(positionBuffer);
				}
			}
		}

		if(dispatches.empty())
			return false;

		// Dispatches of one variation follow each other, so each variation is bound once
		std::stable_sort(dispatches.begin(), dispatches.end(), [](const SkinCacheDeformationDispatch& left, const SkinCacheDeformationDispatch& right)
		{
			return std::less<const SkinCacheDeformationMaterial*>()(left.Material, right.Material);
		});

		graph.AddComputePass("Skin cache deformation", std::move(pass), [dispatches = std::move(dispatches)](FrameGraphPassContext& passContext)
		{
			SkinCacheDeformationMaterial::Execute(passContext, dispatches);
		});

		// The structures of all entries are built in one pass, after the deformation that wrote their positions, which the pass exports
		RayTracingGeometry::AddBuildPass(graph, "Skin cache geometry", geometriesToBuild, std::move(geometryPass));

		return true;
	}

	u64 SkinCache::GetMemorySize() const
	{
		u64 memorySize = 0;
		for(const SkinCacheEntry* entry : mEntries)
			memorySize += entry->GetMemorySize();

		return memorySize;
	}
} // namespace b3d::render
