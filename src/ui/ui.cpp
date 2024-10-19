#include <iostream>
#include <memory>
#include <array>
#include <functional>

#include "../../lib/glad/glad.h"
#include "../../lib/glm/glm.hpp"

#include "ui.h"
#include "text_renderer.h"
#include "../common/shader.h"
#include "../common/input.h"
#include "../common/stat_counter.h"
#include "../engine/render_engine.h"
#include "../engine/asset_manager.h"
#include "../engine/scene_manager.h"

namespace UI
{
    enum HotMenu
    {
        none,
        hm_SceneManager,
        hm_AssetManager,
        hm_Qk
    };

    void Resize(int width, int height);

    void MainMenu(float xoffset, float yoffset, const Menu &submenu);
    void SceneManagerChild(std::string title, float xoffset, float yoffset);

    void DrawList(float xoffset, float yoffset, std::vector<std::string> items, const Menu &submenu, bool gradient = false);
    void DrawArrow(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color);

    std::unique_ptr<Shader> UIshader, defocusShader;

    uint defocusVAO;
    uint menuVAO, menuVBO;
    std::array<float, 8> vertices;


    // ---- replace with config file? ----

    // Menu General
    int OutlineWidth   = 4;
    int MenuStartY = 220;
    glm::vec3 OutlineColor(0.09f);
    glm::vec3 SecondaryColor = glm::vec3(247, 224, 100) / 256.0f; //vec3(255,141,227) / 256.0f; 

    // Title Bar
    int TitleBarPaddingYPX    = 5;
    float TitleBarTextScaling = 0.45f;
    glm::vec3 TitleBarTextColor(0.05f);
    glm::vec3 TitleBarColor   = glm::vec3(126, 214, 128) / 256.0f;

    // Item
    float ItemTextScaling = 0.475f;
    int   ItemPaddingXPX  = 25;
    int   ItemPaddingYPX  = 8;
    int   ItemSelectionThickness = 4;
    int   ItemDistanceShade = 5;
    float ItemShadePct(0.5f);
    glm::vec3 ItemColor(0.15f);
    glm::vec3 ItemTextActive   = glm::vec3(1.0f);
    glm::vec3 ItemTextInactive = glm::vec3(122, 119, 126) / 256.0f;

    // Item Selection modifier
    int stepMultiplier = 3;

    // -----------------------------------

    float itemHeightPX;
    float centerX, centerY;
    float right_edge, left_edge; 
    float top_edge, bottom_edge;
    int titleHeight;

    bool inMenu = false;
    int inSubMenu = false;

    Menu mainMenu;
    Menu sceneManager, assetManager, statistics, engine, qk;
    std::array<Menu, 5> subMenus;

    HotMenu whereAmI = none;

    float m_timer = 0.0f;
    void Render()
    {
        glDisable(GL_DEPTH_TEST);

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
                whereAmI = none;
                Input::SetInputContext(Input::Menu);
            }
        }

        if (Input::KeyPressed(GLFW_KEY_PERIOD))
        {
            Text::SetGlobalTextScaling(Text::GetGlobalTextScaling() + 0.2f);
            Resize(0, 0);
        }
        if (Input::KeyPressed(GLFW_KEY_COMMA))
        {
            Text::SetGlobalTextScaling(Text::GetGlobalTextScaling() - 0.2f);
            Resize(0, 0);
        }

        if (Input::KeyDown(GLFW_KEY_LEFT_SHIFT) && Input::KeyPressed(GLFW_KEY_SPACE) && inMenu == false)
        {
            if (whereAmI == none)
            {
                whereAmI = hm_SceneManager;
                Input::SetInputContext(Input::HotMenu);
            }
            else
            {
                whereAmI = none;
                Input::SetInputContext(Input::Game);
            }
        }

        right_edge   = centerX + mainMenu.MenuWidth / 2;
        left_edge    = centerX - mainMenu.MenuWidth / 2;
        top_edge     = centerY + MenuStartY;
        bottom_edge  = centerY - MenuStartY;

        if (whereAmI == hm_SceneManager)
        {
            int index = 0;
            subMenus[index].Input(SceneManager::Objects.size());
            
            float yoffset = itemHeightPX * subMenus[index].ActiveSubMenu;

            const Menu &submenu = subMenus[index];
            //DrawList(0, yoffset, SceneManager::Objects.size(), submenu, &SceneManager);

            if (Input::KeyPressed(GLFW_KEY_LEFT))
            {
                whereAmI = none;
                inMenu = true;
                inSubMenu = true;
                mainMenu.ActiveSubMenu = 0;
            }
        }

        if (inMenu)
        {   
            m_timer += Statistics::GetDeltaTime();
            if (m_timer >= (1 / 12.0f))
            {
                m_timer = 0.0f;
                if (Input::KeyDown(GLFW_KEY_UP) || Input::KeyPressed(GLFW_KEY_UP)) if (!inSubMenu) mainMenu.ActiveSubMenu = glm::clamp(mainMenu.ActiveSubMenu - 1, 0, (int)subMenus.size() - 1);
                if (Input::KeyDown(GLFW_KEY_DOWN) || Input::KeyPressed(GLFW_KEY_UP)) if (!inSubMenu) mainMenu.ActiveSubMenu = glm::clamp(mainMenu.ActiveSubMenu + 1, 0, (int)subMenus.size() - 1);
            }

            if (Input::KeyPressed(GLFW_KEY_RIGHT)) inSubMenu = true;
            if (Input::KeyPressed(GLFW_KEY_LEFT)) inSubMenu = false;

            // BG tint
            defocusShader->Use();
            glBindVertexArray(defocusVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            UIshader->Use();
            UIshader->SetMatrix4("projection", AssetManager::OrthoProjMat4);

            if (inSubMenu) subMenus[mainMenu.ActiveSubMenu].Render();
            mainMenu.Render();

            Text::Render("Selected SubMenu: " + subMenus[mainMenu.ActiveSubMenu].Title, 25, 65, 0.45f);
            Text::Render("In SubMenu: " + std::to_string(inSubMenu), 25, 45, 0.45f);
            if (inSubMenu) Text::Render("SubMenu Active Item: " + std::to_string(subMenus[mainMenu.ActiveSubMenu].ActiveSubMenu), 25, 25, 0.45f);
        }

        glEnable(GL_DEPTH_TEST);
    }


    void Menu::Initialize(std::string MenuTitle, glm::vec3 TitleBarCol)
    {
        Title = MenuTitle;
        ThemeColor = TitleBarCol;
        MenuWidth = Text::CalculateTextWidth("Scene Manager", ItemTextScaling) + ItemPaddingXPX * 2;
    }

    float sm_timer = 0.0f;
    void Menu::Input(int NumSubMenus)
    {
        int mult = (Input::KeyDown(GLFW_KEY_LEFT_SHIFT)) ? stepMultiplier : 1;

        sm_timer += Statistics::GetDeltaTime();
        if (sm_timer >= (1 / (15.0f * mult)))
        {
            sm_timer = 0.0f;
            if (Input::KeyDown(GLFW_KEY_UP))
            {
                ActiveSubMenu = glm::clamp(ActiveSubMenu - 1, 0, NumSubMenus - 1);
            }

            if (Input::KeyDown(GLFW_KEY_DOWN))
            {
                ActiveSubMenu = glm::clamp(ActiveSubMenu + 1, 0, NumSubMenus - 1);
            }
        }
    }

    void Menu::Render()
    {
        float xoffset = ((mainMenu.MenuWidth + 5) * inSubMenu) / 2.0f;
        
        if (Title == subMenus[mainMenu.ActiveSubMenu].Title)
        {
            xoffset *= -1;

            if (Title == "Scene Manager")
            {
                std::vector<std::string> items;
                int i = 0;
                for (SceneManager::Object &obj : SceneManager::Objects)
                {
                    items.push_back(obj.GetName());
                }

                int index = 0;
                float yoffset = itemHeightPX * subMenus[index].ActiveSubMenu;

                const Menu &submenu = subMenus[index];
                DrawList(xoffset, yoffset, items, submenu, true);
                subMenus[index].Input(items.size());
                //SceneManagerChild(SceneManager::Objects[subMenus[index].ActiveSubMenu].GetName(), xoffset * 3, 0);
            }
            else if (Title == "Asset Manager")
            {
                std::vector<std::string> items{"Meshes", "Textures", "Materials", "Shaders"};

                int index = 1;

                const Menu &submenu = subMenus[index];
                DrawList(xoffset, 0, items, submenu);
                subMenus[index].Input(items.size());
            }
        }
        else
        {
            const Menu &menu = mainMenu;
            MainMenu(xoffset, 0, menu);
        }
    }

    void MainMenu(float xoffset, float yoffset, const Menu &submenu)
    {
        // Outline
        DrawRect(glm::vec2(right_edge + OutlineWidth - xoffset, top_edge + OutlineWidth),
                 glm::vec2(left_edge - OutlineWidth - xoffset, top_edge - titleHeight - itemHeightPX * subMenus.size() - OutlineWidth), OutlineColor);

        // Menu Title
        DrawRect(glm::vec2(right_edge - xoffset, top_edge),
                 glm::vec2(left_edge  - xoffset, top_edge - titleHeight), submenu.ThemeColor);
        Text::Render(submenu.Title, centerX - Text::CalculateTextWidth(submenu.Title, TitleBarTextScaling) / 2.0f - xoffset,
                     top_edge - titleHeight + TitleBarPaddingYPX, TitleBarTextScaling, TitleBarTextColor);

        for (int i = 0; i < subMenus.size(); i++)
        {    
            float itemShade = (i == submenu.ActiveSubMenu) ? 1.0f : 0.5f;
            
            DrawRect(glm::vec2(right_edge - xoffset, top_edge - titleHeight - itemHeightPX * i),
                     glm::vec2(left_edge  - xoffset, top_edge - titleHeight - itemHeightPX * (i + 1)), ItemColor * itemShade);
            glm::vec3 col = (i == submenu.ActiveSubMenu) ? ItemTextActive : ItemTextInactive;
            
            Text::Render(subMenus[i].Title, centerX - submenu.MenuWidth / 2.0f + ItemPaddingXPX - xoffset,
                         top_edge - titleHeight - itemHeightPX * (i + 1) + ItemPaddingYPX, ItemTextScaling, col);
        }

        // Selection Marker
        if (submenu.Title == mainMenu.Title)
        {
            if (inSubMenu)
            {
                DrawArrow(glm::vec2(right_edge - xoffset, top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu)),
                            glm::vec2(right_edge - ItemSelectionThickness - xoffset,  top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu + 1)), submenu.ThemeColor);
            }
            else DrawRect(glm::vec2(right_edge - xoffset, top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu)),
                            glm::vec2(right_edge - ItemSelectionThickness - xoffset,  top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu + 1)), submenu.ThemeColor);
        }
        else
        {
            DrawRect(glm::vec2(right_edge - xoffset, top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu)),
                     glm::vec2(right_edge - ItemSelectionThickness - xoffset, top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu + 1)), submenu.ThemeColor);
        }
    }

    float ChildTitleScaling = 0.75f;
    void SceneManagerChild(std::string title, float xoffset, float yoffset)
    {
        // Outline
        DrawRect(glm::vec2(right_edge + OutlineWidth - xoffset, top_edge + OutlineWidth),
                 glm::vec2(left_edge  - OutlineWidth - xoffset, top_edge - titleHeight - itemHeightPX * 5 - OutlineWidth), OutlineColor);

        // Menu Titles
        DrawRect(glm::vec2(right_edge - xoffset, top_edge),
                 glm::vec2(left_edge  - xoffset, top_edge - titleHeight), glm::vec3(0.2f));
        Text::Render(title, centerX - Text::CalculateTextWidth(title, TitleBarTextScaling) / 2.0f - xoffset,
                     top_edge - titleHeight + TitleBarPaddingYPX, TitleBarTextScaling, ItemTextActive);
    }

    // void (*func)(float _xoffset, float _yoffset, const Menu &_submenu)
    void DrawList(float xoffset, float yoffset, std::vector<std::string> items, const Menu &submenu, bool gradient)
    {
        // Outline
        DrawRect(glm::vec2(right_edge + OutlineWidth - xoffset, top_edge + OutlineWidth + yoffset),
                 glm::vec2(left_edge - OutlineWidth - xoffset, top_edge - titleHeight - itemHeightPX * items.size() - OutlineWidth + yoffset), OutlineColor);
    
        // Menu Title
        DrawRect(glm::vec2(right_edge - xoffset, top_edge + yoffset),
                 glm::vec2(left_edge  - xoffset, top_edge - titleHeight + yoffset), submenu.ThemeColor);
        Text::Render(submenu.Title, centerX - Text::CalculateTextWidth(submenu.Title, TitleBarTextScaling) / 2.0f - xoffset,
                            top_edge - titleHeight + TitleBarPaddingYPX + yoffset, TitleBarTextScaling, TitleBarTextColor);

        for (int i = 0; i < items.size(); i++)
        {
            float itemShade = 1.0f;
            if (!gradient) itemShade = (i == submenu.ActiveSubMenu) ? 1.0f : ItemShadePct;
            else
            {
                float distance = ((ItemDistanceShade - glm::abs(submenu.ActiveSubMenu - i)) / (float)ItemDistanceShade);
                itemShade = glm::pow(glm::pow(distance, 3), 1.0f / 2.2f);
            }
            
            DrawRect(glm::vec2(right_edge - xoffset, top_edge - titleHeight - itemHeightPX * i + yoffset),
                     glm::vec2(left_edge  - xoffset, top_edge - titleHeight - itemHeightPX * (i + 1) + yoffset), ItemColor * itemShade);

            glm::vec3 col = (i == submenu.ActiveSubMenu) ? ItemTextActive : ItemTextInactive;
            Text::Render(items[i], centerX - submenu.MenuWidth / 2.0f + ItemPaddingXPX - xoffset,
                         top_edge - titleHeight - itemHeightPX * (i + 1) + yoffset + ItemPaddingYPX, ItemTextScaling, col);
        }

        // Selection Marker
        DrawRect(glm::vec2(right_edge - xoffset, top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu) + yoffset),
                 glm::vec2(right_edge - ItemSelectionThickness - xoffset, top_edge - titleHeight - itemHeightPX * (submenu.ActiveSubMenu + 1) + yoffset), submenu.ThemeColor);
    }

    void DrawRect(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color)
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

    void DrawArrow(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color)
    {
        vertices = 
        {
            BottomLeft.x, TopRight.y - ItemSelectionThickness,
            BottomLeft.x, BottomLeft.y + ItemSelectionThickness,
            BottomLeft.x + ItemSelectionThickness * 2 + OutlineWidth * 2, (TopRight.y + BottomLeft.y) / 2.0f
        };
        UIshader->Use();
        UIshader->SetVector3("color", color);
        glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(menuVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
    }

    int CalculateMenuSize()
    {
        return 0;
    }

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);

        UIshader      = std::make_unique<Shader>("/../res/shaders/ui/ui");
        defocusShader = std::make_unique<Shader>("/../res/shaders/ui/defocus");

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

        mainMenu.Initialize("hotbox", glm::vec3(126, 214, 128) / 256.0f);

        sceneManager.Initialize("Scene Manager", SecondaryColor);
        assetManager.Initialize("Asset Manager", SecondaryColor);
        statistics.Initialize("Statistics",      SecondaryColor);
        engine.Initialize("Engine",              SecondaryColor);
        qk.Initialize("Qk",                      SecondaryColor);
        
        subMenus[0] = sceneManager;
        subMenus[1] = assetManager;
        subMenus[2] = statistics;
        subMenus[3] = engine;
        subMenus[4] = qk;
    }

    void Resize(int width, int height)
    {
        titleHeight = Text::CalculateMaxTextHeight("A", TitleBarTextScaling) + TitleBarPaddingYPX * 2;
        itemHeightPX = Text::CalculateMaxTextHeight("A", ItemTextScaling) + ItemPaddingYPX * 2;
        centerX = Engine::GetWindowSize().x / 2.0f;
        centerY = Engine::GetWindowSize().y / 2.0f;
        
        UIshader->Use();
        UIshader->SetMatrix4("projection", AssetManager::OrthoProjMat4);
    }
}