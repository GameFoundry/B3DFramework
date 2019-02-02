//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#if IS_B3D
namespace BansheeEngine
#else
namespace bs
#endif
{
    /** @addtogroup Rendering
     *  @{
     */

    public partial class Material
    {
        /// <summary>
        /// Returns a texture assigned to the material. If the material has a sprite texture assigned, this will return
        /// the parent texture of the sprite.
        /// </summary>
        /// <param name="name">Name of the texture parameter.</param>
        /// <returns>Texture assigned to the specified material</returns>
        public RRef<Texture> GetTexture(string name)
        {
            return Internal_getTexture(mCachedPtr, name);
        }

        /// <summary>
        /// Assigns a texture to the specified material parameter. Allows you to specify a surface parameter that allows you
        /// to bind only a sub-set of the texture.
        /// </summary>
        /// <param name="name">Name of the texture parameter.</param>
        /// <param name="texture">Texture resource to assign.</param>
        /// <param name="surface">Subset of the texture to assign.</param>
        public void SetTexture(string name, RRef<Texture> texture, TextureSurface surface)
        {
            Internal_setTexture(mCachedPtr, name, texture, surface.mipLevel, surface.numMipLevels, surface.face, 
                surface.numFaces);
        }

        /// <summary>
        /// Assigns a texture to the specified material parameter.
        /// </summary>
        /// <param name="name">Name of the texture parameter.</param>
        /// <param name="texture">Texture resource to assign.</param>
        public void SetTexture(string name, RRef<Texture> texture)
        {
            Internal_setTexture(mCachedPtr, name, texture, 0, 0, 0, 0);
        }

        /// <summary>
        /// Returns a sprite texture assigned to the material.
        /// </summary>
        /// <param name="name">Name of the texture parameter.</param>
        /// <returns>Texture assigned to the specified material</returns>
        public RRef<SpriteTexture> GetSpriteTexture(string name)
        {
            return Internal_getSpriteTexture(mCachedPtr, name);
        }

        /// <summary>
        /// Assigns a sprite texture to the specified material parameter. Sprite texture is allowed to be animated, or just
        /// used for referencing a subset of a texture atlas.
        /// </summary>
        /// <param name="name">Name of the texture parameter.</param>
        /// <param name="texture">Texture resource to assign.</param>
        public void SetSpriteTexture(string name, RRef<SpriteTexture> texture)
        {
            Internal_setSpriteTexture(mCachedPtr, name, texture);
        }
    }

    /** @} */
}
