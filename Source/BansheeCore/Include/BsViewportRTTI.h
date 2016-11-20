//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsRTTIType.h"
#include "BsViewport.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ViewportRTTI : public RTTIType<Viewport, IReflectable, ViewportRTTI>
	{
	private:
		Rect2& getNormArea(Viewport* obj) { return obj->mNormArea; }
		void setNormArea(Viewport* obj, Rect2& val) { obj->mNormArea = val; }

		bool& getRequireColorClear(Viewport* obj) { return obj->mRequiresColorClear; }
		void setRequireColorClear(Viewport* obj, bool& val) { obj->mRequiresColorClear = val; }

		bool& getRequireDepthClear(Viewport* obj) { return obj->mRequiresDepthClear; }
		void setRequireDepthClear(Viewport* obj, bool& val) { obj->mRequiresDepthClear = val; }

		bool& getRequireStencilClear(Viewport* obj) { return obj->mRequiresStencilClear; }
		void setRequireStencilClear(Viewport* obj, bool& val) { obj->mRequiresStencilClear = val; }

		Color& getClearColor(Viewport* obj) { return obj->mClearColor; }
		void setClearColor(Viewport* obj, Color& val) { obj->mClearColor = val; }

		float& getDepthClearValue(Viewport* obj) { return obj->mDepthClearValue; }
		void setDepthClearValue(Viewport* obj, float& val) { obj->mDepthClearValue = val; }

		UINT16& getStencilClearValue(Viewport* obj) { return obj->mStencilClearValue; }
		void setStencilClearValue(Viewport* obj, UINT16& val) { obj->mStencilClearValue = val; }

		// TODO - Not saving a render target reference. Need to re-think the design on how to reference those. Likely as a handle to resource.
	public:
		ViewportRTTI()
		{
			addPlainField("mNormArea", 0, &ViewportRTTI::getNormArea, &ViewportRTTI::setNormArea);
			addPlainField("mRequiresColorClear", 1, &ViewportRTTI::getRequireColorClear, &ViewportRTTI::setRequireColorClear);
			addPlainField("mRequiresDepthClear", 2, &ViewportRTTI::getRequireDepthClear, &ViewportRTTI::setRequireDepthClear);
			addPlainField("mRequiresStencilClear", 3, &ViewportRTTI::getRequireStencilClear, &ViewportRTTI::setRequireStencilClear);
			addPlainField("mClearColor", 4, &ViewportRTTI::getClearColor, &ViewportRTTI::setClearColor);
			addPlainField("mDepthClearValue", 5, &ViewportRTTI::getDepthClearValue, &ViewportRTTI::setDepthClearValue);
			addPlainField("mStencilClearValue", 6, &ViewportRTTI::getStencilClearValue, &ViewportRTTI::setStencilClearValue);
		}

		void onDeserializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			Viewport* viewport = static_cast<Viewport*>(obj);
			viewport->initialize();
		}

		const String& getRTTIName() override
		{
			static String name = "Viewport";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_Viewport;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return Viewport::createEmpty();
		}
	};

	/** @} */
	/** @endcond */
}