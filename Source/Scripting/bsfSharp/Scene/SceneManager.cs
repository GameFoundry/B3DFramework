//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
using System;
using System.Runtime.CompilerServices;

namespace bs
{
    /** @addtogroup Scene
     *  @{
     */
    public partial class SceneManager
    {
        private static RRef<Prefab> activateOnLoadScene;

        /// <summary>
        /// Returns the main camera that controls the final render surface that is displayed to the user. If the current
        /// scene has no main camera null is returned.
        /// </summary>
        public static Camera Camera
        {
            get
            {
                SceneObject so = Internal_GetMainCameraSceneObject();

                if (so == null)
                    return null;

                return so.GetComponent<Camera>();
            }
        }

        /// <summary>
        /// Loads a new scene asynchronously.
        /// </summary>
        /// <param name="path">Path to the prefab to load.</param>
        /// <returns>Handle to the prefab of the scene at the provided path.</returns>
        public static RRef<Prefab> LoadMainSceneAsync(string path)
        {
            ClearMainScene();

            activateOnLoadScene = Resources.LoadAsReference<Prefab>(path);

            if (activateOnLoadScene != null && activateOnLoadScene.IsLoaded)
            {
                SetActiveMainScene(activateOnLoadScene.Value);
                activateOnLoadScene = null;
            }

            return activateOnLoadScene;
        }


        /// <summary>
        /// Clears all scene objects from the current scene.
        /// </summary>
        public static void ClearMainScene()
        {
            activateOnLoadScene = null;
            Internal_ClearMainScene(false);
        }


        /// <summary>
        /// Called once per frame by the runtime.
        /// </summary>
        internal static void Update()
        {
            if (activateOnLoadScene != null && activateOnLoadScene.IsLoaded)
            {
                SetActiveMainScene(activateOnLoadScene.Value);
                activateOnLoadScene = null;
            }
        }

        /// <summary>
        /// Makes the provided scene resource the current main scene instance.
        /// </summary>
        /// <param name="scene">Previously loaded scene resource.</param>
        private static void SetActiveMainScene(Prefab scene)
        {
            if(scene != null)
            {
                // If scene replace current root node, otherwise just append to the current root node
                if(scene.IsScene)
                    LoadMainScene(scene);
                else
                {
                    ClearMainScene();
                    scene.Instantiate();
                }
            }
            else
            {
                Debug.LogError("Attempting to activate a scene that hasn't finished loading yet.", "Scene");
            }
        }
    }

    /** @} */
}
