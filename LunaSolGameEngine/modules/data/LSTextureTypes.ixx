module;
#include <cstdint>
#include <array>
#include <vector>

export module Data.LSTextureTypes;

export namespace LS
{
    /**
     * @brief Details how pixels are filtered on the texture.
     *
     * This details how the texture should be filtered to control
     * how it will represent itself when viewed. A texture filter
     * controls how the pixels will be presented from the texel (texture pixel).
     * This is generally used to reduce noise of a picture or smooth transition
     * from blockier resolutions when skewed or viewed at a different resolution
     * than its proximity to the camera. (Higher res images
     * from further away are noisier because of the extra detail, and low res images
     * are blockier because of their enlarged pixels when closer to the camera)
     * Note that if Anisotropic is selected, this will result in MIP using ANISOTROPIC
     * filtering mode as well.
    */
    enum class TEX_FILTER_MODE
    {
        DEFAULT,
        POINT, // @brief Also referred to as "nearest", this simply returns the closest pixel
        LINEAR
    };

    enum class TEX_FILTER_METHOD
    {
        POINT,
        BILINEAR,
        TRILINEAR,
        ANISOTROPIC,
        USER_MISC
    };

    enum class TEX_FILTER_MODE_TYPE
    {
        DEFAULT,
        MINIMUM,
        MAXIMUM,
        COMPARE
    };

    /**
     * @brief The Address Mode of a texture
     *
     * Address mode effects the texture's display in relation to
     * it's coordinates (usually denoated as U,V). Given a U,V
     * of (2,2) you would see something along with the following.
     * Wrap - The image is tiled, because wrap copies the image exactly
     * to the sides and above and below the texture.
     * Mirror - The image will be "flipped" - similar to wrap, but the pixels
     * flip after each copy.
     * Clamp - The edges of the pixels weill be stretched to fill out the remainder
     * of the plane while the picture remains in its position.
     * Border - Fill the edges with a border color
     * Mirror Once - A combination of CLAMP and Mirror. After mirroing once
     * the image will then clamp for the remainder of the plane if needed.
    */
    enum class TEX_ADDRESS_MODE
    {
        WRAP,
        MIRROR,
        CLAMP,
        BORDER,
        MIRROR_ONCE
    };

    /**
     * @brief Informs how we want to handle the texture's drawing
     *
     * This sets properties like TEX_ADDRESS_MODE and TEX_FILTER_MODE which
     * detail how the texture will render.
    */
    struct TextureRenderState
    {
        TEX_FILTER_METHOD METHOD;
        TEX_FILTER_MODE MIN_FILTER;//@brief only used if method is "USER_MISC" 
        TEX_FILTER_MODE MAG_FILTER;//@brief only used if method is "USER_MISC" 
        TEX_FILTER_MODE MIP_FILTER;//@brief only used if method is "USER_MISC" 
        TEX_FILTER_MODE_TYPE FILTER_TYPE;
        TEX_ADDRESS_MODE ADDRESS_U;
        TEX_ADDRESS_MODE ADDRESS_V;
        TEX_ADDRESS_MODE ADDRESS_W;
        std::array<float, 4> BorderColor;
        bool operator==(const TextureRenderState& rhs) const
        {
            return (this->METHOD == rhs.METHOD)
                && (this->MIN_FILTER == rhs.MIN_FILTER)
                && (this->MAG_FILTER == rhs.MAG_FILTER)
                && (this->MIP_FILTER == rhs.MIP_FILTER)
                && (this->ADDRESS_U == rhs.ADDRESS_U)
                && (this->ADDRESS_V == rhs.ADDRESS_V)
                && (this->ADDRESS_W == rhs.ADDRESS_W)
                && (this->BorderColor == rhs.BorderColor);
        }

        bool operator!=(const TextureRenderState& rhs) const
        {
            return !(*this == rhs);
        }
    };

    enum class TEXTURE_ARRAY_TYPE : uint8_t
    {
        ARRAY_1D = 1,
        ARRAY_2D = 2,
        ARRAY_3D = 3
    };

   /**
    * @brief Informs us the size of each bytes per pixel (BPP) data type and data type representation.
    *
    * The format is expected to be [Layout][BPP]_[Data Type] and could end up mixed for other
    * data types such as X24SINT_R8 or some other combination.
    * SINT = signed int, meant to represent generally a data type with signed (-/+) values
    * and UINT = unsigned int meant to represent data types that are unsigned (+)
    * FLOAT is used to represent values with decimal values like a floating point
    * UNKNOWN is meant to represent just a format, and the type is not known.
    * UNORM = unsigned normalized meaning something with a range of 0.0-1.0 values
    * SNORM = signed normalized value for ranges between -1.0 - 1.0 values
   */
    enum class PIXEL_COLOR_FORMAT
    {
        UNKNOWN,
        // 8 BPP //
        // ARGB //
        ARGB8_UNKNOWN, ARGB8_SINT, ARGB8_UINT, ARGB8_UNORM, ARGB8_SNORM,
        // ABGR //
        ABGR8_UNKNOWN, ABGR8_SINT, ABGR8_UINT, ABGR8_UNORM, ABGR8_SNORM,
        // RGBA //
        RGBA8_UNKNOWN, RGBA8_SINT, RGBA8_UINT, RGBA8_UNORM, RGBA8_SNORM,
        // BGRA //
        BGRA8_UNKNOWN, BGRA8_SINT, BGRA8_UINT, BGRA8_UNORM, BGRA8_SNORM,

        // 16 BPP //
        // ARGB //
        ARGB16_UNKNOWN, ARGB16_FLOAT, ARGB16_SINT, ARGB16_UINT, ARGB16_UNORM, ARGB16_SNORM,
        // ABGR //
        ABGR16_UNKNOWN, ABGR16_FLOAT, ABGR16_SINT, ABGR16_UINT, ABGR16_UNORM, ABGR16_SNORM,
        // RGBA //
        RGBA16_UNKNOWN, RGBA16_FLOAT, RGBA16_SINT, RGBA16_UINT, RGBA16_UNORM, RGBA16_SNORM,
        // BGRA //
        BGRA16_UNKNOWN, BGRA16_FLOAT, BGRA16_SINT, BGRA16_UINT, BGRA16_UNORM, BGRA16_SNORM,

        // 32 BPP //
        // ARGB //
        ARGB32_UNKNOWN, ARGB32_FLOAT, ARGB32_SINT, ARGB32_UINT,
        // ABGR //
        ABGR32_UNKNOWN, ABGR32_FLOAT, ABGR32_SINT, ABGR32_UINT,
        // RGBA //
        RGBA32_UNKNOWN, RGBA32_FLOAT, RGBA32_SINT, RGBA32_UINT,
        // BGRA //
        BGRA32_UNKNOWN, BGRA32_FLOAT, BGRA32_SINT, BGRA32_UINT,

        // Depth Stencil Specific Formats //
        DEPTH24_UNORM_STENCIL8_UINT, DEPTH32_FLOAT, DEPTH32_SFLOAT_STENCIL8_UINT, DEPTH32_STENCIL8X24_UINT
    };

    struct LSTextureInfo
    {
        PIXEL_COLOR_FORMAT PixelFormat; // @brief How is the data stored?
        uint32_t Width; // @brief Number of Horizontal pixels (X axis)
        uint32_t Height; // @brief Number of Vertial pixels (Y Axis)
        uint32_t MipMapLevels; // @brief Number of mip maps levels this texture has
        uint32_t ArrayCount; // @brief Number of objects if more than one texture
        uint32_t SampleCount; // @brief Used for MSAA count
        uint32_t SampleQuality;// @brief Used for MSAA quality
        TEXTURE_ARRAY_TYPE ArrayType; //@brief Informs how we should read the array
        std::vector<std::byte> Data; //@brief the Texture data
    };
}