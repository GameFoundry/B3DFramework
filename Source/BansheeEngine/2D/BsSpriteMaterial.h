//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "Material/BsMaterialParam.h"
#include "Math/BsVector2I.h"
#include "Image/BsColor.h"

namespace bs
{
	/** @addtogroup 2D-Internal
	 *  @{
	 */

	/** Extension structure that can be used by SpriteMaterial%s to access specialized data. */
	struct SpriteMaterialExtraInfo
	{
		virtual ~SpriteMaterialExtraInfo() { }

		/** Creates a new deep copy of the object. */
		virtual SPtr<SpriteMaterialExtraInfo> clone() const
		{
			return bs_shared_ptr_new<SpriteMaterialExtraInfo>();
		}
	};

	/** Contains information for initializing a sprite material. */
	struct SpriteMaterialInfo
	{
		SpriteMaterialInfo()
			:groupId(0)
		{ }

		/** 
		 * Creates a new deep copy of the object. This is different from standard copy constructor which will just reference
		 * the original "additionalData" field, while this will copy it.
		 */
		SpriteMaterialInfo clone() const
		{
			SpriteMaterialInfo info;
			info.groupId = groupId;
			info.texture = texture;
			info.tint = tint;

			if(additionalData != nullptr)
				info.additionalData = additionalData->clone();

			return info;
		}

		UINT64 groupId;
		HTexture texture;
		Color tint;
		SPtr<SpriteMaterialExtraInfo> additionalData;
	};

	/** Interfaced implemented by materials used for rendering sprites. This is expected to be used as a singleton. */
	class BS_EXPORT SpriteMaterial
	{
	public:
		SpriteMaterial(UINT32 id, const HMaterial& material);
		virtual ~SpriteMaterial();

		/** Returns the unique ID of the sprite material. */
		UINT32 getId() const { return mId; };

		/** 
		 * Generates a hash value that describes the contents of the sprite material info structure. Returned hash doesn't
		 * guarantee that the two objects with the same hash are idential, but rather that the objects are mergeable via
		 * merge().
		 */
		virtual UINT64 getMergeHash(const SpriteMaterialInfo& info) const;

		/** 
		 * Merges two SpriteMaterialInfo%s into one structure. User must guarantee that the two objects are mergeable
		 * by ensuring their merge hashes match (by calling getMergeHash()).
		 *
		 * @param[in, out]	mergeInto	Object that contains the first part of the data, and will contain the result of the
		 *								merge.
		 * @param[in]		mergeFrom	Object that contains the second part of the data to merge, which will be merged into
		 *								the first object.
		 */
		virtual void merge(SpriteMaterialInfo& mergeInto, const SpriteMaterialInfo& mergeFrom) const { }

		/**
		 * Renders the provided mesh using the current material.
		 *
		 * @param[in]	mesh			Mesh to render, containing vertices in screen space.
		 * @param[in]	subMesh			Portion of @p mesh to render.
		 * @param[in]	texture			Optional texture to render the mesh with.
		 * @param[in]	sampler			Optional sampler to render the texture with.
		 * @param[in]	paramBuffer		Buffer containing data GPU parameters.
		 * @param[in]	additionalData	Optional additional data that might be required by the renderer.
		 */
		virtual void render(const SPtr<ct::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<ct::Texture>& texture,
			const SPtr<ct::SamplerState>& sampler, const SPtr<ct::GpuParamBlockBuffer>& paramBuffer,
			const SPtr<SpriteMaterialExtraInfo>& additionalData) const;

	protected:
		/** Perform initialization of core-thread specific objects. */
		virtual void initialize();

		/** Destroys the core thread material. */
		static void destroy(const SPtr<ct::Material>& material, const SPtr<ct::GpuParamsSet>& params);

		UINT32 mId;

		// Core thread only (everything below)
		SPtr<ct::Material> mMaterial;
		std::atomic<bool> mMaterialStored;

		SPtr<ct::GpuParamsSet> mParams;
		UINT32 mParamBufferIdx;
		mutable ct::MaterialParamTexture mTextureParam;
		mutable ct::MaterialParamSampState mSamplerParam;
	};

	/** @} */
}

