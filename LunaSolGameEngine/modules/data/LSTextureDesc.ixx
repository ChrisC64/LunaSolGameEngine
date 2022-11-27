module;
#include "LSEFramework.h"

export module Data.LSTextureDesc;

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
		POINT, // @brief Also referred to as "nearest", this simply returns the closest pixel
		BILINEAR, // @brief Returns the pixel after blending pixels together
		TRILINEAR, // @brief Same concept as BILINEAR but more additional computation for improved visual clarity
		ANISOTROPIC // @brief Generally the best for visual quality, but most expensive computation
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
		TEX_FILTER_MODE FILTER;
		TEX_ADDRESS_MODE ADDRESS_U;
		TEX_ADDRESS_MODE ADDRESS_V;
		TEX_ADDRESS_MODE ADDRESS_W;

		bool operator==(const TextureRenderState& rhs) const
		{
			return (this->FILTER == rhs.FILTER)
				&& (this->ADDRESS_U == rhs.ADDRESS_U)
				&& (this->ADDRESS_V == rhs.ADDRESS_V)
				&& (this->ADDRESS_W == rhs.ADDRESS_W);
		}

		bool operator!=(const TextureRenderState& rhs) const
		{
			return (this->FILTER != rhs.FILTER)
				|| (this->ADDRESS_U != rhs.ADDRESS_U)
				|| (this->ADDRESS_V != rhs.ADDRESS_V)
				|| (this->ADDRESS_W != rhs.ADDRESS_W);
		}
	};

	//TODO: All data ttypes should use a singluar engine wide data enum descriptor, see PIXEL_DATA_FORMAT
	enum class PIXEL_DATA_TYPE
	{
		TYPELESS = 0,
		FLOAT,
		DOUBLE,
		INT32,
		UINT32,
		INT16,
		UINT16,
		BYTE,
		SIGNED_BYTE
	};

	enum class PIXEL_COLOR
	{
		NONE = 0,
		RED,
		GREEN,
		BLUE,
		CYAN,
		MAGENTA,
		YELLOW,
		ALPHA
	};

	enum class TEXTURE_ARRAY_TYPE
	{
		ARRAY_1D = 1,
		ARRAY_2D = 2,
		ARRAY_3D = 3
	};

	enum class PIXEL_FORMAT : int32_t
	{
		RGBA_8,
		ARGB_8,
		BGRA_8,
		ABGR_8,
		RGBA_16,
		ARGB_16,
		BGRA_16,
		ABGR_16
	};

	struct LSTextureDesc
	{
		std::array<PIXEL_COLOR, 4> PixelFormat; // @brief How is the data stored?
		PIXEL_DATA_TYPE PixelDataType; // @brief How the data type is stored to represent each value
		uint32_t Width; // @brief Number of Horizontal pixels (X axis)
		uint32_t Height; // @brief Number of Vertial pixels (Y Axis)
		uint32_t MipMapLevels; // @brief Number of mip maps levels this texture has
		uint32_t ArrayCount; // @brief Number of objects if more than one texture
		uint32_t SampleCount; // @brief Used for MSAA count
		uint32_t SampleQuality;// @brief Used for MSAA quality
		TEXTURE_ARRAY_TYPE ArrayType; //@brief Informs how we should read the array
	};
}