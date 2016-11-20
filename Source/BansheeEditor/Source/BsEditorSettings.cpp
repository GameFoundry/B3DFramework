//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEditorSettings.h"
#include "BsEditorSettingsRTTI.h"

namespace bs
{
	EditorSettings::EditorSettings()
		: mMoveSnapActive(false), mRotateSnapActive(false), mMoveSnap(0.1f), mRotationSnap(20.0f), mGridSize(256)
		, mGridAxisSpacing(1.0f), mActiveSceneTool(1 /* Move */), mActiveCoordinateMode(0), mActivePivotMode(0)
		, mHandleSize(0.10f), mFPSLimit(60), mMouseSensitivity(1.0f)
	{ }
	
	RTTITypeBase* EditorSettings::getRTTIStatic()
	{
		return EditorSettingsRTTI::instance();
	}

	RTTITypeBase* EditorSettings::getRTTI() const
	{
		return EditorSettings::getRTTIStatic();
	}
}