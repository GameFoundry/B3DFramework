//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Importer/B3DImportOptions.h"
#include "Animation/B3DAnimationClip.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Importer
	 *  @{
	 */

	/** Controls what type of collision mesh should be imported during mesh import. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), Assembly(Engine), Assembly(Editor), API(FrameworkOnly)) CollisionMeshType
	{
		None, /**< No collision mesh will be imported. */
		Normal, /**< Normal triangle mesh will be imported. */
		Convex /**< A convex hull will be generated from the source mesh. */
	};

	/** Information about how to split an AnimationClip into multiple separate clips. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), Assembly(Engine), Assembly(Editor), API(FrameworkOnly)) AnimationSplitInfo : IReflectable, IScriptExportable
	{
		B3D_SCRIPT_EXPORT()
		AnimationSplitInfo() = default;

		B3D_SCRIPT_EXPORT()

		AnimationSplitInfo(const String& name, u32 startFrame, u32 endFrame, bool isAdditive = false)
			: Name(name), StartFrame(startFrame), EndFrame(endFrame), IsAdditive(isAdditive)
		{}

		B3D_SCRIPT_EXPORT()
		String Name;

		B3D_SCRIPT_EXPORT()
		u32 StartFrame = 0;

		B3D_SCRIPT_EXPORT()
		u32 EndFrame = 0;

		B3D_SCRIPT_EXPORT()
		bool IsAdditive = false;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class AnimationSplitInfoRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** A set of animation events that will be added to an animation clip during animation import. */
	struct B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), Assembly(Engine), Assembly(Editor), API(FrameworkOnly)) ImportedAnimationEvents : IReflectable, IScriptExportable
	{
		B3D_SCRIPT_EXPORT()
		ImportedAnimationEvents() = default;

		B3D_SCRIPT_EXPORT()
		String Name;

		B3D_SCRIPT_EXPORT()
		Vector<AnimationEvent> Events;

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class ImportedAnimationEventsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/**
	 * Contains import options you may use to control how is a mesh imported from some external format into engine format.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Importer), Assembly(Engine), Assembly(Editor), API(FrameworkOnly)) MeshImportOptions : public ImportOptions
	{
	public:
		MeshImportOptions() = default;

		/**	Determines whether the texture data is also stored in CPU memory. */
		B3D_SCRIPT_EXPORT()
		bool CpuCached = false;

		/**	Determines should mesh normals be imported if available. */
		B3D_SCRIPT_EXPORT()
		bool ImportNormals = true;

		/**	Determines should mesh tangents and bitangents be imported if available. */
		B3D_SCRIPT_EXPORT()
		bool ImportTangents = true;

		/**	Determines should mesh blend shapes be imported if available. */
		B3D_SCRIPT_EXPORT()
		bool ImportBlendShapes = false;

		/**	Determines should mesh skin data like bone weights, indices and bind poses be imported if available. */
		B3D_SCRIPT_EXPORT()
		bool ImportSkin = false;

		/**	Determines should animation clips be imported if available. */
		B3D_SCRIPT_EXPORT()
		bool ImportAnimation = false;

		/**
		 * Enables or disables keyframe reduction. Keyframe reduction will reduce the number of key-frames in an animation
		 * clip by removing identical keyframes, and therefore reducing the size of the clip.
		 */
		B3D_SCRIPT_EXPORT()
		bool ReduceKeyFrames = true;

		/**
		 * Enables or disables import of root motion curves. When enabled, any animation curves in imported animations
		 * affecting the root bone will be available through a set of separate curves in AnimationClip, and they won't be
		 * evaluated through normal animation process. Instead it is expected that the user evaluates the curves manually
		 * and applies them as required.
		 */
		B3D_SCRIPT_EXPORT()
		bool ImportRootMotion = false;

		/** Uniformly scales the imported mesh by the specified value. */
		B3D_SCRIPT_EXPORT()
		float ImportScale = 1.0f;

		/**
		 * Determines what type (if any) of collision mesh should be imported. If enabled the collision mesh will be
		 * available as a sub-resource returned by the importer (along with the normal mesh).
		 */
		B3D_SCRIPT_EXPORT()
		CollisionMeshType CollisionMeshType = CollisionMeshType::None;

		/**
		 * Animation split infos that determine how will the source animation clip be split. If no splits are present the
		 * data will be imported as one clip, but if splits are present the data will be split according to the split infos.
		 * Split infos only affect the primary animation clip, other clips will not be split.
		 */
		B3D_SCRIPT_EXPORT()
		Vector<AnimationSplitInfo> AnimationSplits;

		/** Set of events that will be added to the animation clip, if animation import is enabled. */
		B3D_SCRIPT_EXPORT()
		Vector<ImportedAnimationEvents> AnimationEvents;

		/** Creates a new import options object that allows you to customize how are meshes imported. */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(T))
		static TShared<MeshImportOptions> Create();

		/************************************************************************/
		/* 								SERIALIZATION                      		*/
		/************************************************************************/
	public:
		friend class MeshImportOptionsRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
