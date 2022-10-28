//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPhysicsPrerequisites.h"
#include "Physics/BsPhysicsMesh.h"

namespace bs
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of a PhysicsMesh. */
	class NullPhysicsMesh : public PhysicsMesh
	{
	public:
		NullPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);

	private:
		void Initialize() override;
		void Destroy() override;

		// Note: Must not have its own RTTI type, it's important it shares the same type ID as PhysicsMesh so the
		// system knows to recognize it. Use FPhysicsMesh instead.
	};

	/** Null implementation of the PhysicsMesh foundation, FPhysicsMesh. */
	class FNullPhysicsMesh : public FPhysicsMesh
	{
	public:
		FNullPhysicsMesh(const SPtr<MeshData>& meshData, PhysicsMeshType type);
		~FNullPhysicsMesh() override = default;

		SPtr<MeshData> GetMeshData() const override;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		FNullPhysicsMesh(); // Serialization only

		friend class FNullPhysicsMeshRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;
	};

	/** @} */
} // namespace bs
