//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraphTypes.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DGpuBuffer.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Type of resource managed by the frame graph */
	enum class FrameGraphResourceType
	{
		Texture,
		Buffer
	};

	/**
	 * Base class for frame graph resources.
	 *
	 * In Phase 1, only imported resources are supported.
	 * Transient resources will be added in Phase 4.
	 */
	class B3D_EXPORT FrameGraphResource
	{
	public:
		FrameGraphResource(
			FrameGraphResourceId id,
			const StringView& name,
			FrameGraphResourceType type);

		virtual ~FrameGraphResource() = default;

		/** Returns the resource ID */
		FrameGraphResourceId GetId() const { return mId; }

		/** Returns the resource name (for debugging) */
		const String& GetName() const { return mName; }

		/** Returns the resource type */
		FrameGraphResourceType GetType() const { return mType; }

		/** Returns whether this is a transient resource (always false in Phase 1) */
		virtual bool IsTransient() const { return false; }

	protected:
		FrameGraphResourceId mId;
		String mName; // TODO - Can we make this development only?
		FrameGraphResourceType mType;
	};

	/**
	 * Frame graph texture resource (imported).
	 *
	 * Represents a texture that has been imported into the frame graph from an external source.
	 * The texture lifetime is managed externally and must remain valid for the duration of
	 * frame graph execution.
	 *
	 * Phase 1 Note: Only imported resources are supported.
	 * Phase 4 will add transient (temporary) resources that are automatically allocated.
	 */
	class B3D_EXPORT FrameGraphTextureResource : public FrameGraphResource
	{
	public:
		/**
		 * Constructs a new texture resource.
		 *
		 * @param id		Unique identifier for this resource
		 * @param name		Name for debugging/profiling
		 * @param texture	The underlying texture (must not be null)
		 */
		FrameGraphTextureResource(
			FrameGraphResourceId id,
			const StringView& name,
			const SPtr<Texture>& texture);

		/** Returns the underlying texture */
		const SPtr<Texture>& GetTexture() const { return mTexture; }

		/** Sets the texture (for internal use) */
		void SetTexture(const SPtr<Texture>& texture) { mTexture = texture; }

	private:
		SPtr<Texture> mTexture;
	};

	/**
	 * Frame graph buffer resource (imported).
	 *
	 * Represents a GPU buffer that has been imported into the frame graph from an external source.
	 * The buffer lifetime is managed externally and must remain valid for the duration of
	 * frame graph execution.
	 *
	 * Phase 1 Note: Only imported resources are supported.
	 * Phase 4 will add transient (temporary) resources that are automatically allocated.
	 */
	class B3D_EXPORT FrameGraphBufferResource : public FrameGraphResource
	{
	public:
		/**
		 * Constructs a new buffer resource.
		 *
		 * @param id		Unique identifier for this resource
		 * @param name		Name for debugging/profiling
		 * @param buffer	The underlying buffer (must not be null)
		 */
		FrameGraphBufferResource(
			FrameGraphResourceId id,
			const StringView& name,
			const SPtr<GpuBuffer>& buffer);

		/** Returns the underlying buffer */
		const SPtr<GpuBuffer>& GetBuffer() const { return mBuffer; }

		/** Sets the buffer (for internal use) */
		void SetBuffer(const SPtr<GpuBuffer>& buffer) { mBuffer = buffer; }

	private:
		SPtr<GpuBuffer> mBuffer;
	};

	/** @} */
}
