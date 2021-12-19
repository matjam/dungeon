//
// Created by matjam on 12/17/2021.
//

#pragma once

#include <string>

class RenderComponent {
private:
    std::string m_renderCharacter; // when rendering we use this character
public:
    RenderComponent() = default;

    void setRenderCharacter(std::string c) {
        m_renderCharacter = c;
    };

    inline std::string getRenderCharacter() {
        return m_renderCharacter;
    };
};
