//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Material/BsShaderInclude.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT ShaderIncludeRTTI : public RTTIType <ShaderInclude, Resource, ShaderIncludeRTTI>
	{
	private:
		String& getString(ShaderInclude* obj) { return obj->mString; }
		void setString(ShaderInclude* obj, String& val) { obj->mString = val; }
	public:
		ShaderIncludeRTTI()
		{
			addPlainField("mString", 0, &ShaderIncludeRTTI::getString, &ShaderIncludeRTTI::setString);
		}

		const String& getRTTIName() override
		{
			static String name = "ShaderInclude";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ShaderInclude;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ShaderInclude::_createPtr(""); // Initial string doesn't matter, it'll get overwritten
		}
	};

	/** @} */
	/** @endcond */
}