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
    RasterizerInfo SolidFill_BackCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_FrontCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_NoneCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_BackCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_FrontCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_NoneCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    /////////////////////////////////////////
    // Counter Clockwise Winding Order Set //
    /////////////////////////////////////////

    RasterizerInfo SolidFill_BackCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_FrontCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_NoneCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    RasterizerInfo SolidFill_BackCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_FrontCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };
    
    RasterizerInfo SolidFill_NoneCull_CCWFront_DCD{
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
    RasterizerInfo Wireframe_BackCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_FrontCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_NoneCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_BackCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_FrontCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_NoneCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    /////////////////////////////////////////
    // Counter Clockwise Winding Order Set //
    /////////////////////////////////////////

    RasterizerInfo Wireframe_BackCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_FrontCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_NoneCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    RasterizerInfo Wireframe_BackCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_FrontCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    RasterizerInfo Wireframe_NoneCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    // WIREFRAME SET END //
}