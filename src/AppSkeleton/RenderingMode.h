// RenderingMode.h

#pragma once

///@brief This is just a simple encapsulation of one of five rendering modes
/// and a few functions for cycling through them.
struct RenderingMode
{
    enum outputType {
        Mono_Raw,
        Mono_Buffered,
        SideBySide_Undistorted,
        OVR_SDK,
        OVR_Client,
    };

    RenderingMode() : outputType(Mono_Raw) {}

    outputType outputType;

    void toggleRenderingType()
    {
        if (outputType == Mono_Raw)
            outputType = Mono_Buffered;
        else if (outputType == Mono_Buffered)
            outputType = SideBySide_Undistorted;
        else if (outputType == SideBySide_Undistorted)
            outputType = OVR_SDK;
        else if (outputType == OVR_SDK)
            outputType = OVR_Client;
        else
            outputType = Mono_Raw;
    }

    void toggleRenderingTypeReverse()
    {
        if (outputType == Mono_Raw)
            outputType = OVR_Client;
        else if (outputType == OVR_Client)
            outputType = OVR_SDK;
        else if (outputType == OVR_SDK)
            outputType = SideBySide_Undistorted;
        else if (outputType == SideBySide_Undistorted)
            outputType = Mono_Buffered;
        else
            outputType = Mono_Raw;
    }

    void toggleRenderingTypeMono()
    {
        if (outputType == Mono_Raw)
            outputType = Mono_Buffered;
        else
            outputType = Mono_Raw;
    }

    void toggleRenderingTypeHMD()
    {
        if (outputType == SideBySide_Undistorted)
            outputType = OVR_SDK;
        else if (outputType == OVR_SDK)
            outputType = OVR_Client;
        else
            outputType = SideBySide_Undistorted;
    }

    void toggleRenderingTypeDistortion()
    {
        if (outputType == OVR_SDK)
            outputType = OVR_Client;
        else
            outputType = OVR_SDK;
    }

    void useClientDistortion() { outputType = OVR_Client; }
};
