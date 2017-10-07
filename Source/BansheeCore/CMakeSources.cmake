set(BS_BANSHEECORE_INC_COMPONENTS
	"Components/BsCBoxCollider.h"
	"Components/BsCCollider.h"
	"Components/BsCRigidbody.h"
	"Components/BsCSphereCollider.h"
	"Components/BsCCapsuleCollider.h"
	"Components/BsCPlaneCollider.h"
	"Components/BsCMeshCollider.h"
	"Components/BsCJoint.h"
	"Components/BsCFixedJoint.h"
	"Components/BsCHingeJoint.h"
	"Components/BsCDistanceJoint.h"
	"Components/BsCSliderJoint.h"
	"Components/BsCSphericalJoint.h"
	"Components/BsCD6Joint.h"
	"Components/BsCCharacterController.h"
	"Components/BsCAudioSource.h"
	"Components/BsCAudioListener.h"
	"Components/BsCCamera.h"
	"Components/BsCRenderable.h"
	"Components/BsCLight.h"
	"Components/BsCAnimation.h"
	"Components/BsCBone.h"	
	"Components/BsCReflectionProbe.h"
	"Components/BsCSkybox.h"
	"Components/BsCLightProbeVolume.h"
)

set(BS_BANSHEECORE_INC_PHYSICS
	"Physics/BsPhysicsManager.h"
	"Physics/BsPhysics.h"
	"Physics/BsPhysicsMaterial.h"
	"Physics/BsRigidbody.h"
	"Physics/BsBoxCollider.h"
	"Physics/BsSphereCollider.h"
	"Physics/BsCapsuleCollider.h"
	"Physics/BsPlaneCollider.h"
	"Physics/BsFCollider.h"
	"Physics/BsPhysicsMesh.h"
	"Physics/BsMeshCollider.h"
	"Physics/BsFJoint.h"
	"Physics/BsJoint.h"
	"Physics/BsFixedJoint.h"
	"Physics/BsDistanceJoint.h"
	"Physics/BsHingeJoint.h"
	"Physics/BsSliderJoint.h"
	"Physics/BsSphericalJoint.h"
	"Physics/BsD6Joint.h"
	"Physics/BsCharacterController.h"
	"Physics/BsCollider.h"
	"Physics/BsPhysicsCommon.h"
)

set(BS_BANSHEECORE_INC_CORETHREAD
	"CoreThread/BsCoreThreadQueue.h"
	"CoreThread/BsCoreThread.h"
	"CoreThread/BsCoreObjectManager.h"
	"CoreThread/BsCoreObject.h"
	"CoreThread/BsCommandQueue.h"
	"CoreThread/BsCoreObjectCore.h"
)

set(BS_BANSHEECORE_INC_IMPORTER
	"Importer/BsSpecificImporter.h"
	"Importer/BsImportOptions.h"
	"Importer/BsImporter.h"
	"Importer/BsTextureImportOptions.h"
	"Importer/BsShaderIncludeImporter.h"
	"Importer/BsMeshImportOptions.h"
	"Importer/BsShaderImportOptions.h"
)

set(BS_BANSHEECORE_INC_SCENE
	"Scene/BsComponent.h"
	"Scene/BsGameObject.h"
	"Scene/BsGameObjectHandle.h"
	"Scene/BsGameObjectManager.h"
	"Scene/BsSceneObject.h"
	"Scene/BsSceneManager.h"
	"Scene/BsPrefab.h"
	"Scene/BsPrefabDiff.h"
	"Scene/BsPrefabUtility.h"
)

set(BS_BANSHEECORE_INC_INPUT
	"Input/BsRawInputHandler.h"
	"Input/BsOSInputHandler.h"
	"Input/BsInputFwd.h"
	"Input/BsInput.h"
	"Input/BsMouse.h"
	"Input/BsKeyboard.h"
	"Input/BsGamepad.h"
)

set(BS_BANSHEECORE_INC_RENDERER
	"Renderer/BsLight.h"
	"Renderer/BsRenderable.h"
	"Renderer/BsRendererManager.h"
	"Renderer/BsRendererFactory.h"
	"Renderer/BsRenderer.h"
	"Renderer/BsRendererMeshData.h"
	"Renderer/BsParamBlocks.h"
	"Renderer/BsCamera.h"
	"Renderer/BsRenderSettings.h"
	"Renderer/BsRendererExtension.h"
	"Renderer/BsReflectionProbe.h"
	"Renderer/BsSkybox.h"
	"Renderer/BsLightProbeVolume.h"
	"Renderer/BsIBLUtility.h"
)

set(BS_BANSHEECORE_SRC_LOCALIZATION
	"Localization/BsHString.cpp"
	"Localization/BsStringTable.cpp"
	"Localization/BsStringTableManager.cpp"
)

set(BS_BANSHEECORE_SRC_RTTI
	"RTTI/BsMaterialRTTI.cpp"
)

set(BS_BANSHEECORE_INC_PROFILING
	"Profiling/BsProfilerCPU.h"
	"Profiling/BsProfilerGPU.h"
	"Profiling/BsProfilingManager.h"
	"Profiling/BsRenderStats.h"
)

set(BS_BANSHEECORE_INC_RENDERAPI
	"RenderAPI/BsVideoModeInfo.h"
	"RenderAPI/BsVertexDeclaration.h"
	"RenderAPI/BsVertexData.h"
	"RenderAPI/BsVertexBuffer.h"
	"RenderAPI/BsTimerQuery.h"
	"RenderAPI/BsTextureView.h"
	"RenderAPI/BsSubMesh.h"
	"RenderAPI/BsSamplerState.h"
	"RenderAPI/BsRenderWindow.h"
	"RenderAPI/BsRenderTexture.h"
	"RenderAPI/BsRenderTarget.h"
	"RenderAPI/BsRasterizerState.h"
	"RenderAPI/BsOcclusionQuery.h"
	"RenderAPI/BsIndexBuffer.h"
	"RenderAPI/BsHardwareBuffer.h"
	"RenderAPI/BsGpuProgram.h"
	"RenderAPI/BsGpuParams.h"
	"RenderAPI/BsGpuParamDesc.h"
	"RenderAPI/BsGpuParamBlockBuffer.h"
	"RenderAPI/BsGpuParam.h"
	"RenderAPI/BsGpuBuffer.h"
	"RenderAPI/BsEventQuery.h"
	"RenderAPI/BsDepthStencilState.h"
	"RenderAPI/BsBlendState.h"
	"RenderAPI/BsRenderAPI.h"
	"RenderAPI/BsRenderAPICapabilities.h"
	"RenderAPI/BsViewport.h"
	"RenderAPI/BsCommandBuffer.h"
	"RenderAPI/BsGpuPipelineState.h"
	"RenderAPI/BsGpuPipelineParamInfo.h"
	"RenderAPI/BsVertexDataDesc.h"
)

set(BS_BANSHEECORE_INC_RENDERAPI_MANAGERS
	"Managers/BsRenderWindowManager.h"
	"Managers/BsRenderStateManager.h"
	"Managers/BsQueryManager.h"
	"Managers/BsMeshManager.h"
	"Managers/BsHardwareBufferManager.h"
	"Managers/BsGpuProgramManager.h"
	"Managers/BsRenderAPIManager.h"
	"Managers/BsRenderAPIFactory.h"
	"Managers/BsCommandBufferManager.h"
	"Managers/BsTextureManager.h"
	"Managers/BsResourceListenerManager.h"
)

set(BS_BANSHEECORE_SRC_CORETHREAD
	"CoreThread/BsCommandQueue.cpp"
	"CoreThread/BsCoreObject.cpp"
	"CoreThread/BsCoreObjectManager.cpp"
	"CoreThread/BsCoreThread.cpp"
	"CoreThread/BsCoreThreadQueue.cpp"
	"CoreThread/BsCoreObjectCore.cpp"
)

set(BS_BANSHEECORE_INC_NOFILTER
	"BsCoreApplication.h"
	"BsCorePrerequisites.h"
)

set(BS_BANSHEECORE_INC_MATERIAL
	"Material/BsTechnique.h"
	"Material/BsShader.h"
	"Material/BsPass.h"
	"Material/BsMaterialManager.h"
	"Material/BsMaterial.h"
	"Material/BsMaterialParam.h"
	"Material/BsShaderManager.h"
	"Material/BsMaterialParams.h"
	"Material/BsShaderDefines.h"
	"Material/BsGpuParamsSet.h"
	"Material/BsShaderInclude.h"
)

set(BS_BANSHEECORE_INC_RESOURCES
	"Resources/BsResources.h"
	"Resources/BsResourceManifest.h"
	"Resources/BsResourceHandle.h"
	"Resources/BsResource.h"
	"Resources/BsGpuResourceData.h"
	"Resources/BsResourceMetaData.h"
	"Resources/BsSavedResourceData.h"
	"Resources/BsIResourceListener.h"
)

set(BS_BANSHEECORE_INC_MESH
	"Mesh/BsTransientMesh.h"
	"Mesh/BsMeshHeap.h"
	"Mesh/BsMeshData.h"
	"Mesh/BsMeshBase.h"
	"Mesh/BsMesh.h"
	"Mesh/BsMeshUtility.h"
)

set(BS_BANSHEECORE_INC_IMAGE
	"Image/BsTexture.h"
	"Image/BsPixelData.h"
	"Image/BsPixelUtil.h"
	"Image/BsPixelVolume.h"
)

set(BS_BANSHEECORE_SRC_UTILITY
	"Utility/BsUtility.cpp"
	"Utility/BsDeferredCallManager.cpp"
	"Utility/BsIconUtility.cpp"
	"Utility/BsUUID.cpp"
)

set(BS_BANSHEECORE_INC_TEXT
	"Text/BsTextData.h"
	"Text/BsFontManager.h"
	"Text/BsFontImportOptions.h"
	"Text/BsFontDesc.h"
	"Text/BsFont.h"
)

set(BS_BANSHEECORE_SRC_PROFILING
	"Profiling/BsProfilerCPU.cpp"
	"Profiling/BsProfilerGPU.cpp"
	"Profiling/BsProfilingManager.cpp"
)

set(BS_BANSHEECORE_SRC_COMPONENTS
	"Components/BsCBoxCollider.cpp"
	"Components/BsCCollider.cpp"
	"Components/BsCRigidbody.cpp"
	"Components/BsCSphereCollider.cpp"
	"Components/BsCPlaneCollider.cpp"
	"Components/BsCCapsuleCollider.cpp"
	"Components/BsCMeshCollider.cpp"
	"Components/BsCJoint.cpp"
	"Components/BsCFixedJoint.cpp"
	"Components/BsCHingeJoint.cpp"
	"Components/BsCDistanceJoint.cpp"
	"Components/BsCSliderJoint.cpp"
	"Components/BsCSphericalJoint.cpp"
	"Components/BsCD6Joint.cpp"
	"Components/BsCCharacterController.cpp"
	"Components/BsCAudioSource.cpp"
	"Components/BsCAudioListener.cpp"
	"Components/BsCCamera.cpp"
	"Components/BsCRenderable.cpp"
	"Components/BsCLight.cpp"
	"Components/BsCAnimation.cpp"
	"Components/BsCBone.cpp"	
	"Components/BsCReflectionProbe.cpp"
	"Components/BsCSkybox.cpp"
	"Components/BsCLightProbeVolume.cpp"
)

set(BS_BANSHEECORE_SRC_IMPORTER
	"Importer/BsImporter.cpp"
	"Importer/BsImportOptions.cpp"
	"Importer/BsSpecificImporter.cpp"
	"Importer/BsTextureImportOptions.cpp"
	"Importer/BsShaderIncludeImporter.cpp"
	"Importer/BsMeshImportOptions.cpp"
	"Importer/BsShaderImportOptions.cpp"
)

set(BS_BANSHEECORE_INC_UTILITY
	"Utility/BsCommonTypes.h"
	"Utility/BsUtility.h"
	"Utility/BsDeferredCallManager.h"
	"Utility/BsIconUtility.h"
	"Utility/BsUUID.h"
)

set(BS_BANSHEECORE_INC_RTTI
	"RTTI/BsBlendStateRTTI.h"
	"RTTI/BsComponentRTTI.h"
	"RTTI/BsDepthStencilStateRTTI.h"
	"RTTI/BsFontImportOptionsRTTI.h"
	"RTTI/BsFontRTTI.h"
	"RTTI/BsGameObjectHandleRTTI.h"
	"RTTI/BsGameObjectRTTI.h"
	"RTTI/BsGpuProgramRTTI.h"
	"RTTI/BsGpuResourceDataRTTI.h"
	"RTTI/BsImportOptionsRTTI.h"
	"RTTI/BsMaterialRTTI.h"
	"RTTI/BsMeshBaseRTTI.h"
	"RTTI/BsMeshDataRTTI.h"
	"RTTI/BsPassRTTI.h"
	"RTTI/BsPixelDataRTTI.h"
	"RTTI/BsRasterizerStateRTTI.h"
	"RTTI/BsResourceHandleRTTI.h"
	"RTTI/BsResourceManifestRTTI.h"
	"RTTI/BsResourceRTTI.h"
	"RTTI/BsSamplerStateRTTI.h"
	"RTTI/BsSceneObjectRTTI.h"
	"RTTI/BsShaderRTTI.h"
	"RTTI/BsTechniqueRTTI.h"
	"RTTI/BsTextureRTTI.h"
	"RTTI/BsVertexDataDescRTTI.h"
	"RTTI/BsVertexDeclarationRTTI.h"
	"RTTI/BsTextureImportOptionsRTTI.h"
	"RTTI/BsResourceMetaDataRTTI.h"
	"RTTI/BsViewportRTTI.h"
	"RTTI/BsSavedResourceDataRTTI.h"
	"RTTI/BsShaderIncludeRTTI.h"
	"RTTI/BsMeshImportOptionsRTTI.h"
	"RTTI/BsPrefabRTTI.h"
	"RTTI/BsPrefabDiffRTTI.h"
	"RTTI/BsStringTableRTTI.h"
	"RTTI/BsMaterialParamsRTTI.h"
	"RTTI/BsMeshRTTI.h"
	"RTTI/BsPhysicsMaterialRTTI.h"
	"RTTI/BsCBoxColliderRTTI.h"
	"RTTI/BsCColliderRTTI.h"
	"RTTI/BsCRigidbodyRTTI.h"
	"RTTI/BsCSphereColliderRTTI.h"
	"RTTI/BsCPlaneColliderRTTI.h"
	"RTTI/BsCCapsuleColliderRTTI.h"
	"RTTI/BsCMeshColliderRTTI.h"
	"RTTI/BsCJointRTTI.h"
	"RTTI/BsCFixedJointRTTI.h"
	"RTTI/BsCHingeJointRTTI.h"
	"RTTI/BsCDistanceJointRTTI.h"
	"RTTI/BsCSliderJointRTTI.h"
	"RTTI/BsCSphericalJointRTTI.h"
	"RTTI/BsCD6JointRTTI.h"
	"RTTI/BsCCharacterControllerRTTI.h"
	"RTTI/BsShaderImportOptionsRTTI.h"
	"RTTI/BsPhysicsMeshRTTI.h"
	"RTTI/BsAudioClipRTTI.h"
	"RTTI/BsCAudioSourceRTTI.h"
	"RTTI/BsCAudioListenerRTTI.h"
	"RTTI/BsAnimationClipRTTI.h"
	"RTTI/BsAnimationCurveRTTI.h"
	"RTTI/BsSkeletonRTTI.h"
	"RTTI/BsCCameraRTTI.h"
	"RTTI/BsCameraRTTI.h"
	"RTTI/BsRenderSettingsRTTI.h"
	"RTTI/BsMorphShapesRTTI.h"
	"RTTI/BsAudioClipImportOptionsRTTI.h"
	"RTTI/BsCRenderableRTTI.h"
	"RTTI/BsCLightRTTI.h"
	"RTTI/BsCAnimationRTTI.h"
	"RTTI/BsCBoneRTTI.h"
	"RTTI/BsRenderableRTTI.h"
	"RTTI/BsLightRTTI.h"	
	"RTTI/BsReflectionProbeRTTI.h"
	"RTTI/BsCReflectionProbeRTTI.h"
	"RTTI/BsSkyboxRTTI.h"
	"RTTI/BsLightProbeVolumeRTTI.h"
	"RTTI/BsCLightProbeVolumeRTTI.h"
)

set(BS_BANSHEECORE_SRC_RENDERER
	"Renderer/BsLight.cpp"
	"Renderer/BsRenderable.cpp"
	"Renderer/BsRendererManager.cpp"
	"Renderer/BsRenderer.cpp"
	"Renderer/BsRendererMeshData.cpp"
	"Renderer/BsParamBlocks.cpp"
	"Renderer/BsCamera.cpp"
	"Renderer/BsRenderSettings.cpp"
	"Renderer/BsRendererExtension.cpp"
	"Renderer/BsReflectionProbe.cpp"
	"Renderer/BsSkybox.cpp"
	"Renderer/BsLightProbeVolume.cpp"
	"Renderer/BsIBLUtility.cpp"
)

set(BS_BANSHEECORE_SRC_RESOURCES
	"Resources/BsGpuResourceData.cpp"
	"Resources/BsResource.cpp"
	"Resources/BsResourceHandle.cpp"
	"Resources/BsResourceManifest.cpp"
	"Resources/BsResources.cpp"
	"Resources/BsResourceMetaData.cpp"
	"Resources/BsSavedResourceData.cpp"
	"Resources/BsIResourceListener.cpp"
)

set(BS_BANSHEECORE_SRC_MESH
	"Mesh/BsMesh.cpp"
	"Mesh/BsMeshBase.cpp"
	"Mesh/BsMeshData.cpp"
	"Mesh/BsMeshHeap.cpp"
	"Mesh/BsTransientMesh.cpp"
	"Mesh/BsMeshUtility.cpp"
)

set(BS_BANSHEECORE_SRC_IMAGE
	"Image/BsPixelData.cpp"
	"Image/BsTexture.cpp"
	"Image/BsPixelUtil.cpp"
)

set(BS_BANSHEECORE_SRC_MATERIAL
	"Material/BsMaterial.cpp"
	"Material/BsMaterialManager.cpp"
	"Material/BsPass.cpp"
	"Material/BsShader.cpp"
	"Material/BsTechnique.cpp"
	"Material/BsMaterialParam.cpp"
	"Material/BsShaderManager.cpp"
	"Material/BsMaterialParams.cpp"
	"Material/BsShaderDefines.cpp"
	"Material/BsGpuParamsSet.cpp"
	"Material/BsShaderInclude.cpp"
)

set(BS_BANSHEECORE_SRC_INPUT
	"Input/BsInput.cpp"
	"Input/BsOSInputHandler.cpp"
	"Input/BsRawInputHandler.cpp"
)

set(BS_BANSHEECORE_INC_LOCALIZATION
	"Localization/BsHString.h"
	"Localization/BsStringTable.h"
	"Localization/BsStringTableManager.h"
)

set(BS_BANSHEECORE_SRC_TEXT
	"Text/BsFont.cpp"
	"Text/BsFontImportOptions.cpp"
	"Text/BsFontManager.cpp"
	"Text/BsTextData.cpp"
)

set(BS_BANSHEECORE_SRC_RENDERAPI
	"RenderAPI/BsBlendState.cpp"
	"RenderAPI/BsDepthStencilState.cpp"
	"RenderAPI/BsEventQuery.cpp"
	"RenderAPI/BsGpuBuffer.cpp"
	"RenderAPI/BsGpuParam.cpp"
	"RenderAPI/BsGpuParamBlockBuffer.cpp"
	"RenderAPI/BsGpuParams.cpp"
	"RenderAPI/BsGpuProgram.cpp"
	"RenderAPI/BsIndexBuffer.cpp"
	"RenderAPI/BsOcclusionQuery.cpp"
	"RenderAPI/BsRasterizerState.cpp"
	"RenderAPI/BsRenderTarget.cpp"
	"RenderAPI/BsRenderTexture.cpp"
	"RenderAPI/BsRenderWindow.cpp"
	"RenderAPI/BsSamplerState.cpp"
	"RenderAPI/BsTextureView.cpp"
	"RenderAPI/BsTimerQuery.cpp"
	"RenderAPI/BsVertexBuffer.cpp"
	"RenderAPI/BsVertexData.cpp"
	"RenderAPI/BsVertexDeclaration.cpp"
	"RenderAPI/BsVideoModeInfo.cpp"
	"RenderAPI/BsRenderAPI.cpp"
	"RenderAPI/BsRenderAPICapabilities.cpp"
	"RenderAPI/BsViewport.cpp"
	"RenderAPI/BsCommandBuffer.cpp"
	"RenderAPI/BsGpuPipelineState.cpp"
	"RenderAPI/BsGpuPipelineParamInfo.cpp"
	"RenderAPI/BsVertexDataDesc.cpp"
)

set(BS_BANSHEECORE_SRC_RENDERAPI_MANAGERS
	"Managers/BsGpuProgramManager.cpp"
	"Managers/BsHardwareBufferManager.cpp"
	"Managers/BsMeshManager.cpp"
	"Managers/BsQueryManager.cpp"
	"Managers/BsRenderStateManager.cpp"
	"Managers/BsRenderWindowManager.cpp"
	"Managers/BsRenderAPIManager.cpp"
	"Managers/BsCommandBufferManager.cpp"
	"Managers/BsTextureManager.cpp"
	"Managers/BsResourceListenerManager.cpp"
)

set(BS_BANSHEECORE_SRC_NOFILTER
	"BsCoreApplication.cpp"
)

set(BS_BANSHEECORE_SRC_PHYSICS
	"Physics/BsPhysicsManager.cpp"
	"Physics/BsPhysics.cpp"
	"Physics/BsPhysicsMaterial.cpp"
	"Physics/BsCollider.cpp"
	"Physics/BsRigidbody.cpp"
	"Physics/BsBoxCollider.cpp"
	"Physics/BsSphereCollider.cpp"
	"Physics/BsPlaneCollider.cpp"
	"Physics/BsCapsuleCollider.cpp"
	"Physics/BsFCollider.cpp"
	"Physics/BsPhysicsMesh.cpp"
	"Physics/BsMeshCollider.cpp"
	"Physics/BsFJoint.cpp"
	"Physics/BsJoint.cpp"
	"Physics/BsFixedJoint.cpp"
	"Physics/BsDistanceJoint.cpp"
	"Physics/BsHingeJoint.cpp"
	"Physics/BsSliderJoint.cpp"
	"Physics/BsSphericalJoint.cpp"
	"Physics/BsD6Joint.cpp"
	"Physics/BsCharacterController.cpp"
)

set(BS_BANSHEECORE_SRC_SCENE
	"Scene/BsComponent.cpp"
	"Scene/BsGameObject.cpp"
	"Scene/BsGameObjectHandle.cpp"
	"Scene/BsGameObjectManager.cpp"
	"Scene/BsSceneObject.cpp"
	"Scene/BsSceneManager.cpp"
	"Scene/BsPrefab.cpp"
	"Scene/BsPrefabDiff.cpp"
	"Scene/BsPrefabUtility.cpp"
)

set(BS_BANSHEECORE_INC_AUDIO
	"Audio/BsAudio.h"
	"Audio/BsAudioClip.h"
	"Audio/BsAudioListener.h"
	"Audio/BsAudioSource.h"
	"Audio/BsAudioClipImportOptions.h"
	"Audio/BsAudioUtility.h"
	"Audio/BsAudioManager.h"
)

set(BS_BANSHEECORE_SRC_AUDIO
	"Audio/BsAudio.cpp"
	"Audio/BsAudioClip.cpp"
	"Audio/BsAudioListener.cpp"
	"Audio/BsAudioSource.cpp"
	"Audio/BsAudioClipImportOptions.cpp"
	"Audio/BsAudioUtility.cpp"
	"Audio/BsAudioManager.cpp"
)

set(BS_BANSHEECORE_INC_ANIMATION
	"Animation/BsAnimationCurve.h"
	"Animation/BsAnimationClip.h"
	"Animation/BsSkeleton.h"
	"Animation/BsAnimation.h"
	"Animation/BsAnimationManager.h"
	"Animation/BsCurveCache.h"
	"Animation/BsAnimationUtility.h"
	"Animation/BsSkeletonMask.h"
	"Animation/BsMorphShapes.h"
)

set(BS_BANSHEECORE_SRC_ANIMATION
	"Animation/BsAnimationCurve.cpp"
	"Animation/BsAnimationClip.cpp"
	"Animation/BsSkeleton.cpp"
	"Animation/BsAnimation.cpp"
	"Animation/BsAnimationManager.cpp"
	"Animation/BsAnimationUtility.cpp"
	"Animation/BsSkeletonMask.cpp"
	"Animation/BsMorphShapes.cpp"
)

set(BS_BANSHEECORE_INC_PLATFORM
	"Platform/BsPlatform.h"
	"Platform/BsFolderMonitor.h"
	"Platform/BsDropTarget.h"
)

set(BS_BANSHEECORE_SRC_PLATFORM
		"Platform/BsDropTarget.cpp"
		)

set(BS_BANSHEECORE_INC_PLATFORM_WIN32
	"Win32/BsWin32DropTarget.h"
	"Win32/BsWin32Platform.h"
	"Win32/BsWin32Input.h"
)

set(BS_BANSHEECORE_SRC_PLATFORM_WIN32
	"Win32/BsWin32FolderMonitor.cpp"
	"Win32/BsWin32Platform.cpp"
	"Win32/BsWin32DropTarget.cpp"
	"Win32/BsWin32Input.cpp"
	"Win32/BsWin32Mouse.cpp"
	"Win32/BsWin32Keyboard.cpp"
	"Win32/BsWin32Gamepad.cpp"
)

set(BS_BANSHEECORE_INC_PLATFORM_UNIX
	"Linux/BsLinuxPlatform.h"
	"Linux/BsLinuxWindow.h"
	"Linux/BsLinuxDropTarget.h"
)

set(BS_BANSHEECORE_SRC_PLATFORM_UNIX
	"Linux/BsLinuxPlatform.cpp"
	"Linux/BsLinuxWindow.cpp"
	"Linux/BsLinuxDropTarget.cpp"
	"Linux/BsLinuxFolderMonitor.cpp"
)

if(WIN32)
	list(APPEND BS_BANSHEECORE_INC_PLATFORM ${BS_BANSHEECORE_INC_PLATFORM_WIN32})
	list(APPEND BS_BANSHEECORE_SRC_PLATFORM ${BS_BANSHEECORE_SRC_PLATFORM_WIN32})
elseif(LINUX)
	list(APPEND BS_BANSHEECORE_INC_PLATFORM ${BS_BANSHEECORE_INC_PLATFORM_UNIX})
	list(APPEND BS_BANSHEECORE_SRC_PLATFORM ${BS_BANSHEECORE_SRC_PLATFORM_UNIX})
endif()

source_group("Header Files\\Components" FILES ${BS_BANSHEECORE_INC_COMPONENTS})
source_group("Header Files\\Physics" FILES ${BS_BANSHEECORE_INC_PHYSICS})
source_group("Header Files\\CoreThread" FILES ${BS_BANSHEECORE_INC_CORETHREAD})
source_group("Header Files\\Importer" FILES ${BS_BANSHEECORE_INC_IMPORTER})
source_group("Header Files\\Scene" FILES ${BS_BANSHEECORE_INC_SCENE})
source_group("Header Files\\Input" FILES ${BS_BANSHEECORE_INC_INPUT})
source_group("Header Files\\Platform" FILES ${BS_BANSHEECORE_INC_PLATFORM})
source_group("Header Files\\Renderer" FILES ${BS_BANSHEECORE_INC_RENDERER})
source_group("Source Files\\Localization" FILES ${BS_BANSHEECORE_SRC_LOCALIZATION})
source_group("Source Files\\RTTI" FILES ${BS_BANSHEECORE_SRC_RTTI})
source_group("Header Files\\Profiling" FILES ${BS_BANSHEECORE_INC_PROFILING})
source_group("Header Files\\RenderAPI" FILES ${BS_BANSHEECORE_INC_RENDERAPI})
source_group("Header Files\\RenderAPI\\Managers" FILES ${BS_BANSHEECORE_INC_RENDERAPI_MANAGERS})
source_group("Source Files\\CoreThread" FILES ${BS_BANSHEECORE_SRC_CORETHREAD})
source_group("Header Files" FILES ${BS_BANSHEECORE_INC_NOFILTER})
source_group("Header Files\\Material" FILES ${BS_BANSHEECORE_INC_MATERIAL})
source_group("Header Files\\Resources" FILES ${BS_BANSHEECORE_INC_RESOURCES})
source_group("Source Files\\Utility" FILES ${BS_BANSHEECORE_SRC_UTILITY})
source_group("Header Files\\Text" FILES ${BS_BANSHEECORE_INC_TEXT})
source_group("Source Files\\Profiling" FILES ${BS_BANSHEECORE_SRC_PROFILING})
source_group("Source Files\\Components" FILES ${BS_BANSHEECORE_SRC_COMPONENTS})
source_group("Source Files\\Platform" FILES ${BS_BANSHEECORE_SRC_PLATFORM})
source_group("Source Files\\Importer" FILES ${BS_BANSHEECORE_SRC_IMPORTER})
source_group("Header Files\\Utility" FILES ${BS_BANSHEECORE_INC_UTILITY})
source_group("Header Files\\RTTI" FILES ${BS_BANSHEECORE_INC_RTTI})
source_group("Source Files\\Renderer" FILES ${BS_BANSHEECORE_SRC_RENDERER})
source_group("Source Files\\Resources" FILES ${BS_BANSHEECORE_SRC_RESOURCES})
source_group("Source Files\\Material" FILES ${BS_BANSHEECORE_SRC_MATERIAL})
source_group("Source Files\\Input" FILES ${BS_BANSHEECORE_SRC_INPUT})
source_group("Header Files\\Localization" FILES ${BS_BANSHEECORE_INC_LOCALIZATION})
source_group("Source Files\\Text" FILES ${BS_BANSHEECORE_SRC_TEXT})
source_group("Source Files\\RenderAPI" FILES ${BS_BANSHEECORE_SRC_RENDERAPI})
source_group("Source Files\\RenderAPI\\Managers" FILES ${BS_BANSHEECORE_SRC_RENDERAPI_MANAGERS})
source_group("Source Files" FILES ${BS_BANSHEECORE_SRC_NOFILTER})
source_group("Source Files\\Physics" FILES ${BS_BANSHEECORE_SRC_PHYSICS})
source_group("Source Files\\Scene" FILES ${BS_BANSHEECORE_SRC_SCENE})
source_group("Header Files\\Audio" FILES ${BS_BANSHEECORE_INC_AUDIO})
source_group("Source Files\\Audio" FILES ${BS_BANSHEECORE_SRC_AUDIO})
source_group("Header Files\\Animation" FILES ${BS_BANSHEECORE_INC_ANIMATION})
source_group("Source Files\\Animation" FILES ${BS_BANSHEECORE_SRC_ANIMATION})
source_group("Header Files\\Image" FILES ${BS_BANSHEECORE_INC_IMAGE})
source_group("Source Files\\Image" FILES ${BS_BANSHEECORE_SRC_IMAGE})
source_group("Header Files\\Mesh" FILES ${BS_BANSHEECORE_INC_MESH})
source_group("Source Files\\Mesh" FILES ${BS_BANSHEECORE_SRC_MESH})

set(BS_BANSHEECORE_SRC
	${BS_BANSHEECORE_INC_COMPONENTS}
	${BS_BANSHEECORE_INC_PHYSICS}
	${BS_BANSHEECORE_INC_CORETHREAD}
	${BS_BANSHEECORE_INC_IMPORTER}
	${BS_BANSHEECORE_INC_SCENE}
	${BS_BANSHEECORE_INC_INPUT}
	${BS_BANSHEECORE_INC_PLATFORM}
	${BS_BANSHEECORE_INC_RENDERER}
	${BS_BANSHEECORE_SRC_LOCALIZATION}
	${BS_BANSHEECORE_SRC_RTTI}
	${BS_BANSHEECORE_INC_PROFILING}
	${BS_BANSHEECORE_INC_RENDERAPI}
	${BS_BANSHEECORE_SRC_CORETHREAD}
	${BS_BANSHEECORE_INC_NOFILTER}
	${BS_BANSHEECORE_INC_MATERIAL}
	${BS_BANSHEECORE_INC_RESOURCES}
	${BS_BANSHEECORE_SRC_UTILITY}
	${BS_BANSHEECORE_INC_TEXT}
	${BS_BANSHEECORE_SRC_PROFILING}
	${BS_BANSHEECORE_SRC_COMPONENTS}
	${BS_BANSHEECORE_SRC_PLATFORM}
	${BS_BANSHEECORE_SRC_IMPORTER}
	${BS_BANSHEECORE_INC_UTILITY}
	${BS_BANSHEECORE_INC_RTTI}
	${BS_BANSHEECORE_SRC_RENDERER}
	${BS_BANSHEECORE_SRC_RESOURCES}
	${BS_BANSHEECORE_SRC_MATERIAL}
	${BS_BANSHEECORE_SRC_INPUT}
	${BS_BANSHEECORE_INC_LOCALIZATION}
	${BS_BANSHEECORE_SRC_TEXT}
	${BS_BANSHEECORE_SRC_RENDERAPI}
	${BS_BANSHEECORE_SRC_NOFILTER}
	${BS_BANSHEECORE_SRC_PHYSICS}
	${BS_BANSHEECORE_SRC_SCENE}
	${BS_BANSHEECORE_INC_AUDIO}
	${BS_BANSHEECORE_SRC_AUDIO}
	${BS_BANSHEECORE_INC_ANIMATION}
	${BS_BANSHEECORE_SRC_ANIMATION}
	${BS_BANSHEECORE_INC_RENDERAPI_MANAGERS}
	${BS_BANSHEECORE_SRC_RENDERAPI_MANAGERS}
	${BS_BANSHEECORE_INC_IMAGE}
	${BS_BANSHEECORE_SRC_IMAGE}
	${BS_BANSHEECORE_INC_MESH}
	${BS_BANSHEECORE_SRC_MESH}
)
