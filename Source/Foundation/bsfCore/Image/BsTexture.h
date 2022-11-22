//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "RenderAPI/BsHardwareBuffer.h"
#include "Image/BsPixelUtil.h"
#include "RenderAPI/BsTextureView.h"
#include "Math/BsVector3I.h"

namespace bs
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Flags that describe how is a texture used. */
	enum B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) TextureUsage
	{
		/** A regular texture that is not often or ever updated from the CPU. */
		TU_STATIC B3D_SCRIPT_EXPORT(ExportName(Default)) = GBU_STATIC,
		/** A regular texture that is often updated by the CPU. */
		TU_DYNAMIC B3D_SCRIPT_EXPORT(ExportName(Dynamic)) = GBU_DYNAMIC,
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

	/** Descriptor structure used for initialization of a Texture. */
	struct TEXTURE_DESC
	{
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
		u32 NumMips = 0;

		/** Describes how the caller plans on using the texture in the pipeline. */
		i32 Usage = TU_DEFAULT;

		/**
		 * If true the texture data is assumed to have been gamma corrected and will be converted back to linear space when
		 * sampled on GPU.
		 */
		bool HwGamma = false;

		/** Number of samples per pixel. Set to 1 or 0 to use the default of a single sample per pixel. */
		u32 NumSamples = 0;

		/** Number of texture slices to create if creating a texture array. Ignored for 3D textures. */
		u32 NumArraySlices = 1;
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

		/**
		 * Coordinates to write the source pixels to. The destination texture must have enough pixels to fit the entire
		 * source volume.
		 */
		Vector3I DestinationPosition;

		B3D_CORE_EXPORT static const TextureCopyInformation kDefault;
	};

	/** Properties of a Texture. Shared between sim and core thread versions of a Texture. */
	class B3D_CORE_EXPORT TextureProperties
	{
	public:
		TextureProperties() = default;
		TextureProperties(const TEXTURE_DESC& desc);

		/**	Gets the type of texture. */
		TextureType GetTextureType() const { return mDesc.Type; }

		/**
		 * Gets the number of mipmaps to be used for this texture. This number excludes the top level map (which is always
		 * assumed to be present).
		 */
		u32 GetNumMipmaps() const { return mDesc.NumMips; }

		/**
		 * Determines does the texture contain gamma corrected data. If true then the GPU will automatically convert the
		 * pixels to linear space before reading from the texture, and convert them to gamma space when writing to the
		 * texture.
		 */
		bool IsHardwareGammaEnabled() const { return mDesc.HwGamma; }

		/**	Gets the number of samples used for multisampling (0 or 1 if multisampling is not used). */
		u32 GetNumSamples() const { return mDesc.NumSamples; }

		/**	Returns the height of the texture.  */
		u32 GetHeight() const { return mDesc.Height; }

		/**	Returns the width of the texture. */
		u32 GetWidth() const { return mDesc.Width; }

		/**	Returns the depth of the texture (only applicable for 3D textures). */
		u32 GetDepth() const { return mDesc.Depth; }

		/**	Returns a value that signals the engine in what way is the texture expected to be used. */
		int GetUsage() const { return mDesc.Usage; }

		/**	Returns the pixel format for the texture surface. */
		PixelFormat GetFormat() const { return mDesc.Format; }

		/**	Returns true if the texture has an alpha layer. */
		bool HasAlpha() const;

		/**
		 * Returns the number of faces this texture has. This includes array slices (if texture is an array texture),
		 * as well as cube-map faces.
		 */
		u32 GetNumFaces() const;

		/** Returns the number of array slices of the texture (if the texture is an array texture). */
		u32 GetNumArraySlices() const { return mDesc.NumArraySlices; }

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

		TEXTURE_DESC mDesc;
	};

	/**
	 * Abstract class representing a texture. Specific render systems have their own Texture implementations. Internally
	 * represented as one or more surfaces with pixels in a certain number of dimensions, backed by a hardware buffer.
	 *
	 * @note	Sim thread.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering)) Texture : public Resource
	{
	public:
		/**
		 * Updates the texture with new data. Provided data buffer will be locked until the operation completes.
		 *
		 * @param[in]	data				Pixel data to write. User must ensure it is in format and size compatible with
		 *									the texture.
		 * @param[in]	face				Texture face to write to.
		 * @param[in]	mipLevel			Mipmap level to write to.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be
		 *									discarded. This can make the operation faster. Resources with certain buffer
		 *									types might require this flag to be in a specific state otherwise the operation
		 *									will fail.
		 * @return							Async operation object you can use to track operation completion.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		AsyncOp WriteData(const SPtr<PixelData>& data, u32 face = 0, u32 mipLevel = 0, bool discardEntireBuffer = false);

		/**
		 * Reads internal texture data to the provided previously allocated buffer. Provided data buffer will be locked
		 * until the operation completes.
		 *
		 * @param[out]	data		Pre-allocated buffer of proper size and format where data will be read to. You can use
		 *							TextureProperties::allocBuffer() to allocate a buffer of a correct format and size.
		 * @param[in]	face		Texture face to read from.
		 * @param[in]	mipLevel	Mipmap level to read from.
		 * @return					Async operation object you can use to track operation completion.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		AsyncOp ReadData(const SPtr<PixelData>& data, u32 face = 0, u32 mipLevel = 0);

		/**
		 * Reads internal texture data into a newly allocated buffer.
		 *
		 * @param[in]	face		Texture face to read from.
		 * @param[in]	mipLevel	Mipmap level to read from.
		 * @return					Async operation object that will contain the buffer with the data once the operation
		 *							completes.
		 *
		 * @note This is an @ref asyncMethod "asynchronous method".
		 */
		B3D_SCRIPT_EXPORT(ExportName(GetGPUPixels))
		TAsyncOp<SPtr<PixelData>> ReadData(u32 face = 0, u32 mipLevel = 0);

		/**
		 * Reads data from the cached system memory texture buffer into the provided buffer.
		 *
		 * @param[out]	data		Pre-allocated buffer of proper size and format where data will be read to. You can use
		 *							TextureProperties::allocBuffer() to allocate a buffer of a correct format and size.
		 * @param[in]	face		Texture face to read from.
		 * @param[in]	mipLevel	Mipmap level to read from.
		 *
		 * @note
		 * The data read is the cached texture data. Any data written to the texture from the GPU or core thread will not
		 * be reflected in this data. Use readData() if you require those changes.
		 * @note
		 * The texture must have been created with TU_CPUCACHED usage otherwise this method will not return any data.
		 */
		void ReadCachedData(PixelData& data, u32 face = 0, u32 mipLevel = 0);

		/**	Returns properties that contain information about the texture. */
		const TextureProperties& GetProperties() const { return mProperties; }

		/**	Retrieves a core implementation of a texture usable only from the core thread. */
		SPtr<ct::Texture> GetCore() const;

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/

		/**
		 * Creates a new empty texture.
		 *
		 * @param[in]	desc  	Description of the texture to create.
		 */
		static HTexture Create(const TEXTURE_DESC& desc);

		/**
		 * Creates a new 2D or 3D texture initialized using the provided pixel data. Texture will not have any mipmaps.
		 *
		 * @param[in]	pixelData			Data to initialize the texture width.
		 * @param[in]	usage				Describes planned texture use.
		 * @param[in]	hwGammaCorrection	If true the texture data is assumed to have been gamma corrected and will be
		 *									converted back to linear space when sampled on GPU.
		 */
		static HTexture Create(const SPtr<PixelData>& pixelData, int usage = TU_DEFAULT, bool hwGammaCorrection = false);

		/** @name Internal
		 *  @{
		 */

		/** Same as create() excepts it creates a pointer to the texture instead of a texture handle. */
		static SPtr<Texture> CreatePtrInternal(const TEXTURE_DESC& desc);

		/** Same as create() excepts it creates a pointer to the texture instead of a texture handle. */
		static SPtr<Texture> CreatePtrInternal(const SPtr<PixelData>& pixelData, int usage = TU_DEFAULT, bool hwGammaCorrection = false);

		/** @} */

	protected:
		friend class TextureManager;

		Texture(const TEXTURE_DESC& desc);
		Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& pixelData);

		void Initialize() override;
		SPtr<ct::CoreObject> CreateCore() const override;

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
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup Resources-Internal
		 *  @{
		 */

		/**
		 * Core thread version of a bs::Texture.
		 *
		 * @note	Core thread.
		 */
		class B3D_CORE_EXPORT Texture : public CoreObject
		{
		public:
			Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& initData, GpuDeviceFlags deviceMask);

			virtual ~Texture() {}

			void Initialize() override;

			/**
			 * Locks the buffer for reading or writing.
			 *
			 * @param[in]	options 	Options for controlling what you may do with the locked data.
			 * @param[in]	mipLevel	(optional) Mipmap level to lock.
			 * @param[in]	face		(optional) Texture face to lock.
			 * @param[in]	deviceIdx	Index of the device whose memory to map. If the buffer doesn't exist on this device,
			 *							the method returns null.
			 * @param[in]	queueIdx	Device queue to perform the read/write operations on. See @ref queuesDoc.
			 *
			 * @note
			 * If you are just reading or writing one block of data use readData()/writeData() methods as they can be much faster
			 * in certain situations.
			 */
			PixelData Lock(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0);

			/**
			 * Unlocks a previously locked buffer. After the buffer is unlocked, any data returned by lock becomes invalid.
			 *
			 * @see	lock()
			 */
			void Unlock();

			/**
			 * Copies the contents a subresource in this texture to another texture. Texture format and size of the subresource
			 * must match.
			 *
			 * You are allowed to copy from a multisampled to non-multisampled surface, which will resolve the multisampled
			 * surface before copying.
			 *
			 * @param[in]	target				Texture that contains the destination subresource.
			 * @param[in]	desc				Structure used for customizing the copy operation.
			 * @param[in]	commandBuffer		Command buffer to queue the copy operation on. If null, main command buffer is
			 *									used.
			 */
			void Copy(const SPtr<Texture>& target, const TextureCopyInformation& desc = TextureCopyInformation::kDefault, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/**
			 * Sets all the pixels of the specified face and mip level to the provided value.
			 *
			 * @param[in]	value			Color to clear the pixels to.
			 * @param[in]	mipLevel		Mip level to clear.
			 * @param[in]	face			Face (array index or cubemap face) to clear.
			 * @param[in]	queueIdx		Device queue to perform the write operation on. See @ref queuesDoc.
			 */
			void Clear(const Color& value, u32 mipLevel = 0, u32 face = 0, u32 queueIdx = 0);

			/**
			 * Reads data from the texture buffer into the provided buffer.
			 *
			 * @param[out]	dest		Previously allocated buffer to read data into.
			 * @param[in]	mipLevel	(optional) Mipmap level to read from.
			 * @param[in]	face		(optional) Texture face to read from.
			 * @param[in]	deviceIdx	Index of the device whose memory to read. If the buffer doesn't exist on this device,
			 *							no data will be read.
			 * @param[in]	queueIdx	Device queue to perform the read operation on. See @ref queuesDoc.
			 */
			void ReadData(PixelData& dest, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0);

			/**
			 * Performs a non-blocking read operation. The GPU will execute the read when the command buffer reaches the execution point
			 * and the asynchronous operation will be signaled with the return value.
			 *
			 * @param[in]	mipLevel		(optional) Mipmap level to read from.
			 * @param[in]	face			(optional) Texture face to read from.
			 * @param[in]	deviceIndex		Index of the device whose memory to read. If the buffer doesn't exist on this device,
			 *								no data will be read.
			 * @param[in]	commandBuffer	Command buffer to queue the operation on. Main command buffer is used.
			 */
			virtual TAsyncOp<SPtr<PixelData>> ReadDataAsync(u32 mipLevel = 0, u32 face = 0, u32 deviceIndex = 0, const SPtr<CommandBuffer>& commandBuffer = nullptr);

			/**
			 * Writes data from the provided buffer into the texture buffer.
			 *
			 * @param[in]	src					Buffer to retrieve the data from.
			 * @param[in]	mipLevel			(optional) Mipmap level to write into.
			 * @param[in]	face				(optional) Texture face to write into.
			 * @param[in]	discardWholeBuffer	(optional) If true any existing texture data will be discard. This can improve
			 *									performance of the write operation.
			 * @param[in]	queueIdx			Device queue to perform the write operation on. See @ref queuesDoc.
			 */
			void WriteData(const PixelData& src, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, u32 queueIdx = 0);

			/**	Returns properties that contain information about the texture. */
			const TextureProperties& GetProperties() const { return mProperties; }

			/************************************************************************/
			/* 								STATICS		                     		*/
			/************************************************************************/

			/**
			 * @copydoc bs::Texture::Create(const TEXTURE_DESC&)
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			static SPtr<Texture> Create(const TEXTURE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/**
			 * @copydoc bs::Texture::Create(const SPtr<PixelData>&, int, bool)
			 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
			 */
			static SPtr<Texture> Create(const SPtr<PixelData>& pixelData, int usage = TU_DEFAULT, bool hwGammaCorrection = false, GpuDeviceFlags deviceMask = GDF_DEFAULT);

			/************************************************************************/
			/* 								TEXTURE VIEW                      		*/
			/************************************************************************/

			/**
			 * Requests a texture view for the specified mip and array ranges. Returns an existing view of one for the specified
			 * ranges already exists, otherwise creates a new one. You must release all views by calling ReleaseView() when done.
			 *
			 * @note	Core thread only.
			 */
			SPtr<TextureView> RequestView(const TextureSurface& surface, GpuViewUsage usage);

			/** Returns a plain white texture. */
			static SPtr<Texture> WHITE;

			/** Returns a plain black texture. */
			static SPtr<Texture> BLACK;

			/** Returns a plain normal map texture with normal pointing up (in Y direction). */
			static SPtr<Texture> NORMAL;

		protected:
			/** @copydoc Lock */
			virtual PixelData LockImpl(GpuLockOptions options, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) = 0;

			/** @copydoc Unlock */
			virtual void UnlockImpl() = 0;

			/** @copydoc Copy */
			virtual void CopyImpl(const SPtr<Texture>& target, const TextureCopyInformation& desc, const SPtr<CommandBuffer>& commandBuffer) = 0;

			/** @copydoc ReadData */
			virtual void ReadDataImpl(PixelData& dest, u32 mipLevel = 0, u32 face = 0, u32 deviceIdx = 0, u32 queueIdx = 0) = 0;

			/** @copydoc WriteData */
			virtual void WriteDataImpl(const PixelData& src, u32 mipLevel = 0, u32 face = 0, bool discardWholeBuffer = false, u32 queueIdx = 0) = 0;

			/** @copydoc Clear */
			virtual void ClearImpl(const Color& value, u32 mipLevel = 0, u32 face = 0, u32 queueIdx = 0);

			/************************************************************************/
			/* 								TEXTURE VIEW                      		*/
			/************************************************************************/

			/**	Creates a view of a specific subresource in a texture. */
			virtual SPtr<TextureView> CreateView(const TextureViewInformation& desc);

			/** Releases all internal texture view references. */
			void ClearBufferViews();

			UnorderedMap<TextureViewInformation, SPtr<TextureView>, TextureView::HashFunction, TextureView::EqualFunction> mTextureViews;
			TextureProperties mProperties;
			SPtr<PixelData> mInitData;
		};

		/** @} */
	} // namespace ct
} // namespace bs
