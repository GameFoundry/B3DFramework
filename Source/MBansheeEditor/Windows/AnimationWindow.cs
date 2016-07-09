//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
using BansheeEngine;

namespace BansheeEditor
{
    /** @addtogroup Windows
     *  @{
     */

    /// <summary>
    /// Displays animation curve editor window.
    /// </summary>
    internal class AnimationWindow : EditorWindow
    {
        private GUITimeline timeline;

        /// <summary>
        /// Opens the animation window.
        /// </summary>
        [MenuItem("Windows/Animation", ButtonModifier.CtrlAlt, ButtonCode.A, 6000)]
        private static void OpenGameWindow()
        {
            OpenWindow<AnimationWindow>();
        }

        /// <inheritdoc/>
        protected override LocString GetDisplayName()
        {
            return new LocEdString("Animation");
        }

        private void OnInitialize()
        {
            timeline = new GUITimeline(GUI, 300, 40);
        }

        private void OnEditorUpdate()
        {
            //int position = (int)(MathEx.Sin(Time.RealElapsed)*50.0f + 50.0f);
            //canvas.SetPosition(position, 0);
        }

        private void OnDestroy()
        {
            // TODO
        }
    }

    /** @} */
}
