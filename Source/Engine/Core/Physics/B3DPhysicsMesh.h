//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Resources/B3DResource.h"

namespace b3d
{
	/** @addtogroup Physics
	 *  @{
	 */

	class IPhysicsMeshImplementation;

	/**
	 * Represents a physics mesh that can be used with a MeshCollider. Physics mesh can be a generic triangle mesh
	 * or a convex mesh. Convex meshes are limited to 255 faces.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Physics)) PhysicsMesh : public Resource
	{
	public:
		PhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);
		virtual ~PhysicsMesh() = default;

		/** Returns the type of the physics mesh. */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Getter))
		PhysicsMeshType GetType() const { return mType; }

		/** Returns the mesh's indices and vertices. */
		SPtr<MeshData> GetMeshData() const;

		/**
		 * Creates a new physics mesh.
		 *
		 * @param	meshData	Index and vertices of the mesh data.
		 * @param	type		Type of the mesh. If convex the provided mesh geometry will be converted into a convex
		 *							mesh (that might not be the same as the provided mesh data).
		 */
		static HPhysicsMesh Create(const SPtr<MeshData>& meshData, PhysicsMeshType type = PhysicsMeshType::Convex);

		/** @name Internal
		 *  @{
		 */

		/** Returns the internal implementation of the physics mesh. */
		virtual IPhysicsMeshImplementation* GetImplementation() { return mImplementation.get(); }

		/**
		 * @copydoc Create()
		 *
		 * For internal use. Requires manual initialization after creation.
		 */
		static SPtr<PhysicsMesh> CreateShared(const SPtr<MeshData>& meshData, PhysicsMeshType type);

		/** Creates an empty and uninitialized object instance. To be used by serialization. */
		static SPtr<PhysicsMesh> CreateEmpty();

		/** @} */

	protected:
		void Initialize() override;
		void Destroy() override;

		SPtr<IPhysicsMeshImplementation> mImplementation;
		SPtr<MeshData> mInitMeshData; // Transient, only used during initalization
		PhysicsMeshType mType; // Transient, only used during initalization

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PhysicsMeshRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
	/** @addtogroup Physics-Internal
	 *  @{
	 */

	/** Low-level interface for a physics mesh, to be implemented by the physics plugin to provide functionality. */
	class B3D_EXPORT IPhysicsMeshImplementation : public IReflectable
	{
	public:
		virtual ~IPhysicsMeshImplementation() = default;

		/** Returns the mesh's indices and vertices. */
		virtual SPtr<MeshData> GetMeshData() const = 0;

	protected:
		friend class PhysicsMesh;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class PhysicsMeshImplementationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
