//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Material/BsMaterialParam.h"
#include "Math/BsVector2I.h"
#include "Image/BsColor.h"
#include "Material/BsShaderVariation.h"

namespace bs
{
	/** @addtogroup 2D-Internal
	 *  @{
	 */

	/** Type of transparency supported by a sprite material. */
	enum class SpriteMaterialTransparency
	{
		/** No transparency supported. */
		Opaque = 0,

		/** Transparency is deduced from the alpha value of the sprite color. */
		Alpha = 1,

		/** Same as Alpha, except it is assumed the sprite color has been premultiplied by the alpha value. */
		Premultiplied = 2
	};

	/** Extension structure that can be used by SpriteMaterial%s to access specialized data. */
	struct SpriteMaterialExtraInfo
	{
		virtual ~SpriteMaterialExtraInfo() = default;

		/** Creates a new deep copy of the object. */
		virtual SPtr<SpriteMaterialExtraInfo> Clone() const
		{
			return B3DMakeShared<SpriteMaterialExtraInfo>();
		}
	};

	/** Contains information for initializing a sprite material. */
	struct SpriteMaterialInfo
	{
		SpriteMaterialInfo() = default;

		/**
		 * Creates a new deep copy of the object. This is different from standard copy constructor which will just reference
		 * the original "additionalData" field, while this will copy it.
		 */
		SpriteMaterialInfo Clone() const
		{
			SpriteMaterialInfo info;
			info.GroupId = GroupId;
			info.Texture = Texture;
			info.SpriteTexture = SpriteTexture;
			info.Tint = Tint;
			info.AnimationStartTime = AnimationStartTime;

			if(AdditionalData != nullptr)
				info.AdditionalData = AdditionalData->Clone();

			return info;
		}

		u64 GroupId = 0;
		HTexture Texture;
		HSpriteTexture SpriteTexture;
		Color Tint;
		float AnimationStartTime = 0.0f;
		SPtr<SpriteMaterialExtraInfo> AdditionalData;
	};

	/** Interfaced implemented by materials used for rendering sprites. This is expected to be used as a singleton. */
	class BS_EXPORT SpriteMaterial
	{
	public:
		SpriteMaterial(u32 id, const HMaterial& material, ShaderVariation variation = ShaderVariation::kEmpty, bool allowBatching = true);
		virtual ~SpriteMaterial();

		/** Returns the unique ID of the sprite material. */
		u32 GetId() const { return mId; };

		/** Determines is this material allowed to be batched with other materials with the same merge hash. */
		bool AllowBatching() const { return mAllowBatching; }

		/**
		 * Generates a hash value that describes the contents of the sprite material info structure. Returned hash doesn't
		 * guarantee that the two objects with the same hash are identical, but rather that the objects are mergeable via
		 * merge().
		 */
		virtual u64 GetMergeHash(const SpriteMaterialInfo& info) const;

		/**
		 * Merges two SpriteMaterialInfo%s into one structure. User must guarantee that the two objects are mergeable
		 * by ensuring their merge hashes match (by calling getMergeHash()).
		 *
		 * @param[in, out]	mergeInto	Object that contains the first part of the data, and will contain the result of the
		 *								merge.
		 * @param[in]		mergeFrom	Object that contains the second part of the data to merge, which will be merged into
		 *								the first object.
		 */
		virtual void Merge(SpriteMaterialInfo& mergeInto, const SpriteMaterialInfo& mergeFrom) const {}

		/**
		 * Renders the provided mesh using the current material.
		 *
		 * @param[in]	mesh			Mesh to render, containing vertices in screen space.
		 * @param[in]	subMesh			Portion of @p mesh to render.
		 * @param[in]	texture			Optional texture to render the mesh with.
		 * @param[in]	sampler			Optional sampler to render the texture with.
		 * @param[in]	paramBuffer		Buffer containing data GPU parameters.
		 * @param[in]	additionalData	Optional additional data that might be required by the renderer.
		 * @param[in]	alphaOnly		If true the material will only render the alpha value. Render target is expected to
		 *								have a stencil buffer attached and the value will be written only if stencil value is 0,
		 *								after which the stencil value will be incremented by one. (i.e. only first element that
		 *								writes to a pixel stores its alpha value).
		 */
		virtual void Render(const SPtr<ct::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<ct::Texture>& texture, const SPtr<ct::SamplerState>& sampler, const SPtr<ct::GpuParamBlockBuffer>& paramBuffer, const SPtr<SpriteMaterialExtraInfo>& additionalData, bool alphaOnly) const;

	protected:
		/** Perform initialization of core-thread specific objects. */
		virtual void Initialize();

		/** Destroys the core thread material. */
		static void Destroy(const SPtr<ct::Material>& material, const SPtr<ct::GpuParamsSet>& params, const SPtr<ct::GpuParamsSet>& alphaParams);

		u32 mId;
		bool mAllowBatching;

		// Core thread only (everything below)
		SPtr<ct::Material> mMaterial;
		u32 mTechnique;
		u32 mAlphaTechnique;
		std::atomic<bool> mMaterialStored;

		SPtr<ct::GpuParamsSet> mParams;
		SPtr<ct::GpuParamsSet> mAlphaParams;
		u32 mParamBufferIdx;
		u32 mAlphaParamBufferIdx;
		mutable ct::MaterialParamTexture mTextureParam;
		mutable ct::MaterialParamSampState mSamplerParam;
	};

	/** @} */
} // namespace bs
