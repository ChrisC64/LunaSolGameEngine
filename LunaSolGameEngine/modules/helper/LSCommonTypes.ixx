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
    LSDrawState SolidFill_BackCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    LSDrawState SolidFill_FrontCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    LSDrawState SolidFill_NoneCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };
    
    LSDrawState SolidFill_BackCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    
    LSDrawState SolidFill_FrontCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    
    LSDrawState SolidFill_NoneCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    /////////////////////////////////////////
    // Counter Clockwise Winding Order Set //
    /////////////////////////////////////////

    LSDrawState SolidFill_BackCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    LSDrawState SolidFill_FrontCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    LSDrawState SolidFill_NoneCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    LSDrawState SolidFill_BackCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };
    
    LSDrawState SolidFill_FrontCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };
    
    LSDrawState SolidFill_NoneCull_CCWFront_DCD{
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
    LSDrawState Wireframe_BackCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    LSDrawState Wireframe_FrontCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    LSDrawState Wireframe_NoneCull_CCFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = true
    };

    LSDrawState Wireframe_BackCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };

    LSDrawState Wireframe_FrontCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };

    LSDrawState Wireframe_NoneCull_CCFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = false,
        .IsDepthClipEnabled = false
    };
    /////////////////////////////////////////
    // Counter Clockwise Winding Order Set //
    /////////////////////////////////////////

    LSDrawState Wireframe_BackCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    LSDrawState Wireframe_FrontCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    LSDrawState Wireframe_NoneCull_CCWFront_DCE{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    LSDrawState Wireframe_BackCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    LSDrawState Wireframe_FrontCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    LSDrawState Wireframe_NoneCull_CCWFront_DCD{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = false
    };

    // WIREFRAME SET END //
}