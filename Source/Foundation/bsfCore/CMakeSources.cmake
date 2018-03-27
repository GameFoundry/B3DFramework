set(BS_CORE_INC_COMPONENTS
	"bsfCore/Components/BsCBoxCollider.h"
	"bsfCore/Components/BsCCollider.h"
	"bsfCore/Components/BsCRigidbody.h"
	"bsfCore/Components/BsCSphereCollider.h"
	"bsfCore/Components/BsCCapsuleCollider.h"
	"bsfCore/Components/BsCPlaneCollider.h"
	"bsfCore/Components/BsCMeshCollider.h"
	"bsfCore/Components/BsCJoint.h"
	"bsfCore/Components/BsCFixedJoint.h"
	"bsfCore/Components/BsCHingeJoint.h"
	"bsfCore/Components/BsCDistanceJoint.h"
	"bsfCore/Components/BsCSliderJoint.h"
	"bsfCore/Components/BsCSphericalJoint.h"
	"bsfCore/Components/BsCD6Joint.h"
	"bsfCore/Components/BsCCharacterController.h"
	"bsfCore/Components/BsCAudioSource.h"
	"bsfCore/Components/BsCAudioListener.h"
	"bsfCore/Components/BsCCamera.h"
	"bsfCore/Components/BsCRenderable.h"
	"bsfCore/Components/BsCLight.h"
	"bsfCore/Components/BsCAnimation.h"
	"bsfCore/Components/BsCBone.h"	
	"bsfCore/Components/BsCReflectionProbe.h"
	"bsfCore/Components/BsCSkybox.h"
	"bsfCore/Components/BsCLightProbeVolume.h"
)

set(BS_CORE_INC_PHYSICS
	"bsfCore/Physics/BsPhysicsManager.h"
	"bsfCore/Physics/BsPhysics.h"
	"bsfCore/Physics/BsPhysicsMaterial.h"
	"bsfCore/Physics/BsRigidbody.h"
	"bsfCore/Physics/BsBoxCollider.h"
	"bsfCore/Physics/BsSphereCollider.h"
	"bsfCore/Physics/BsCapsuleCollider.h"
	"bsfCore/Physics/BsPlaneCollider.h"
	"bsfCore/Physics/BsFCollider.h"
	"bsfCore/Physics/BsPhysicsMesh.h"
	"bsfCore/Physics/BsMeshCollider.h"
	"bsfCore/Physics/BsFJoint.h"
	"bsfCore/Physics/BsJoint.h"
	"bsfCore/Physics/BsFixedJoint.h"
	"bsfCore/Physics/BsDistanceJoint.h"
	"bsfCore/Physics/BsHingeJoint.h"
	"bsfCore/Physics/BsSliderJoint.h"
	"bsfCore/Physics/BsSphericalJoint.h"
	"bsfCore/Physics/BsD6Joint.h"
	"bsfCore/Physics/BsCharacterController.h"
	"bsfCore/Physics/BsCollider.h"
	"bsfCore/Physics/BsPhysicsCommon.h"
)

set(BS_CORE_INC_CORETHREAD
	"bsfCore/CoreThread/BsCoreThreadQueue.h"
	"bsfCore/CoreThread/BsCoreThread.h"
	"bsfCore/CoreThread/BsCoreObjectManager.h"
	"bsfCore/CoreThread/BsCoreObject.h"
	"bsfCore/CoreThread/BsCommandQueue.h"
	"bsfCore/CoreThread/BsCoreObjectCore.h"
)

set(BS_CORE_INC_IMPORTER
	"bsfCore/Importer/BsSpecificImporter.h"
	"bsfCore/Importer/BsImportOptions.h"
	"bsfCore/Importer/BsImporter.h"
	"bsfCore/Importer/BsTextureImportOptions.h"
	"bsfCore/Importer/BsShaderIncludeImporter.h"
	"bsfCore/Importer/BsMeshImportOptions.h"
	"bsfCore/Importer/BsShaderImportOptions.h"
)

set(BS_CORE_INC_SCENE
	"bsfCore/Scene/BsComponent.h"
	"bsfCore/Scene/BsGameObject.h"
	"bsfCore/Scene/BsGameObjectHandle.h"
	"bsfCore/Scene/BsGameObjectManager.h"
	"bsfCore/Scene/BsSceneObject.h"
	"bsfCore/Scene/BsSceneManager.h"
	"bsfCore/Scene/BsPrefab.h"
	"bsfCore/Scene/BsPrefabDiff.h"
	"bsfCore/Scene/BsPrefabUtility.h"
	"bsfCore/Scene/BsTransform.h"
	"bsfCore/Scene/BsSceneActor.h"
)

set(BS_CORE_INC_INPUT
	"bsfCore/Input/BsInputFwd.h"
	"bsfCore/Input/BsInput.h"
	"bsfCore/Input/BsMouse.h"
	"bsfCore/Input/BsKeyboard.h"
	"bsfCore/Input/BsGamepad.h"
)

set(BS_CORE_INC_RENDERER
	"bsfCore/Renderer/BsLight.h"
	"bsfCore/Renderer/BsRenderable.h"
	"bsfCore/Renderer/BsRendererManager.h"
	"bsfCore/Renderer/BsRendererFactory.h"
	"bsfCore/Renderer/BsRenderer.h"
	"bsfCore/Renderer/BsRendererMeshData.h"
	"bsfCore/Renderer/BsParamBlocks.h"
	"bsfCore/Renderer/BsCamera.h"
	"bsfCore/Renderer/BsRenderSettings.h"
	"bsfCore/Renderer/BsRendererExtension.h"
	"bsfCore/Renderer/BsReflectionProbe.h"
	"bsfCore/Renderer/BsSkybox.h"
	"bsfCore/Renderer/BsLightProbeVolume.h"
	"bsfCore/Renderer/BsIBLUtility.h"
)

set(BS_CORE_SRC_LOCALIZATION
	"bsfCore/Localization/BsHString.cpp"
	"bsfCore/Localization/BsStringTable.cpp"
	"bsfCore/Localization/BsStringTableManager.cpp"
)

set(BS_CORE_SRC_RTTI
	"bsfCore/Private/RTTI/BsMaterialRTTI.cpp"
)

set(BS_CORE_INC_PROFILING
	"bsfCore/Profiling/BsProfilerCPU.h"
	"bsfCore/Profiling/BsProfilerGPU.h"
	"bsfCore/Profiling/BsProfilingManager.h"
	"bsfCore/Profiling/BsRenderStats.h"
)

set(BS_CORE_INC_RENDERAPI
	"bsfCore/RenderAPI/BsVideoModeInfo.h"
	"bsfCore/RenderAPI/BsVertexDeclaration.h"
	"bsfCore/RenderAPI/BsVertexData.h"
	"bsfCore/RenderAPI/BsVertexBuffer.h"
	"bsfCore/RenderAPI/BsTimerQuery.h"
	"bsfCore/RenderAPI/BsTextureView.h"
	"bsfCore/RenderAPI/BsSubMesh.h"
	"bsfCore/RenderAPI/BsSamplerState.h"
	"bsfCore/RenderAPI/BsRenderWindow.h"
	"bsfCore/RenderAPI/BsRenderTexture.h"
	"bsfCore/RenderAPI/BsRenderTarget.h"
	"bsfCore/RenderAPI/BsRasterizerState.h"
	"bsfCore/RenderAPI/BsOcclusionQuery.h"
	"bsfCore/RenderAPI/BsIndexBuffer.h"
	"bsfCore/RenderAPI/BsHardwareBuffer.h"
	"bsfCore/RenderAPI/BsGpuProgram.h"
	"bsfCore/RenderAPI/BsGpuParams.h"
	"bsfCore/RenderAPI/BsGpuParamDesc.h"
	"bsfCore/RenderAPI/BsGpuParamBlockBuffer.h"
	"bsfCore/RenderAPI/BsGpuParam.h"
	"bsfCore/RenderAPI/BsGpuBuffer.h"
	"bsfCore/RenderAPI/BsEventQuery.h"
	"bsfCore/RenderAPI/BsDepthStencilState.h"
	"bsfCore/RenderAPI/BsBlendState.h"
	"bsfCore/RenderAPI/BsRenderAPI.h"
	"bsfCore/RenderAPI/BsRenderAPICapabilities.h"
	"bsfCore/RenderAPI/BsViewport.h"
	"bsfCore/RenderAPI/BsCommandBuffer.h"
	"bsfCore/RenderAPI/BsGpuPipelineState.h"
	"bsfCore/RenderAPI/BsGpuPipelineParamInfo.h"
	"bsfCore/RenderAPI/BsVertexDataDesc.h"
)

set(BS_CORE_INC_RENDERAPI_MANAGERS
	"bsfCore/Managers/BsRenderWindowManager.h"
	"bsfCore/Managers/BsRenderStateManager.h"
	"bsfCore/Managers/BsQueryManager.h"
	"bsfCore/Managers/BsMeshManager.h"
	"bsfCore/Managers/BsHardwareBufferManager.h"
	"bsfCore/Managers/BsGpuProgramManager.h"
	"bsfCore/Managers/BsRenderAPIManager.h"
	"bsfCore/Managers/BsRenderAPIFactory.h"
	"bsfCore/Managers/BsCommandBufferManager.h"
	"bsfCore/Managers/BsTextureManager.h"
	"bsfCore/Managers/BsResourceListenerManager.h"
)

set(BS_CORE_SRC_CORETHREAD
	"bsfCore/CoreThread/BsCommandQueue.cpp"
	"bsfCore/CoreThread/BsCoreObject.cpp"
	"bsfCore/CoreThread/BsCoreObjectManager.cpp"
	"bsfCore/CoreThread/BsCoreThread.cpp"
	"bsfCore/CoreThread/BsCoreThreadQueue.cpp"
	"bsfCore/CoreThread/BsCoreObjectCore.cpp"
)

set(BS_CORE_INC_NOFILTER
	"bsfCore/BsCoreApplication.h"
	"bsfCore/BsCorePrerequisites.h"
)

set(BS_CORE_INC_MATERIAL
	"bsfCore/Material/BsTechnique.h"
	"bsfCore/Material/BsShader.h"
	"bsfCore/Material/BsPass.h"
	"bsfCore/Material/BsMaterialManager.h"
	"bsfCore/Material/BsMaterial.h"
	"bsfCore/Material/BsMaterialParam.h"
	"bsfCore/Material/BsShaderManager.h"
	"bsfCore/Material/BsMaterialParams.h"
	"bsfCore/Material/BsGpuParamsSet.h"
	"bsfCore/Material/BsShaderInclude.h"
	"bsfCore/Material/BsShaderVariation.h"
)

set(BS_CORE_INC_RESOURCES
	"bsfCore/Resources/BsResources.h"
	"bsfCore/Resources/BsResourceManifest.h"
	"bsfCore/Resources/BsResourceHandle.h"
	"bsfCore/Resources/BsResource.h"
	"bsfCore/Resources/BsGpuResourceData.h"
	"bsfCore/Resources/BsResourceMetaData.h"
	"bsfCore/Resources/BsSavedResourceData.h"
	"bsfCore/Resources/BsIResourceListener.h"
)

set(BS_CORE_INC_MESH
	"bsfCore/Mesh/BsTransientMesh.h"
	"bsfCore/Mesh/BsMeshHeap.h"
	"bsfCore/Mesh/BsMeshData.h"
	"bsfCore/Mesh/BsMeshBase.h"
	"bsfCore/Mesh/BsMesh.h"
	"bsfCore/Mesh/BsMeshUtility.h"
)

set(BS_CORE_INC_IMAGE
	"bsfCore/Image/BsTexture.h"
	"bsfCore/Image/BsPixelData.h"
	"bsfCore/Image/BsPixelUtil.h"
	"bsfCore/Image/BsPixelVolume.h"
)

set(BS_CORE_SRC_UTILITY
	"bsfCore/Utility/BsUtility.cpp"
	"bsfCore/Utility/BsDeferredCallManager.cpp"
	"bsfCore/Utility/BsIconUtility.cpp"
)

set(BS_CORE_INC_TEXT
	"bsfCore/Text/BsTextData.h"
	"bsfCore/Text/BsFontImportOptions.h"
	"bsfCore/Text/BsFontDesc.h"
	"bsfCore/Text/BsFont.h"
)

set(BS_CORE_SRC_PROFILING
	"bsfCore/Profiling/BsProfilerCPU.cpp"
	"bsfCore/Profiling/BsProfilerGPU.cpp"
	"bsfCore/Profiling/BsProfilingManager.cpp"
)

set(BS_CORE_SRC_COMPONENTS
	"bsfCore/Components/BsCBoxCollider.cpp"
	"bsfCore/Components/BsCCollider.cpp"
	"bsfCore/Components/BsCRigidbody.cpp"
	"bsfCore/Components/BsCSphereCollider.cpp"
	"bsfCore/Components/BsCPlaneCollider.cpp"
	"bsfCore/Components/BsCCapsuleCollider.cpp"
	"bsfCore/Components/BsCMeshCollider.cpp"
	"bsfCore/Components/BsCJoint.cpp"
	"bsfCore/Components/BsCFixedJoint.cpp"
	"bsfCore/Components/BsCHingeJoint.cpp"
	"bsfCore/Components/BsCDistanceJoint.cpp"
	"bsfCore/Components/BsCSliderJoint.cpp"
	"bsfCore/Components/BsCSphericalJoint.cpp"
	"bsfCore/Components/BsCD6Joint.cpp"
	"bsfCore/Components/BsCCharacterController.cpp"
	"bsfCore/Components/BsCAudioSource.cpp"
	"bsfCore/Components/BsCAudioListener.cpp"
	"bsfCore/Components/BsCCamera.cpp"
	"bsfCore/Components/BsCRenderable.cpp"
	"bsfCore/Components/BsCLight.cpp"
	"bsfCore/Components/BsCAnimation.cpp"
	"bsfCore/Components/BsCBone.cpp"	
	"bsfCore/Components/BsCReflectionProbe.cpp"
	"bsfCore/Components/BsCSkybox.cpp"
	"bsfCore/Components/BsCLightProbeVolume.cpp"
)

set(BS_CORE_SRC_IMPORTER
	"bsfCore/Importer/BsImporter.cpp"
	"bsfCore/Importer/BsImportOptions.cpp"
	"bsfCore/Importer/BsSpecificImporter.cpp"
	"bsfCore/Importer/BsTextureImportOptions.cpp"
	"bsfCore/Importer/BsShaderIncludeImporter.cpp"
	"bsfCore/Importer/BsMeshImportOptions.cpp"
	"bsfCore/Importer/BsShaderImportOptions.cpp"
)

set(BS_CORE_INC_UTILITY
	"bsfCore/Utility/BsCommonTypes.h"
	"bsfCore/Utility/BsUtility.h"
	"bsfCore/Utility/BsDeferredCallManager.h"
	"bsfCore/Utility/BsIconUtility.h"
)

set(BS_CORE_INC_RTTI
	"bsfCore/Private/RTTI/BsBlendStateRTTI.h"
	"bsfCore/Private/RTTI/BsComponentRTTI.h"
	"bsfCore/Private/RTTI/BsDepthStencilStateRTTI.h"
	"bsfCore/Private/RTTI/BsFontImportOptionsRTTI.h"
	"bsfCore/Private/RTTI/BsFontRTTI.h"
	"bsfCore/Private/RTTI/BsGameObjectHandleRTTI.h"
	"bsfCore/Private/RTTI/BsGameObjectRTTI.h"
	"bsfCore/Private/RTTI/BsGpuProgramRTTI.h"
	"bsfCore/Private/RTTI/BsGpuResourceDataRTTI.h"
	"bsfCore/Private/RTTI/BsImportOptionsRTTI.h"
	"bsfCore/Private/RTTI/BsMaterialRTTI.h"
	"bsfCore/Private/RTTI/BsMeshBaseRTTI.h"
	"bsfCore/Private/RTTI/BsMeshDataRTTI.h"
	"bsfCore/Private/RTTI/BsPassRTTI.h"
	"bsfCore/Private/RTTI/BsPixelDataRTTI.h"
	"bsfCore/Private/RTTI/BsRasterizerStateRTTI.h"
	"bsfCore/Private/RTTI/BsResourceHandleRTTI.h"
	"bsfCore/Private/RTTI/BsResourceManifestRTTI.h"
	"bsfCore/Private/RTTI/BsResourceRTTI.h"
	"bsfCore/Private/RTTI/BsSamplerStateRTTI.h"
	"bsfCore/Private/RTTI/BsSceneObjectRTTI.h"
	"bsfCore/Private/RTTI/BsShaderRTTI.h"
	"bsfCore/Private/RTTI/BsTechniqueRTTI.h"
	"bsfCore/Private/RTTI/BsTextureRTTI.h"
	"bsfCore/Private/RTTI/BsVertexDataDescRTTI.h"
	"bsfCore/Private/RTTI/BsVertexDeclarationRTTI.h"
	"bsfCore/Private/RTTI/BsTextureImportOptionsRTTI.h"
	"bsfCore/Private/RTTI/BsResourceMetaDataRTTI.h"
	"bsfCore/Private/RTTI/BsViewportRTTI.h"
	"bsfCore/Private/RTTI/BsSavedResourceDataRTTI.h"
	"bsfCore/Private/RTTI/BsShaderIncludeRTTI.h"
	"bsfCore/Private/RTTI/BsMeshImportOptionsRTTI.h"
	"bsfCore/Private/RTTI/BsPrefabRTTI.h"
	"bsfCore/Private/RTTI/BsPrefabDiffRTTI.h"
	"bsfCore/Private/RTTI/BsStringTableRTTI.h"
	"bsfCore/Private/RTTI/BsMaterialParamsRTTI.h"
	"bsfCore/Private/RTTI/BsMeshRTTI.h"
	"bsfCore/Private/RTTI/BsPhysicsMaterialRTTI.h"
	"bsfCore/Private/RTTI/BsCBoxColliderRTTI.h"
	"bsfCore/Private/RTTI/BsCColliderRTTI.h"
	"bsfCore/Private/RTTI/BsCRigidbodyRTTI.h"
	"bsfCore/Private/RTTI/BsCSphereColliderRTTI.h"
	"bsfCore/Private/RTTI/BsCPlaneColliderRTTI.h"
	"bsfCore/Private/RTTI/BsCCapsuleColliderRTTI.h"
	"bsfCore/Private/RTTI/BsCMeshColliderRTTI.h"
	"bsfCore/Private/RTTI/BsCJointRTTI.h"
	"bsfCore/Private/RTTI/BsCFixedJointRTTI.h"
	"bsfCore/Private/RTTI/BsCHingeJointRTTI.h"
	"bsfCore/Private/RTTI/BsCDistanceJointRTTI.h"
	"bsfCore/Private/RTTI/BsCSliderJointRTTI.h"
	"bsfCore/Private/RTTI/BsCSphericalJointRTTI.h"
	"bsfCore/Private/RTTI/BsCD6JointRTTI.h"
	"bsfCore/Private/RTTI/BsCCharacterControllerRTTI.h"
	"bsfCore/Private/RTTI/BsShaderImportOptionsRTTI.h"
	"bsfCore/Private/RTTI/BsPhysicsMeshRTTI.h"
	"bsfCore/Private/RTTI/BsAudioClipRTTI.h"
	"bsfCore/Private/RTTI/BsCAudioSourceRTTI.h"
	"bsfCore/Private/RTTI/BsCAudioListenerRTTI.h"
	"bsfCore/Private/RTTI/BsAnimationClipRTTI.h"
	"bsfCore/Private/RTTI/BsAnimationCurveRTTI.h"
	"bsfCore/Private/RTTI/BsSkeletonRTTI.h"
	"bsfCore/Private/RTTI/BsCCameraRTTI.h"
	"bsfCore/Private/RTTI/BsCameraRTTI.h"
	"bsfCore/Private/RTTI/BsRenderSettingsRTTI.h"
	"bsfCore/Private/RTTI/BsMorphShapesRTTI.h"
	"bsfCore/Private/RTTI/BsAudioClipImportOptionsRTTI.h"
	"bsfCore/Private/RTTI/BsCRenderableRTTI.h"
	"bsfCore/Private/RTTI/BsCLightRTTI.h"
	"bsfCore/Private/RTTI/BsCAnimationRTTI.h"
	"bsfCore/Private/RTTI/BsCBoneRTTI.h"
	"bsfCore/Private/RTTI/BsRenderableRTTI.h"
	"bsfCore/Private/RTTI/BsLightRTTI.h"	
	"bsfCore/Private/RTTI/BsReflectionProbeRTTI.h"
	"bsfCore/Private/RTTI/BsCReflectionProbeRTTI.h"
	"bsfCore/Private/RTTI/BsSkyboxRTTI.h"
	"bsfCore/Private/RTTI/BsLightProbeVolumeRTTI.h"
	"bsfCore/Private/RTTI/BsCLightProbeVolumeRTTI.h"
	"bsfCore/Private/RTTI/BsTransformRTTI.h"
	"bsfCore/Private/RTTI/BsAudioListenerRTTI.h"
	"bsfCore/Private/RTTI/BsAudioSourceRTTI.h"
	"bsfCore/Private/RTTI/BsShaderVariationRTTI.h"
)

set(BS_CORE_SRC_RENDERER
	"bsfCore/Renderer/BsLight.cpp"
	"bsfCore/Renderer/BsRenderable.cpp"
	"bsfCore/Renderer/BsRendererManager.cpp"
	"bsfCore/Renderer/BsRenderer.cpp"
	"bsfCore/Renderer/BsRendererMeshData.cpp"
	"bsfCore/Renderer/BsParamBlocks.cpp"
	"bsfCore/Renderer/BsCamera.cpp"
	"bsfCore/Renderer/BsRenderSettings.cpp"
	"bsfCore/Renderer/BsRendererExtension.cpp"
	"bsfCore/Renderer/BsReflectionProbe.cpp"
	"bsfCore/Renderer/BsSkybox.cpp"
	"bsfCore/Renderer/BsLightProbeVolume.cpp"
	"bsfCore/Renderer/BsIBLUtility.cpp"
)

set(BS_CORE_SRC_RESOURCES
	"bsfCore/Resources/BsGpuResourceData.cpp"
	"bsfCore/Resources/BsResource.cpp"
	"bsfCore/Resources/BsResourceHandle.cpp"
	"bsfCore/Resources/BsResourceManifest.cpp"
	"bsfCore/Resources/BsResources.cpp"
	"bsfCore/Resources/BsResourceMetaData.cpp"
	"bsfCore/Resources/BsSavedResourceData.cpp"
	"bsfCore/Resources/BsIResourceListener.cpp"
)

set(BS_CORE_SRC_MESH
	"bsfCore/Mesh/BsMesh.cpp"
	"bsfCore/Mesh/BsMeshBase.cpp"
	"bsfCore/Mesh/BsMeshData.cpp"
	"bsfCore/Mesh/BsMeshHeap.cpp"
	"bsfCore/Mesh/BsTransientMesh.cpp"
	"bsfCore/Mesh/BsMeshUtility.cpp"
)

set(BS_CORE_SRC_IMAGE
	"bsfCore/Image/BsPixelData.cpp"
	"bsfCore/Image/BsTexture.cpp"
	"bsfCore/Image/BsPixelUtil.cpp"
)

set(BS_CORE_SRC_MATERIAL
	"bsfCore/Material/BsMaterial.cpp"
	"bsfCore/Material/BsMaterialManager.cpp"
	"bsfCore/Material/BsPass.cpp"
	"bsfCore/Material/BsShader.cpp"
	"bsfCore/Material/BsTechnique.cpp"
	"bsfCore/Material/BsMaterialParam.cpp"
	"bsfCore/Material/BsShaderManager.cpp"
	"bsfCore/Material/BsMaterialParams.cpp"
	"bsfCore/Material/BsGpuParamsSet.cpp"
	"bsfCore/Material/BsShaderInclude.cpp"
	"bsfCore/Material/BsShaderVariation.cpp"
)

set(BS_CORE_SRC_INPUT
	"bsfCore/Input/BsInput.cpp"
)

set(BS_CORE_INC_LOCALIZATION
	"bsfCore/Localization/BsHString.h"
	"bsfCore/Localization/BsStringTable.h"
	"bsfCore/Localization/BsStringTableManager.h"
)

set(BS_CORE_SRC_TEXT
	"bsfCore/Text/BsFont.cpp"
	"bsfCore/Text/BsFontImportOptions.cpp"
	"bsfCore/Text/BsTextData.cpp"
)

set(BS_CORE_SRC_RENDERAPI
	"bsfCore/RenderAPI/BsBlendState.cpp"
	"bsfCore/RenderAPI/BsDepthStencilState.cpp"
	"bsfCore/RenderAPI/BsEventQuery.cpp"
	"bsfCore/RenderAPI/BsGpuBuffer.cpp"
	"bsfCore/RenderAPI/BsGpuParam.cpp"
	"bsfCore/RenderAPI/BsGpuParamBlockBuffer.cpp"
	"bsfCore/RenderAPI/BsGpuParams.cpp"
	"bsfCore/RenderAPI/BsGpuProgram.cpp"
	"bsfCore/RenderAPI/BsIndexBuffer.cpp"
	"bsfCore/RenderAPI/BsOcclusionQuery.cpp"
	"bsfCore/RenderAPI/BsRasterizerState.cpp"
	"bsfCore/RenderAPI/BsRenderTarget.cpp"
	"bsfCore/RenderAPI/BsRenderTexture.cpp"
	"bsfCore/RenderAPI/BsRenderWindow.cpp"
	"bsfCore/RenderAPI/BsSamplerState.cpp"
	"bsfCore/RenderAPI/BsTextureView.cpp"
	"bsfCore/RenderAPI/BsTimerQuery.cpp"
	"bsfCore/RenderAPI/BsVertexBuffer.cpp"
	"bsfCore/RenderAPI/BsVertexData.cpp"
	"bsfCore/RenderAPI/BsVertexDeclaration.cpp"
	"bsfCore/RenderAPI/BsVideoModeInfo.cpp"
	"bsfCore/RenderAPI/BsRenderAPI.cpp"
	"bsfCore/RenderAPI/BsRenderAPICapabilities.cpp"
	"bsfCore/RenderAPI/BsViewport.cpp"
	"bsfCore/RenderAPI/BsCommandBuffer.cpp"
	"bsfCore/RenderAPI/BsGpuPipelineState.cpp"
	"bsfCore/RenderAPI/BsGpuPipelineParamInfo.cpp"
	"bsfCore/RenderAPI/BsVertexDataDesc.cpp"
	"bsfCore/RenderAPI/BsGpuParamDesc.cpp"
)

set(BS_CORE_SRC_RENDERAPI_MANAGERS
	"bsfCore/Managers/BsGpuProgramManager.cpp"
	"bsfCore/Managers/BsHardwareBufferManager.cpp"
	"bsfCore/Managers/BsMeshManager.cpp"
	"bsfCore/Managers/BsQueryManager.cpp"
	"bsfCore/Managers/BsRenderStateManager.cpp"
	"bsfCore/Managers/BsRenderWindowManager.cpp"
	"bsfCore/Managers/BsRenderAPIManager.cpp"
	"bsfCore/Managers/BsCommandBufferManager.cpp"
	"bsfCore/Managers/BsTextureManager.cpp"
	"bsfCore/Managers/BsResourceListenerManager.cpp"
)

set(BS_CORE_SRC_NOFILTER
	"bsfCore/BsCoreApplication.cpp"
)

set(BS_CORE_SRC_PHYSICS
	"bsfCore/Physics/BsPhysicsManager.cpp"
	"bsfCore/Physics/BsPhysics.cpp"
	"bsfCore/Physics/BsPhysicsMaterial.cpp"
	"bsfCore/Physics/BsCollider.cpp"
	"bsfCore/Physics/BsRigidbody.cpp"
	"bsfCore/Physics/BsBoxCollider.cpp"
	"bsfCore/Physics/BsSphereCollider.cpp"
	"bsfCore/Physics/BsPlaneCollider.cpp"
	"bsfCore/Physics/BsCapsuleCollider.cpp"
	"bsfCore/Physics/BsFCollider.cpp"
	"bsfCore/Physics/BsPhysicsMesh.cpp"
	"bsfCore/Physics/BsMeshCollider.cpp"
	"bsfCore/Physics/BsFJoint.cpp"
	"bsfCore/Physics/BsJoint.cpp"
	"bsfCore/Physics/BsFixedJoint.cpp"
	"bsfCore/Physics/BsDistanceJoint.cpp"
	"bsfCore/Physics/BsHingeJoint.cpp"
	"bsfCore/Physics/BsSliderJoint.cpp"
	"bsfCore/Physics/BsSphericalJoint.cpp"
	"bsfCore/Physics/BsD6Joint.cpp"
	"bsfCore/Physics/BsCharacterController.cpp"
)

set(BS_CORE_SRC_SCENE
	"bsfCore/Scene/BsComponent.cpp"
	"bsfCore/Scene/BsGameObject.cpp"
	"bsfCore/Scene/BsGameObjectHandle.cpp"
	"bsfCore/Scene/BsGameObjectManager.cpp"
	"bsfCore/Scene/BsSceneObject.cpp"
	"bsfCore/Scene/BsSceneManager.cpp"
	"bsfCore/Scene/BsPrefab.cpp"
	"bsfCore/Scene/BsPrefabDiff.cpp"
	"bsfCore/Scene/BsPrefabUtility.cpp"
	"bsfCore/Scene/BsTransform.cpp"
	"bsfCore/Scene/BsSceneActor.cpp"
)

set(BS_CORE_INC_AUDIO
	"bsfCore/Audio/BsAudio.h"
	"bsfCore/Audio/BsAudioClip.h"
	"bsfCore/Audio/BsAudioListener.h"
	"bsfCore/Audio/BsAudioSource.h"
	"bsfCore/Audio/BsAudioClipImportOptions.h"
	"bsfCore/Audio/BsAudioUtility.h"
	"bsfCore/Audio/BsAudioManager.h"
)

set(BS_CORE_SRC_AUDIO
	"bsfCore/Audio/BsAudio.cpp"
	"bsfCore/Audio/BsAudioClip.cpp"
	"bsfCore/Audio/BsAudioListener.cpp"
	"bsfCore/Audio/BsAudioSource.cpp"
	"bsfCore/Audio/BsAudioClipImportOptions.cpp"
	"bsfCore/Audio/BsAudioUtility.cpp"
	"bsfCore/Audio/BsAudioManager.cpp"
)

set(BS_CORE_INC_ANIMATION
	"bsfCore/Animation/BsAnimationCurve.h"
	"bsfCore/Animation/BsAnimationClip.h"
	"bsfCore/Animation/BsSkeleton.h"
	"bsfCore/Animation/BsAnimation.h"
	"bsfCore/Animation/BsAnimationManager.h"
	"bsfCore/Animation/BsCurveCache.h"
	"bsfCore/Animation/BsAnimationUtility.h"
	"bsfCore/Animation/BsSkeletonMask.h"
	"bsfCore/Animation/BsMorphShapes.h"
)

set(BS_CORE_SRC_ANIMATION
	"bsfCore/Animation/BsAnimationCurve.cpp"
	"bsfCore/Animation/BsAnimationClip.cpp"
	"bsfCore/Animation/BsSkeleton.cpp"
	"bsfCore/Animation/BsAnimation.cpp"
	"bsfCore/Animation/BsAnimationManager.cpp"
	"bsfCore/Animation/BsAnimationUtility.cpp"
	"bsfCore/Animation/BsSkeletonMask.cpp"
	"bsfCore/Animation/BsMorphShapes.cpp"
)

set(BS_CORE_INC_PLATFORM
	"bsfCore/Platform/BsPlatform.h"
	"bsfCore/Platform/BsFolderMonitor.h"
	"bsfCore/Platform/BsDropTarget.h"
)

set(BS_CORE_SRC_PLATFORM
	"bsfCore/Platform/BsDropTarget.cpp"
)

set(BS_CORE_INC_PLATFORM_WIN32
	"bsfCore/Private/Win32/BsWin32DropTarget.h"
	"bsfCore/Private/Win32/BsWin32Platform.h"
	"bsfCore/Private/Win32/BsWin32Input.h"
)

set(BS_CORE_SRC_PLATFORM_WIN32
	"bsfCore/Private/Win32/BsWin32FolderMonitor.cpp"
	"bsfCore/Private/Win32/BsWin32Platform.cpp"
	"bsfCore/Private/Win32/BsWin32DropTarget.cpp"
	"bsfCore/Private/Win32/BsWin32Input.cpp"
	"bsfCore/Private/Win32/BsWin32Mouse.cpp"
	"bsfCore/Private/Win32/BsWin32Keyboard.cpp"
	"bsfCore/Private/Win32/BsWin32Gamepad.cpp"
)

set(BS_CORE_INC_PLATFORM_LINUX
	"bsfCore/Private/Linux/BsLinuxPlatform.h"
	"bsfCore/Private/Linux/BsLinuxWindow.h"
	"bsfCore/Private/Linux/BsLinuxDropTarget.h"
	"bsfCore/Private/Linux/BsLinuxInput.h"
)

set(BS_CORE_SRC_PLATFORM_LINUX
	"bsfCore/Private/Linux/BsLinuxPlatform.cpp"
	"bsfCore/Private/Linux/BsLinuxWindow.cpp"
	"bsfCore/Private/Linux/BsLinuxDropTarget.cpp"
	"bsfCore/Private/Linux/BsLinuxFolderMonitor.cpp"
	"bsfCore/Private/Linux/BsLinuxInput.cpp"
	"bsfCore/Private/Linux/BsLinuxGamepad.cpp"
	"bsfCore/Private/Linux/BsLinuxMouse.cpp"
	"bsfCore/Private/Linux/BsLinuxKeyboard.cpp"
)

set(BS_CORE_INC_PLATFORM_MACOS
	"bsfCore/Private/MacOS/BsMacOSInput.h"
	"bsfCore/Private/MacOS/BsMacOSWindow.h"
	"bsfCore/Private/MacOS/BsMacOSPlatform.h"
	"bsfCore/Private/MacOS/BsMacOSDropTarget.h"
)

set(BS_CORE_SRC_PLATFORM_MACOS
	"bsfCore/Private/MacOS/BsMacOSInput.cpp"
	"bsfCore/Private/MacOS/BsMacOSGamepad.cpp"
	"bsfCore/Private/MacOS/BsMacOSMouse.cpp"
	"bsfCore/Private/MacOS/BsMacOSKeyboard.cpp"
	"bsfCore/Private/MacOS/BsMacOSFolderMonitor.cpp"
	"bsfCore/Private/MacOS/BsMacOSDropTarget.cpp"
	"bsfCore/Private/MacOS/BsMacOSWindow.mm"
	"bsfCore/Private/MacOS/BsMacOSPlatform.mm"
)

if(MSVC)
	list(APPEND BS_CORE_INC_PLATFORM "bsfCore/VSVisualizations.natvis")
endif()

if(WIN32)
	list(APPEND BS_CORE_INC_PLATFORM ${BS_CORE_INC_PLATFORM_WIN32})
	list(APPEND BS_CORE_SRC_PLATFORM ${BS_CORE_SRC_PLATFORM_WIN32})
elseif(LINUX)
	list(APPEND BS_CORE_INC_PLATFORM ${BS_CORE_INC_PLATFORM_LINUX})
	list(APPEND BS_CORE_SRC_PLATFORM ${BS_CORE_SRC_PLATFORM_LINUX})
elseif(APPLE)
	list(APPEND BS_CORE_INC_PLATFORM ${BS_CORE_INC_PLATFORM_MACOS})
	list(APPEND BS_CORE_SRC_PLATFORM ${BS_CORE_SRC_PLATFORM_MACOS})
endif()

source_group("Components" FILES ${BS_CORE_INC_COMPONENTS} ${BS_CORE_SRC_COMPONENTS})
source_group("Physics" FILES ${BS_CORE_INC_PHYSICS} ${BS_CORE_SRC_PHYSICS})
source_group("CoreThread" FILES ${BS_CORE_INC_CORETHREAD} ${BS_CORE_SRC_CORETHREAD})
source_group("Importer" FILES ${BS_CORE_INC_IMPORTER} ${BS_CORE_SRC_IMPORTER})
source_group("Scene" FILES ${BS_CORE_INC_SCENE} ${BS_CORE_SRC_SCENE})
source_group("Input" FILES ${BS_CORE_INC_INPUT} ${BS_CORE_SRC_INPUT})
source_group("Platform" FILES ${BS_CORE_INC_PLATFORM} ${BS_CORE_SRC_PLATFORM})
source_group("Renderer" FILES ${BS_CORE_INC_RENDERER} ${BS_CORE_SRC_RENDERER})
source_group("Profiling" FILES ${BS_CORE_INC_PROFILING} ${BS_CORE_SRC_PROFILING})
source_group("RenderAPI" FILES ${BS_CORE_INC_RENDERAPI} ${BS_CORE_SRC_RENDERAPI})
source_group("RenderAPI\\Managers" FILES ${BS_CORE_INC_RENDERAPI_MANAGERS} ${BS_CORE_SRC_RENDERAPI_MANAGERS})
source_group("Material" FILES ${BS_CORE_INC_MATERIAL} ${BS_CORE_SRC_MATERIAL})
source_group("Resources" FILES ${BS_CORE_INC_RESOURCES} ${BS_CORE_SRC_RESOURCES})
source_group("Text" FILES ${BS_CORE_INC_TEXT} ${BS_CORE_SRC_TEXT})
source_group("Utility" FILES ${BS_CORE_INC_UTILITY} ${BS_CORE_SRC_UTILITY})
source_group("RTTI" FILES ${BS_CORE_INC_RTTI} ${BS_CORE_SRC_RTTI})
source_group("Localization" FILES ${BS_CORE_INC_LOCALIZATION} ${BS_CORE_SRC_LOCALIZATION})
source_group("Audio" FILES ${BS_CORE_INC_AUDIO} ${BS_CORE_SRC_AUDIO})
source_group("Animation" FILES ${BS_CORE_INC_ANIMATION} ${BS_CORE_SRC_ANIMATION})
source_group("Image" FILES ${BS_CORE_INC_IMAGE} ${BS_CORE_SRC_IMAGE})
source_group("Mesh" FILES ${BS_CORE_INC_MESH} ${BS_CORE_SRC_MESH})
source_group("" FILES ${BS_CORE_INC_NOFILTER} ${BS_CORE_SRC_NOFILTER})

if(APPLE)
	source_group("MacOS" FILES ${BS_CORE_INC_PLATFORM_MACOS} ${BS_CORE_SRC_PLATFORM_MACOS})
endif()

set(BS_CORE_SRC
	${BS_CORE_INC_COMPONENTS}
	${BS_CORE_INC_PHYSICS}
	${BS_CORE_INC_CORETHREAD}
	${BS_CORE_INC_IMPORTER}
	${BS_CORE_INC_SCENE}
	${BS_CORE_INC_INPUT}
	${BS_CORE_INC_PLATFORM}
	${BS_CORE_INC_RENDERER}
	${BS_CORE_SRC_LOCALIZATION}
	${BS_CORE_SRC_RTTI}
	${BS_CORE_INC_PROFILING}
	${BS_CORE_INC_RENDERAPI}
	${BS_CORE_SRC_CORETHREAD}
	${BS_CORE_INC_NOFILTER}
	${BS_CORE_INC_MATERIAL}
	${BS_CORE_INC_RESOURCES}
	${BS_CORE_SRC_UTILITY}
	${BS_CORE_INC_TEXT}
	${BS_CORE_SRC_PROFILING}
	${BS_CORE_SRC_COMPONENTS}
	${BS_CORE_SRC_PLATFORM}
	${BS_CORE_SRC_IMPORTER}
	${BS_CORE_INC_UTILITY}
	${BS_CORE_INC_RTTI}
	${BS_CORE_SRC_RENDERER}
	${BS_CORE_SRC_RESOURCES}
	${BS_CORE_SRC_MATERIAL}
	${BS_CORE_SRC_INPUT}
	${BS_CORE_INC_LOCALIZATION}
	${BS_CORE_SRC_TEXT}
	${BS_CORE_SRC_RENDERAPI}
	${BS_CORE_SRC_NOFILTER}
	${BS_CORE_SRC_PHYSICS}
	${BS_CORE_SRC_SCENE}
	${BS_CORE_INC_AUDIO}
	${BS_CORE_SRC_AUDIO}
	${BS_CORE_INC_ANIMATION}
	${BS_CORE_SRC_ANIMATION}
	${BS_CORE_INC_RENDERAPI_MANAGERS}
	${BS_CORE_SRC_RENDERAPI_MANAGERS}
	${BS_CORE_INC_IMAGE}
	${BS_CORE_SRC_IMAGE}
	${BS_CORE_INC_MESH}
	${BS_CORE_SRC_MESH}
)
