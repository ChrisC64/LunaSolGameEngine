module;
#include "LSEFramework.h"

export module Engine.LSDevice;
export import Data.LSTextureDesc;
export import Util.StdUtils;

export namespace LS
{
	enum class DEVICE_TYPE : int8_t
	{
		HARDWARE,
		SOFTWARE,
		UNKNOWN
	};

	enum class DEVICE_API
	{
		D3D11,
		D3D12,
		VULKAN
	};

	/**
	 * @brief Details how the object should be drawn
	 */
	enum class FILL_STATE : int8_t
	{
		FILL,
		WIREFRAME
	};

	/**
	 * @brief Details how faces should be culled
	 *
	 * None means draw all faces, even those not visible.
	 * Front means cull front facing triangles
	 * Back will cull back facing triangles
	 */
	enum class CULL_METHOD : int8_t
	{
		NONE,
		FRONT,
		BACK
	};

	enum class DEVICE_EVENT : int32_t
	{
		ON_DEVICE_CREATE,
		ON_DEVICE_SHUTDOWN,
		ON_DEVICE_LOST,
		ON_DEVICE_RESTORE,
		ON_RENDER_TARGET_RESIZE_START,
		ON_RENDER_TARGET_RESIZE_COMPLETE
	};

	// Callbacks // 
	using OnDeviceEvent = std::function<void(DEVICE_EVENT)>;

	struct LSDrawState
	{
		FILL_STATE Fill;
		CULL_METHOD Cull;
		bool IsFrontCounterClockwise; // @brief Front Face drawn counterclockwise = true, false if not 
		bool IsDepthClipEnabled;

		bool operator==(const LSDrawState& rhs) const
		{
			return Fill == rhs.Fill &&
				IsFrontCounterClockwise == rhs.IsFrontCounterClockwise &&
				Cull == rhs.Cull &&
				IsDepthClipEnabled == rhs.IsDepthClipEnabled;
		}

		bool operator!=(const LSDrawState& rhs) const
		{
			return Fill != rhs.Fill ||
				IsFrontCounterClockwise != rhs.IsFrontCounterClockwise ||
				Cull != rhs.Cull ||
				IsDepthClipEnabled != rhs.IsDepthClipEnabled;
		}
	};
	

	struct LSDrawStateHashFunc
	{
		template<typename T = LS::LSDrawState>
		std::size_t operator()(T const& t) const noexcept
		{
			std::size_t h1 = LS::Utils::HashEnum(t.Fill);
			std::size_t h2 = t.IsFrontCounterClockwise ? 1 : 0;
			std::size_t h3 = LS::Utils::HashEnum(t.Cull);
			std::size_t h4 = t.IsDepthClipEnabled ? 1 : 0;

			return h1 ^ h2 ^ h3 ^ h4;
		}
	};

	struct LSSwapchainInfo
	{
		uint32_t		BufferSize{ 2u };
		uint32_t		Width;
		uint32_t		Height;
		PIXEL_FORMAT	PixelFormat{ PIXEL_FORMAT::RGBA_8 };
		bool			IsStereoScopic{ false };
		uint16_t		MSCount{ 1 };
		uint16_t		MSQuality{ 0 };
	};

	struct LSDeviceInfo
	{
		std::string Info;
	};

	struct LSDeviceSettings
	{
		int32_t FPSTarget;
		bool IsVSync;
		int32_t FrameBufferCount;
		LSTextureDesc RenderTargetDesc;
		LSSwapchainInfo SwapchainInfo;
	};

    struct LSDevice
    {
		DEVICE_TYPE DeviceType;
		DEVICE_API DeviceApi;
		OnDeviceEvent OnDeviceEvent;
    };
}