//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"
#include "RenderAPI/B3DGpuBuffer.h"
#include "Image/B3DPixelUtility.h"
#include "RenderAPI/B3DTextureView.h"
#include "Math/B3DVector3I.h"

namespace b3d
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Flags that describe how is a texture used. */
	enum B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) TextureUsage
	{
		/** A regular texture that is not often or ever updated from the CPU. */
		TU_STATIC B3D_SCRIPT_EXPORT(ExportName(Default)) = 1 << 0,
		/** A regular texture that is often updated by the CPU. */
		TU_DYNAMIC B3D_SCRIPT_EXPORT(ExportName(Dynamic)) = 1 << 1,
		/** Texture that can be rendered to by the GPU. */
		TU_RENDERTARGET B3D_SCRIPT_EXPORT(ExportName(Render)) = 0x200,
		/** Texture used as a depth/stencil buffer by the GPU. */
		TU_DEPTHSTENCIL B3D_SCRIPT_EXPORT(ExportName(DepthStencil)) = 0x400,
		/** Texture that allows load/store operations from the GPU program. */
		TU_LOADSTORE B3D_SCRIPT_EXPORT(ExportName(LoadStore)) = 0x800,
		/** All mesh data will also be cached in CPU memory, making it available for fast read access from the CPU. */
		TU_CPUCACHED B3D_SCRIPT_EXPORT(ExportName(CPUCached)) = 0x1000,
		/** Allows the CPU to directly read the texture data buffers from the GPU. */
		TU_CPUREADABLE B3D_SCRIPT_EXPORT(ExportName(CPUReadable)) = 0x2000,
		/** Allows you to retrieve views of the texture using a format different from one specified on creation. */
		TU_MUTABLEFORMAT B3D_SCRIPT_EXPORT(ExportName(MutableFormat)) = 0x4000,
		/** Default (most common) texture usage. */
		TU_DEFAULT B3D_SCRIPT_EXPORT(Exclude(true)) = TU_STATIC
	};

	/**	Texture mipmap options. */
	enum TextureMipmap
	{
		MIP_UNLIMITED = 0x7FFFFFFF /**< Create all mip maps down to 1x1. */
	};

	/** Information about a Texture. */
	struct TextureInformation
	{
		/** Optional name of the texture. Used primarily for easier debugging. */
		String Name;

		/** Type of the texture. */
		TextureType Type = TEX_TYPE_2D;

		/** Format of pixels in the texture. */
		PixelFormat Format = PF_RGBA8;

		/** Width of the texture in pixels. */
		u32 Width = 1;

		/** Height of the texture in pixels. */
		u32 Height = 1;

		/** Depth of the texture in pixels (Must be 1 for 2D textures). */
		u32 Depth = 1;

		/** Number of mip-maps the texture has. This number excludes the full resolution map. */
		u32 MipMapCount = 0;

		/** Describes how the caller plans on using the texture in the pipeline. */
		i32 Usage = TU_DEFAULT;

		/** If true the texture data is assumed to be in SRGB space and will be converted back to linear space when sampled on GPU. */
		bool UseHardwareSRGB = false;

		/** Number of samples per pixel. Set to 1 or 0 to use the default of a single sample per pixel. */
		u32 SampleCount = 0;

		/** Number of texture slices to create if creating a texture array. Ignored for 3D textures. */
		u32 ArraySliceCount = 1;
	};

	/** Descriptor structure used for initialization of a Texture. */
	struct B3D_EXPORT TextureCreateInformation : TextureInformation
	{
		TextureCreateInformation() = default;
		TextureCreateInformation(const TextureInformation& other)
			:TextureInformation(other)
		{ }

		TextureCreateInformation(const SPtr<PixelData>& initialData);

		SPtr<PixelData> InitialData;
	};

	/** Structure used for specifying information about a texture copy operation. */
	struct TextureCopyInformation
	{
		/**
		 * Face from which to copy. This can be an entry in an array of textures, or a single face of a cube map. If cubemap
		 * array, then each array entry takes up six faces.
		 */
		u32 SourceFace = 0;

		/** Mip level from which to copy. */
		u32 SourceMip = 0;

		/** Pixel volume from which to copy from. This defaults to all pixels of the face. */
		PixelVolume SourceVolume = PixelVolume(0, 0, 0, 0, 0, 0);

		/**
		 * Face to which to copy. This can be an entry in an array of textures, or a single face of a cube map. If cubemap
		 * array, then each array entry takes up six faces.
		 */
		u32 DestinationFace = 0;

		/** Mip level to which to copy. */
		u32 DestinationMip = 0;

		/** Number of faces to copy. */
		u32 FaceCount = 1; 

		/**
		 * Coordinates to write the source pixels to. The destination texture must have enough pixels to fit the entire
		 * source volume.
		 */
		Vector3I DestinationPosition;

		B3D_EXPORT static const TextureCopyInformation kDefault;
	};

	/** Structure used for specifying information about a texture blit operation. */
	struct TextureBlitInformation
	{
		/**
		 * Face from which to blit. This can be an entry in an array of textures, or a single face of a cube map. If cubemap
		 * array, then each array entry takes up six faces.
		 */
		u32 SourceFace = 0;

		/** Mip level from which to blit. */
		u32 SourceMip = 0;

		/** Pixel volume from which to blit from. This defaults to all pixels of the face. */
		PixelVolume SourceVolume = PixelVolume(0, 0, 0, 0, 0, 0);

		/**
		 * Face to which to blit. This can be an entry in an array of textures, or a single face of a cube map. If cubemap
		 * array, then each array entry takes up six faces.
		 */
		u32 DestinationFace = 0;

		/** Mip level to which to blit. */
		u32 DestinationMip = 0;

		/** Number of faces to blit. */
		u32 FaceCount = 1; 

		/** Pixel volume to which to blit to. This defaults to all pixels of the face. */
		PixelVolume DestinationVolume = PixelVolume(0, 0, 0, 0, 0, 0);

		B3D_EXPORT static const TextureBlitInformation kDefault;
	};

	/** Properties of a Texture. Shared between main and render thread counterparts of a Texture. */
	struct B3D_EXPORT TextureProperties : TextureInformation
	{
	public:
		TextureProperties() = default;
		TextureProperties(const TextureCreateInformation& createInformation);

		/**	Returns true if the texture has an alpha layer. */
		bool HasAlpha() const;

		/**
		 * Returns the number of faces this texture has. This includes array slices (if texture is an array texture),
		 * as well as cube-map faces.
		 */
		u32 GetFaceCount() const;

		/**
		 * Allocates a buffer that exactly matches the format of the texture described by these properties, for the provided
		 * face and mip level. This is a helper function, primarily meant for creating buffers when reading from, or writing
		 * to a texture.
		 *
		 * @note	Thread safe.
		 */
		SPtr<PixelData> AllocBuffer(u32 face, u32 mipLevel) const;

	protected:
		friend class TextureRTTI;
		friend class Texture;

		/**
		 * Maps a sub-resource index to an exact face and mip level. Sub-resource indexes are used when reading or writing
		 * to the resource.
		 */
		void MapFromSubresourceIdx(u32 subresourceIdx, u32& face, u32& mip) const;

		/**
		 * Map a face and a mip level to a sub-resource index you can use for updating or reading a specific sub-resource.
		 */
		u32 MapToSubresourceIdx(u32 face, u32 mip) const;
	};

	/** Represents a part of a texture. */
	enum class GpuTextureAspectFlag
	{
		Color	= 1 << 0,
		Depth	= 1 << 1,
		Stencil = 1 << 2,
	};

	using GpuTextureAspectFlags = Flags<GpuTextureAspectFlag>;
	B3D_FLAGS_OPERATORS(GpuTextureAspectFlag)

	/** Represents a range of subresources in a texture. */
	struct GpuTextureSubresourceRange
	{
		GpuTextureSubresourceRange(u32 baseMipLevel = 0, u32 mipLevelCount = 1, u32 baseArrayLayer = 0, u32 arrayLayerCount = 1, GpuTextureAspectFlags aspectMask = GpuTextureAspectFlag::Color)
			:BaseMipLevel(baseMipLevel), MipLevelCount(mipLevelCount), BaseArrayLayer(baseArrayLayer), ArrayLayerCount(arrayLayerCount), AspectMask(aspectMask)
		{ }

		/** Creates a subresource range covering only the highest (first) mip level. */
		static GpuTextureSubresourceRange TopMip(u32 baseArrayLayer = 0, u32 arrayLayerCount = 1, GpuTextureAspectFlags aspectMask = GpuTextureAspectFlag::Color | GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil)
		{
			return GpuTextureSubresourceRange(0, 1, baseArrayLayer, arrayLayerCount, aspectMask);
		}

		/** Creates a subresource range covering all subresources. */
		static GpuTextureSubresourceRange AllSubresources(GpuTextureAspectFlags aspectMask = GpuTextureAspectFlag::Color | GpuTextureAspectFlag::Depth | GpuTextureAspectFlag::Stencil)
		{
			return GpuTextureSubresourceRange(0, ~0u, 0, ~0u, aspectMask);
		}

		u32 BaseMipLevel = 0;
		u32 MipLevelCount = 1;
		u32 BaseArrayLayer = 0;
		u32 ArrayLayerCount = 1;
		GpuTextureAspectFlags AspectMask = GpuTextureAspectFlag::Color;
	};

	/** Identifies a single texture subresource (face and mip level). */
	struct GpuTextureSubresource
	{
		u32 MipLevel = 0;
		u32 ArrayLayer = 0;

		GpuTextureSubresource(u32 mipLevel = 0, u32 arrayLayer = 0)
			: MipLevel(mipLevel), ArrayLayer(arrayLayer)
		{ }

		bool operator==(const GpuTextureSubresource& other) const
		{
			return ArrayLayer == other.ArrayLayer && MipLevel == other.MipLevel;
		}
	};

	/** Contains the row pitch and slice height for an image subresource. */
	struct ImageSubresourcePitch
	{
		ImageSubresourcePitch(u32 rowPitch = 0, u32 sliceHeight = 0)
			: RowPitch(rowPitch), SliceHeight(sliceHeight)
		{ }

		/** Number of blocks before advancing to the next row. For non-compressed formats this is equal to the number of pixels. For compressed it depends on the block size. */
		u32 RowPitch = 0;

		/** Number of block columns before advancing to the next slice. For non-compressed formats this is equal to the number of pixels. For compressed it depends on the block size. */
		u32 SliceHeight = 0;
	};

	/**
	 * RAII wrapper returned by Texture::Map operation. Contains PixelData with mapped memory buffer.
	 * Automatically flushes on destruction for write operations. Move-only semantics.
	 * Templated to support both main thread and render thread variants.
	 */
	template <bool IsRenderProxy>
	class TGpuTextureMappedScope
	{
	public:
		using TextureType = CoreVariantType<render::Texture, IsRenderProxy>;

		TGpuTextureMappedScope() = default;

		TGpuTextureMappedScope(PixelData pixelData, SPtr<TextureType> texture, GpuTextureSubresource subresource, GpuMapOptions options)
			: mPixelData(std::move(pixelData)), mTexture(std::move(texture)), mSubresource(subresource), mOptions(options)
		{}

		TGpuTextureMappedScope(TGpuTextureMappedScope&& other) noexcept
			: mPixelData(std::move(other.mPixelData)), mTexture(std::move(other.mTexture)), mSubresource(other.mSubresource), mOptions(other.mOptions)
		{
			other.mTexture = nullptr;
		}

		TGpuTextureMappedScope& operator=(TGpuTextureMappedScope&& other) noexcept
		{
			if(this != &other)
			{
				Unmap();
				mPixelData = std::move(other.mPixelData);
				mTexture = std::move(other.mTexture);
				mSubresource = other.mSubresource;
				mOptions = other.mOptions;
				other.mTexture = nullptr;
			}
			return *this;
		}

		TGpuTextureMappedScope(const TGpuTextureMappedScope&) = delete;
		TGpuTextureMappedScope& operator=(const TGpuTextureMappedScope&) = delete;

		~TGpuTextureMappedScope() { Unmap(); }

		/** Explicitly releases the mapping. Safe to call multiple times. Flushes if write mapping. */
		void Unmap()
		{
			if(mTexture != nullptr && mPixelData.GetData() != nullptr)
			{
				if(mOptions.IsSet(GpuMapOption::Write))
				{
					if constexpr(IsRenderProxy)
						mTexture->Flush(mSubresource.MipLevel, mSubresource.ArrayLayer);
					else
						mTexture->MarkRenderProxyDataDirty();
				}

				mTexture = nullptr;
				mPixelData.SetExternalBuffer(nullptr);
			}
		}

		/** Returns the PixelData containing the mapped memory buffer. */
		PixelData& GetPixelData() { return mPixelData; }
		const PixelData& GetPixelData() const { return mPixelData; }

		/** Returns the texture being mapped. */
		const SPtr<TextureType>& GetTexture() const { return mTexture; }

		/** Returns the subresource being mapped. */
		const GpuTextureSubresource& GetSubresource() const { return mSubresource; }

		/** Returns true if the mapping is valid. */
		bool IsValid() const { return mTexture != nullptr && mPixelData.GetData() != nullptr; }
		explicit operator bool() const { return IsValid(); }

	private:
		PixelData mPixelData;
		SPtr<TextureType> mTexture;
		GpuTextureSubresource mSubresource;
		GpuMapOptions mOptions;
	};

	using GpuTextureMappedScope = TGpuTextureMappedScope<false>;

	/**
	 * Abstract class representing a texture. Specific render systems have their own Texture implementations. Internally
	 * represented as one or more surfaces with pixels in a certain number of dimensions, backed by a hardware buffer.
	 *
	 * @note	Main thread.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Texture : public Resource
	{
	public:
		/**
		 * Updates the texture with new data. Provided data buffer will be locked until the operation completes.
		 *
		 * @param	data					Pixel data to write. User must ensure it is in format and size compatible with
		 *									the texture.
		 * @param	face					Texture face to write to.
		 * @param	mipLevel				Mipmap level to write to.
		 * @param	discardEntireBuffer		When true the existing contents of the resource you are updating will be
		 *									discarded. This can make the operation faster. Resources with certain buffer
		 *									types might require this flag to be in a specific state otherwise the operation
		 *									will fail.
		 * @return							Async operation object you can use to track operation completion.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		TAsyncOp<void> WriteData(const SPtr<PixelData>& data, u32 face = 0, u32 mipLevel = 0, bool discardEntireBuffer = false);

		/**
		 * Reads internal texture data to the provided previously allocated buffer. Provided data buffer will be locked
		 * until the operation completes.
		 *
		 * @param	data		Pre-allocated buffer of proper size and format where data will be read to. You can use
		 *						TextureProperties::allocBuffer() to allocate a buffer of a correct format and size.
		 * @param	face		Texture face to read from.
		 * @param	mipLevel	Mipmap level to read from.
		 * @return				Async operation object you can use to track operation completion.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		TAsyncOp<void> ReadData(const SPtr<PixelData>& data, u32 face = 0, u32 mipLevel = 0);

		/**
		 * Reads internal texture data into a newly allocated buffer.
		 *
		 * @param	face		Texture face to read from.
		 * @param	mipLevel	Mipmap level to read from.
		 * @return				Async operation object that will contain the buffer with the data once the operation
		 *						completes.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetGPUPixels))
		TAsyncOp<SPtr<PixelData>> ReadData(u32 face = 0, u32 mipLevel = 0);

		/**
		 * Reads data from the cached system memory texture buffer into the provided buffer.
		 *
		 * @param	data		Pre-allocated buffer of proper size and format where data will be read to. You can use
		 *						TextureProperties::allocBuffer() to allocate a buffer of a correct format and size.
		 * @param	face		Texture face to read from.
		 * @param	mipLevel	Mipmap level to read from.
		 *
		 * @note
		 * The data read is the cached texture data. Any data written to the texture from the GPU or render thread will not
		 * be reflected in this data. Use ReadData() if you require those changes.
		 * @note
		 * The texture must have been created with TU_CPUCACHED usage otherwise this method will not return any data.
		 */
		void ReadCachedData(PixelData& data, u32 face = 0, u32 mipLevel = 0);

		/**	Returns properties that contain information about the texture. */
		const TextureProperties& GetProperties() const { return mProperties; }

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/

		/**
		 * Creates a new texture.
		 *
		 * @param	createInformation  	Description of the texture to create.
		 */
		static HTexture Create(const TextureCreateInformation& createInformation);

		/** @name Internal
		 *  @{
		 */

		/** Same as Create() excepts it creates a pointer to the texture instead of a texture handle. */
		static SPtr<Texture> CreateShared(const TextureCreateInformation& createInformation);

		/** Creates an empty texture with default parameters. Requires an explicit call to Initialize() before use. Primarily intended for deserialization. */
		static SPtr<Texture> CreateEmpty();

		/** @} */

	protected:
		friend class TextureManager;

		Texture(const TextureCreateInformation& createInformation, const SPtr<PixelData>& pixelData);
		Texture(const TextureCreateInformation& createInformation);

		void Initialize() override;
		SPtr<render::RenderProxy> CreateRenderProxy() const override;

		/** Calculates the size of the texture, in bytes. */
		u32 CalculateSize() const;

		/**
		 * Creates buffers used for caching of CPU texture data.
		 *
		 * @note	Make sure to initialize all texture properties before calling this.
		 */
		void CreateCpuBuffers();

		/**	Updates the cached CPU buffers with new data. */
		void UpdateCpuBuffers(u32 subresourceIdx, const PixelData& data);

	protected:
		Vector<SPtr<PixelData>> mCPUSubresourceData;
		TextureProperties mProperties;
		mutable SPtr<PixelData> mInitData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		Texture() = default; // Serialization only

		friend class TextureRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Resources-Internal
		 *  @{
		 */

		using GpuTextureMappedScope = TGpuTextureMappedScope<true>;

		/**
		 * Render thread counterpart of a b3d::Texture.
		 *
		 * @note	Render thread.
		 */
		class B3D_EXPORT Texture : public RenderProxy
		{
		public:
			Texture(const TextureCreateInformation& createInformation);
			virtual ~Texture() {}

			void Initialize() override;

			/** Assigns an name to the image, primarily used for easier debugging. */
			virtual void SetName(const StringView& name) { mName = name; }

			/** Returns the name of the image. Primarily used for debugging purposes. */
			const String& GetName() const { return mName; }

			/**
			 * Maps a texture subresource for CPU access.
			 *
			 * @param mipLevel		Mipmap level to map.
			 * @param arrayLayer	Texture face to map.
			 * @param options		Specifies read/write intent for the mapping.
			 * @return				RAII scope containing PixelData with mapped memory.
			 *
			 * @note Only works for directly mappable textures (TU_DYNAMIC with LINEAR tiling).
			 * @note Returns invalid scope if texture is not directly mappable.
			 */
			virtual GpuTextureMappedScope Map(u32 mipLevel, u32 arrayLayer, GpuMapOptions options) = 0;

			/** Returns the GPU device this texture belongs to. */
			virtual GpuDevice& GetGpuDevice() const = 0;

			/**
			 * Reads data from the texture buffer into the provided buffer.
			 *
			 * @note	If the texture cannot be directly mapped by the CPU this will internally create a staging buffer, on which the
			 *			contents will be copied before being read by the CPU, using an internal command buffer.
			 * @note	If the texture is currently being used by the GPU, this method will block until the GPU is done executing, so you should call this
			 *			method in very rare circumstances.
			 *
			 * @param	destination		Previously allocated buffer to read data into.
			 * @param	mipLevel		(optional) Mipmap level to read from.
			 * @param	face			(optional) Texture face to read from.
			 * @param	gpuQueue		GPU queue on which to perform the read. If not specified the default queue will be used.
			 */
			void ReadData(PixelData& destination, u32 mipLevel = 0, u32 face = 0, const SPtr<GpuQueue>& gpuQueue = nullptr);

			/**
			 * Performs a non-blocking read operation. The GPU will execute the read when the command buffer reaches the execution point
			 * and the asynchronous operation will be signaled with the return value.
			 *
			 * @param	commandBuffer	Command buffer to queue the operation on.
			 * @param	mipLevel		(optional) Mipmap level to read from.
			 * @param	face			(optional) Texture face to read from.
			 * @return					Operation that will be signaled when the data is ready to be read.
			 */
			virtual TAsyncOp<SPtr<PixelData>> ReadDataAsync(GpuCommandBuffer& commandBuffer, u32 mipLevel = 0, u32 face = 0);

			/**
			 * Flushes CPU writes to the specified subresource to make them visible to the GPU.
			 * Only relevant for directly mappable textures with non-coherent memory.
			 *
			 * @param mipLevel		Mipmap level to flush.
			 * @param arrayLayer	Array layer to flush.
			 */
			virtual void Flush(u32 mipLevel, u32 arrayLayer) {}

			/**
			 * Invalidates GPU writes to the specified subresource to make them visible to the CPU.
			 * Only relevant for directly mappable textures with non-coherent memory.
			 *
			 * @param mipLevel		Mipmap level to invalidate.
			 * @param arrayLayer	Array layer to invalidate.
			 */
			virtual void Invalidate(u32 mipLevel, u32 arrayLayer) {}

			/**
			 * Returns pitch information for a particular image subresource.
			 *
			 * @param face		Face (array slice or cubemap face) of the subresource.
			 * @param mipLevel	Mipmap level of the subresource.
			 * @return			Row and slice pitch information for the subresource.
			 */
			virtual ImageSubresourcePitch GetPitchForSubresource(u32 face, u32 mipLevel) const = 0;

			/** Recreates the underlying texture. Note this will clear all currently written data. Old texture will be released once its done being used. */
			virtual void RecreateInternalTexture() = 0;

			/** Returns a pointer to persistently mapped memory, or nullptr if not mappable. */
			void* GetMappedMemory() const { return mMappedMemory; }

			/**
			 * Returns which GPU queues are currently using the specified subresource.
			 * @param mipLevel		Mipmap level.
			 * @param arrayLayer	Texture arrayLayer (or cubemap face).
			 * @param accessFlags	Filter by read/write access type.
			 */
			virtual GpuQueueMask GetUseMask(u32 mipLevel, u32 arrayLayer, GpuAccessFlags accessFlags = GpuAccessFlag::Read | GpuAccessFlag::Write) const = 0;

			/** Returns how many command buffers the specified subresource is bound to. */
			virtual u32 GetBoundCount(u32 mipLevel, u32 arrayLevel) const = 0;

			/** Returns how many submitted command buffers are using the specified subresource. */
			virtual u32 GetUseCount(u32 mipLevel, u32 arrayLevel) const = 0;

			/**	Returns properties that contain information about the texture. */
			const TextureProperties& GetProperties() const { return mProperties; }

			/************************************************************************/
			/* 								TEXTURE VIEW                      		*/
			/************************************************************************/

			/**
			 * Requests a texture view for the specified mip and array ranges. Returns an existing view of one for the specified
			 * ranges already exists, otherwise creates a new one. You must release all views by calling ReleaseView() when done.
			 *
			 * @param	surface		Texture surface to create a view for.
			 * @param	usage		Usage of the texture view.
			 *
			 * @note	Render thread only.
			 */
			SPtr<TextureView> RequestView(const TextureSurface& surface, GpuViewUsage usage);

			/** Returns a plain white texture. */
			static SPtr<Texture> kWhite;

			/** Returns a plain black texture. */
			static SPtr<Texture> kBlack;

			/** Returns a plain pink texture. */
			static SPtr<Texture> kPink;

			/** Returns a plain normal map texture with normal pointing up (in Y direction). */
			static SPtr<Texture> kNormal;

		protected:
			/** @copydoc ReadData */
			virtual void ReadDataInternal(PixelData& destination, u32 mipLevel = 0, u32 face = 0, const SPtr<GpuQueue>& gpuQueue = nullptr) = 0;

			/************************************************************************/
			/* 								TEXTURE VIEW                      		*/
			/************************************************************************/

			/**	Creates a view of a specific subresource in a texture. */
			virtual SPtr<TextureView> CreateView(const TextureViewInformation& desc);

			/** Releases all internal texture view references. */
			void ClearBufferViews();

			UnorderedMap<TextureViewInformation, SPtr<TextureView>, TextureView::HashFunction, TextureView::EqualFunction> mTextureViews;
			String mName;
			TextureProperties mProperties;
			SPtr<PixelData> mInitData;
			void* mMappedMemory = nullptr;
		};

		/** Flags controlling texture write behavior. */
		enum class TextureWriteFlag
		{
			/**
			 * Default flag. Performs the write assuming the image subresource is not currently bound to a command buffer, or used by the GPU.
			 * If the image subresource is in use by the GPU or bound to a command buffer, it's expected the caller will provide a command buffer
			 * on which to queue the write operation on, to ensure previous uses are not disturbed. Otherwise write fails.
			 */
			Normal = 0,

			/**
			 * If the image subresource is currently being used on the GPU the system will internally allocate new memory for the subresource
			 * and write to the new memory. Old subresource will remain for whatever purpose it was used for until execution finishes,
			 * at which point it will be freed. Caller must ensure to either fully write in the all the image subresources,
			 * as anything not written by the caller will be undefined. Useful if you don't care about previous image contents.
			 */
			Discard = 1 << 0,

			/**
			 * If the image subresource is currently being used on the GPU the system will still let you update it. It's up to the
			 * caller not to update the same  region as the GPU is operating on, while respecting any other rules required
			 * by the low-level render API when doing such an operation (such as issuing memory barriers, flushing* memory and
			 * respecting granularity). Use only when you know what you are doing.
			 */
			NoOverwrite = 1 << 1
		};

		using TextureWriteFlags = Flags<TextureWriteFlag>;
		B3D_FLAGS_OPERATORS(TextureWriteFlag)

		/** Utility class for working with textures, providing helper methods for staging buffer operations and data transfer. */
		struct B3D_EXPORT TextureUtility
		{
			/**
			 * Creates a staging buffer sized appropriately for texture data transfer.
			 *
			 * @param texture		Texture for which to create the staging buffer.
			 * @param mipLevel		Mip level of the texture subresource.
			 * @param readable		True if the buffer needs to be CPU-readable (for readback), false if CPU-writeable (for upload).
			 * @return				Newly created staging buffer sized for the specified texture subresource.
			 */
			static SPtr<GpuBuffer> CreateStagingBuffer(const SPtr<Texture>& texture, u32 mipLevel, bool readable);

			/**
			 * Creates a staging buffer with the specified size.
			 *
			 * @param texture		Texture for which to create the staging buffer.
			 * @param pixelData		Pixel data structure with initialized row/depth pitch, used to determine buffer size.
			 * @param readable		True if the buffer needs to be CPU-readable (for readback), false if CPU-writeable (for upload).
			 * @return				Newly created staging buffer.
			 */
			static SPtr<GpuBuffer> CreateStagingBuffer(const SPtr<Texture>& texture, const PixelData& pixelData, bool readable);

			/**
			 * Writes pixel data to a texture subresource.
			 *
			 * This method automatically chooses the optimal path:
			 * - For directly mappable textures: Uses Map() + BulkPixelConversion
			 * - For non-mappable textures: Uses staging buffer + CopyBufferToTexture
			 *
			 * @param texture		Texture to write data to.
			 * @param source		Pixel data to write. Must be compatible with texture format and dimensions.
			 * @param mipLevel		Destination mipmap level.
			 * @param arrayLayer	Destination texture face (array layer or cubemap face).
			 * @param flags			Optional flags controlling write behavior.
			 * @param commandBuffer	Command buffer for staging operations. If null, uses internal transfer buffer.
			 *
			 * @note Render thread only.
			 */
			static void Write(const SPtr<Texture>& texture, const PixelData& source, u32 mipLevel = 0, u32 arrayLayer = 0, TextureWriteFlags flags = TextureWriteFlag::Normal, SPtr<GpuCommandBuffer> commandBuffer = nullptr);

			/**
			 * Sets all the pixels of the specified face and mip level to the provided value.
			 *
			 * @param	texture			Texture to write data to.
			 * @param	value			Color to clear the pixels to.
			 * @param	mipLevel		Mip level to clear.
			 * @param	arrayLayer		Array layer (or cubemap face) to clear.
			 * @param	commandBuffer	Command buffer on which to encode the staging texture copy, in case the texture is not directly writeable.
			 *							If not provided the operation will be queued on an internal command buffer that will be submitted before
			 *							any regular command buffer submission.
			 */
			static void Clear(const SPtr<Texture>& texture, const Color& value, u32 mipLevel = 0, u32 arrayLayer = 0, const SPtr<GpuCommandBuffer>& commandBuffer = nullptr);
		};

		/** @} */
	} // namespace render
} // namespace b3d
