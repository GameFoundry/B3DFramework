//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsReflectionCubemapCache.h"
#include "BsFileSystem.h"
#include "BsIReflectable.h"
#include "BsRTTIType.h"
#include "BsFileSerializer.h"

namespace bs { namespace ct
{
	struct CachedTextureData : IReflectable
	{
		TextureType type;
		UINT32 numFaces;
		UINT32 numMips;
		Vector<SPtr<PixelData>> pixelData;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CachedTextureDataRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	class CachedTextureDataRTTI : public RTTIType <CachedTextureData, IReflectable, CachedTextureDataRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(type, 0)
			BS_RTTI_MEMBER_PLAIN(numFaces, 1)
			BS_RTTI_MEMBER_PLAIN(numMips, 2)
			BS_RTTI_MEMBER_REFLPTR_ARRAY(pixelData, 3)
		BS_END_RTTI_MEMBERS
	public:
		CachedTextureDataRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "CachedTextureData";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_CachedTextureData;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<CachedTextureData>();
		}
	};

	RTTITypeBase* CachedTextureData::getRTTIStatic()
	{
		return CachedTextureDataRTTI::instance();
	}

	RTTITypeBase* CachedTextureData::getRTTI() const
	{
		return getRTTIStatic();
	}

	ReflectionCubemapCache::~ReflectionCubemapCache()
	{
		for (auto& entry : mTextureInfos)
		{
			if(entry.second.needsSaving)
			{
				LOGWRN("Shutting down reflection cubemap cache manager but not all textures were saved on disk before "
					   "shutdown.");
				break;
			}
		}
	}

	void ReflectionCubemapCache::initCache(const Path& path)
	{
		mDataPath = path;

		auto visitFile = [&](const Path& filePath) -> bool
		{
			String uuid = filePath.getFilename(false);

			TextureInfo& texInfo = mTextureInfos[uuid];
			texInfo.texture = nullptr;
			texInfo.needsSaving = false;
			texInfo.dirty = false;

			return true;
		};

		FileSystem::iterate(path, visitFile, nullptr, false);
	}

	void ReflectionCubemapCache::notifyDirty(const String& uuid)
	{
		auto iterFind = mTextureInfos.find(uuid);
		if (iterFind != mTextureInfos.end())
			iterFind->second.dirty = true;
	}

	bool ReflectionCubemapCache::isDirty(const String& uuid) const
	{
		auto iterFind = mTextureInfos.find(uuid);
		if (iterFind != mTextureInfos.end())
			return iterFind->second.dirty;

		return true;
	}

	void ReflectionCubemapCache::setCachedTexture(const String& uuid, const SPtr<Texture>& texture)
	{
		TextureInfo& texInfo = mTextureInfos[uuid];
		texInfo.texture = texture;
		texInfo.needsSaving = true;
		texInfo.dirty = false;
	}

	SPtr<Texture> ReflectionCubemapCache::getCachedTexture(const String& uuid) const
	{
		auto iterFind = mTextureInfos.find(uuid);
		if (iterFind == mTextureInfos.end())
			return nullptr;

		TextureInfo& texInfo = iterFind->second;
		if (texInfo.texture != nullptr)
			return texInfo.texture;

		Path filePath = mDataPath + uuid + ".asset";
		if (!FileSystem::exists(filePath))
			return nullptr;

		FileDecoder fd(filePath);
		SPtr<CachedTextureData> textureData = std::static_pointer_cast<CachedTextureData>(fd.decode());

		if (textureData == nullptr || textureData->pixelData.size() == 0 || textureData->pixelData[0] == nullptr)
			return nullptr;

		TEXTURE_DESC desc;
		desc.type = textureData->type;
		desc.format = textureData->pixelData[0]->getFormat();
		desc.width = textureData->pixelData[0]->getWidth();
		desc.height = textureData->pixelData[0]->getHeight();
		desc.depth = textureData->pixelData[0]->getDepth();

		if (desc.type == TEX_TYPE_CUBE_MAP)
			desc.numArraySlices = textureData->numFaces / 6;
		else
			desc.numArraySlices = textureData->numFaces;

		desc.numMips = textureData->numMips;
		
		SPtr<Texture> texture = Texture::create(desc);
		for(UINT32 face = 0; face < textureData->numFaces; face++)
		{
			for(UINT32 mip = 0; mip < textureData->numMips; mip++)
			{
				UINT32 srcIdx = face * textureData->numMips + mip;
				if (srcIdx >= textureData->pixelData.size())
					continue;

				texture->writeData(*textureData->pixelData[srcIdx], mip, face, true);
			}
		}

		texInfo.texture = texture;
		return texture;
	}

	void ReflectionCubemapCache::unloadCachedTexture(const String& uuid)
	{
		auto iterFind = mTextureInfos.find(uuid);
		if (iterFind != mTextureInfos.end())
		{
			// Not allowed to unload if it requires saving (should only happen during development time)
			if (!iterFind->second.needsSaving)
				iterFind->second.texture = nullptr;
		}
	}

	void ReflectionCubemapCache::saveCache(const Path& path)
	{
		for(auto& entry : mTextureInfos)
		{
			TextureInfo& texInfo = entry.second;
			if (!texInfo.needsSaving)
				continue;

			auto& texProps = texInfo.texture->getProperties();
			
			SPtr<CachedTextureData> textureData = bs_shared_ptr_new<CachedTextureData>();
			textureData->type = texProps.getTextureType();
			textureData->numFaces = texProps.getNumFaces();
			textureData->numMips = texProps.getNumMipmaps() + 1;

			for (UINT32 face = 0; face < textureData->numFaces; face++)
			{
				for (UINT32 mip = 0; mip < textureData->numMips; mip++)
				{
					SPtr<PixelData> pixelData = texProps.allocBuffer(face, mip);
					texInfo.texture->readData(*pixelData, mip, face);

					textureData->pixelData.push_back(pixelData);
				}
			}

			Path filePath = path + entry.first + ".asset";
			FileEncoder fe(filePath);
			fe.encode(textureData.get());

			texInfo.needsSaving = false;
		}
	}
}}
