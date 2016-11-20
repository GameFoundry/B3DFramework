//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**	Material manager handles material creation. */
	class BS_CORE_EXPORT MaterialManager : public Module<MaterialManager>
	{
	public:
		/**
		 * Creates a new material without any assigned shader.
		 *
		 * @note	Make sure to call Material::setShader() before using it.
		 */
		SPtr<Material> create() const;

		/** Creates a new material with the specified shader. */
		SPtr<Material> create(const HShader& shader) const;

		/**
		 * Creates a new empty material without initializing it.
		 *
		 * @note	You must manually call Material::initialize() after creation.
		 */
		SPtr<Material> createEmpty() const;
	};

	/** @} */
}