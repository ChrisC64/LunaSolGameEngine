export module Data.LSCommonTypes;

export import Engine.LSDevice;

export namespace LS
{
    // Some Default Common Types //
    LSDrawState SolidFill_Back_CCW{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    LSDrawState Wireframe_Back_CCW{
        .Fill = LS::FILL_STATE::WIREFRAME,
        .Cull = LS::CULL_METHOD::BACK,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
    
    LSDrawState SolidFill_Front_CCW{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::FRONT,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };

    LSDrawState SolidFill_None_CCW{
        .Fill = LS::FILL_STATE::FILL,
        .Cull = LS::CULL_METHOD::NONE,
        .IsFrontCounterClockwise = true,
        .IsDepthClipEnabled = true
    };
}