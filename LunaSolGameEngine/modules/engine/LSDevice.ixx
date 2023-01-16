module;
#include "LSEFramework.h"

export module Engine.LSDevice;

import Engine.LSWindow;
import LSData;
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
        DIRECT3D_11,
        DIRECT3D_12,
        VULKAN,
        OPEN_GL
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
        ZERO, // @brief Use a set of 0s for the blend factor equation
        ONE, // @brief Use a set of 1s for the blend factor equation 
        SRC_COLOR, // @brief Use the src color's RGB values
        DEST_COLOR, // @brief Use the destination's RGB values
        SRC_ALPHA, // @brief Use the src's alpha channel 
        DEST_ALPHA, // @brief Use the desintation's alpha channel
        INV_SRC_COLOR, // @brief Inverse (1 - R, 1 - G, 1 - B) the color based on the src RGB
        INV_SRC_ALPHA, // @brief Inverse the Alpha (1 - A, 1 - A, 1 - A) from the src values
        INV_DEST_COLOR, // @brief Inverse (1 - R, 1 - G, 1 - B) the color based on the dest RGB
        INV_DEST_ALPHA, // @brief Inverse the Alpha (1 - A, 1 - A, 1 - A) from the dest values
    };

    /**
     * @brief Details the action to take on the blending procedure.
    */
    export enum class BLEND_OPERATION
    {
        BLEND_ADD, // @brief Blends data by adding data from first and second objects
        BLEND_SUB, // @brief Blends data by subtracting data from first and second object
        BLEND_REV_SUB, // @brief Blend data by subtracting from second to first object
        BLEND_MIN, // @brief Find the minimum average between each src data and blend together 
        BLEND_MAX // @brief Find the max data between both sources and blend them
    };

    export enum class COLOR_CHANNEL_MASK : uint8_t
    {
        RED = 1,
        GREEN = 2,
        BLUE = 4,
        ALPHA = 8,
        ALL = ( ( (RED | GREEN) | BLUE) | ALPHA)
    };

    export struct LSBlendState
    {
        bool IsAlphaSampling;
        bool IsIndepdentBlend;
        bool IsEnabled;
        BLEND_OPERATION BlendOpRGB;
        BLEND_FACTOR SrcBF;
        BLEND_FACTOR DestBF;
        BLEND_OPERATION BlendOpAlpha;
        BLEND_FACTOR AlphaSrcBF;
        BLEND_FACTOR AlphaDestBF;
        COLOR_CHANNEL_MASK Mask;
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
        NEVER_PASS = 0, //@brief condition never will pass
        LESS_PASS, //@brief condition is true when less than only
        LESSS_EQUAL_PASS, //@brief condition is true when less than or equal to
        EQUAL, //@brief condition is equal to 
        NOT_EQUAL, //@brief condition is when they are not equal
        GREATER_PASS, //@brief condition is true when greater than
        GREATER_EQUAL_PASS, //@brief condition is true when greater than or equal to
        ALWAYS_PASS //@brief condition will always result in true
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
        FILL_STATE Fill;
        CULL_METHOD Cull;
        bool IsFrontCounterClockwise; // @brief Front Face drawn counterclockwise = true, false if not 
        bool IsDepthClipEnabled;

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
        template<typename T = LS::RasterizerInfo>
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
        uint16_t            MSCount{ 1 };
        uint16_t            MSQuality{ 0 };
    };

    export struct LSDeviceInfo
    {
        std::string Info;
    };

    export enum class DEPTH_STENCIL_WRITE_MASK
    {
        ZERO,// @brief 0x0
        ALL // @brief 0xFFFFFFFF
    };

    /**
     * @brief A depth stencil object
    */
    export struct DepthStencil
    {
        LSTextureInfo DepthBuffer;
        bool IsDepthEnabled;
        bool IsStencilEnabled;
        EVAL_COMPARE DepthComparison;// @brief How to handle depth comparison data with destination data
        bool ShouldWriteToDepthMask;// @brief Turn on/off writing to the depth mask
        DEPTH_STENCIL_WRITE_MASK DepthWriteMask; //@brief portion of depth stencil buffer that can be modified
        uint8_t StencilWriteMask;// @brief The mask to perform operations on depth-stencil buffer reads
        uint8_t StencilReadMask;// @brief The mask to perform operations on depth-stencil buffer writes
        /**
         * @brief Information on how to handle depth stencil operations
        */
        struct DepthStencilOps
        {
            DEPTH_STENCIL_OPS StencilFailOp;// @brief stencil test fails to pass
            DEPTH_STENCIL_OPS StencilPassDepthFailOp; //@brief stencil test passes but depth test fails
            DEPTH_STENCIL_OPS StencilPassOp; // @brief Stencil test passes,
            EVAL_COMPARE StencilTestFunc; // @brief function to perform for this stencil operation
        };

        DepthStencilOps FrontFace;// @brief Operations for front facing pixels
        DepthStencilOps BackFace;// @brief Operations for back facing pixels
    };

    export struct LSDeviceSettings
    {
        int32_t FPSTarget;
        bool IsVSync;
        int32_t FrameBufferCount;
        LSTextureInfo RenderTargetDesc;
        LSSwapchainInfo SwapchainInfo;
        DEVICE_TYPE DeviceType;
        DEVICE_API DeviceApi;
        LS::LSWindowHandle WindowHandle;
    };

    export using ShaderMap = std::unordered_map<SHADER_TYPE, std::vector<std::byte>>;

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
        ShaderMap Shaders;
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
        */
        virtual void Present() noexcept = 0;
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
        [[nodiscard]] virtual bool InitDevice(const LSDeviceSettings& settings) noexcept = 0;
        [[nodiscard]] virtual auto CreateContext() noexcept -> LSOptional<Ref<ILSContext>> = 0;
        virtual void Shutdown() noexcept = 0;
        /**
         * @brief Creates a pipeline state for the given pipeline descriptor
         * @param pipelineDescriptor A set of settings to construct a pipeline state for the GPU to use for rendering
         * @return True if pipeline was created successfully, false if there was a failure.
        */
        [[nodiscard]] virtual bool CreateePipelineState(const LS::PipelineDescriptor& pipelineDescriptor) noexcept = 0;

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