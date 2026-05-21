//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"
#include "Math/B3DVector3I.h"
#include "Math/B3DAABox.h"
#include "Importer/B3DSpecificImporter.h"

namespace b3d
{
	namespace render
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
		AABox Bounds = AABox::kEmpty;
	};

	/** @} */

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/** Common functionality for both the main and render thread variants of VectorField. */
	template <bool IsRenderProxy>
	class B3D_EXPORT TVectorField
	{
	public:
		using TextureType = TShared<CoreVariantType<Texture, IsRenderProxy>>;

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
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Particles)) VectorField : public Resource, public TVectorField<false>
	{
	public:
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
		static TShared<VectorField> CreateShared(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		/** Creates the resource without initializing it. */
		static TShared<VectorField> CreateEmpty();

		/** @} */

	protected:
		VectorField(const VECTOR_FIELD_DESC& desc, const Vector<Vector3>& values);

		TShared<render::RenderProxy> CreateRenderProxy() const override;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		VectorField() = default; // Serialization only

		friend class VectorFieldRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */

	namespace render
	{
		/** @addtogroup Particles-Internal
		 *  @{
		 */

		/** Render  thread version of a b3d::VectorField. */
		class B3D_EXPORT VectorField : public RenderProxy, public TVectorField<true>
		{
		public:
			VectorField(const VECTOR_FIELD_DESC& desc, const TShared<Texture>& texture);
		};

		/** @} */
	} // namespace render

	/** @addtogroup Particles-Internal
	 *  @{
	 */

	/**	Imports vector fields from Fluid Grid ASCII (.fga) files. */
	class B3D_EXPORT FGAImporter : public SpecificImporter
	{
	public:
		bool IsExtensionSupported(const String& ext) const override;
		bool IsMagicNumberSupported(const u8* magicNumPtr, u32 numBytes) const override;
		TShared<Resource> Import(const Path& filePath, TShared<const ImportOptions> importOptions) override;
	};

	/** @} */
} // namespace b3d
