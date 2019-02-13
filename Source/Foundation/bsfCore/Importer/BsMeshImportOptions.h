//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Importer/BsImportOptions.h"
#include "Animation/BsAnimationClip.h"

namespace bs
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Controls what type of collision mesh should be imported during mesh import. */
	enum class BS_SCRIPT_EXPORT(m:Importer,api:bsf) CollisionMeshType
	{
		None, /**< No collision mesh will be imported. */
		Normal, /**< Normal triangle mesh will be imported. */
		Convex /**< A convex hull will be generated from the source mesh. */
	};

	/** Information about how to split an AnimationClip into multiple separate clips. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Importer,api:bsf) AnimationSplitInfo : IReflectable
	{
		BS_SCRIPT_EXPORT()
		AnimationSplitInfo() = default;

		BS_SCRIPT_EXPORT()
		AnimationSplitInfo(const String& name, UINT32 startFrame, UINT32 endFrame, bool isAdditive = false)
			: name(name), startFrame(startFrame), endFrame(endFrame), isAdditive(isAdditive)
		{ }

		BS_SCRIPT_EXPORT()
		String name;

		BS_SCRIPT_EXPORT()
		UINT32 startFrame = 0;

		BS_SCRIPT_EXPORT()
		UINT32 endFrame = 0;

		BS_SCRIPT_EXPORT()
		bool isAdditive = false;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AnimationSplitInfoRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** A set of animation events that will be added to an animation clip during animation import. */
	struct BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Importer,api:bsf) ImportedAnimationEvents : IReflectable
	{
		BS_SCRIPT_EXPORT()
		ImportedAnimationEvents() = default;

		BS_SCRIPT_EXPORT()
		String name;

		BS_SCRIPT_EXPORT()
		Vector<AnimationEvent> events;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ImportedAnimationEventsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/**
	 * Contains import options you may use to control how is a mesh imported from some external format into engine format.
	 */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Importer,api:bsf) MeshImportOptions : public ImportOptions
	{
	public:
		MeshImportOptions() = default;

		/**	Determines whether the texture data is also stored in CPU memory. */
		BS_SCRIPT_EXPORT()
		bool cpuCached = false;

		/**	Determines should mesh normals be imported if available. */
		BS_SCRIPT_EXPORT()
		bool importNormals = true;

		/**	Determines should mesh tangents and bitangents be imported if available. */
		BS_SCRIPT_EXPORT()
		bool importTangents = true;

		/**	Determines should mesh blend shapes be imported if available. */
		BS_SCRIPT_EXPORT()
		bool importBlendShapes = false;

		/**	Determines should mesh skin data like bone weights, indices and bind poses be imported if available. */
		BS_SCRIPT_EXPORT()
		bool importSkin = false;

		/**	Determines should animation clips be imported if available. */
		BS_SCRIPT_EXPORT()
		bool importAnimation = false;

		/**	
		 * Enables or disables keyframe reduction. Keyframe reduction will reduce the number of key-frames in an animation
		 * clip by removing identical keyframes, and therefore reducing the size of the clip.
		 */
		BS_SCRIPT_EXPORT()
		bool reduceKeyFrames = true;

		/**	
		 * Enables or disables import of root motion curves. When enabled, any animation curves in imported animations 
		 * affecting the root bone will be available through a set of separate curves in AnimationClip, and they won't be
		 * evaluated through normal animation process. Instead it is expected that the user evaluates the curves manually
		 * and applies them as required.
		 */
		BS_SCRIPT_EXPORT()
		bool importRootMotion = false;

		/** Uniformly scales the imported mesh by the specified value. */
		BS_SCRIPT_EXPORT()
		float importScale = 1.0f;

		/**	
		 * Determines what type (if any) of collision mesh should be imported. If enabled the collision mesh will be
		 * available as a sub-resource returned by the importer (along with the normal mesh). 
		 */
		BS_SCRIPT_EXPORT()
		CollisionMeshType collisionMeshType = CollisionMeshType::None;

		/** 
		 * Animation split infos that determine how will the source animation clip be split. If no splits are present the 
		 * data will be imported as one clip, but if splits are present the data will be split according to the split infos.
		 * Split infos only affect the primary animation clip, other clips will not be split.
		 */
		BS_SCRIPT_EXPORT()
		Vector<AnimationSplitInfo> animationSplits;

		/** Set of events that will be added to the animation clip, if animation import is enabled. */
		BS_SCRIPT_EXPORT()
		Vector<ImportedAnimationEvents> animationEvents;

		/** Creates a new import options object that allows you to customize how are meshes imported. */
		BS_SCRIPT_EXPORT(ec:T)
		static SPtr<MeshImportOptions> create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MeshImportOptionsRTTI;
		static RTTITypeBase* getRTTIStatic();
		RTTITypeBase* getRTTI() const override;
	};

	/** @} */
}