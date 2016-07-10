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
        private GUIFloatField startField;
        private GUIFloatField endField;
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
            startField = new GUIFloatField(new LocEdString("Start"), 50);
            endField = new GUIFloatField(new LocEdString("End"), 50);
            fpsField = new GUIIntField(new LocEdString("FPS"), 50);

            endField.Value = 60.0f;
            fpsField.Value = 1;

            startField.OnChanged += x => timeline.SetRange(x, endField.Value);
            endField.OnChanged += x => timeline.SetRange(startField.Value, x);
            fpsField.OnChanged += x => timeline.SetFPS(x);

            GUILayout buttonLayout = GUI.AddLayoutX();
            buttonLayout.AddElement(startField);
            buttonLayout.AddElement(endField);
            buttonLayout.AddElement(fpsField);

            timeline = new GUITimeline(GUI, 300, 20);
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
