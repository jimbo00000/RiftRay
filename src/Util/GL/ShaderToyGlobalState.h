// ShaderToyGlobalState.h

#pragma once

struct ShaderToyGlobalState
{
    bool animatedThumbnails;
    bool panesAsPortals;

    ShaderToyGlobalState()
        : animatedThumbnails(false)
        , panesAsPortals(false)
    {}
};
