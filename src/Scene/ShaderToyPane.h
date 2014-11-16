// ShaderToyPane.h

#pragma once

#ifdef _WIN32
#  define WINDOWS_LEAN_AND_MEAN
#  define NOMINMAX
#  include <windows.h>
#endif

#include "Pane.h"

class ShaderToy;
#include "ShaderToyFunctions.h"

///@brief 
class ShaderToyPane : public Pane
{
public:
    ShaderToyPane();
    virtual ~ShaderToyPane();

    virtual void initGL();
    virtual void DrawPaneWithShader(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const ShaderWithVariables& sh) const;
    
    void DrawPaneAsPortal(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object) const;

    void DrawShaderInfoText(
        const ShaderWithVariables& fsh,
        const BMFont& fnt
        ) const;

    void RenderThumbnail() const;
    virtual void DrawToFBO() const;

    void SetTextureLibraryPointer(std::map<std::string, textureChannel>* pTL) { m_pTexLibrary = pTL; }

    ShaderToy* m_pShadertoy;
protected:
    std::map<std::string, textureChannel>* m_pTexLibrary;
    GLuint m_vao;

private: // Disallow copy ctor and assignment operator
    ShaderToyPane(const ShaderToyPane&);
    ShaderToyPane& operator=(const ShaderToyPane&);
};
