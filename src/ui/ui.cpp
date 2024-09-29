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
    int MainMenuStartY = 220;
    glm::vec3 OutlineColor(0.15f);
    glm::vec3 SecondaryColor = glm::vec3(247, 224, 100) / 256.0f; //vec3(255,141,227) / 256.0f; 

    // Title Bar
    int TitleBarPaddingYPX    = 5;
    float TitleBarTextScaling = 0.45f;
    glm::vec3 TitleBarTextColor(0.05f);
    glm::vec3 TitleBarColor   = glm::vec3(126, 214, 128) / 256.0f;

    // Item
    float ItemTextScaling = 0.475f;
    int   ItemPaddingXPX  = 25;
    int   ItemPaddingYPX  = 10;
    int   ItemSelectionThickness = 3;
    glm::vec3 ItemColor(0.15f);
    glm::vec3 ItemTextActive   = glm::vec3(1.0f);
    glm::vec3 ItemTextInactive = glm::vec3(122, 119, 126) / 256.0f;

    // -----------------------------------


    float titleHeightPX;
    float itemHeightPX;
    // int   mainMenuWidth = 280;
    // int   activeItem = 0;
    float centerX, centerY;
    float right_edge, left_edge; 
    float top_edge, bottom_edge;

    bool inMenu = false;
    int inSubMenu = false;
    std::array<std::string, 5> items = {"SceneManager", "AssetManager", "Statistics", "Engine", "qk"};

    Menu mainMenu("hotbox");

    void Render()
    {
        if (Input::KeyPressed(GLFW_KEY_TAB))
        {
            if (inMenu)
            {
                inMenu = false;
                Input::SetInputContext(Input::Game);
            }
            else
            {
                inMenu = true;
                Input::SetInputContext(Input::Menu);
            }
        }

        if (Input::KeyPressed(GLFW_KEY_PERIOD))
        {
            Text::SetGlobalTextScaling(Text::GetGlobalTextScaling() + 0.2f);
            WindowResize();
        }
        if (Input::KeyPressed(GLFW_KEY_COMMA))
        {
            Text::SetGlobalTextScaling(Text::GetGlobalTextScaling() - 0.2f);
            WindowResize();
        }

        if (inMenu)
        {
            right_edge   = centerX + mainMenu.MenuWidth / 2;
            left_edge    = centerX - mainMenu.MenuWidth / 2;
            top_edge     = centerY + MainMenuStartY;
            bottom_edge  = centerY - MainMenuStartY;

            if (Input::KeyPressed(GLFW_KEY_UP))
            {
                mainMenu.ActiveItem = glm::clamp(mainMenu.ActiveItem - 1, 0, (int)items.size() - 1);
            }
            if (Input::KeyPressed(GLFW_KEY_DOWN))
            {
                mainMenu.ActiveItem = glm::clamp(mainMenu.ActiveItem + 1, 0, (int)items.size() - 1);
            }
            if (Input::KeyPressed(GLFW_KEY_RIGHT))
            {
                inSubMenu = true;
                //activeItem = 0;
            }
            if (Input::KeyPressed(GLFW_KEY_LEFT))
            {
                inSubMenu = false;
            }

            // BG tint
            defocusShader->Use();
            glBindVertexArray(defocusVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            UIshader->Use();
            UIshader->SetMatrix4("projection", AssetManager::OrthoProjMat4);

            // DrawMenu("hotbox", 0);
            /* if (inSubMenu > 0)
            {
                DrawMenu("test", 1);
            } */

            mainMenu.Render();

            std::string mw = "Menu Width: " + std::to_string(mainMenu.MenuWidth);
            Text::Render(mw, 25, 85, 0.45f);
            Text::Render("Active Item: " + std::to_string(mainMenu.ActiveItem), 25, 65, 0.45f);
            Text::Render("In SubMenu: " + std::to_string(inSubMenu), 25, 45, 0.45f);
            Text::Render("In Menu: " + std::to_string(inMenu), 25, 25, 0.45f);

            /* DrawMenuRect(glm::vec2(left_edge + ItemSelectionThickness, top_edge - titleHeightPX - itemHeightPX * (activeItem)),
                         glm::vec2(left_edge,  top_edge - titleHeightPX - itemHeightPX * (activeItem + 1)), SecondaryColor); */
        }
    }

    Menu::Menu(std::string MenuTitle)
    {
        Title = MenuTitle;
        TitleHeight = Text::CalculateMaxTextHeight(MenuTitle, TitleBarTextScaling, true) + TitleBarPaddingYPX * 2;
        MenuWidth   = Text::CalculateTextWidth(MenuTitle, ItemTextScaling) + ItemPaddingXPX * 2;
    }

    void Menu::Render()
    {
        {
            float xoffset = 0; // (mainMenuWidth + 100) * -depth + (mainMenuWidth + 100) * inSubMenu;

            // Outline
            DrawMenuRect(glm::vec2(right_edge + outlineWidth - xoffset, top_edge + outlineWidth),
                         glm::vec2(left_edge - outlineWidth - xoffset, top_edge - titleHeightPX - itemHeightPX * items.size() - outlineWidth), OutlineColor);

            // Main Menu Title
            DrawMenuRect(glm::vec2(right_edge - xoffset, top_edge),
                         glm::vec2(left_edge  - xoffset, top_edge - titleHeightPX), TitleBarColor);
            Text::Render(Title, centerX - MenuWidth / 2 - xoffset,
                                top_edge - titleHeightPX + TitleBarPaddingYPX, TitleBarTextScaling, TitleBarTextColor);

            for (int i = 0; i < items.size(); i++)
            {    
                float distance  = ((items.size() - glm::abs(ActiveItem - i)) / (float)items.size());
                float itemShade = distance * distance * distance;
                itemShade = glm::pow(itemShade, 1.0f / 2.2f);
                
                DrawMenuRect(glm::vec2(right_edge - xoffset, top_edge - titleHeightPX - itemHeightPX * i),
                             glm::vec2(left_edge  - xoffset, top_edge - titleHeightPX - itemHeightPX * (i + 1)), ItemColor * itemShade);
                glm::vec3 col = (i == ActiveItem) ? ItemTextActive : ItemTextInactive;
                Text::Render(items[i], centerX  - MenuWidth / 2.0f + ItemPaddingXPX - xoffset,
                                        top_edge - titleHeightPX - itemHeightPX * (i + 1) + ItemPaddingYPX, ItemTextScaling, col);
            }

            // Selection Marker
            DrawMenuRect(glm::vec2(right_edge - xoffset, top_edge - titleHeightPX - itemHeightPX * (ActiveItem)),
                         glm::vec2(right_edge - ItemSelectionThickness - xoffset,  top_edge - titleHeightPX - itemHeightPX * (ActiveItem + 1)), SecondaryColor);
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
        UIshader      = std::make_unique<Shader>("/../res/shaders/ui");
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
        itemHeightPX  = Text::CalculateMaxTextHeight(items[0].substr(0, 1), ItemTextScaling, true) + ItemPaddingYPX * 2;
        centerX = Engine::GetWindowSize().x / 2.0f;
        centerY = Engine::GetWindowSize().y / 2.0f;

        UIshader->Use();
        UIshader->SetMatrix4("projection", AssetManager::OrthoProjMat4);
    }
}