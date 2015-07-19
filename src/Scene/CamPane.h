// CamPane.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "Pane.h"

///@brief Hold and display an FBO for a "hand-held camera" to render to.
class CamPane : public Pane
{
public:
    CamPane();
    virtual ~CamPane();

    virtual void initGL();

    virtual void DrawPaneWithShader(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const ShaderWithVariables& sh) const;

    // Set Hydra pointer
    // Use m_paneRenderBuffer to draw to

protected:

private: // Disallow copy ctor and assignment operator
    CamPane(const CamPane&);
    CamPane& operator=(const CamPane&);
};
