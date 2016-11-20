//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsSettings.h"
#include "BsDegree.h"

namespace bs
{
	/** @addtogroup Settings
	 *  @{
	 */

	struct RecentProject;

	/**	Contains various globally accessible editor preferences. */
	class BS_ED_EXPORT EditorSettings : public Settings
	{
	public:
		EditorSettings();

		/**	Checks is snapping enabled for move handles in scene view. */
		bool getMoveHandleSnapActive() const { return mMoveSnapActive; }

		/**	Checks is angle snapping enabled for rotate handles in scene view. */
		bool getRotateHandleSnapActive() const { return mRotateSnapActive; }

		/**	Gets the snap amount if move snapping is enabled. All move handles will move in multiples of this amount. */
		float getMoveHandleSnap() const { return mMoveSnap; }

		/**
		 * Gets the snap amount if rotate snapping is enabled. All rotate handles will rotate in multiples of this amount.
		 */
		Degree getRotationHandleSnap() const { return mRotationSnap; }

		/**	Returns the size that determines to total size of the scene view grid (its width and height). */
		UINT32 getGridSize() const { return mGridSize; }

		/**	Returns the distance between scene view grid lines. */
		float getGridSpacing() const { return mGridAxisSpacing; }

		/**	Gets the default size of all scene view handles. */
		float getHandleSize() const { return mHandleSize; }

		/**	Returns the currently active scene view tool (for example move, rotate, etc.). */
		UINT32 getActiveSceneTool() const { return mActiveSceneTool; }

		/**	Returns the currently active coordinate mode for scene view (for example global/local). */
		UINT32 getActiveCoordinateMode() const { return mActiveCoordinateMode; }

		/**	Returns the currently active pivot mode for scene view (for example pivot/center). */
		UINT32 getActivePivotMode() const { return mActivePivotMode; }

		/**	Retrieves the path to the last project open in the editor. */
		Path getLastOpenProject() const { return mLastOpenProject; }

		/** Retrieves whether the last open project should be automatically loaded on editor start up. */
		bool getAutoLoadLastProject() const { return mAutoLoadLastProject; }

		/**	Retrieves a list of most recently loaded project paths and their last access times. */
		const Vector<RecentProject>& getRecentProjects() const { return mRecentProjects; }

		/** Retrieves the maximum number of frames per second the editor is allowed to execute. Zero means infinite. */
		UINT32 getFPSLimit() const { return mFPSLimit; }

		/** 
		 * Retrieves a value that controls sensitivity of mouse movements. This doesn't apply to mouse cursor. 
		 * Default value is 1.0f.
		 */
		float getMouseSensitivity() const { return mMouseSensitivity; }

		/**	Enables/disables snapping for move handles in scene view. */
		void setMoveHandleSnapActive(bool snapActive) { mMoveSnapActive = snapActive; markAsDirty(); }

		/**	Enables/disables angle snapping for rotate handles in scene view. */
		void setRotateHandleSnapActive(bool snapActive) { mRotateSnapActive = snapActive; markAsDirty(); }

		/**	Sets the move snap amount. All move handles will move in multiples of this amount. */
		void setMoveHandleSnap(float value) { mMoveSnap = value; markAsDirty(); }

		/**	Sets the rotate snap amount. All rotate handles will rotate in multiples of this amount. */
		void setRotationHandleSnap(Degree value) { mRotationSnap = value; markAsDirty(); }

		/**	Sets the size that determines to total size of the scene view grid (its width and height). */
		void setGridSize(UINT32 value) { mGridSize = value; markAsDirty(); }

		/**	Sets the distance between scene view grid lines. */
		void setGridSpacing(float value) { mGridAxisSpacing = value; markAsDirty(); }

		/**	Sets the default size of all scene view handles. */
		void setHandleSize(float value) { mHandleSize = value; markAsDirty(); }

		/**	Changes the currently active scene view tool (for example move, rotate, etc.). */
		void setActiveSceneTool(UINT32 value) { mActiveSceneTool = value; markAsDirty(); }

		/**	Changes the currently active coordinate mode for scene view (for example global/local). */
		void setActiveCoordinateMode(UINT32 value) { mActiveCoordinateMode = value; markAsDirty(); }

		/**	Changes the currently active pivot mode for scene view (for example pivot/center). */
		void setActivePivotMode(UINT32 value) { mActivePivotMode = value; markAsDirty(); }

		/**	Sets the path to the last project open in the editor. */
		void setLastOpenProject(const Path& value) { mLastOpenProject = value; markAsDirty(); }

		/** Sets whether the last open project should be automatically loaded on editor start up. */
		void setAutoLoadLastProject(bool value) { mAutoLoadLastProject = value; markAsDirty(); }

		/**	Sets a list of most recently loaded project paths and their last access times. */
		void setRecentProjects(const Vector<RecentProject>& value) { mRecentProjects = value; markAsDirty(); }

		/** Sets the maximum number of frames per second the editor is allowed to execute. Zero means infinite. */
		void setFPSLimit(UINT32 limit) { mFPSLimit = limit; markAsDirty(); }

		/** 
		 * Sets a value that controls sensitivity of mouse movements. This doesn't apply to mouse cursor. 
		 * Default value is 1.0f.
		 */
		void setMouseSensitivity(float value) { mMouseSensitivity = value; markAsDirty(); }

	private:
		bool mMoveSnapActive;
		bool mRotateSnapActive;

		float mMoveSnap;
		Degree mRotationSnap;

		UINT32 mGridSize;
		float mGridAxisSpacing;

		UINT32 mActiveSceneTool;
		UINT32 mActiveCoordinateMode;
		UINT32 mActivePivotMode;

		float mHandleSize;
		UINT32 mFPSLimit;
		float mMouseSensitivity;

		Path mLastOpenProject;
		bool mAutoLoadLastProject;
		Vector<RecentProject> mRecentProjects;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class EditorSettingsRTTI;
		static RTTITypeBase* getRTTIStatic();
		virtual RTTITypeBase* getRTTI() const override;
	};

	/**	Data about a recently loaded project. */
	struct RecentProject
	{
		Path path;
		UINT64 accessTimestamp;
	};

	/** @} */
}