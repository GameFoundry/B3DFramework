//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsLightProbeCache.h"
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

	LightProbeCache::~LightProbeCache()
	{
		for (auto& entry : mProbeInfos)
		{
			if(entry.second.radiance.needsSaving || entry.second.irradiance.needsSaving)
			{
				LOGWRN("Shutting down reflection cubemap cache manager but not all textures were saved on disk before "
					   "shutdown.");
				break;
			}
		}
	}

	void LightProbeCache::initCache(const Path& path)
	{
		mDataPath = path;

		auto visitFile = [&](const Path& filePath) -> bool
		{
			String uuid = filePath.getFilename(false);
			mProbeInfos.insert(std::make_pair(uuid, ProbeInfo()));

			return true;
		};

		FileSystem::iterate(path, visitFile, nullptr, false);
	}

	void LightProbeCache::notifyDirty(const String& uuid)
	{
		auto iterFind = mProbeInfos.find(uuid);
		if (iterFind != mProbeInfos.end())
		{
			iterFind->second.radiance.dirty = true;
			iterFind->second.irradiance.dirty = true;
		}
	}

	bool LightProbeCache::isRadianceDirty(const String& uuid) const
	{
		auto iterFind = mProbeInfos.find(uuid);
		if (iterFind != mProbeInfos.end())
			return iterFind->second.radiance.dirty;

		return true;
	}

	bool LightProbeCache::isIrradianceDirty(const String& uuid) const
	{
		auto iterFind = mProbeInfos.find(uuid);
		if (iterFind != mProbeInfos.end())
			return iterFind->second.irradiance.dirty;

		return true;
	}

	void LightProbeCache::setCachedRadianceTexture(const String& uuid, const SPtr<Texture>& texture)
	{
		ProbeInfo& probeInfo = mProbeInfos[uuid];
		TextureInfo& texInfo = probeInfo.radiance;
		texInfo.texture = texture;
		texInfo.needsSaving = true;
		texInfo.dirty = false;
	}

	void LightProbeCache::setCachedIrradianceTexture(const String& uuid, const SPtr<Texture>& texture)
	{
		ProbeInfo& probeInfo = mProbeInfos[uuid];
		TextureInfo& texInfo = probeInfo.irradiance;
		texInfo.texture = texture;
		texInfo.needsSaving = true;
		texInfo.dirty = false;
	}

	SPtr<Texture> LightProbeCache::getCachedRadianceTexture(const String& uuid) const
	{
		auto iterFind = mProbeInfos.find(uuid);
		if (iterFind == mProbeInfos.end())
			return nullptr;

		TextureInfo& texInfo = iterFind->second.radiance;
		if (texInfo.texture != nullptr)
			return texInfo.texture;

		Path filePath = mDataPath + "R_" + uuid + ".asset";
		texInfo.texture = loadCachedTexture(filePath);
		
		return texInfo.texture;
	}

	SPtr<Texture> LightProbeCache::getCachedIrradianceTexture(const String& uuid) const
	{
		auto iterFind = mProbeInfos.find(uuid);
		if (iterFind == mProbeInfos.end())
			return nullptr;

		TextureInfo& texInfo = iterFind->second.irradiance;
		if (texInfo.texture != nullptr)
			return texInfo.texture;

		Path filePath = mDataPath + "IRR_" + uuid + ".asset";
		texInfo.texture = loadCachedTexture(filePath);

		return texInfo.texture;
	}

	void LightProbeCache::unloadCachedTexture(const String& uuid)
	{
		auto iterFind = mProbeInfos.find(uuid);
		if (iterFind != mProbeInfos.end())
		{
			// Not allowed to unload if it requires saving (should only happen during development time)
			if (!iterFind->second.radiance.needsSaving)
				iterFind->second.radiance.texture = nullptr;

			if (!iterFind->second.irradiance.needsSaving)
				iterFind->second.irradiance.texture = nullptr;
		}
	}

	void LightProbeCache::saveCache(const Path& path)
	{
		for(auto& entry : mProbeInfos)
		{
			ProbeInfo& probeInfo = entry.second;

			Path radiancePath = path + "R_" + entry.first + ".asset";
			saveCachedTexture(probeInfo.radiance, radiancePath);
			probeInfo.radiance.needsSaving = false;

			Path irradiancePath = path + "IRRR_" + entry.first + ".asset";
			saveCachedTexture(probeInfo.irradiance, irradiancePath);
			probeInfo.irradiance.needsSaving = false;
		}
	}

	SPtr<Texture> LightProbeCache::loadCachedTexture(const Path& path) const
	{
		if (!FileSystem::exists(path))
			return nullptr;

		FileDecoder fd(path);
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
		for (UINT32 face = 0; face < textureData->numFaces; face++)
		{
			for (UINT32 mip = 0; mip < textureData->numMips; mip++)
			{
				UINT32 srcIdx = face * textureData->numMips + mip;
				if (srcIdx >= textureData->pixelData.size())
					continue;

				texture->writeData(*textureData->pixelData[srcIdx], mip, face, true);
			}
		}

		return texture;
	}

	void LightProbeCache::saveCachedTexture(const TextureInfo& texInfo, const Path& path)
	{
		if (!texInfo.needsSaving)
			return;

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
;
		FileEncoder fe(path);
		fe.encode(textureData.get());
	}
}}
