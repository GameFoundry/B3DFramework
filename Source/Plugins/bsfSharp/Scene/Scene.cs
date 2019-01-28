//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Scene
     *  @{
     */

    /// <summary>
    /// Handles operations with the active scene (level).
    /// </summary>
    public static class Scene
    {
        /// <summary>
        /// Returns the name of the scene prefab. This is empty if scene hasn't been saved yet.
        /// </summary>
        public static string ActiveSceneName { get { return activeSceneName; } }

        /// <summary>
        /// Returns the UUID of the scene prefab. This is empty if scene hasn't been saved yet.
        /// </summary>
        internal static UUID ActiveSceneUUID { get { return activeSceneUUID; } }

        /// <summary>
        /// Checks is the loaded scene a generic scene object group, instead of an actual scene. 
        /// <see cref="Prefab.IsScene"/>.
        /// </summary>
        internal static bool IsGenericPrefab { get { return isGenericPrefab; } }

        private static string activeSceneName = "Unnamed";
        private static UUID activeSceneUUID;
        private static bool isGenericPrefab = false;
        private static RRef<Prefab> activateOnLoadScene;

        /// <summary>
        /// Returns the root scene object for the current scene.
        /// </summary>
        public static SceneObject Root
        {
            get { return Internal_GetRoot(); }
        }

        /// <summary>
        /// Returns the main camera that controls the final render surface that is displayed to the user. If the current
        /// scene has no main camera null is returned.
        /// </summary>
        public static Camera Camera
        {
            get
            {
                SceneObject so = Internal_GetMainCameraSO();

                if (so == null)
                    return null;

                return so.GetComponent<Camera>();
            }
        }

        /// <summary>
        /// Clears all scene objects from the current scene.
        /// </summary>
        public static void Clear()
        {
            Internal_ClearScene();

            activeSceneUUID = UUID.Empty;
            activeSceneName = "Unnamed";
            activateOnLoadScene = null;
        }

        /// <summary>
        /// Loads a new scene.
        /// </summary>
        /// <param name="path">Path to the prefab to load.</param>
        /// <returns>Prefab of the scene at the provided path.</returns>
        public static Prefab Load(string path)
        {
            Clear();

            Prefab scene = Resources.Load<Prefab>(path);
            SetActive(scene);

            return scene;
        }

        /// <summary>
        /// Loads a new scene asynchronously.
        /// </summary>
        /// <param name="path">Path to the prefab to load.</param>
        /// <returns>Handle to the prefab of the scene at the provided path.</returns>
        public static RRef<Prefab> LoadAsync(string path)
        {
            Clear();

            activateOnLoadScene = Resources.LoadAsync<Prefab>(path); 

            if(activateOnLoadScene != null && activateOnLoadScene.IsLoaded)
                SetActive(activateOnLoadScene.Value);

            return activateOnLoadScene;
        }
        /// <summary>
        /// Sets the currently active scene to the provided scene.
        /// </summary>
        /// <param name="scene">Scene which to set as active.</param>
        internal static void SetActive(Prefab scene)
        {
            activateOnLoadScene = null;

            if (scene != null)
            {
                activeSceneUUID = scene.UUID;
                activeSceneName = scene.Name;
                isGenericPrefab = !scene.IsScene;

                Internal_SetActiveScene(scene.GetCachedPtr());
            }
        }

        /// <summary>
        /// Called once per frame by the runtime.
        /// </summary>
        private static void OnUpdate()
        {
            if (activateOnLoadScene != null && activateOnLoadScene.IsLoaded)
                SetActive(activateOnLoadScene.Value);
        }

        /// <summary>
        /// Wrapper around scene name static field because Mono has problems accessing static fields directly.
        /// </summary>
        /// <returns>Name of the currently active scene.</returns>
        private static string GetSceneName()
        {
            return activeSceneName;
        }

        /// <summary>
        /// Wrapper around scene name static field because Mono has problems accessing static fields directly.
        /// </summary>
        /// <param name="name">Name of the currently active scene.</param>
        private static void SetSceneName(string name)
        {
            activeSceneName = name;
        }

        /// <summary>
        /// Wrapper around scene UUID static field because Mono has problems accessing static fields directly.
        /// </summary>
        /// <returns>UUID of the currently active scene.</returns>
        private static UUID GetSceneUUID()
        {
            return activeSceneUUID;
        }

        /// <summary>
        /// Wrapper around scene UUID static field because Mono has problems accessing static fields directly.
        /// </summary>
        /// <param name="uuid">UUID of the currently active scene.</param>
        private static void SetSceneUUID(UUID uuid)
        {
            activeSceneUUID = uuid;
        }

        /// <summary>
        /// Wrapper around the isGenericPrefab static field because Mono has problems accessing static fields directly.
        /// </summary>
        /// <returns>True if the loaded scene a generic scene object group, instead of an actual scene.</returns>
        private static bool GetIsGenericPrefab()
        {
            return isGenericPrefab;
        }

        /// <summary>
        /// Wrapper around scene UUID static field because Mono has problems accessing static fields directly.
        /// </summary>
        /// <param name="isGenericPrefab">Determines is the loaded scene a generic scene object group, instead of an actual scene.
        ///                    </param>
        private static void SetIsGenericPrefab(bool isGenericPrefab)
        {
            Scene.isGenericPrefab = isGenericPrefab;
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_SetActiveScene(IntPtr prefab);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetRoot();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ClearScene();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetMainCameraSO();
    }

    /** @} */
}
