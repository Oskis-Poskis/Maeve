#include <iostream>
#include <memory>
#include <array>

#include "../../lib/glad/glad.h"
#include "../../lib/glm/glm.hpp"

#include "ui.h"
#include "text_renderer.h"
#include "../common/shader.h"
#include "../common/input.h"
#include "../engine/render_engine.h"
#include "../engine/asset_manager.h"

namespace UI
{
    void DrawMenuRect(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color);

    std::unique_ptr<Shader> UIshader, defocusShader;

    uint defocusVAO;
    uint menuVAO, menuVBO;
    std::array<float, 8> vertices;

    // ---- replace with config file? ----

    // Menu General
    int outlineWidth   = 4;
    int MainMenuStartY = 160;
    glm::vec3 OutlineColor(0.15f);
    glm::vec3 SecondaryColor = glm::vec3(247, 224, 100) / 256.0f;

    // Title Bar
    int TitleBarPaddingYPX    = 8;
    glm::vec3 TitleBarColor   = glm::vec3(126, 214, 128) / 256.0f;
    float TitleBarTextScaling = 0.65f;
    glm::vec3 TitleBarTextColor(0.15f);

    // Item
    float ItemTextScaling = 0.475f;
    int   ItemPaddingXPX  = 35;
    int   ItemPaddingYPX  = 12;
    int   ItemSelectionThickness = 3;
    glm::vec3 ItemColor(0.15f);
    glm::vec3 ItemTextActive   = glm::vec3(1.0f);
    glm::vec3 ItemTextInactive = glm::vec3(142, 139, 146) / 256.0f;

    // -----------------------------------

    float titleHeightPX;
    float itemHeightPX;
    int   mainMenuWidth = 280;
    int   activeItem = 2;
    float centerX, centerY;

    bool inMenu = false;
    std::string menuTitle = ".________.";
    std::array<std::string, 8> items = {"SceneManger", "AssetManger", "Statistics", "Engine", "qk", "temp", "temp", "temp"};

    void Render()
    {
        if (Input::KeyPressed(GLFW_KEY_TAB))
        {
            if (inMenu)inMenu = false;
            else inMenu = true;
        }

        if (inMenu)
        {
            float right_edge   = centerX + mainMenuWidth / 2;
            float left_edge    = centerX - mainMenuWidth / 2;
            float top_edge     = centerY + MainMenuStartY;
            float bottom_edge  = centerY - MainMenuStartY;

            if (Input::KeyPressed(GLFW_KEY_UP))
            {
                activeItem = glm::clamp(activeItem - 1, 0, (int)items.size() - 1);
            }
            if (Input::KeyPressed(GLFW_KEY_DOWN))
            {
                activeItem = glm::clamp(activeItem + 1, 0, (int)items.size() - 1);
            }

            // BG tint
            defocusShader->Use();
            glBindVertexArray(defocusVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            UIshader->Use();
            UIshader->SetMatrix4("projection", AssetManager::OrthoProjMat4);

            // Outline
            DrawMenuRect(glm::vec2(right_edge + outlineWidth, top_edge + outlineWidth), glm::vec2(left_edge - outlineWidth, top_edge - titleHeightPX - itemHeightPX * items.size() - outlineWidth), OutlineColor);

            // Main Menu Title
            DrawMenuRect(glm::vec2(right_edge, top_edge),
                         glm::vec2(left_edge,  top_edge - titleHeightPX), TitleBarColor);
            Text::Render(menuTitle, centerX  - Text::CalculateTextWidth(menuTitle, TitleBarTextScaling) / 2,
                                    top_edge - titleHeightPX + TitleBarPaddingYPX, TitleBarTextScaling, TitleBarTextColor);

            // Items
            for (int i = 0; i < items.size(); i++)
            {    
                float distance = ((items.size() - glm::abs(activeItem - i)) / (float)items.size());
                float itemShade = distance * distance * distance;
                itemShade = glm::pow(itemShade, 1.0f / 2.2f);
                
                DrawMenuRect(glm::vec2(right_edge, top_edge - titleHeightPX - itemHeightPX * i),
                             glm::vec2(left_edge,  top_edge - titleHeightPX - itemHeightPX * (i + 1)), ItemColor * itemShade);
                glm::vec3 col = (i == activeItem) ? ItemTextActive : ItemTextInactive;
                Text::Render(items[i], centerX  - Text::CalculateTextWidth(items[i], ItemTextScaling) / 2,
                                       top_edge - titleHeightPX - itemHeightPX * (i + 1) + ItemPaddingYPX, ItemTextScaling, col);
            }

            //DrawMenuRect(glm::vec2(right_edge, top_edge - titleHeightPX - itemHeightPX * (activeItem + 1) + ItemSelectionThickness),
            //             glm::vec2(left_edge,  top_edge - titleHeightPX - itemHeightPX * (activeItem + 1)), SecondaryColor);
                         
            DrawMenuRect(glm::vec2(right_edge, top_edge - titleHeightPX - itemHeightPX * (activeItem)),
                         glm::vec2(right_edge - ItemSelectionThickness,  top_edge - titleHeightPX - itemHeightPX * (activeItem + 1)), SecondaryColor);
        }
    }

    void DrawMenuRect(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color)
    {
        vertices = 
        {
            TopRight.x,   TopRight.y,
            BottomLeft.x, TopRight.y,
            BottomLeft.x, BottomLeft.y,
            TopRight.x,   BottomLeft.y,
        };
        UIshader->Use();
        UIshader->SetVector3("color", color);
        glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(menuVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void Initialize()
    {
        UIshader = std::make_unique<Shader>("/../res/shaders/ui");
        defocusShader = std::make_unique<Shader>("/../res/shaders/defocus");

        glGenVertexArrays(1, &menuVAO);
        glGenBuffers(1, &menuVBO);

        glBindVertexArray(menuVAO);
        glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 6, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

        // Dummy VAO for fullscreen quad vert shader
        glGenVertexArrays(1, &defocusVAO);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void WindowResize()
    {
        titleHeightPX = Text::CalculateMaxTextHeight(menuTitle, TitleBarTextScaling, true) + TitleBarPaddingYPX * 2;
        itemHeightPX  = Text::CalculateMaxTextHeight(items[0].substr(0, 1), ItemTextScaling, true) + ItemPaddingYPX * 2;
        mainMenuWidth  = Text::CalculateTextWidth(items[0], ItemTextScaling) + ItemPaddingXPX * 2;
        centerX = Engine::GetWindowSize().x / 2.0f;
        centerY = Engine::GetWindowSize().y / 2.0f;

        UIshader->Use();
        UIshader->SetMatrix4("projection", AssetManager::OrthoProjMat4);
    }
}