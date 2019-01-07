//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
			BS_RTTI_MEMBER_PLAIN(mCullDistanceFactor, 6)
		BS_END_RTTI_MEMBERS

	public:
		void onDeserializationEnded(IReflectable* obj, SerializationContext* context) override
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
