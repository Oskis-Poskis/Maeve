#include <iostream>
#include <memory>
#include <array>
#include <functional>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    void Resize(int width, int height);
    void RecalcMenuWidths(Menu* targetmenu);

    void DrawList(int xoffset, int yoffset, std::vector<std::string> items, Menu &submenu, bool gradient = false);
    void DrawArrow(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color);

    std::unique_ptr<Shader> menuShader, defocusShader;

    unsigned int defocusVAO;
    unsigned int menuVAO, menuVBO;
    std::array<int, 8> vertices;

    // ---- replace with config file? ----

    // Menu General
    int OutlineWidth   = 2;
    int MenuStartY = 220;
    float animSpeed = 5.0f;
    glm::vec3 OutlineColor(0.09f);
    glm::vec3 MainMenuColor  = glm::vec3(126, 204, 118) / 256.0f;
    glm::vec3 SecondaryColor = glm::vec3(247, 224, 100) / 256.0f; //vec3(255,141,227) / 256.0f;
    glm::vec3 TertieryColor  = glm::vec3(169, 167, 227) / 256.0f; //vec3(255,141,227) / 256.0f;

    // Title Bar
    int TitleBarPaddingYPX    = 5;
    float TitleBarTextScaling = 0.5f;
    glm::vec3 TitleBarTextColor(0.05f);

    // Item
    float ItemTextScaling = 0.55f;
    int   ItemPaddingXPX  = 25;
    int   ItemPaddingYPX  = 7;
    int   ItemSelectionThickness = 3;
    int   ItemDistanceShade = 6;
    float ItemShadePct(0.5f);
    glm::vec3 ItemColor(0.15f);
    glm::vec3 ItemTextActive   = glm::vec3(1.0f);
    glm::vec3 ItemTextInactive = glm::vec3(122, 119, 126) / 256.0f;

    // Item Selection modifier
    int stepMultiplier = 3;

    // -----------------------------------

    int itemHeightPX;
    int centerX, centerY;
    int right_edge, left_edge; 
    int top_edge;
    int titleHeight;

    Menu mainMenu;
         Menu sceneManager;
         Menu assetManager;
              Menu meshes;
              Menu lights;
              Menu materials;
              Menu textures;
         Menu statistics;
         Menu engine;
         Menu qk;

    bool inMenu = false;
    bool grabbingMenu = false;
    int menuDepth = 0;
    float xoffset = 0;

    Menu* activemenu;
    Menu* toplevelmenu;

    void Eject()
    {
        inMenu = false;    
    }

    void Render()
    {
        glDisable(GL_DEPTH_TEST);

        if (Input::KeyPressed(GLFW_KEY_TAB)) {
            inMenu = !inMenu;
            Input::SetInputContext(inMenu ? Input::Menu : Input::Game);
        }
        if (Input::KeyDown(GLFW_KEY_ESCAPE)) inMenu = false;

        if (Input::KeyDown(GLFW_KEY_LEFT_SHIFT)) {
            // Scene manager
            if (Input::KeyPressed(GLFW_KEY_SPACE)) {
                activemenu = mainMenu.SubMenus[0];
                mainMenu.SelectedSubMenu = 0;

                inMenu = true;
                Input::SetInputContext(Input::Menu);
            }
            if (Input::KeyPressed(GLFW_KEY_A))
            {
                activemenu = &assetManager;
                mainMenu.SelectedSubMenu = 1;

                inMenu = true;
                Input::SetInputContext(Input::Menu);
            }
        }
        // Asset manager
        if (Input::KeyDown(GLFW_KEY_LEFT_CONTROL)) {
            if (Input::KeyPressed(GLFW_KEY_SPACE)) {
                activemenu = mainMenu.SubMenus[1];
                mainMenu.SelectedSubMenu = 1;

                inMenu = true;
                Input::SetInputContext(Input::Menu);
            }
        }

        if (Input::KeyPressed(GLFW_KEY_PERIOD)) {
            Text::SetGlobalTextScaling(Text::GetGlobalTextScaling() + 0.2f);
            Resize(0, 0);
            RecalcMenuWidths(toplevelmenu);
        }
        if (Input::KeyPressed(GLFW_KEY_COMMA)) {
            Text::SetGlobalTextScaling(Text::GetGlobalTextScaling() - 0.2f);
            Resize(0, 0);
            RecalcMenuWidths(toplevelmenu);
        }

        if (inMenu) {
            // if (isRectHovered({ left_edge, top_edge - titleHeight },
            //                   { left_edge + activemenu->MenuWidth, top_edge /* + yoffset*/ }) || grabbingMenu)
            // {
            //     if (Input::LeftMBDown()) grabbingMenu = true;
            // }
            if (Input::KeyPressed(GLFW_KEY_G)) grabbingMenu = !grabbingMenu;
            if (grabbingMenu) {
                if (Input::LeftMBDown()) grabbingMenu = false;
                centerX += Input::GetMouseDeltaX();
                centerY += Input::GetMouseDeltaY();
            }

            right_edge  = centerX + mainMenu.MenuWidth / 2;
            left_edge   = centerX - mainMenu.MenuWidth / 2;
            top_edge    = centerY + MenuStartY;

            // BG tint
            defocusShader->Use();
            glBindVertexArray(defocusVAO);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            activemenu->Render();
        }

        glEnable(GL_DEPTH_TEST);
    }

    float blend = 0;
    float elapsed = 0;
    void Menu::Render()
    {
        Input(NumItems);

        menuShader->Use();
        menuShader->SetMatrix4("projection", AM::OrthoProjMat4);

        elapsed = glm::clamp(elapsed + Stats::GetDeltaTime() * animSpeed, 0.0f, 1.0f);
        blend = glm::smoothstep(0.0f, 1.0f, elapsed);
        xoffset = glm::mix(0, mainMenu.MenuWidth + OutlineWidth, blend);

        Text::Render("Address: " + std::to_string(reinterpret_cast<uintptr_t>(this)), 25, 85, TitleBarTextScaling);
        Text::Render("Active menu: " + activemenu->Title, 25, 65, TitleBarTextScaling);
        if (parent) Text::Render("Parent: " + parent->Title, 25, 45, TitleBarTextScaling);
        Text::Render("Selected item: " + std::to_string(SelectedSubMenu), 25, 25, TitleBarTextScaling);

        int yoffset = CenteredList ? itemHeightPX * (SelectedSubMenu % MaxItemsUntilWrap) : 0;

        if (HasItems) {
            NumItems = Items->size();
            DrawList(0, yoffset, *Items, *this, CenteredList);
        }
        else {
            std::vector<std::string> items;
            for (Menu* obj : SubMenus) items.push_back(obj->Title);
            NumItems = items.size();
            DrawList(0, yoffset, items, *this, CenteredList);
        }
    }

    float y_timer = 0.0f;
    float x_timer = 0.0f;
    void Menu::Input(int NumSubMenus)
    {
        if (ExtraInput) ExtraInput();

        if (Input::KeyPressed(GLFW_KEY_HOME)) SelectedSubMenu = 0;
        if (Input::KeyPressed(GLFW_KEY_END))
        {
            if (SubMenus.size() > 0) SelectedSubMenu = SubMenus.size() - 1;
            else if (Items) SelectedSubMenu = Items->size() - 1;
        }

        int mult = (Input::KeyDown(GLFW_KEY_LEFT_SHIFT)) ? stepMultiplier : 1;

        x_timer += Stats::GetDeltaTime();
        if (x_timer >= 1 / (5.0f * mult) || Input::KeyPressed(GLFW_KEY_RIGHT) || Input::KeyPressed(GLFW_KEY_LEFT)) {
            x_timer = 0.0f;

            if (Input::KeyDown(GLFW_KEY_RIGHT) || Input::KeyPressed(GLFW_KEY_RIGHT))
            {
                if (SubMenus.size() > 0) activemenu = SubMenus[SelectedSubMenu];
                else if (Items && GetActiveColumn() != GetNumColumns()) SelectedSubMenu = WrapIndex(SelectedSubMenu, Items->size() - 1);
            }
            if ((Input::KeyDown(GLFW_KEY_LEFT) || Input::KeyPressed(GLFW_KEY_LEFT)) && !IsTopLevel)
            {
                if ((!Items) || (Items && SelectedSubMenu < MaxItemsUntilWrap)) activemenu = parent;
                else if (Items && SelectedSubMenu >= MaxItemsUntilWrap) SelectedSubMenu = WrapIndex(SelectedSubMenu, Items->size(), true);
            }   
        }

        y_timer += Stats::GetDeltaTime();
        if (y_timer >= 1 / (15.0f * mult)) {
            y_timer = 0.0f;

            if (Input::KeyDown(GLFW_KEY_UP))
            {
                if (SubMenus.size() > 0) SelectedSubMenu = glm::clamp(SelectedSubMenu - 1, 0, NumSubMenus - 1);
                else if (Items) SelectedSubMenu = glm::clamp(SelectedSubMenu - 1, 0, NumSubMenus - 1);
                // && ((SelectedSubMenu) % MaxItemsUntilWrap) != 0
            }
            if (Input::KeyDown(GLFW_KEY_DOWN))
            {
                if (SubMenus.size() > 0) SelectedSubMenu = glm::clamp(SelectedSubMenu + 1, 0, NumSubMenus - 1);
                else if (Items) SelectedSubMenu = glm::clamp(SelectedSubMenu + 1, 0, NumSubMenus - 1);
                // Items && ((SelectedSubMenu + 1) % MaxItemsUntilWrap) != 0
            }
        }
    }

    int Menu::WrapIndex(int index, int size, bool negate)
    {
        return glm::clamp(index + MaxItemsUntilWrap * (negate ? -1 : 1), 0, size);
    }

    int Menu::GetNumColumns()
    {
        if (Items) return Items->size() / MaxItemsUntilWrap;
        else return 0;
    }

    int Menu::GetActiveColumn()
    {
        return SelectedSubMenu / MaxItemsUntilWrap;
    }

    void Menu::Initialize(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* items)
    {
        Initialize(MenuTitle, ThemeCol);
        HasItems = true;
        Items = items;
    }

    void Menu::Initialize(std::string MenuTitle, glm::vec3 ThemeCol)
    {
        Title = MenuTitle;
        ThemeColor = ThemeCol;
        MenuWidth = Text::CalculateTextWidth("Scene Manager", ItemTextScaling) + ItemPaddingXPX * 2;
        Items = nullptr;
    }

    void Menu::AddSubMenu(Menu* menu)
    {
        menu->parent = this;
        SubMenus.push_back(menu);
    }

    void DrawList(int xoffset_, int yoffset, std::vector<std::string> items, Menu &submenu, bool gradient)
    {
        int xoffset = xoffset_ - (submenu.SelectedSubMenu / submenu.MaxItemsUntilWrap) * submenu.MenuWidth;

        int vis_below_above = 4;

        if (submenu.BGopacity > 0.0f)
        {
            // Header
            if (submenu.HiearchyView)
            {
                if (submenu.HasHeaderBar)
                {
                    DrawRect(glm::vec2(left_edge, top_edge + glm::min(vis_below_above * itemHeightPX, yoffset)),
                             submenu.MenuWidth, titleHeight, submenu.ThemeColor, submenu.BGopacity);
                }
            }
            else
            {
                if (submenu.HasHeaderBar)
                {
                    DrawRect(glm::vec2(left_edge, top_edge + yoffset),
                             submenu.MenuWidth, titleHeight, submenu.ThemeColor, submenu.BGopacity);
                }
            }
        }
        
        // Title
        if (submenu.HiearchyView) {
            Text::Render(submenu.Title, centerX - Text::CalculateTextWidth(submenu.Title, TitleBarTextScaling) / 2,
                         top_edge - titleHeight + TitleBarPaddingYPX + glm::min(vis_below_above * itemHeightPX, yoffset), TitleBarTextScaling, glm::mix(glm::vec3(0.9), TitleBarTextColor, submenu.BGopacity));
        }
        else {
            Text::Render(submenu.Title, centerX - Text::CalculateTextWidth(submenu.Title, TitleBarTextScaling) / 2,
                        top_edge - titleHeight + TitleBarPaddingYPX + yoffset, TitleBarTextScaling, glm::mix(glm::vec3(0.9), TitleBarTextColor, submenu.BGopacity));
        }

        for (int i = 0; i < items.size(); i++)
        {
            int xoffoffset = (i / submenu.MaxItemsUntilWrap) * submenu.MenuWidth + xoffset;
            int yoffoffset = i % submenu.MaxItemsUntilWrap;

            if (submenu.BGopacity > 0.0f)
            {
                float itemShade = 0.0f;
                if (!gradient) itemShade = (i == submenu.SelectedSubMenu) ? 1.0f : ItemShadePct;
                else
                {
                    if (i / submenu.MaxItemsUntilWrap == submenu.SelectedSubMenu / submenu.MaxItemsUntilWrap)
                    {
                        float distance = ((ItemDistanceShade - glm::abs(submenu.SelectedSubMenu - i)) / (float)ItemDistanceShade);
                        itemShade = glm::pow(glm::pow(distance, 3), 1.0f / 2.2f);
                    }
                }
                
                if (submenu.HiearchyView)
                {
                    if (i <= submenu.SelectedSubMenu + vis_below_above && i >= submenu.SelectedSubMenu - vis_below_above)
                    {
                        DrawRect(glm::vec2(left_edge + xoffoffset, top_edge - titleHeight - itemHeightPX * yoffoffset + yoffset),
                                submenu.MenuWidth, itemHeightPX, ItemColor * itemShade, submenu.BGopacity);
                        if ((i == submenu.SelectedSubMenu + vis_below_above && submenu.SelectedSubMenu >= vis_below_above)) {
                            Text::Render(std::to_string(items.size() - i) + "...", centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoffset,
                                         top_edge - titleHeight - itemHeightPX * (yoffoffset + 1) + yoffset + ItemPaddingYPX, ItemTextScaling, ItemTextInactive);
                        }
                        else if ((i == submenu.SelectedSubMenu - vis_below_above)) {
                            Text::Render(std::to_string(i + 1) + "...", centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoffset,
                                         top_edge - titleHeight - itemHeightPX * (yoffoffset + 1) + yoffset + ItemPaddingYPX, ItemTextScaling, ItemTextInactive);
                        }
                    }
                }
                else
                {
                    DrawRect(glm::vec2(left_edge + xoffoffset, top_edge - titleHeight - itemHeightPX * yoffoffset + yoffset),
                             submenu.MenuWidth, itemHeightPX, ItemColor * itemShade, submenu.BGopacity);
                }
            }
            glm::vec3 col = (i == submenu.SelectedSubMenu) ? ItemTextActive : ItemTextInactive;

            if (submenu.HiearchyView)
            {
                if (i <= submenu.SelectedSubMenu + vis_below_above && i > submenu.SelectedSubMenu - vis_below_above && (i != submenu.SelectedSubMenu + vis_below_above || submenu.SelectedSubMenu < vis_below_above))
                {
                    Text::Render(items[i], centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoffoffset,
                                 top_edge - titleHeight - itemHeightPX * (yoffoffset + 1) + yoffset + ItemPaddingYPX, ItemTextScaling, col);
                }
            }
            else
            {
                Text::Render(items[i], centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoffoffset,
                             top_edge - titleHeight - itemHeightPX * (yoffoffset + 1) + yoffset + ItemPaddingYPX, ItemTextScaling, col);
            }
        }

        int cursorxoffset = (submenu.SelectedSubMenu / submenu.MaxItemsUntilWrap) * submenu.MenuWidth;
        int cursoryoffset = submenu.SelectedSubMenu % submenu.MaxItemsUntilWrap;

        // Cursor
        DrawRect(glm::vec2(right_edge - ItemSelectionThickness, top_edge - titleHeight - itemHeightPX * cursoryoffset + yoffset),
                 ItemSelectionThickness, itemHeightPX, submenu.ThemeColor);
    }

    void DrawRect(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 Color, float Opacity)
    {
        vertices = 
        {
            TopRight.x,   TopRight.y,
            BottomLeft.x, TopRight.y,
            BottomLeft.x, BottomLeft.y,
            TopRight.x,   BottomLeft.y,
        };
        menuShader->Use();
        menuShader->SetVector3("color", Color);
        menuShader->SetFloat("blend", Opacity);
        glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(menuVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void DrawRect(glm::ivec2 TopLeft, int width, int height, glm::vec3 Color, float Opacity)
    {
        vertices = 
        {
            TopLeft.x + width, TopLeft.y,
            TopLeft.x,  TopLeft.y,
            TopLeft.x,  TopLeft.y - height,
            TopLeft.x + width, TopLeft.y - height,
        };
        menuShader->Use();
        menuShader->SetVector3("color", Color);
        menuShader->SetFloat("blend", Opacity);
        glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(menuVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    bool isRectHovered(glm::ivec2 BottomLeft, glm::ivec2 TopRight)
    {
        glm::ivec2 cursor_px = glm::ivec2(Input::GetMouseX(), Input::GetMouseY());
        cursor_px.y = Engine::GetWindowSize().y - cursor_px.y;
        

        if (cursor_px.y < TopRight.y && cursor_px.y > BottomLeft.y &&
            cursor_px.x < TopRight.x && cursor_px.x > BottomLeft.x) return true;
        return false;
    }

    void DrawArrow(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 color)
    {
        vertices = 
        {
            BottomLeft.x, TopRight.y - ItemSelectionThickness,
            BottomLeft.x, BottomLeft.y + ItemSelectionThickness,
            BottomLeft.x + ItemSelectionThickness * 2, (TopRight.y + BottomLeft.y) / 2
        };
        menuShader->Use();
        menuShader->SetVector3("color", color);
        glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(menuVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
    }

    void RecalcMenuWidths(Menu* targetmenu)
    {
        targetmenu->MenuWidth = Text::CalculateTextWidth("Scene Manager", ItemTextScaling) + ItemPaddingXPX * 2;
        for (auto &menu : targetmenu->SubMenus)
        {
            menu->MenuWidth = Text::CalculateTextWidth("Scene Manager", ItemTextScaling) + ItemPaddingXPX * 2;
            RecalcMenuWidths(menu);
        }
    }

    void MeshesInput()
    {
        if (Input::KeyPressed(GLFW_KEY_ENTER))
        {
            std::string name = AM::MeshNames[activemenu->SelectedSubMenu];
            SM::Object* obj = new SM::Object(name, name);
            SM::AddNode(obj);
        }
    }

    void LightsInput()
    {
        if (Input::KeyPressed(GLFW_KEY_ENTER))
        {
            std::string name = AM::LightNames[activemenu->SelectedSubMenu];
            SM::Light* light = new SM::Light("Point Light", SM::LightType::Point, glm::vec3(0), 10.0f, glm::vec3(1), 5.0f);
            SM::AddNode(light);
        }
    }

    void SceneManagerInput()
    {
        SM::SelectSceneType(activemenu->SelectedSubMenu);
        if (Input::KeyPressed(GLFW_KEY_ENTER))
        {
            activemenu->HiearchyView = !activemenu->HiearchyView;
            if (activemenu->HiearchyView)
            {
                activemenu->MaxItemsUntilWrap = 10000;
            }
            else activemenu->MaxItemsUntilWrap = 24;
        }
    }

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);

        menuShader    = std::make_unique<Shader>("/res/shaders/ui/menu");
        defocusShader = std::make_unique<Shader>("/res/shaders/ui/defocus");

        glGenVertexArrays(1, &menuVAO);
        glGenBuffers(1, &menuVBO);

        glBindVertexArray(menuVAO);
        glBindBuffer(GL_ARRAY_BUFFER, menuVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(int) * 2 * 6, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(int) * 2, 0);

        // Dummy VAO for fullscreen quad vert shader
        glGenVertexArrays(1, &defocusVAO);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        mainMenu.Initialize("hotbox", MainMenuColor);
        mainMenu.IsTopLevel = true;
        mainMenu.BGopacity = 0.0f;
        mainMenu.HasHeaderBar = false;
        {
            sceneManager.Initialize("Scene Manager", SecondaryColor, &SM::SceneNodeNames);
            sceneManager.CenteredList = true;
            sceneManager.ExtraInput = SceneManagerInput;

            assetManager.Initialize("Asset Manager", TertieryColor);
            {
                meshes.Initialize("Meshes", SecondaryColor, &AM::MeshNames);
                meshes.ExtraInput = MeshesInput;

                lights.Initialize("Lights", SecondaryColor, &AM::LightNames);
                lights.ExtraInput = LightsInput;

                materials.Initialize("Materials", SecondaryColor);

                textures.Initialize("Textures", SecondaryColor);
            }
            assetManager.AddSubMenu(&meshes);
            assetManager.AddSubMenu(&lights);
            assetManager.AddSubMenu(&materials);
            assetManager.AddSubMenu(&textures);
            
            statistics.Initialize("Statistics", SecondaryColor);
            engine.Initialize("Engine",         SecondaryColor);
            qk.Initialize("Qk",                 SecondaryColor);
        }
        
        mainMenu.AddSubMenu(&sceneManager);
        mainMenu.AddSubMenu(&assetManager);
        mainMenu.AddSubMenu(&statistics);
        mainMenu.AddSubMenu(&engine);
        mainMenu.AddSubMenu(&qk);

        activemenu = &mainMenu;
        toplevelmenu = &mainMenu;
    }

    void Resize(int width, int height)
    {
        titleHeight = Text::CalculateMaxTextAscent("A", TitleBarTextScaling) + TitleBarPaddingYPX * 2;
        itemHeightPX = Text::CalculateMaxTextAscent("A", ItemTextScaling) + ItemPaddingYPX * 2;
        centerX = Engine::GetWindowSize().x / 2.0f;
        centerY = Engine::GetWindowSize().y / 2.0f;
        
        menuShader->Use();
        menuShader->SetMatrix4("projection", AM::OrthoProjMat4);
    }
}