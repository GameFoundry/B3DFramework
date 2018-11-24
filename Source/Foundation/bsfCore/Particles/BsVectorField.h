//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Resources/BsResource.h"
#include "Math/BsVector3I.h"
#include "Math/BsAABox.h"
#include "Importer/BsSpecificImporter.h"

namespace bs 
{
	namespace ct { class VectorField; }

	/** @addtogroup Particles
	 *  @{
	 */

	/** Descriptor structure used for initialization of a VectorField. */
	struct BS_SCRIPT_EXPORT(m:Particles,pl:true,n:VectorFieldOptions) VECTOR_FIELD_DESC
	{
		/** Number of entries in the vector field along the X axis. */
		UINT32 countX = 1;

		/** Number of entries in the vector field along the Y axis. */
		UINT32 countY = 1;

		/** Number of entries in the vector field along the Z axis. */
		UINT32 countZ = 1;

		/** Spatial bounds of the vector field. */
		AABox bounds = AABox::BOX_EMPTY;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	namespace detail
	{
		/** Common functionality for both the sim and core thread variants of VectorField. */
		template<bool Core>
		class BS_CORE_EXPORT TVectorField
		{
		public:
			using TextureType = SPtr<CoreVariantType<Texture, Core>>;

			TVectorField() = default;
			TVectorField(const VECTOR_FIELD_DESC& desc)
				:mDesc(desc)
			{ }
			virtual ~TVectorField() = default;

			/** Returns the internal texture representing the vector field. */
			TextureType getTexture() const { return mTexture; }

			/** Returns a structure describing the properties of the object. */
			const VECTOR_FIELD_DESC& getDesc() const { return mDesc; }

		protected:
			VECTOR_FIELD_DESC mDesc;
			TextureType mTexture;
		};
	}

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/** 
	 * Represents a three dimensional field of vectors. It is represented by spatial bounds which are split into a grid
	 * of values with user-defined density, where each grid cell is assigned a vector.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Particles) VectorField : public Resource, public detail::TVectorField<false>
	{
	public:
		/**	Retrieves a core implementation of a vector field usable only from the core thread. */
		SPtr<ct::VectorField> getCore() const;

		/************************************************************************/
		/* 								STATICS		                     		*/
		/************************************************************************/

		/**
		 * Creates a new vector field.
		 *
		 * @param[in]	desc  	Description of the vector field to create.
		 * @param[in]	values	Values to assign to the vector field. Number of entries must match 
		 *						countX * countY * countZ.
		 */
		BS_SCRIPT_EXPORT(ec:T)
		static HVectorField create(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		/** @name Internal 
		 *  @{
		 */

		/** Same as create() excepts it creates a pointer to the vector field instead of a handle. */
		static SPtr<VectorField> _createPtr(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		/** Creates the resource without initializing it. */
		static SPtr<VectorField> _createEmpty();

		/** @} */

	protected:
		VectorField(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		/** @copydoc CoreObject::createCore */
		SPtr<ct::CoreObject> createCore() const override;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		VectorField() = default; // Serialization only

		friend class VectorFieldRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup Particles-Internal
		 *  @{
		 */

		/** Core thread version of a bs::VectorField. */
		class BS_CORE_EXPORT VectorField : public CoreObject, public detail::TVectorField<true>
		{
		public:
			VectorField(const VECTOR_FIELD_DESC& desc, const SPtr<Texture>& texture);
		};

		/** @} */
	}

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/**	Imports vector fields from Fluid Grid ASCII (.fga) files. */
	class BS_CORE_EXPORT FGAImporter : public SpecificImporter
	{
	public:
		/** @copydoc SpecificImporter::isExtensionSupported */
		bool isExtensionSupported(const String& ext) const override;

		/** @copydoc SpecificImporter::isMagicNumberSupported */
		bool isMagicNumberSupported(const UINT8* magicNumPtr, UINT32 numBytes) const override;

		/** @copydoc SpecificImporter::import */
		SPtr<Resource> import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;
	};

	/** @} */
}
