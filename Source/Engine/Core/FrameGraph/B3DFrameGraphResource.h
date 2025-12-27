//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "B3DFrameGraphTypes.h"
#include "Image/B3DTexture.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "RenderAPI/B3DRenderTarget.h"
#include "Utility/B3DCommonTypes.h"

namespace b3d::render
{
	/** @addtogroup RenderAPI
	 *  @{
	 */

	/** Type of resource managed by the frame graph */
	enum class FrameGraphResourceType
	{
		Texture,
		Buffer,
		RenderTarget
	};

	/**
	 * Base class for frame graph resources.
	 *
	 * Supports both imported (externally managed) and transient (automatically allocated) resources.
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

		/** Returns whether this is a transient resource */
		bool IsTransient() const { return mIsTransient; }

	protected:
		FrameGraphResourceId mId;
		String mName; // TODO - Can we make this development only?
		FrameGraphResourceType mType;
		bool mIsTransient = false;
	};

	/**
	 * Frame graph texture resource.
	 *
	 * Represents either an imported texture (externally managed) or a transient texture
	 * (automatically allocated). For imported resources, the texture lifetime is managed
	 * externally and must remain valid for the duration of frame graph execution.
	 * For transient resources, the texture is allocated during Execute() based on the
	 * stored descriptor.
	 */
	class B3D_EXPORT FrameGraphTextureResource : public FrameGraphResource
	{
	public:
		/**
		 * Constructs an imported texture resource.
		 *
		 * @param id		Unique identifier for this resource
		 * @param name		Name for debugging/profiling
		 * @param texture	The underlying texture (must not be null)
		 */
		FrameGraphTextureResource(
			FrameGraphResourceId id,
			const StringView& name,
			const SPtr<Texture>& texture);

		/**
		 * Constructs a transient texture resource.
		 * The texture will be null until allocated during Execute().
		 *
		 * @param id		Unique identifier for this resource
		 * @param name		Name for debugging/profiling
		 */
		FrameGraphTextureResource(
			FrameGraphResourceId id,
			const StringView& name);

		/** Returns the underlying texture (may be null for transients before allocation) */
		const SPtr<Texture>& GetTexture() const { return mTexture; }

		/** Sets the texture (used when allocating transients) */
		void SetTexture(const SPtr<Texture>& texture) { mTexture = texture; }

	private:
		SPtr<Texture> mTexture;
	};

	/**
	 * Frame graph buffer resource.
	 *
	 * Represents either an imported buffer (externally managed) or a transient buffer
	 * (automatically allocated). For imported resources, the buffer lifetime is managed
	 * externally and must remain valid for the duration of frame graph execution.
	 * For transient resources, the buffer is allocated during Execute() based on the
	 * stored descriptor.
	 */
	class B3D_EXPORT FrameGraphBufferResource : public FrameGraphResource
	{
	public:
		/**
		 * Constructs an imported buffer resource.
		 *
		 * @param id		Unique identifier for this resource
		 * @param name		Name for debugging/profiling
		 * @param buffer	The underlying buffer (must not be null)
		 */
		FrameGraphBufferResource(
			FrameGraphResourceId id,
			const StringView& name,
			const SPtr<GpuBuffer>& buffer);

		/**
		 * Constructs a transient buffer resource.
		 * The buffer will be null until allocated during Execute().
		 *
		 * @param id		Unique identifier for this resource
		 * @param name		Name for debugging/profiling
		 */
		FrameGraphBufferResource(
			FrameGraphResourceId id,
			const StringView& name);

		/** Returns the underlying buffer (may be null for transients before allocation) */
		const SPtr<GpuBuffer>& GetBuffer() const { return mBuffer; }

		/** Sets the buffer (used when allocating transients) */
		void SetBuffer(const SPtr<GpuBuffer>& buffer) { mBuffer = buffer; }

	private:
		SPtr<GpuBuffer> mBuffer;
	};

	/**
	 * Frame graph render target resource (imported).
	 *
	 * Represents a render target (e.g., swap chain) that has been imported into the frame graph.
	 * This is used for resources that cannot be accessed as standalone textures, such as
	 * swap chain backbuffers from RenderWindow.
	 *
	 * The render target lifetime is managed externally and must remain valid for the duration
	 * of frame graph execution.
	 */
	class B3D_EXPORT FrameGraphRenderTargetResource : public FrameGraphResource
	{
	public:
		/**
		 * Constructs a new render target resource.
		 *
		 * @param id			Unique identifier for this resource
		 * @param name			Name for debugging/profiling
		 * @param renderTarget	The underlying render target (must not be null)
		 * @param surface		Which surface of the render target to use (e.g., RT_COLOR0 for backbuffer)
		 */
		FrameGraphRenderTargetResource(
			FrameGraphResourceId id,
			const StringView& name,
			const SPtr<RenderTarget>& renderTarget,
			RenderSurfaceMaskBits surface);

		/** Returns the underlying render target */
		const SPtr<RenderTarget>& GetRenderTarget() const { return mRenderTarget; }

		/** Returns which surface of the render target is being used */
		RenderSurfaceMaskBits GetSurface() const { return mSurface; }

	private:
		SPtr<RenderTarget> mRenderTarget;
		RenderSurfaceMaskBits mSurface;
	};

	/** @} */
}
