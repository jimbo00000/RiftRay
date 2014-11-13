// ShaderPane.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "Pane.h"

class ShaderToy;

///@brief 
class ShaderPane : public Pane
{
public:
    ShaderPane();
    virtual ~ShaderPane();

    virtual void initGL();
    virtual void DrawPaneWithShader(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const ShaderWithVariables& sh) const;

    void DrawPaneAsPortal(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object,
        const ShaderWithVariables& sh) const;

    ShaderToy* m_pShadertoy;

private: // Disallow copy ctor and assignment operator
    ShaderPane(const ShaderPane&);
    ShaderPane& operator=(const ShaderPane&);
};
