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

    virtual void DrawPaneAsPortal(
        const glm::mat4& modelview,
        const glm::mat4& projection,
        const glm::mat4& object,
        const glm::mat4& paneMatrix=glm::mat4(1.0f)) const;

    virtual void DrawShaderInfoText(
        const ShaderWithVariables& fsh,
        const BMFont& fnt
        ) const;

    virtual void RenderThumbnail() const;
    virtual void DrawToFBO() const;

    virtual void SetTextureLibraryPointer(std::map<std::string, textureChannel>* pTL) { m_pTexLibrary = pTL; }
    virtual void SetFontShaderPointer(const ShaderWithVariables* pS) { m_pFontShader = pS; }
    virtual void SetFontPointer(const BMFont* pF) { m_pFont = pF; }

    ShaderToy* m_pShadertoy;
protected:
    std::map<std::string, textureChannel>* m_pTexLibrary;
    const ShaderWithVariables* m_pFontShader;
    const BMFont* m_pFont;
    GLuint m_vao;

private: // Disallow copy ctor and assignment operator
    ShaderToyPane(const ShaderToyPane&);
    ShaderToyPane& operator=(const ShaderToyPane&);
};
