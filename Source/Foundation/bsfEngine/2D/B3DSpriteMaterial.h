//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Material/BsMaterialParam.h"
#include "Image/BsColor.h"
#include "Material/BsShaderVariation.h"
#include "Renderer/BsGpuDataParameterBlock.h"

namespace b3d
{
	namespace render
	{
		class MeshBase;
		struct SpriteMaterialInfo;
	}

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

	/** Common functionality for both main and render thread thread variants of SpriteMaterialInformation. */
	template <bool IsRenderProxy>
	struct TSpriteMaterialInfo
	{
		using TextureType = CoreVariantHandleType<Texture, IsRenderProxy>;
		using SpriteImageType = CoreVariantHandleType<SpriteImage, IsRenderProxy>;

		TSpriteMaterialInfo() = default;

		/**
		 * Creates a new deep copy of the object. This is different from standard copy constructor which will just reference
		 * the original "additionalData" field, while this will copy it.
		 */
		TSpriteMaterialInfo Clone() const
		{
			TSpriteMaterialInfo info;
			info.GroupId = GroupId;
			info.Texture = Texture;
			info.SpriteImage = SpriteImage;
			info.Tint = Tint;
			info.AnimationStartTime = AnimationStartTime;

			if(AdditionalData != nullptr)
				info.AdditionalData = AdditionalData->Clone();

			return info;
		}

		u64 GroupId = 0;
		TextureType Texture;
		SpriteImageType SpriteImage; /**< Sprite image used to generate the sprite. Used for animation only. */
		SPtr<render::SpriteImageAllocation> SpriteImageAllocation; /**< Allocation handle to keep the sprite image alive as long as needed. */
		Color Tint;
		float AnimationStartTime = 0.0f;
		SPtr<SpriteMaterialExtraInfo> AdditionalData;
	};

	/** Contains information for initializing a sprite material. */
	struct SpriteMaterialInfo : TSpriteMaterialInfo<false>
	{
		using TSpriteMaterialInfo::TSpriteMaterialInfo;
	};

	/** Interfaced implemented by materials used for rendering sprites. This is expected to be used as a singleton. */
	class B3D_EXPORT SpriteMaterial
	{
	public:
		SpriteMaterial(u32 id, const HMaterial& material, ShaderVariationParameters variation = ShaderVariationParameters::kEmpty, bool allowBatching = true);
		virtual ~SpriteMaterial();

		/** Returns the unique ID of the sprite material. */
		u32 GetId() const { return mId; }

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
		 * @param	commandBuffer		Command buffer to encode the render commands  on.
		 * @param	mesh				Mesh to render, containing vertices in screen space.
		 * @param	subMesh				Portion of @p mesh to render.
		 * @param	texture				Optional texture to render the mesh with.
		 * @param	sampler				Optional sampler to render the texture with.
		 * @param	uniformBuffer		Buffer containing data GPU parameters, created from GUISpriteUniformBufferDefinition.
		 * @param	clipRegionBuffer	Buffer containing regions against all rendered sprite quads will be culled/clipped against.
		 * @param	clipRegionCount		Number of regions in @p clipRegionBuffer.
		 * @param	additionalData		Optional additional data that might be required by the renderer.
		 */
		virtual void Render(render::GpuCommandBuffer& commandBuffer, const SPtr<render::MeshBase>& mesh, const SubMesh& subMesh, const SPtr<render::Texture>& texture, const SPtr<SamplerState>& sampler, const SPtr<render::GpuBuffer>& uniformBuffer, const SPtr<render::GpuBuffer>& clipRegionBuffer, u32 clipRegionCount, const SPtr<SpriteMaterialExtraInfo>& additionalData) const;

		/** Writes the provided parameters into a uniform buffer created from GUISpriteUniformBufferDefinition. */
		static void PopulateUniformBuffer(const SPtr<render::GpuBuffer>& buffer, const Vector2I& viewportOffset, float inverseViewportWidth, float inverseViewportHeight, bool flipY, float animationTime, u32 clipRegionCount, const Matrix4& transform, const render::SpriteMaterialInfo& materialInformation);
	protected:
		/** Perform initialization of render-thread specific objects. */
		virtual void Initialize();

		/** Destroys the render thread material. */
		static void Destroy(const SPtr<render::Material>& material, const SPtr<render::GpuParamsSet>& withClippingParams, const SPtr<render::GpuParamsSet>& withoutClippingParams);

		struct MaterialVariationInformation
		{
			u32 VariationIndex = ~0u;
			u32 UniformBufferIndex = ~0u;
			SPtr<render::GpuParamsSet> ParameterSet;

			TGpuParameterBuffer<true> VerticesBufferParameter;
			TGpuParameterBuffer<true> ClipRegionsBufferParameter;
		};

		u32 mId;
		bool mAllowBatching;

		// Render thread only (everything below)
		SPtr<render::Material> mMaterial;
		MaterialVariationInformation mWithoutClippingVariation;
		MaterialVariationInformation mWithClippingVariation;

		std::atomic<bool> mMaterialStored;

		mutable render::MaterialParameterSampledTexture mTextureParameter;
		mutable render::MaterialParameterSampler mSamplerParameter;
	};

	namespace render
	{
		B3D_PARAM_BLOCK_BEGIN(GUISpriteUniformBufferDefinition)
			B3D_PARAM_BLOCK_ENTRY(Matrix4, gWorldTransform)
			B3D_PARAM_BLOCK_ENTRY(float, gInvViewportWidth)
			B3D_PARAM_BLOCK_ENTRY(float, gInvViewportHeight)
			B3D_PARAM_BLOCK_ENTRY(Vector2I, gViewportOffset)
			B3D_PARAM_BLOCK_ENTRY(Color, gTint)
			B3D_PARAM_BLOCK_ENTRY(Vector4, gUVSizeOffset)
			B3D_PARAM_BLOCK_ENTRY(float, gViewportYFlip)
			B3D_PARAM_BLOCK_ENTRY(u32, gClipRegionCount)
		B3D_PARAM_BLOCK_END

		extern GUISpriteUniformBufferDefinition gGUISpriteUniformBufferDefinition;

		/** @copydoc b3d::SpriteMaterialInfo */
		struct SpriteMaterialInfo : TSpriteMaterialInfo<true>
		{
			SpriteMaterialInfo() = default;

			/** Initializes the object from the main thread variant. */
			SpriteMaterialInfo(const TSpriteMaterialInfo<false>& other);
		};
	} // namespace render

	/** @} */
} // namespace b3d
