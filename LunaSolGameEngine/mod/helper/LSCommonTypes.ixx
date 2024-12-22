export module Helper.LSCommonTypes;

import Engine.LSDevice;

export namespace LS
{
    // Some Default Common Types //
    // States that are compiled into the following order: {FILL_STATE]_{CULL_METHOD}_{ (CC|CCW)(Front|Back) }_{DCE|DCD}
    // Fill State - Wireframe or Solid Fill
    // Cull Method - FrontCull, BackCull, NoneCull
    // Winding order of Triangles - CW (Clockwise) or CCW (Counter Clockwise)
    // Depth Clip Enabled (DCE) or Depth Clip Disabled (DCD)
    
    /////////////////////////////////
    // Clockwise Winding Order Set //
    /////////////////////////////////
    /**
    * @brief Solid fill with back faces culled and triangles in CW order. Depth Clip Enabled (DCE)
    */
    RasterizerInfo SolidFill_BackCull_FCW_DCE
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_FrontCull_FCW_DCE
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_NoneCull_FCW_DCE
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_BackCull_FCW_DCD
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_FrontCull_FCW_DCD
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_NoneCull_FCW_DCD
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };

    /////////////////////////////////////////
    // Counter Clockwise Winding Order Set //
    /////////////////////////////////////////
    RasterizerInfo SolidFill_BackCull_FCCW_DCE
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_FrontCull_FCCW_DCE
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_NoneCull_FCCW_DCE
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_BackCull_FCCW_DCD
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_FrontCull_FCCW_DCD
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_NoneCull_FCCW_DCD
    {
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    // WIREFRAME SET BEGIN //

    /////////////////////////////////
    // Clockwise Winding Order Set //
    /////////////////////////////////
    /**
    * @brief Solid fill with back faces culled and triangles in CW order. Depth Clip Enabled (DCE)
    */
    RasterizerInfo Wireframe_BackCull_FCW_DCE
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_FrontCull_FCW_DCE
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_NoneCull_FCW_DCE
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_BackCull_FCW_DCD
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_FrontCull_FCW_DCD
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_NoneCull_FCW_DCD
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    
    /////////////////////////////////////////
    // Counter Clockwise Winding Order Set //
    /////////////////////////////////////////
    RasterizerInfo Wireframe_BackCull_FCCW_DCE
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_FrontCull_FCCW_DCE
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_NoneCull_FCCW_DCE
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_BackCull_FCCW_DCD
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_FrontCull_FCCW_DCD
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_NoneCull_FCCW_DCD
    {
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    // WIREFRAME SET END //

    /////////////////////////
    // DEPTH STENCIL BEGIN //
    /////////////////////////

    DepthStencil DepthNone
    {
        .IsDepthEnabled = false,
        .DepthBufferWriteAll = false,
        .DepthComparison = LS::EVAL_COMPARE::NEVER_PASS,
        .IsStencilEnabled = false,
        .StencilWriteMask = 0xFF,
        .StencilReadMask = 0xFF,
        .FrontFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
        .BackFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
    };
    
    DepthStencil DepthDefault
    {
        .IsDepthEnabled = true,
        .DepthBufferWriteAll = true,
        .DepthComparison = LS::EVAL_COMPARE::LESS_PASS,
        .IsStencilEnabled = false,
        .StencilWriteMask = 0xFF,
        .StencilReadMask = 0xFF,
        .FrontFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
        .BackFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
    };

    DepthStencil DepthRead
    {
        .IsDepthEnabled = true,
        .DepthBufferWriteAll = false,
        .DepthComparison = LS::EVAL_COMPARE::LESS_PASS,
        .IsStencilEnabled = false,
        .StencilWriteMask = 0xFF,
        .StencilReadMask = 0xFF,
        .FrontFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
        .BackFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
    };

    DepthStencil DepthReverseZ
    {
        .IsDepthEnabled = true,
        .DepthBufferWriteAll = true,
        .DepthComparison = LS::EVAL_COMPARE::GREATER_EQUAL_PASS,
        .IsStencilEnabled = false,
        .StencilWriteMask = 0xFF,
        .StencilReadMask = 0xFF,
        .FrontFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
        .BackFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
    };

    DepthStencil DepthReadReverseZ
    {
        .IsDepthEnabled = true,
        .DepthBufferWriteAll = false,
        .DepthComparison = LS::EVAL_COMPARE::GREATER_EQUAL_PASS,
        .IsStencilEnabled = false,
        .StencilWriteMask = 0xFF,
        .StencilReadMask = 0xFF,
        .FrontFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
        .BackFace = { .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::KEEP, .BothPassOp = DEPTH_STENCIL_OPS::KEEP, .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS },
    };

    DepthStencil DepthStencilDefault
    {
        .IsDepthEnabled = true,
        .DepthBufferWriteAll = true,
        .DepthComparison = LS::EVAL_COMPARE::LESS_PASS,
        .IsStencilEnabled = true,
        .StencilWriteMask = 0xFF,
        .StencilReadMask = 0xFF,
        .FrontFace = {
            .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, 
            .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::INCR_THEN_WRAP, 
            .BothPassOp = DEPTH_STENCIL_OPS::KEEP, 
            .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS 
        },
        .BackFace = {
            .StencilFailOp = DEPTH_STENCIL_OPS::KEEP, 
            .StencilPassDepthFailOp = DEPTH_STENCIL_OPS::DECR_THEN_WRAP, 
            .BothPassOp = DEPTH_STENCIL_OPS::KEEP, 
            .StencilTestFunc = EVAL_COMPARE::ALWAYS_PASS 
        },
    };

    /////////////////////////
    // DEPTH STENCIL END   //
    /////////////////////////
}