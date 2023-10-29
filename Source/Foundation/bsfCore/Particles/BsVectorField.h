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
	namespace ct
	{
		class VectorField;
	}

	/** @addtogroup Particles
	 *  @{
	 */

	/** Descriptor structure used for initialization of a VectorField. */
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Particles), ExportAsStruct(true), ExportName(VectorFieldOptions)) VECTOR_FIELD_DESC
	{
		/** Number of entries in the vector field along the X axis. */
		u32 CountX = 1;

		/** Number of entries in the vector field along the Y axis. */
		u32 CountY = 1;

		/** Number of entries in the vector field along the Z axis. */
		u32 CountZ = 1;

		/** Spatial bounds of the vector field. */
		AABox Bounds = AABox::kBoxEmpty;
	};

	/** @} */

	/** @addtogroup Implementation
	 *  @{
	 */

	/** Common functionality for both the sim and core thread variants of VectorField. */
	template <bool IsRenderProxy>
	class B3D_CORE_EXPORT TVectorField
	{
	public:
		using TextureType = SPtr<CoreVariantType<Texture, IsRenderProxy>>;

		TVectorField() = default;

		TVectorField(const VECTOR_FIELD_DESC& desc)
			: mDesc(desc)
		{}

		virtual ~TVectorField() = default;

		/** Returns the internal texture representing the vector field. */
		TextureType GetTexture() const { return mTexture; }

		/** Returns a structure describing the properties of the object. */
		const VECTOR_FIELD_DESC& GetDesc() const { return mDesc; }

	protected:
		VECTOR_FIELD_DESC mDesc;
		TextureType mTexture;
	};

	/** @} */

	/** @addtogroup Particles
	 *  @{
	 */

	/**
	 * Represents a three dimensional field of vectors. It is represented by spatial bounds which are split into a grid
	 * of values with user-defined density, where each grid cell is assigned a vector.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) VectorField : public Resource, public TVectorField<false>
	{
	public:
		/**	Retrieves a core implementation of a vector field usable only from the core thread. */
		SPtr<ct::VectorField> GetCore() const;

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
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static HVectorField Create(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		/** @name Internal
		 *  @{
		 */

		/** Same as create() excepts it creates a pointer to the vector field instead of a handle. */
		static SPtr<VectorField> CreatePtrInternal(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		/** Creates the resource without initializing it. */
		static SPtr<VectorField> CreateEmptyInternal();

		/** @} */

	protected:
		VectorField(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		SPtr<ct::RenderProxy> CreateRenderProxy() const override;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		VectorField() = default; // Serialization only

		friend class VectorFieldRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */

	namespace ct
	{
		/** @addtogroup Particles-Internal
		 *  @{
		 */

		/** Core thread version of a bs::VectorField. */
		class B3D_CORE_EXPORT VectorField : public RenderProxy, public TVectorField<true>
		{
		public:
			VectorField(const VECTOR_FIELD_DESC& desc, const SPtr<Texture>& texture);
		};

		/** @} */
	} // namespace ct

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/**	Imports vector fields from Fluid Grid ASCII (.fga) files. */
	class B3D_CORE_EXPORT FGAImporter : public SpecificImporter
	{
	public:
		bool IsExtensionSupported(const String& ext) const override;
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;
		SPtr<Resource> Import(const Path& filePath, SPtr<const ImportOptions> importOptions) override;
	};

	/** @} */
} // namespace bs
