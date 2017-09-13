//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using System.Runtime.CompilerServices;

namespace BansheeEngine
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
        }

        /// <summary>
        /// Loads a new scene.
        /// </summary>
        /// <param name="path">Path to the prefab to load.</param>
        public static void Load(string path)
        {
            Clear();
            Prefab scene = Internal_LoadScene(path);

            SetActive(scene);
        }

        /// <summary>
        /// Sets the currently active scene to the provided scene.
        /// </summary>
        /// <param name="scene">Scene which to set as active.</param>
        internal static void SetActive(Prefab scene)
        {
            if (scene != null)
            {
                activeSceneUUID = scene.UUID;
                activeSceneName = scene.Name;
                isGenericPrefab = !scene.IsScene;
            }
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
        private static extern Prefab Internal_LoadScene(string path);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetRoot();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void Internal_ClearScene();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern SceneObject Internal_GetMainCameraSO();
    }

    /** @} */
}
