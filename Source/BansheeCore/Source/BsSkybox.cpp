//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSkybox.h"
#include "BsSkyboxRTTI.h"
#include "BsSceneObject.h"
#include "BsFrameAlloc.h"
#include "BsTexture.h"
#include "BsRenderer.h"
#include "BsUUID.h"

namespace bs
{
    SkyboxBase::SkyboxBase()
        : mIsActive(true)
    { }

    template <bool Core>
    TSkybox<Core>::TSkybox()
        : SkyboxBase()
    { }

    template class TSkybox<true>;
    template class TSkybox<false>;

    Skybox::Skybox()
    { }

    SPtr<ct::Skybox> Skybox::getCore() const
    {
        return std::static_pointer_cast<ct::Skybox>(mCoreSpecific);
    }

    SPtr<Skybox> Skybox::create()
    {
        Skybox* skybox = new (bs_alloc<Skybox>()) Skybox();
        SPtr<Skybox> skyboxPtr = bs_core_ptr<Skybox>(skybox);
        skyboxPtr->_setThisPtr(skyboxPtr);
        skyboxPtr->mUUID = UUIDGenerator::generateRandom();
        skyboxPtr->initialize();

        return skyboxPtr;
    }

    SPtr<ct::CoreObject> Skybox::createCore() const
    {
        ct::Skybox* skybox = new (bs_alloc<ct::Skybox>()) ct::Skybox();
        SPtr<ct::Skybox> skyboxPtr = bs_shared_ptr<ct::Skybox>(skybox);
        skyboxPtr->mUUID = mUUID;
        skyboxPtr->_setThisPtr(skyboxPtr);

        return skyboxPtr;
    }

    CoreSyncData Skybox::syncToCore(FrameAlloc* allocator)
    {
        UINT32 size = 0;
        size += rttiGetElemSize(mIsActive);
        size += rttiGetElemSize(sizeof(SPtr<ct::Texture>));
        size += rttiGetElemSize(mUUID);

        UINT8* buffer = allocator->alloc(size);

        char* dataPtr = (char*)buffer;
        dataPtr = rttiWriteElem(mIsActive, dataPtr);
        dataPtr = rttiWriteElem(mUUID, dataPtr);

        SPtr<ct::Texture>* texture = new (dataPtr) SPtr<ct::Texture>();
        if (mTexture.isLoaded(false))
            *texture = mTexture->getCore();
        else
            *texture = nullptr;

        dataPtr += sizeof(SPtr<ct::Texture>);

        return CoreSyncData(buffer, size);
    }

    void Skybox::_markCoreDirty()
    {
        markCoreDirty();
    }

    RTTITypeBase* Skybox::getRTTIStatic()
    {
        return SkyboxRTTI::instance();
    }

    RTTITypeBase* Skybox::getRTTI() const
    {
        return Skybox::getRTTIStatic();
    }

    namespace ct
    {
        Skybox::Skybox()
        { }

        Skybox::~Skybox()
        {
            gRenderer()->notifySkyboxRemoved(this);
        }

        void Skybox::initialize()
        {
            gRenderer()->notifySkyboxAdded(this);

            CoreObject::initialize();
        }

        void Skybox::syncToCore(const CoreSyncData& data)
        {
            char* dataPtr = (char*)data.getBuffer();

            bool oldIsActive = mIsActive;

            dataPtr = rttiReadElem(mIsActive, dataPtr);
            dataPtr = rttiReadElem(mUUID, dataPtr);

            SPtr<Texture>* texture = (SPtr<Texture>*)dataPtr;

            mTexture = *texture;
            texture->~SPtr<Texture>();
            dataPtr += sizeof(SPtr<Texture>);

            if (oldIsActive != mIsActive)
            {
                if (mIsActive)
                    gRenderer()->notifySkyboxAdded(this);
                else
                    gRenderer()->notifySkyboxRemoved(this);
            }
            else
            {
                gRenderer()->notifySkyboxRemoved(this);
                gRenderer()->notifySkyboxAdded(this);
            }
        }
    }
}