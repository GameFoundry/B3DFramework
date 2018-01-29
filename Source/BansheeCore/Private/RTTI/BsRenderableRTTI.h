//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Renderer/BsRenderable.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_CORE_EXPORT RenderableRTTI : public RTTIType<Renderable, IReflectable, RenderableRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(mTransform, 0)
			BS_RTTI_MEMBER_PLAIN(mActive, 1)
			BS_RTTI_MEMBER_PLAIN(mMobility, 2)
			BS_RTTI_MEMBER_REFL(mMesh, 3)
			BS_RTTI_MEMBER_PLAIN(mLayer, 4)
			BS_RTTI_MEMBER_REFL_ARRAY(mMaterials, 5)
		BS_END_RTTI_MEMBERS

	public:
		RenderableRTTI()
			:mInitMembers(this)
		{ }

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