//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DFMODPrerequisites.h"
#include "Components/B3DAudioListener.h"
#include "Math/B3DTransform.h"

namespace b3d
{
	/** @addtogroup FMOD
	 *  @{
	 */

	/** FMOD implementation of an AudioListener. */
	class FMODAudioListener : public IAudioListenerImplementation
	{
	public:
		FMODAudioListener();
		~FMODAudioListener() override;

		void SetTransform(const Transform& transform) override;
		void SetVelocity(const Vector3& velocity) override;

	private:
		friend class FMODAudio;

		/** Called by the FMODAudio system when the listener list changes. */
		void Rebuild(i32 id);

		Transform mTransform;
		Vector3 mVelocity = Vector3::kZero;
		i32 mId;
	};

	/** @} */
} // namespace b3d
