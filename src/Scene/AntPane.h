// AntPane.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "Pane.h"

///@brief 
class AntPane : public Pane
{
public:
    AntPane();
    virtual ~AntPane();

    virtual void initGL();
    virtual void timestep(float dt);

    virtual void DrawPaneWithShader(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const ShaderWithVariables& sh) const;

    virtual void DrawToFBO() const;
    virtual void OnMouseClick(int state, int x, int y);
    virtual void OnMouseMove(int x, int y);
    virtual void ResizeTweakbar();

protected:

private: // Disallow copy ctor and assignment operator
    AntPane(const AntPane&);
    AntPane& operator=(const AntPane&);
};
