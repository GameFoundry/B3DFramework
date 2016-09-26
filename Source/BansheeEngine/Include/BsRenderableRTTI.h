//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsRTTIType.h"
#include "BsRenderable.h"

namespace BansheeEngine
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT RenderableRTTI : public RTTIType<Renderable, IReflectable, RenderableRTTI>
	{
	private:
		HMesh& getMesh(Renderable* obj) { return obj->mMesh; }
		void setMesh(Renderable* obj, HMesh& val) { obj->mMesh = val; }

		UINT64& getLayer(Renderable* obj) { return obj->mLayer; }
		void setLayer(Renderable* obj, UINT64& val) { obj->mLayer = val; }

		HMaterial& getMaterial(Renderable* obj, UINT32 idx) { return obj->mMaterials[idx]; }
		void setMaterial(Renderable* obj, UINT32 idx, HMaterial& val) { obj->setMaterial(idx, val); }
		UINT32 getNumMaterials(Renderable* obj) { return (UINT32)obj->mMaterials.size(); }
		void setNumMaterials(Renderable* obj, UINT32 num) { obj->mMaterials.resize(num); }

	public:
		RenderableRTTI()
		{
			addReflectableField("mMesh", 0, &RenderableRTTI::getMesh, &RenderableRTTI::setMesh);
			addPlainField("mLayer", 1, &RenderableRTTI::getLayer, &RenderableRTTI::setLayer);
			addReflectableArrayField("mMaterials", 2, &RenderableRTTI::getMaterial, 
				&RenderableRTTI::getNumMaterials, &RenderableRTTI::setMaterial, &RenderableRTTI::setNumMaterials);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			// Note: Since this is a CoreObject I should call initialize() right after deserialization,
			// but since this specific type is used in Components we delay initialization until Component
			// itself does it. Keep this is mind in case this ever needs to be deserialized for non-Component 
			// purposes (you'll need to call initialize manually).
		}

		const String& getRTTIName() override
		{
			static String name = "Renderable";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Renderable;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Renderable::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}