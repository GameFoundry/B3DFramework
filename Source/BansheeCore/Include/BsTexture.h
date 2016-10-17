//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsResource.h"
#include "BsHardwareBuffer.h"
#include "BsPixelUtil.h"
#include "BsTextureView.h"

namespace BansheeEngine 
{
	/** @addtogroup Resources
	 *  @{
	 */

	/**	Flags that describe how is a texture used. */
    enum TextureUsage
    {
		TU_STATIC = GBU_STATIC, /**< A regular texture that is not often or ever updated from the CPU. */
		TU_DYNAMIC = GBU_DYNAMIC, /**< A regular texture that is often updated by the CPU. */
		TU_RENDERTARGET = 0x200, /**< Texture that can be rendered to by the GPU. */
		TU_DEPTHSTENCIL = 0x400, /**< Texture used as a depth/stencil buffer by the GPU. */
		TU_LOADSTORE = 0x800, /**< Texture that allows load/store operations from the GPU program. */
		TU_CPUCACHED = 0x1000, /**< Ensures all texture data will also be cached in system memory. */
		TU_DEFAULT = TU_STATIC
    };

	/**	Available texture types. */
    enum TextureType
    {
		TEX_TYPE_1D = 1, /**< One dimensional texture. Just a row of pixels. */
		TEX_TYPE_2D = 2, /**< Two dimensional texture. */
		TEX_TYPE_3D = 3, /**< Three dimensional texture. */
		TEX_TYPE_CUBE_MAP = 4 /**< Texture consisting out of six 2D textures describing an inside of a cube. Allows special sampling. */
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
		TextureType type = TEX_TYPE_2D;

		/** Format of pixels in the texture. */
		PixelFormat format = PF_R8G8B8A8;

		/** Width of the texture in pixels. */
		UINT32 width = 1;

		/** Height of the texture in pixels. */
		UINT32 height = 1;

		/** Depth of the texture in pixels (Must be 1 for 2D textures). */
		UINT32 depth = 1;

		/** Number of mip-maps the texture has. This number excludes the full resolution map. */
		UINT32 numMips = 0;

		/** Describes how the caller plans on using the texture in the pipeline. */
		INT32 usage = TU_DEFAULT;

		/** 
		 * If true the texture data is assumed to have been gamma corrected and will be converted back to linear space when
		 * sampled on GPU.
		 */
		bool hwGamma = false; 

		/** Number of samples per pixel. Set to 1 or 0 to use the default of a single sample per pixel. */
		UINT32 numSamples = 0;

		/** Number of texture slices to create if creating a texture array. Ignored for 3D textures. */
		UINT32 numArraySlices = 1;
	};

	/** Properties of a Texture. Shared between sim and core thread versions of a Texture. */
	class BS_CORE_EXPORT TextureProperties
	{
	public:
		TextureProperties();
		TextureProperties(const TEXTURE_DESC& desc);

		/**	Gets the type of texture. */
        TextureType getTextureType() const { return mDesc.type; }

        /**
		 * Gets the number of mipmaps to be used for this texture. This number excludes the top level map (which is always
		 * assumed to be present).
         */
        UINT32 getNumMipmaps() const {return mDesc.numMips;}

		/** Gets whether this texture will be set up so that on sampling it, hardware gamma correction is applied. */
		bool isHardwareGammaEnabled() const { return mDesc.hwGamma; }

		/**	Gets the number of samples used for multisampling (0 if multisampling is not used). */
		UINT32 getNumSamples() const { return mDesc.numSamples; }

        /**	Returns the height of the texture.  */
        UINT32 getHeight() const { return mDesc.height; }

        /**	Returns the width of the texture. */
        UINT32 getWidth() const { return mDesc.width; }

        /**	Returns the depth of the texture (only applicable for 3D textures). */
        UINT32 getDepth() const { return mDesc.depth; }

        /**	Returns texture usage (TextureUsage) of this texture. */
        int getUsage() const { return mDesc.usage; }

		/**	Returns the pixel format for the texture surface. */
		PixelFormat getFormat() const { return mDesc.format; }

        /**	Returns true if the texture has an alpha layer. */
        bool hasAlpha() const;

        /**	Returns the number of faces this texture has. */
        UINT32 getNumFaces() const;

		/** Returns the number of array slices of the texture (if the texture is an array texture). */
		UINT32 getNumArraySlices() const { return mDesc.numArraySlices; }

		/**
		 * Maps a sub-resource index to an exact face and mip level. Sub-resource indexes are used when reading or writing
		 * to the resource.
		 * 			
		 * @note	
		 * Sub-resource index is only valid for the instance it was created on. You cannot use a sub-resource index from a
		 * different texture and expect to get valid result. Modifying the resource so the number of sub-resources changes
		 * invalidates all sub-resource indexes.
		 */
		void mapFromSubresourceIdx(UINT32 subresourceIdx, UINT32& face, UINT32& mip) const;

		/**
		 * Map a face and a mip level to a sub-resource index you can use for updating or reading a specific sub-resource.
		 * 			
		 * @note	
		 * Generated sub-resource index is only valid for the instance it was created on. Modifying the resource so the 
		 * number of sub-resources changes, invalidates all sub-resource indexes.
		 */
		UINT32 mapToSubresourceIdx(UINT32 face, UINT32 mip) const;

		/**
		 * Allocates a buffer you may use for storage when reading or writing a sub-resource. You need to allocate such a 
		 * buffer if you are calling readSubresource().
		 *
		 * You can retrieve a sub-resource index by calling mapToSubresourceIdx().
		 * 			
		 * @note	Thread safe.
		 */
		SPtr<PixelData> allocateSubresourceBuffer(UINT32 subresourceIdx) const;

	protected:
		friend class TextureRTTI;
		friend class Texture;

		TEXTURE_DESC mDesc;
	};

	/**
	 * Abstract class representing a texture. Specific render systems have their own Texture implementations. Internally
	 * represented as one or more surfaces with pixels in a certain number of dimensions, backed by a hardware buffer.
	 *
	 * @note	Sim thread.
	 */
    class BS_CORE_EXPORT Texture : public Resource
    {
    public:
		/**
		 * Updates the texture with new data. The actual write will be queued for later execution on the core thread. 
		 * Provided data buffer will be locked until the operation completes.
		 *
		 * @param[in]	accessor			Accessor to queue the operation on.
		 * @param[in]	subresourceIdx		Index of the subresource to write. Retrieved from 
		 *									TextureProperties::mapToSubresourceIdx().
		 * @param[in]	data				Pixel data to write. User must ensure it is in format and size compatible with 
		 *									the texture.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be 
		 *									discarded. This can make the operation faster. Resources with certain buffer 
		 *									types might require this flag to be in a specific state otherwise the operation 
		 *									will fail.
		 * @return							Async operation object you can use to track operation completion.
		 */
		AsyncOp writeSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<PixelData>& data, 
			bool discardEntireBuffer);

		/**
		 * Reads internal texture data to the provided previously allocated buffer. The read is queued for execution on the
		 * core thread and not executed immediately. Provided data buffer will be locked until the operation completes.
		 *
		 * @param[in]	accessor			Accessor to queue the operation on.
		 * @param[in]	subresourceIdx		Index of the subresource to read. Retrieved from
		 *									TextureProperties::mapToSubresourceIdx().
		 * @param[out]	data				Pre-allocated buffer of proper size and format where data will be read to. You 
		 *									can use TextureProperties::allocateSubresourceBuffer() to allocate a valid 
		 *									buffer.
		 * @return							Async operation object you can use to track operation completion.
		 *
		 * @see		TextureCore::readSubresource
		 */
		AsyncOp readSubresource(CoreAccessor& accessor, UINT32 subresourceIdx, const SPtr<PixelData>& data);

		/**
		 * Reads data from the cached system memory texture buffer into the provided buffer. 
		 * 		  
		 * @param[out]	dest		Previously allocated buffer to read data into.
		 * @param[in]	mipLevel	(optional) Mipmap level to read from.
		 * @param[in]	face		(optional) Texture face to read from.
		 *
		 * @note	
		 * The data read is the cached texture data. Any data written to the texture from the GPU or core thread will not 
		 * be reflected in this data. Use readSubresource() if you require those changes.
		 * @note
		 * The texture must have been created with TU_CPUCACHED usage otherwise this method will not return any data.
		 */
		void readData(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0);

		/**	Returns properties that contain information about the texture. */
		const TextureProperties& getProperties() const { return mProperties; }

		/**	Retrieves a core implementation of a texture usable only from the core thread. */
		SPtr<TextureCore> getCore() const;

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/

		/**
		 * Creates a new empty texture.
		 *
		 * @param[in]	desc  	Description of the texture to create.
		 */
		static HTexture create(const TEXTURE_DESC& desc);

		/**
		 * Creates a new 2D or 3D texture initialized using the provided pixel data. Texture will not have any mipmaps.
		 *
		 * @param[in]	pixelData			Data to initialize the texture width.
		 * @param[in]	usage				Describes planned texture use.
		 * @param[in]	hwGammaCorrection	If true the texture data is assumed to have been gamma corrected and will be
		 *									converted back to linear space when sampled on GPU.
		 */
		static HTexture create(const SPtr<PixelData>& pixelData, int usage = TU_DEFAULT, bool hwGammaCorrection = false);

		/** @name Internal 
		 *  @{
		 */

		/**
		 * @copydoc	create(const TEXTURE_DESC&)
		 *
		 * @note	Internal method. Creates a texture pointer without a handle. Use create() for normal usage.
		 */
		static SPtr<Texture> _createPtr(const TEXTURE_DESC& desc);

		/**
		 * @copydoc	create(const SPtr<PixelData>&, int, bool)
		 *
		 * @note	Internal method. Creates a texture pointer without a handle. Use create() for normal usage.
		 */
		static SPtr<Texture> _createPtr(const SPtr<PixelData>& pixelData, int usage = TU_DEFAULT, 
			bool hwGammaCorrection = false);

		/** @} */

    protected:
		friend class TextureManager;

		Texture(const TEXTURE_DESC& desc);
		Texture(const TEXTURE_DESC& desc, const SPtr<PixelData>& pixelData);

		/** @copydoc Resource::initialize */
		void initialize() override;

		/** @copydoc CoreObject::createCore */
		SPtr<CoreObjectCore> createCore() const override;

		/** Calculates the size of the texture, in bytes. */
		UINT32 calculateSize() const;

		/**
		 * Creates buffers used for caching of CPU texture data.
		 *
		 * @note	Make sure to initialize all texture properties before calling this.
		 */
		void createCPUBuffers();

		/**	Updates the cached CPU buffers with new data. */
		void updateCPUBuffers(UINT32 subresourceIdx, const PixelData& data);

	protected:
		Vector<SPtr<PixelData>> mCPUSubresourceData;
		TextureProperties mProperties;
		mutable SPtr<PixelData> mInitData;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		Texture(); // Serialization only

		friend class TextureRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
    };

	/** @} */

	/** @addtogroup Resources-Internal
	 *  @{
	 */

	/**
	 * Core thread version of a Texture.
	 *
	 * @note	Core thread.
	 */
	class BS_CORE_EXPORT TextureCore : public CoreObjectCore
	{
	public:
		TextureCore(const TEXTURE_DESC& desc, const SPtr<PixelData>& initData, GpuDeviceFlags deviceMask);
		virtual ~TextureCore() {}


		/** @copydoc CoreObjectCore::initialize */
		void initialize() override;

		/**
		 * Updates a part of the texture with the provided data.
		 *
		 * @param[in]	subresourceIdx		Index of the subresource to update, if the texture has more than one.
		 * @param[in]	data				Data to update the texture with.
		 * @param[in]	discardEntireBuffer When true the existing contents of the resource you are updating will be 
		 *									discarded. This can make the operation faster. Resources with certain buffer 
		 *									types might require this flag to be in a specific state otherwise the operation 
		 *									will fail.
		 */
		virtual void writeSubresource(UINT32 subresourceIdx, const PixelData& data, bool discardEntireBuffer);

		/**
		 * Reads a part of the current resource into the provided @p data parameter. Data buffer needs to be pre-allocated.
		 *
		 * @param[in]	subresourceIdx		Index of the subresource to update, if the texture has more than one.
		 * @param[out]	data				Buffer that will receive the data. Should be allocated with 
		 *									allocateSubresourceBuffer() to ensure it is of valid type and size.
		 */
		virtual void readSubresource(UINT32 subresourceIdx, PixelData& data);

		/**
		 * Locks the buffer for reading or writing.
		 *
		 * @param[in]	options 	Options for controlling what you may do with the locked data.
		 * @param[in]	mipLevel	(optional) Mipmap level to lock.
		 * @param[in]	face		(optional) Texture face to lock.					
		 * @return					Pointer to the buffer data. Only valid until you call unlock().
		 * 			
		 * @note	
		 * If you are just reading or writing one block of data use readData()/writeData() methods as they can be much faster
		 * in certain situations.
		 */
		PixelData lock(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0);

		/** 
		 * Unlocks a previously locked buffer. After the buffer is unlocked, any data returned by lock becomes invalid. 
		 *
		 * @see	lock()
		 */
		void unlock();

		/**
		 * Copies the contents a subresource in this texture to another texture. Texture format and size of the subresource
		 * must match.
		 *
		 * You are allowed to copy from a multisampled to non-multisampled surface, which will resolve the multisampled
		 * surface before copying.
		 *
		 * @param[in]	srcSubresourceIdx	Index of the subresource to copy from.
		 * @param[in]	destSubresourceIdx	Index of the subresource to copy to.
		 * @param[in]	target				Texture that contains the destination subresource.
		 */
		void copy(UINT32 srcSubresourceIdx, UINT32 destSubresourceIdx, const SPtr<TextureCore>& target);

		/**
		 * Reads data from the texture buffer into the provided buffer.
		 * 		  
		 * @param[out]	dest		Previously allocated buffer to read data into.
		 * @param[in]	mipLevel	(optional) Mipmap level to read from.
		 * @param[in]	face		(optional) Texture face to read from.
		 */
		virtual void readData(PixelData& dest, UINT32 mipLevel = 0, UINT32 face = 0) = 0;

		/**
		 * Writes data from the provided buffer into the texture buffer.
		 * 		  
		 * @param[in]	src					Buffer to retrieve the data from.
		 * @param[in]	mipLevel			(optional) Mipmap level to write into.
		 * @param[in]	face				(optional) Texture face to write into.
		 * @param[in]	discardWholeBuffer	(optional) If true any existing texture data will be discard. This can improve 
		 *									performance of the write operation.
		 */
		virtual void writeData(const PixelData& src, UINT32 mipLevel = 0, UINT32 face = 0, bool discardWholeBuffer = false) = 0;

		/**	Returns properties that contain information about the texture. */
		const TextureProperties& getProperties() const { return mProperties; }

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/

		/** 
		 * @copydoc Texture::create(const TEXTURE_DESC&) 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<TextureCore> create(const TEXTURE_DESC& desc, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/** 
		 * @copydoc Texture::create(const SPtr<PixelData>&, int, bool) 
		 * @param[in]	deviceMask		Mask that determines on which GPU devices should the object be created on.
		 */
		static SPtr<TextureCore> create(const SPtr<PixelData>& pixelData, int usage = TU_DEFAULT, 
			bool hwGammaCorrection = false, GpuDeviceFlags deviceMask = GDF_DEFAULT);

		/************************************************************************/
		/* 								TEXTURE VIEW                      		*/
		/************************************************************************/

		/**
		 * Requests a texture view for the specified mip and array ranges. Returns an existing view of one for the specified
		 * ranges already exists, otherwise creates a new one. You must release all views by calling releaseView() when done.
		 *
		 * @note	Core thread only.
		 */
		static SPtr<TextureView> requestView(const SPtr<TextureCore>& texture, UINT32 mostDetailMip, UINT32 numMips,
			UINT32 firstArraySlice, UINT32 numArraySlices, GpuViewUsage usage);

		/**
		 * Releases the view. View won't actually get destroyed until all references to it are released.
		 *
		 * @note	Core thread only.
		 */
		static void releaseView(const SPtr<TextureView>& view);

		/** Returns a plain white texture. */
		static SPtr<TextureCore> WHITE;

		/** Returns a plain black texture. */
		static SPtr<TextureCore> BLACK;

		/** Returns a plain normal map texture with normal pointing up (in Y direction). */
		static SPtr<TextureCore> NORMAL;
	protected:
		/** @copydoc lock */
		virtual PixelData lockImpl(GpuLockOptions options, UINT32 mipLevel = 0, UINT32 face = 0) = 0;

		/** @copydoc unlock */
		virtual void unlockImpl() = 0;

		/** 
		 * API specific implementation of copy(). 
		 *
		 * @param[in]	srcFace			Face index to copy from.
		 * @param[in]	srcMipLevel		Mip level to copy from.
		 * @param[in]	destFace		Face index to copy to.
		 * @param[in]	destMipLevel	Mip level to copy to.
		 * @param[in]	target			Texture to copy to.
		 */
		virtual void copyImpl(UINT32 srcFace, UINT32 srcMipLevel, UINT32 destFace, UINT32 destMipLevel, 
			const SPtr<TextureCore>& target) = 0;

		/************************************************************************/
		/* 								TEXTURE VIEW                      		*/
		/************************************************************************/

		/**	Creates a new empty/undefined texture view. */
		virtual SPtr<TextureView> createView(const SPtr<TextureCore>& texture, const TEXTURE_VIEW_DESC& desc);

		/**
		 * Releases all internal texture view references. Views won't get destroyed if there are external references still 
		 * held.
		 */
		void clearBufferViews();

		/** Holds a single texture view with a usage reference count. */
		struct TextureViewReference
		{
			TextureViewReference(SPtr<TextureView> _view)
				:view(_view), refCount(0)
			{ }

			SPtr<TextureView> view;
			UINT32 refCount;
		};

		UnorderedMap<TEXTURE_VIEW_DESC, TextureViewReference*, TextureView::HashFunction, TextureView::EqualFunction> mTextureViews;
		TextureProperties mProperties;
		SPtr<PixelData> mInitData;
	};

	/** @} */
}