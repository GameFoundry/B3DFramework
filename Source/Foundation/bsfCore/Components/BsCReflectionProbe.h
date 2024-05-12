//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Renderer/BsReflectionProbe.h"
#include "Scene/BsComponent.h"

namespace bs
{
	/** @addtogroup Components-Core
	 *  @{
	 */

	/**
	 * @copydoc	ReflectionProbe
	 *
	 * @note	Wraps ReflectionProbe as a Component.
	 */
	class B3D_CORE_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Rendering), ExportName(ReflectionProbe)) CReflectionProbe : public Component
	{
	public:
		CReflectionProbe(const HSceneObject& parent);
		virtual ~CReflectionProbe();

		/** @copydoc ReflectionProbe::GetType */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Getter))
		ReflectionProbeType GetType() const { return mInternal->GetType(); }

		/** @copydoc ReflectionProbe::SetType */
		B3D_SCRIPT_EXPORT(ExportName(Type), Property(Setter))
		void SetType(ReflectionProbeType type) { mInternal->SetType(type); }

		/** @copydoc ReflectionProbe::GetRadius */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Getter))
		float GetRadius() const { return mInternal->GetRadius(); }

		/** @copydoc ReflectionProbe::SetRadius */
		B3D_SCRIPT_EXPORT(ExportName(Radius), Property(Setter))
		void SetRadius(float radius) { mInternal->SetRadius(radius); }

		/** @copydoc ReflectionProbe::GetExtents */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Getter))
		Vector3 GetExtents() const { return mInternal->GetExtents(); }

		/** @copydoc ReflectionProbe::SetExtents */
		B3D_SCRIPT_EXPORT(ExportName(Extents), Property(Setter))
		void SetExtents(const Vector3& extents) { mInternal->SetExtents(extents); }

		/** Retrieves transition distance set by SetTransitionDistance(). */
		float GetTransitionDistance() const { return mInternal->GetTransitionDistance(); }

		/** @copydoc ReflectionProbe::SetTransitionDistance */
		void SetTransitionDistance(float distance) { mInternal->SetTransitionDistance(distance); }

		/** @copydoc ReflectionProbe::GetCustomTexture */
		B3D_SCRIPT_EXPORT(ExportName(CustomTexture), Property(Getter))
		HTexture GetCustomTexture() const { return mInternal->GetCustomTexture(); }

		/** @copydoc ReflectionProbe::SetCustomTexture */
		B3D_SCRIPT_EXPORT(ExportName(CustomTexture), Property(Setter))
		void SetCustomTexture(const HTexture& texture) { mInternal->SetCustomTexture(texture); }

		/** @copydoc ReflectionProbe::GetBounds */
		Sphere GetBounds() const;

		/** @copydoc ReflectionProbe::Capture */
		B3D_SCRIPT_EXPORT(ExportName(Capture))
		void Capture() { mInternal->Capture(); }

		/** @name Internal
		 *  @{
		 */

		/**	Returns the reflection probe that this component wraps. */
		SPtr<ReflectionProbe> GetReflectionProbeInternal() const { return mInternal; }

		/** @} */

	protected:
		mutable SPtr<ReflectionProbe> mInternal;

		/************************************************************************/
		/* 						COMPONENT OVERRIDES                      		*/
		/************************************************************************/
	protected:
		friend class SceneObject;

		void OnBeginPlay() override;
		void OnDestroyed() override;
		void Update() override {}

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class CReflectionProbeRTTI;
		static RTTITypeBase* GetRttiStatic();
		RTTITypeBase* GetRtti() const override;

	protected:
		CReflectionProbe(); // Serialization only
	};

	/** @} */
} // namespace bs
