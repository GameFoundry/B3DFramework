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
        private GUIFloatField lengthField;
        private GUIIntField fpsField;

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
            lengthField = new GUIFloatField(new LocEdString("Length"), 50);
            fpsField = new GUIIntField(new LocEdString("FPS"), 50);

            lengthField.Value = 60.0f;
            fpsField.Value = 1;

            lengthField.OnChanged += x => timeline.SetRange(lengthField.Value);
            fpsField.OnChanged += x => timeline.SetFPS(x);

            GUILayout buttonLayout = GUI.AddLayoutX();
            buttonLayout.AddElement(lengthField);
            buttonLayout.AddElement(fpsField);

            timeline = new GUITimeline(GUI, Width, 20);
        }

        protected override void WindowResized(int width, int height)
        {
            timeline.SetSize(width, 20);
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
