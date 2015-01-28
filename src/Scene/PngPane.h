// PngPane.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "Pane.h"

///@brief 
class PngPane : public Pane
{
public:
    PngPane();
    virtual ~PngPane();

    virtual void initGL();

    virtual void DrawPaneWithShader(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const ShaderWithVariables& sh) const;

protected:

private: // Disallow copy ctor and assignment operator
    PngPane(const PngPane&);
    PngPane& operator=(const PngPane&);
};
