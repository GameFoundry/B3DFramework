//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPhysicsPrerequisites.h"
#include "Physics/B3DPhysicsMesh.h"

namespace b3d
{
	/** @addtogroup NullPhysics
	 *  @{
	 */

	/** Null implementation of IPhysicsMeshImplementation. */
	class NullPhysicsMeshImplementation : public IPhysicsMeshImplementation
	{
	public:
		NullPhysicsMeshImplementation(const TShared<MeshData>& meshData, PhysicsMeshType type);
		~NullPhysicsMeshImplementation() override = default;

		TShared<MeshData> GetMeshData() const override;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		NullPhysicsMeshImplementation(); // Serialization only

		friend class NullPhysicsMeshImplementationRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;

	private:
		TShared<MeshData> mMeshData;
	};

	/** @} */
} // namespace b3d
