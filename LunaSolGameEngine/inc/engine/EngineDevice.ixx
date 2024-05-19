module;
#include <cstdint>
#include <functional>
#include <queue>
#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <array>
export module Engine.LSDevice;

import Engine.LSWindow;
import Engine.EngineCodes;
import Engine.Shader;
import Engine.Defines;
import LSEDataLib;
import Util.StdUtils;

namespace LS
{
    export enum class DEVICE_TYPE : int8_t 
    {
        HARDWARE,
        SOFTWARE,
        UNKNOWN
    };

    export enum class DEVICE_API
    {
        NONE,
        DIRECTX_11,
        DIRECTX_12,
    };

    /**
     * @brief Details how the object should be drawn
     */
    export enum class FILL_STATE : int8_t 
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
    export enum class CULL_METHOD : int8_t 
    {
        NONE,
        FRONT,
        BACK
    };

    export enum class DEVICE_EVENT : int32_t 
    {
        ON_DEVICE_CREATE,
        ON_DEVICE_SHUTDOWN,
        ON_DEVICE_LOST,
        ON_DEVICE_RESTORE,
        ON_RENDER_TARGET_RESIZE_START,
        ON_RENDER_TARGET_RESIZE_COMPLETE
    };

    /**
     * @brief Informs what we should do with the RGBA values
     *
     * Blend factors use a formula:
     * C = RGB color
     * A = Alpha
     * F = Blend Factor (this enum)
     * OP = LS::Core::BLEND_OPERATION
     *
     * Given the formula
     * C = Csrc X Fsrc + Cdst X Fdst
     * A = Asrc X Fsrc + Adst X Fdst
     *
     * The blend factor is what you'll plug as F in the
     * equation above and what happens to the outcome
     * will be reflected in the equation.
     * An example:
     * If F was value of ONE and INV_SRC_ALPHA for C
     * C = Csrc X (1, 1, 1) (for RGB) + Cdst X (1 - Alpha, 1 - Alpha, 1 - Alpha)
     *
     * Alpha would be the 4th component of a color struct and not the output A in the
     * example above. A will be the Alpha channels of the end of the draw.
     * See descriptions of each blend factor below.
     *
     * Source (src) is the given object's input values and destination is the returned
     * value. When blending, you'll generally have multiple blends to create the final
     * output, so considering the order in which you draw objects you may notice multiple
     * transparencies or other blending setups to create the effect you desire.
     */
    export enum class BLEND_FACTOR 
    {
        ZERO,           // @brief Use a set of 0s for the blend factor equation
        ONE,            // @brief Use a set of 1s for the blend factor equation
        SRC_COLOR,      // @brief Use the src color's RGB values
        DEST_COLOR,     // @brief Use the destination's RGB values
        SRC_ALPHA,      // @brief Use the src's alpha channel
        DEST_ALPHA,     // @brief Use the desintation's alpha channel
        INV_SRC_COLOR,  // @brief Inverse (1 - R, 1 - G, 1 - B) the color based on the src RGB
        INV_SRC_ALPHA,  // @brief Inverse the Alpha (1 - A, 1 - A, 1 - A) from the src values
        INV_DEST_COLOR, // @brief Inverse (1 - R, 1 - G, 1 - B) the color based on the dest RGB
        INV_DEST_ALPHA, // @brief Inverse the Alpha (1 - A, 1 - A, 1 - A) from the dest values
    };

    /**
     * @brief Details the action to take on the blending procedure.
     */
    export enum class BLEND_OPERATION 
    {
        BLEND_ADD,     // @brief Blends data by adding data from first and second objects
        BLEND_SUB,     // @brief Blends data by subtracting data from first and second object
        BLEND_REV_SUB, // @brief Blend data by subtracting from second to first object
        BLEND_MIN,     // @brief Find the minimum average between each src data and blend together
        BLEND_MAX      // @brief Find the max data between both sources and blend them
    };

    export enum class COLOR_CHANNEL_MASK : uint8_t 
    {
        RED = 1,
        GREEN = 2,
        BLUE = 4,
        ALPHA = 8,
        ALL = (((RED | GREEN) | BLUE) | ALPHA)
    };

    // @brief One of the various supported blend types, use Custom to create a custom operation
    export enum class BlendType
    {
        OPAQUE_BLEND,
        ALPHA_BLEND,
        ADDITIVE_BLEND,
        PRE_MULT_ALPHA_BLEND,
        CUSTOM_BLEND,
    };

    // @brief The struct defining RGB and Alpha channel blend operations
    export struct BlendOps
    {
        struct Channel
        {
            BLEND_OPERATION BlendOp;
            BLEND_FACTOR Src;
            BLEND_FACTOR Dest;
        };

        Channel Rgb;
        Channel Alpha;
        COLOR_CHANNEL_MASK Mask;
    };

    /**
     * @brief The object used to create and represent blend state for rendering
     */
    export struct LSBlendState
    {
        BlendType BlendType;
        struct Custom
        {
            bool IsAlphaSampling;
            bool IsIndepdentBlend;
            bool IsEnabled;
            // @brief Pointer to each Render Target support (at least one required)
            std::vector<BlendOps> Targets;
        };

        Custom CustomOp;
    };

    /**
     * @brief Details how to use the vertex buffer to render the vertices given
     */
    export enum class PRIMITIVE_TOPOLOGY 
    {
        // TRIANGLES //
        TRIANGLE_LIST,
        TRIANGLE_STRIP,
        TRIANGLE_FAN,
        TRIANGLE_LIST_ADJ,
        TRIANGLE_STRIP_ADJ,
        // POINTS //
        POINT_LIST,
        // LINES //
        LINE_LIST,
        LINE_STRIP,
        LINE_LIST_ADJ,
        LINE_STRIP_ADJ
    };

    /**
     * @brief an evaluator comparison enumerator
     *
     * Used for comparison operations that may require translation.
     * Based on the idea of what is required for the evaluation to "pass"
     * so NEVER_PASS for example means no pass regardless of state
     * but ALL_PASS means all comparisons will always be "true" or "pass"
     * the test.
     */
    export enum class EVAL_COMPARE : uint8_t 
    {
        NEVER_PASS = 0,     // @brief condition never will pass
        LESS_PASS,          // @brief condition is true when less than only
        LESSS_EQUAL_PASS,   // @brief condition is true when less than or equal to
        EQUAL,              // @brief condition is equal to
        NOT_EQUAL,          // @brief condition is when they are not equal
        GREATER_PASS,       // @brief condition is true when greater than
        GREATER_EQUAL_PASS, // @brief condition is true when greater than or equal to
        ALWAYS_PASS         // @brief condition will always result in true
    };

    /**
     * @brief How to perform operations with the depth stencil tests
     */
    export enum class DEPTH_STENCIL_OPS 
    {
        KEEP,
        ZERO,
        REPLACE,
        INVERT,
        INCR_THEN_CLAMP,
        INCR_THEN_WRAP,
        DECR_THEN_CLAMP,
        DECR_THEN_WRAP,
    };

    /**
     * @brief
     */
    export struct LSSamplerState
    {
        uint32_t AnisotropyLevel = 0;
        float MinLOD = 0.0f;
        float MaxLOD = std::numeric_limits<float>::max();
        float MipLODBias = 0.0f;
        TextureRenderState TextureRenderState;
        EVAL_COMPARE Evaluator;
    };

    export enum class CPU_ACCESS_FLAG 
    {
        NOT_SET,
        READ_ONLY,
        WRITE_ONLY,
        READ_AND_WRITE
    };

    // Callbacks //
    export using OnDeviceEvent = std::function<void(DEVICE_EVENT)>;

    export struct RasterizerInfo
    {
        FILL_STATE Fill = FILL_STATE::FILL;
        CULL_METHOD Cull = CULL_METHOD::BACK;
        bool IsFrontCounterClockwise = true; // @brief Front Face drawn counterclockwise = true, false if not
        bool IsDepthClipEnabled = false;

        bool operator==(const RasterizerInfo& rhs) const
        {
            return Fill == rhs.Fill &&
                IsFrontCounterClockwise == rhs.IsFrontCounterClockwise &&
                Cull == rhs.Cull &&
                IsDepthClipEnabled == rhs.IsDepthClipEnabled;
        }

        bool operator!=(const RasterizerInfo& rhs) const
        {
            return Fill != rhs.Fill ||
                IsFrontCounterClockwise != rhs.IsFrontCounterClockwise ||
                Cull != rhs.Cull ||
                IsDepthClipEnabled != rhs.IsDepthClipEnabled;
        }
    };

    export struct LSDrawStateHashFunc
    {
        template <typename T = LS::RasterizerInfo>
        std::size_t operator()(T const& t) const noexcept
        {
            std::size_t h1 = LS::Utils::HashEnum(t.Fill);
            std::size_t h2 = t.IsFrontCounterClockwise ? 1 : 0;
            std::size_t h3 = LS::Utils::HashEnum(t.Cull);
            std::size_t h4 = t.IsDepthClipEnabled ? 1 : 0;

            return h1 ^ h2 ^ h3 ^ h4;
        }
    };

    export struct LSSwapchainInfo
    {
        uint32_t            BufferSize{ 2u };
        uint32_t            Width;
        uint32_t            Height;
        PIXEL_COLOR_FORMAT  PixelFormat{ PIXEL_COLOR_FORMAT::RGBA8_UNORM };
        bool                IsStereoScopic{ false };
        uint16_t            MSCount{ 1u };
        uint16_t            MSQuality{ 0u };
    };

    export struct LSDeviceInfo
    {
        std::string Info;
    };

    /**
     * @brief A depth stencil object
     */
    export struct DepthStencil
    {
        bool IsDepthEnabled;                     // @brief Turn on/off (true/false) depth feature 
        bool DepthBufferWriteAll;                // @brief Writes all/none to depth buffer (true/false) 
        EVAL_COMPARE DepthComparison;            // @brief How to handle depth comparison data with destination data
        bool IsStencilEnabled;                   // @brief Turn on stencil (true) or off (false)
        uint8_t StencilWriteMask;                // @brief The mask to perform operations on depth-stencil buffer reads (usually 0x00)
        uint8_t StencilReadMask;                 // @brief The mask to perform operations on depth-stencil buffer writes (usually 0xFF)

        /**
         * @brief Information on how to handle depth stencil operations
         */
        struct DepthStencilOps
        {
            DEPTH_STENCIL_OPS StencilFailOp;          // @brief stencil test fails to pass
            DEPTH_STENCIL_OPS StencilPassDepthFailOp; // @brief stencil test passes but depth test fails
            DEPTH_STENCIL_OPS BothPassOp;             // @brief Stencil AND Depth Pass 
            EVAL_COMPARE StencilTestFunc;             // @brief function to perform for this stencil operation
        };

        DepthStencilOps FrontFace; // @brief Operations for front facing pixels
        DepthStencilOps BackFace;  // @brief Operations for back facing pixels
    };

    export struct LSDeviceSettings
    {
        uint32_t FPSTarget = 60;
        uint32_t FrameBufferCount = 2;
        uint32_t Width;
        uint32_t Height;
        PIXEL_COLOR_FORMAT PixelFormat;
        bool IsVSync;
        DEVICE_API DeviceApi;
        DEVICE_TYPE DeviceType;
    };

    /**
     * @brief Helper function to create the LSDeviceSettings with the Window's given resolution
     * @param window The window to use
     * @param apiType The API to use
     * @param vSynceEnabled True = enabled, False = disabled
     * @param backBufferSize Size of the back buffer queue
     * @param fpsTarget Target FPS (not currently used)
     * @param pixelFormat Format of the pixels to use
     * @param type The device type to create (Hardware or Software rendering)
     * @return
     */
    export auto CreateDeviceSettings(uint32_t width, uint32_t height, DEVICE_API apiType, bool vSynceEnabled = false,
        uint32_t backBufferSize = 2, uint32_t fpsTarget = 60,
        PIXEL_COLOR_FORMAT pixelFormat = PIXEL_COLOR_FORMAT::RGBA8_UNORM,
        DEVICE_TYPE type = DEVICE_TYPE::HARDWARE) -> LSDeviceSettings
    {
        return { .FPSTarget = fpsTarget, .FrameBufferCount = backBufferSize,
            .Width = width, .Height = height, .PixelFormat = pixelFormat,
            .IsVSync = vSynceEnabled, .DeviceApi = apiType, .DeviceType = type };
    }

    export using ShaderPipeline = std::unordered_map<SHADER_TYPE, std::vector<std::byte>>;

    export struct BufferMap
    {
        BUFFER_BIND_TYPE BindStage;
        uint16_t BindSlot;
        std::vector<std::byte> Data;
    };

    export struct TextureMap
    {
        uint16_t BindSlot;
        LSTextureInfo Texture;
    };

    export struct SamplerMap
    {
        uint16_t BindSlot;
        LSSamplerState Sampler;
    };

    // Pipeline State //
    /**
     * @brief A system for the different states that construct a graphics pipeline
     *
     * Necessary components to render a frame:
     * - Rasterizer state (RasterizerInfo) - (Wireframe/Solid and how triangles are drawn, culled, etc.)
     * - Depth stencil
     * - Blend state - (opaque, alpha, reverse z, etc.)
     * - Shaders - what shaders are in use during this pipeline
     * - Input layout - the layout of data for the vertex shader
     * - Buffers - what buffers are in use for this: Verrtex, Index, Constant Buffers per shader type (maybe later version add this?)
     * - Topology (PRIMITIVE_TOPOLOGY enum) - the way primitives will be drawn
     * - Resources - samplers, textures, and other shader resources - perhaps store as some key values
     *    so we aren't holding pointers, and can access them in some resource manager.
     */
    export struct PipelineDescriptor
    {
        RasterizerInfo RasterizeState;
        LSBlendState BlendState;
        DepthStencil DepthStencil;
        ShaderPipeline Shaders;
        LSShaderInputSignature ShaderSignature;
        PRIMITIVE_TOPOLOGY Topology;
        LSTextureInfo RenderTarget;

        // Resources //
        std::vector<SamplerMap> Samplers;
        std::vector<TextureMap> Textures;
        std::vector<BufferMap> Buffers;
    };

    export class ILSContext
    {
    protected:
        ILSContext() = default;

    public:
        virtual ~ILSContext() = default;
        ILSContext(const ILSContext&) = delete;
        ILSContext(ILSContext&&) = default;
        ILSContext& operator=(const ILSContext&) = delete;
        ILSContext& operator=(ILSContext&&) = default;

        /**
         * @brief Sets the pipeline state and all required components to prepare the draw
         * @param pipeline
         */
        virtual void PreparePipeline(Ref<PipelineDescriptor> pipeline) noexcept = 0;

        /**
         * @brief Clears the render target associated with the given pipeline
         * @param color The color values to set for the render target
         */
        virtual void Clear(const std::array<float, 4>& color) noexcept = 0;

        /**
         * @brief Finalize the work and prepare it for presentation to the render target
         * @param syncInterval the frame to sync with, 0 = immediate, and N means to wait N frames before producing
         */
        virtual void Present(uint32_t syncInterval) noexcept = 0;

        virtual void Finish() noexcept = 0;
    };

    /**
     * @brief Represents the GPU physical device
     */
    export class ILSDevice
    {
    protected:
        ILSDevice() = default;

        bool m_bIsInitialized = false;

    public:
        virtual ~ILSDevice() = default;
        ILSDevice(ILSDevice&&) = default;
        ILSDevice(const ILSDevice&) = default;
        ILSDevice& operator=(const ILSDevice&) = default;
        ILSDevice& operator=(ILSDevice&&) = default;

        OnDeviceEvent OnDeviceEvent;

        /**
         * @brief Initializes the device
         * @return true if successful, false if failed.
         */
        [[nodiscard]] virtual auto InitDevice(const LSDeviceSettings& settings, LS::LSWindowBase* pWindow) noexcept -> LS::System::ErrorCode = 0;
        [[nodiscard]] virtual auto CreateContext() noexcept -> Nullable<Ref<ILSContext>> = 0;
        virtual void Shutdown() noexcept = 0;
    };

    export class IRenderer
    {
    protected:
        explicit IRenderer(SharedRef<ILSDevice>& pDevice) : m_pDevice(pDevice)
        {
        }

        SharedRef<ILSDevice> m_pDevice;
        std::queue<SharedRef<ILSContext>> m_RenderQueue;

    public:
        virtual ~IRenderer() = default;

        /**
         * @brief Creates a context from the device
         * @return the context object
         */
        virtual auto CreateContext() noexcept -> Ref<ILSContext> = 0;

        virtual auto CreateTexture(const LSTextureInfo& info) noexcept -> Nullable<GuidUL> = 0;

        /**
         * @brief Prepares the Context's commands for the render queue to be presented
         * @param pContext The context with commands to perform with
         */
        virtual void QueueCommands(Ref<ILSContext>& pContext) noexcept = 0;

        ILSDevice* GetDevice() const
        {
            return m_pDevice.get();
        }

        const ILSDevice* GetDeviceConst() const
        {
            return m_pDevice.get();
        }
    };
}