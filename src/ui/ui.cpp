#include <iostream>
#include <memory>
#include <array>
#include <functional>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "ui.h"
#include "text_renderer.h"
#include "../common/qk.h"
#include "../common/shader.h"
#include "../common/input.h"
#include "../common/stat_counter.h"
#include "../engine/render_engine.h"
#include "../engine/asset_manager.h"
#include "../engine/scene_manager.h"
#include "../engine/deferred/deffered_manager.h"
#include "../third-party/tfl/tinyfiledialogs.h"

namespace UI
{
    void Resize(int width, int height);
    void ReloadShaders();
    void RecalcMenuWidths(Menu* targetmenu);

    void DrawList(int xoffset, int yoffset, std::vector<std::string>* items, Menu &submenu, bool gradient = false);
    void DrawArrow(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color);

    std::unique_ptr<Shader> rectShader, gradientRectShader, frostedRectShader, circleShader, defocusShader, colorWheelShader;

    unsigned int defocusVAO;
    unsigned int rectVAO, rectVBO;
    std::array<int, 8> vertices;

    // ---- replace with config file? ----

    // Menu General
    int OutlineWidth = 2;
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
         Menu debug;

    std::vector<std::string> MN_DebugItems = { "None", "BVH", "Deferred", "Stats", "Shadow Map" };

    bool inMenu       = false;
    bool grabbingMenu = false;
    int menuDepth = 0;
    float xoffset = 0;

    Menu* activemenu;
    Menu* toplevelmenu;

    int slidingSliderID = -1;
    int currentSliderID =  0;

    uintptr_t activeInputBoxID = -1;

    void Eject()
    {
        if (inMenu) {
            inMenu = false;
            Input::SetInputContext(Input::Game);
        }
    }

    void Render()
    {
        currentSliderID = 0;

        if (Input::KeyPressed(GLFW_KEY_TAB)) {
            inMenu = !inMenu;
            Input::SetInputContext(inMenu ? Input::Menu : Input::Game);
            if (activemenu->Title == sceneManager.Title) sceneManager.SelectedSubMenu = SM::GetSelectedIndex(); 
        }
        if (Input::KeyDown(GLFW_KEY_ESCAPE)) {
            Eject();
        }

        if (Input::KeyDown(GLFW_KEY_LEFT_SHIFT)) {
            // Scene manager
            if (Input::KeyPressed(GLFW_KEY_SPACE)) {
                activemenu = mainMenu.SubMenus[0];
                mainMenu.SelectedSubMenu = 0;
                sceneManager.SelectedSubMenu = SM::GetSelectedIndex();

                inMenu = !inMenu;
                Input::SetInputContext(Input::Menu);
            }
            // Asset Manager
            if (Input::KeyPressed(GLFW_KEY_A))
            {
                activemenu = &assetManager;
                mainMenu.SelectedSubMenu = 1;

                inMenu = !inMenu;
                Input::SetInputContext(Input::Menu);
            }
        }

        if (Input::KeyPressed(GLFW_KEY_PERIOD) && Input::GetInputContext() != Input::TextInput && Input::GetInputContext() != Input::Transforming) {
            Text::SetGlobalTextScaling(Text::GetGlobalTextScaling() + 0.2f);
            Resize(0, 0);
            RecalcMenuWidths(toplevelmenu);
        }
        if (Input::KeyPressed(GLFW_KEY_COMMA) && Input::GetInputContext() != Input::TextInput && Input::GetInputContext() != Input::Transforming) {
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
                if (Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1)) grabbingMenu = false;
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

            // DrawRectBlurred(Engine::GetWindowSize(), glm::ivec2(0));

            activemenu->Render();
        }
    }

    float blend = 0;
    float elapsed = 0;
    void Menu::Render()
    {
        Input(NumItems);

        rectShader->Use();
        rectShader->SetMatrix4("projection", AM::OrthoProjMat4);

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
            DrawList(0, yoffset, Items, *this, CenteredList);
        }
        else {
            std::vector<std::string> items;
            for (Menu* obj : SubMenus) items.push_back(obj->Title);
            NumItems = items.size();
            DrawList(0, yoffset, &items, *this, CenteredList);
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

        if (Input::KeyPressed(GLFW_KEY_ENTER))
        {
            if (SubMenus.size() > 0) activemenu = SubMenus[SelectedSubMenu];
            else if (!Items->empty() && GetActiveColumn() != GetNumColumns()) SelectedSubMenu = WrapIndex(SelectedSubMenu, Items->size() - 1);
        }

        x_timer += Stats::GetDeltaTime();
        if (x_timer >= 1 / (5.0f * mult) || Input::KeyPressed(GLFW_KEY_RIGHT) || Input::KeyPressed(GLFW_KEY_LEFT)) {
            x_timer = 0.0f;

            if (Input::KeyDown(GLFW_KEY_RIGHT) || Input::KeyPressed(GLFW_KEY_RIGHT))
            {
                if (!SubMenus.empty()) activemenu = SubMenus[SelectedSubMenu];
                else if (!Items->empty() && GetActiveColumn() != GetNumColumns()) SelectedSubMenu = WrapIndex(SelectedSubMenu, Items->size() - 1);
            }
            if ((Input::KeyDown(GLFW_KEY_LEFT) || Input::KeyPressed(GLFW_KEY_LEFT)) && !IsTopLevel)
            {
                if ((!Items) || (!Items->empty() && SelectedSubMenu < MaxItemsUntilWrap)) activemenu = parent;
                else if (Items && SelectedSubMenu >= MaxItemsUntilWrap) SelectedSubMenu = WrapIndex(SelectedSubMenu, Items->size(), true);
            }
        }

        y_timer += Stats::GetDeltaTime();
        if (y_timer >= 1 / (15.0f * mult)) {
            y_timer = 0.0f;

            if (Input::KeyDown(GLFW_KEY_UP))
            {
                if (!SubMenus.empty()) SelectedSubMenu = glm::clamp(SelectedSubMenu - 1, 0, NumSubMenus - 1);
                else if (Items) SelectedSubMenu = glm::clamp(SelectedSubMenu - 1, 0, NumSubMenus - 1);
                // && ((SelectedSubMenu) % MaxItemsUntilWrap) != 0
            }
            if (Input::KeyDown(GLFW_KEY_DOWN))
            {
                if (!SubMenus.empty()) SelectedSubMenu = glm::clamp(SelectedSubMenu + 1, 0, NumSubMenus - 1);
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
    }

    void Menu::AddSubMenu(Menu* menu)
    {
        menu->parent = this;
        SubMenus.push_back(menu);
    }

    void DrawList(int xoffset_, int yoffset, std::vector<std::string>* items, Menu &submenu, bool gradient)
    {
        int vis_below_above = 4;
        int xoffset = xoffset_ - (submenu.SelectedSubMenu / submenu.MaxItemsUntilWrap) * submenu.MenuWidth;

        // Frosted Background
        if (submenu.FrostedBG)
        {
            glm::vec2 bottomLeft = { left_edge, top_edge - titleHeight - itemHeightPX * items->size() + yoffset };
            glm::vec2 topRight   = { right_edge, top_edge + yoffset };

            DrawRect(topRight + (float)OutlineWidth, bottomLeft - (float)OutlineWidth, OutlineColor);
            DrawRectBlurred(topRight, bottomLeft, glm::vec3(0.6f));
        }

        if (submenu.HasHeaderBar)
        {
            float headerYOffset = submenu.HiearchyView ? glm::min(vis_below_above * itemHeightPX, yoffset) : yoffset;

            DrawRect(glm::vec2(left_edge, top_edge + headerYOffset),
                     submenu.MenuWidth, titleHeight, submenu.ThemeColor);
        }

        // Title
        {
            float titleYOffset = submenu.HiearchyView ? glm::min(vis_below_above * itemHeightPX, yoffset) : yoffset;

            Text::Render(submenu.Title,
                        centerX - Text::CalculateTextWidth(submenu.Title, TitleBarTextScaling) / 2,
                        top_edge - titleHeight + TitleBarPaddingYPX + titleYOffset,
                        TitleBarTextScaling,
                        glm::mix(glm::vec3(0.9), TitleBarTextColor, submenu.BGopacity));
        }

        // Items
        int maxVisibleColumns = 7;
        int selectedColumn = submenu.SelectedSubMenu / submenu.MaxItemsUntilWrap;
        int halfVisibleColumns = maxVisibleColumns / 2;
        int numColumns = items->size() / submenu.MaxItemsUntilWrap;

        // Calculate min and max visible columns based on selection
        int minVisibleColumn = selectedColumn - halfVisibleColumns;
        int maxVisibleColumn = selectedColumn + halfVisibleColumns;

        // Clamp to 0 to avoid negative indices
        minVisibleColumn = glm::max(0, minVisibleColumn);

        for (int i = 0; i < items->size(); i++)
        {
            int column = i / submenu.MaxItemsUntilWrap;
            int row    = i % submenu.MaxItemsUntilWrap;

            // Only draw if inside visible columns
            if ((column < minVisibleColumn || column > maxVisibleColumn) && numColumns >= maxVisibleColumns)
                continue;

            int xoff = column * submenu.MenuWidth + xoffset;
            int yoff = row;

            glm::vec2 itemPos = glm::vec2(left_edge + xoff, top_edge - titleHeight - itemHeightPX * yoff + yoffset);

            float itemShade = 0.0f;
            if (submenu.BGopacity > 0.0f)
            {
                if (!gradient) itemShade = (i == submenu.SelectedSubMenu) ? 1.0f : ItemShadePct;
                else if (i / submenu.MaxItemsUntilWrap == submenu.SelectedSubMenu / submenu.MaxItemsUntilWrap)
                {
                    float distance = (ItemDistanceShade - glm::abs(submenu.SelectedSubMenu - i)) / (float)ItemDistanceShade;
                    itemShade = glm::pow(glm::pow(distance, 3.0f), 1.0f / 2.2f);
                }

                if (submenu.HiearchyView)
                {
                    bool inRange = (i <= submenu.SelectedSubMenu + vis_below_above && i >= submenu.SelectedSubMenu - vis_below_above);
                    if (inRange)
                    {
                        DrawRect(itemPos, submenu.MenuWidth, itemHeightPX, ItemColor * itemShade, submenu.BGopacity);

                        if (i == submenu.SelectedSubMenu + vis_below_above && submenu.SelectedSubMenu >= vis_below_above)
                        {
                            Text::Render(std::to_string(items->size() - i) + "...",
                                        centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoffset,
                                        top_edge - titleHeight - itemHeightPX * (yoff + 1) + yoffset + ItemPaddingYPX,
                                        ItemTextScaling, ItemTextInactive);
                        }
                        else if (i == submenu.SelectedSubMenu - vis_below_above)
                        {
                            Text::Render(std::to_string(i + 1) + "...",
                                        centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoffset,
                                        top_edge - titleHeight - itemHeightPX * (yoff + 1) + yoffset + ItemPaddingYPX,
                                        ItemTextScaling, ItemTextInactive);
                        }
                    }
                }
                else DrawRect(itemPos, submenu.MenuWidth, itemHeightPX, ItemColor * itemShade, submenu.BGopacity);
            }

            glm::vec3 textColor = (i == submenu.SelectedSubMenu) ? ItemTextActive : ItemTextInactive;

            if (submenu.HiearchyView)
            {
                bool visible = (i <= submenu.SelectedSubMenu + vis_below_above && i > submenu.SelectedSubMenu - vis_below_above
                                && (i != submenu.SelectedSubMenu + vis_below_above || submenu.SelectedSubMenu < vis_below_above));
                if (visible)
                {
                    Text::Render((*items)[i],
                                centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoff,
                                top_edge - titleHeight - itemHeightPX * (yoff + 1) + yoffset + ItemPaddingYPX,
                                ItemTextScaling, textColor);
                }
            }
            else
            {
                Text::Render((*items)[i],
                            centerX - submenu.MenuWidth / 2 + ItemPaddingXPX + xoff,
                            top_edge - titleHeight - itemHeightPX * (yoff + 1) + yoffset + ItemPaddingYPX,
                            ItemTextScaling, textColor);
            }
        }

        // Cursor
        {
            int cursorx = (submenu.SelectedSubMenu / submenu.MaxItemsUntilWrap) * submenu.MenuWidth;
            int cursory = submenu.SelectedSubMenu % submenu.MaxItemsUntilWrap;

            DrawRect(glm::vec2(right_edge - ItemSelectionThickness,
                     top_edge - titleHeight - itemHeightPX * cursory + yoffset),
                     ItemSelectionThickness, itemHeightPX, submenu.ThemeColor);
        }
    }

    void DrawQuad(glm::ivec2 Center, int radius, glm::vec3 Color, float Opacity)
    {
        vertices = 
        {
            Center.x + radius, Center.y + radius,
            Center.x - radius, Center.y + radius,
            Center.x - radius, Center.y - radius,
            Center.x + radius, Center.y - radius,
        };
        rectShader->Use();
        rectShader->SetVector3("color", Color);
        rectShader->SetFloat("blend", Opacity);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
        rectShader->Use();
        rectShader->SetVector3("color", Color);
        rectShader->SetFloat("blend", Opacity);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void DrawRectGradient(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 Color1, glm::vec3 Color2)
    {
        vertices = 
        {
            TopRight.x,   TopRight.y,
            BottomLeft.x, TopRight.y,
            BottomLeft.x, BottomLeft.y,
            TopRight.x,   BottomLeft.y,
        };
        gradientRectShader->Use();
        gradientRectShader->SetVector3("color1", Color1);
        gradientRectShader->SetVector3("color2", Color2);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void DrawRectBlurred(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 Tint)
    {
        vertices = 
        {
            TopRight.x,   TopRight.y,
            BottomLeft.x, TopRight.y,
            BottomLeft.x, BottomLeft.y,
            TopRight.x,   BottomLeft.y,
        };

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Deferred::GBuffers[Deferred::GShaded]);
        frostedRectShader->Use();
        frostedRectShader->SetInt("sceneTexture", 0);
        frostedRectShader->SetVector2("textureSize", { Engine::GetWindowSize().x, Engine::GetWindowSize().y });
        frostedRectShader->SetVector3("tint", Tint);

        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    void DrawCircle(glm::ivec2 Center, glm::vec3 Color, int OuterRadius, int InnerRadius, float Opacity)
    {
        vertices = 
        {
            Center.x + OuterRadius, Center.y + OuterRadius,
            Center.x - OuterRadius, Center.y + OuterRadius,
            Center.x - OuterRadius, Center.y - OuterRadius,
            Center.x + OuterRadius, Center.y - OuterRadius,
        };
        circleShader->Use();
        circleShader->SetVector3("color", Color);
        circleShader->SetFloat("outerRadiusPX", OuterRadius);
        circleShader->SetFloat("innerRadiusPX", InnerRadius);
        circleShader->SetFloat("blend",   Opacity);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
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
        rectShader->Use();
        rectShader->SetVector3("color", Color);
        rectShader->SetFloat("blend", Opacity);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
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

    bool isRotating = false;
    void DrawColorWheel(glm::ivec2 Center, glm::vec3 StartColor, glm::vec3& OutColor, int OuterRadius, int InnerRadius)
    {
        DrawCircle(Center, glm::vec3(0.1f), OuterRadius + 2, InnerRadius - 2);

        vertices = 
        {
            Center.x + OuterRadius, Center.y + OuterRadius,
            Center.x - OuterRadius, Center.y + OuterRadius,
            Center.x - OuterRadius, Center.y - OuterRadius,
            Center.x + OuterRadius, Center.y - OuterRadius,
        };
        colorWheelShader->Use();
        colorWheelShader->SetFloat("outerRadiusPX", OuterRadius);
        colorWheelShader->SetFloat("innerRadiusPX", InnerRadius);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        float diff   = OuterRadius - InnerRadius;
        float middle = (OuterRadius + InnerRadius) / 2.0f;

        glm::vec2 mouse_pos = glm::vec2(Input::GetMouseX(), Engine::GetWindowSize().y - Input::GetMouseY());

        glm::vec3 hsv = qk::RGBToHSV(StartColor);

        float angle = glm::radians(hsv.x);
        float distance = glm::distance(mouse_pos, { Center.x, Center.y });
        if ((((Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1) && distance < OuterRadius && distance > InnerRadius)) || Input::KeyDown(GLFW_KEY_R) || isRotating) && slidingSliderID == -1)
        {
            isRotating = true;
            glm::vec2 neutral = mouse_pos - glm::vec2(Center.x, Center.y);
            angle = -std::atan2f(neutral.x, neutral.y) + std::numbers::pi / 2.0f;
        }
        else {
            isRotating = false;
        }
        
        if (isRotating) {
            if (!Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1)) {
                isRotating = false;
            }
        }
        
        float angle_deg = glm::degrees(angle);
        if (angle_deg < 0) angle_deg += 360.0f;

        glm::ivec2 pos = Center;
        pos.x += cos(angle) * middle;
        pos.y += sin(angle) * middle;
        
        UI::DrawCircle(pos, glm::vec3(0.95f), diff / 2 - 2);
        UI::DrawCircle(pos, qk::HSVToRGB({ angle_deg, 1.0f, 1.0f }), diff / 3 - 2);
        
        OutColor = qk::HSVToRGB({ angle_deg, 1.0f, 1.0f });
    }

    void DrawSlider(glm::ivec2 TopRight, glm::ivec2 BottomLeft, float StartValue, float& OutValue, int HotKey, bool Horizontal, bool Gradient, glm::vec3 Color1, glm::vec3 Color2)
    {
        int id = currentSliderID++; // grab and increment ID automatically

        if (Gradient) {
            UI::DrawRect(TopRight + 2, BottomLeft - 2, glm::vec3(0.085f));
            UI::DrawRectGradient(TopRight, BottomLeft, Color1, Color2);
        }
        else {
            UI::DrawRect(TopRight + 2, BottomLeft - 2, glm::vec3(0.085f));
            UI::DrawRect(TopRight, BottomLeft, glm::vec3(1.0f));
        }

        OutValue = StartValue;

        if (Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1) && isRectHovered(BottomLeft, TopRight) && slidingSliderID == -1 && !isRotating) {
            slidingSliderID = id;
        }

        if (Input::KeyDown(HotKey) && Input::GetInputContext() != Input::TextInput) {
            slidingSliderID = id;
            Input::ResetKeyPress(HotKey);
        }

        if (!Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1) && slidingSliderID == id && !Input::KeyDown(HotKey)) {
            slidingSliderID = -1;
        }

        if (slidingSliderID == id) {
            float mouseDelta   = Horizontal ? Input::GetMouseDeltaX()     : Input::GetMouseDeltaY();
            float sliderLength = Horizontal ? (TopRight.x - BottomLeft.x) : (TopRight.y - BottomLeft.y);

            if (sliderLength != 0.0f) {
                float deltaValue = mouseDelta / sliderLength;
                OutValue += deltaValue;
            }
        
            OutValue = glm::clamp(OutValue, 0.0f, 1.0f);
        }

        float handlePCTG;
        if (Horizontal) handlePCTG = glm::mix(BottomLeft.x, TopRight.x, OutValue);
        else            handlePCTG = glm::mix(BottomLeft.y, TopRight.y, OutValue);

        int handleWidth  = 12;
        int handleHeight = 6;

        glm::ivec2 handleTopRight;
        glm::ivec2 handleBottomLeft;

        if (Horizontal) {
            handleTopRight   = glm::ivec2(handlePCTG + handleHeight / 2, TopRight.y   + handleWidth / 2);
            handleBottomLeft = glm::ivec2(handlePCTG - handleHeight / 2, BottomLeft.y - handleWidth / 2);
        }
        else {
            handleTopRight   = glm::ivec2(TopRight.x   + handleWidth / 2, handlePCTG + handleHeight / 2);
            handleBottomLeft = glm::ivec2(BottomLeft.x - handleWidth / 2, handlePCTG - handleHeight / 2);
        }

        UI::DrawRect(handleTopRight + glm::ivec2(2), handleBottomLeft - glm::ivec2(2), glm::vec3(0.085f));
        UI::DrawRect(handleTopRight,                 handleBottomLeft,                 glm::vec3(1.0f));
    }

    void DrawInputBox(glm::ivec2 TopRight, glm::ivec2 BottomLeft, std::string& OutText, int HotKey, bool OnlyNumbers, bool OnlyIntegers)
    {
        // Generate a unique ID based on the address of the input text
        uintptr_t id = reinterpret_cast<uintptr_t>(&OutText);

        // Handle clicking to focus
        if (activeInputBoxID == -1 && (Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1) && isRectHovered(BottomLeft, TopRight)) && slidingSliderID == -1 && !isRotating)
        {
            OutText.clear();
            activeInputBoxID = id;
            Input::EnableTextInput();
            Input::SetInputContext(Input::TextInput);
        }

        // Handle hotkey focus
        if (Input::KeyPressed(HotKey) && activeInputBoxID == -1)
        {
            OutText.clear();
            activeInputBoxID = id;
            Input::EnableTextInput();
            Input::SetInputContext(Input::TextInput);
        }

        // If focused, capture input
        if (activeInputBoxID == id)
        {
            OutText += Input::GetTypedCharacters(OnlyNumbers, OnlyIntegers);

            if (Input::KeyPressed(GLFW_KEY_BACKSPACE) && !OutText.empty())
            {
                OutText.pop_back(); // Handle backspace
            }

            if (Input::KeyPressed(GLFW_KEY_ENTER) || Input::KeyPressed(GLFW_KEY_ESCAPE) ||
                (Input::MouseButtonDown(GLFW_MOUSE_BUTTON_1) && !isRectHovered(BottomLeft, TopRight)))
            {
                if (OnlyNumbers && !OnlyIntegers)
                {
                    if (OutText.size() == 0) OutText = "0.000";
                    else
                    {
                        size_t dotPos = OutText.find('.');
                        if (dotPos == std::string::npos) OutText += ".000";
                        else
                        {
                            size_t decimalCount = OutText.size() - dotPos - 1;
                            if (decimalCount < 3) OutText.append(3 - decimalCount, '0');
                        }
                    }
                }

                activeInputBoxID = -1; // Finish editing
                Input::SetInputContext(Input::GetPreviousInputContext());
                Input::DisableTextInput();
            }

            UI::DrawRect(TopRight + 2, BottomLeft - 2, glm::vec3(0.85f));
            UI::DrawRect(TopRight, BottomLeft, glm::vec3(0.15f));

            Text::RenderCentered(OutText, (BottomLeft.x + TopRight.x) / 2, BottomLeft.y + 2, 0.5f, glm::vec3(0.95f));

            // Now draw the cursor at the end of the text
            float textWidth = Text::CalculateTextWidth(OutText, 0.5f); // Get the width of the current text
            float cursorX = (BottomLeft.x + TopRight.x) / 2 + textWidth / 2; // Position of the cursor (after text)

            // Draw the cursor (just a thin vertical line)
            glm::ivec2 cursorTopLeft(cursorX, BottomLeft.y + 2); // Adjust the Y position to align with the text
            glm::ivec2 cursorBottomRight(cursorX + 2, TopRight.y - 2); // Width of the cursor (a thin line)
            UI::DrawRect(cursorTopLeft, cursorBottomRight, glm::vec3(0.95f)); // Draw cursor
        }
        else
        {
            UI::DrawRect(TopRight + 2, BottomLeft - 2, glm::vec3(0.085f));
            UI::DrawRect(TopRight, BottomLeft, glm::vec3(0.125f));

            Text::RenderCentered(OutText, (BottomLeft.x + TopRight.x) / 2, BottomLeft.y + 2, 0.5f, glm::vec3(0.8f));
        }
    }

    void DrawArrow(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 color)
    {
        vertices = 
        {
            BottomLeft.x, TopRight.y - ItemSelectionThickness,
            BottomLeft.x, BottomLeft.y + ItemSelectionThickness,
            BottomLeft.x + ItemSelectionThickness * 2, (TopRight.y + BottomLeft.y) / 2
        };
        rectShader->Use();
        rectShader->SetVector3("color", color);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
        glBindVertexArray(rectVAO);
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
    
    void MainMenuInput()
    {
        for (int key = GLFW_KEY_1; key <= GLFW_KEY_6; ++key) {
            if (Input::KeyPressed(key))
            {
                int submenuIndex = key - GLFW_KEY_1;
                mainMenu.SelectedSubMenu = submenuIndex;
                // activemenu = mainMenu.SubMenus[submenuIndex];
            }
        }
    }

    void MeshesInput()
    {
        if (Input::KeyPressed(GLFW_KEY_ENTER))
        {
            std::string name = AM::MeshNames[activemenu->SelectedSubMenu];
            SM::Object* obj = new SM::Object(name, name);
            SM::AddNode(obj);
            SM::SelectSceneNode(SM::SceneNodes.size() - 1);
            if (!Input::KeyDown(GLFW_KEY_LEFT_SHIFT)) Eject();
        }

        if (Input::KeyPressed(GLFW_KEY_0))
        {
            const char *filters[] = { "*.obj" };
            char* loadPath = tinyfd_openFileDialog(
                "Load OBJ",
                Stats::ProjectPath.c_str(),
                1, filters, "OBJ files", 0
            );
            if (loadPath)
            {
                std::string loadPathS = loadPath;
                std::string name      = qk::GetFileName(loadPathS, true);

                if (qk::StringEndsWith(loadPathS, ".obj"))
                {
                    AM::IO::LoadObjAsync(loadPathS, name);
                    activemenu = &meshes;
                }
            }
        }

        if (Input::KeyPressed(GLFW_KEY_9))
        {
            // pop up a folder chooser
            char* folderPath = tinyfd_selectFolderDialog(
                "Select OBJ Folder",
                Stats::ProjectPath.c_str()
            );
            if (folderPath)
            {
                std::string folderS = folderPath;
                // use the folder name as mesh‐group prefix
                std::string prefix  = qk::GetFileName(folderS, true);

                // asynchronously load every .obj in that folder
                AM::IO::LoadObjFolderAsync(folderS, prefix);
                activemenu = &meshes;
            }
        }

    }

    void LightsInput()
    {
        if (Input::KeyPressed(GLFW_KEY_ENTER))
        {
            std::string name = AM::LightNames[activemenu->SelectedSubMenu];
            SM::Light* light = new SM::Light("Point Light", SM::LightType::Point, glm::vec3(0), 10.0f, glm::vec3(1), 5.0f);
            SM::AddNode(light);
            SM::SelectSceneNode(SM::SceneNodes.size() - 1);
            if (!Input::KeyDown(GLFW_KEY_LEFT_SHIFT)) Eject();
        }
    }

    void AssetManagerInput()
    {
        for (int key = GLFW_KEY_1; key <= GLFW_KEY_4; ++key) {
            if (Input::KeyPressed(key))
            {
                int submenuIndex = key - GLFW_KEY_1;
                assetManager.SelectedSubMenu = submenuIndex;
                activemenu = assetManager.SubMenus[submenuIndex];
            }
        }
    }

    void SceneManagerInput()
    {
        SM::SelectSceneNode(activemenu->SelectedSubMenu);
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

    void DebugInput()
    {
        if (Input::KeyPressed(GLFW_KEY_ENTER))
            Engine::debugMode = static_cast<Engine::DebugMode>(debug.SelectedSubMenu + 1);

        for (int key = GLFW_KEY_1; key <= GLFW_KEY_5; ++key) {
            if (Input::KeyPressed(key))
            {
                int submenuIndex = key - GLFW_KEY_1;
                debug.SelectedSubMenu = submenuIndex;
                Engine::debugMode = static_cast<Engine::DebugMode>(submenuIndex + 1);
            }
        }
    }

    void Initialize()
    {
        Engine::RegisterResizeCallback(Resize);
        Engine::RegisterEditorReloadShadersFunction(ReloadShaders);

        rectShader         = std::make_unique<Shader>("/res/shaders/ui/rect");
        gradientRectShader = std::make_unique<Shader>("/res/shaders/ui/gradientrect");
        frostedRectShader  = std::make_unique<Shader>("/res/shaders/ui/frostedrect");
        circleShader       = std::make_unique<Shader>("/res/shaders/ui/circle");
        defocusShader      = std::make_unique<Shader>("/res/shaders/ui/defocus");
        colorWheelShader   = std::make_unique<Shader>("/res/shaders/ui/colorwheel");

        glGenVertexArrays(1, &rectVAO);
        glGenBuffers(1, &rectVBO);

        glBindVertexArray(rectVAO);
        glBindBuffer(GL_ARRAY_BUFFER, rectVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(int) * 2 * 6, NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_INT, GL_FALSE, sizeof(int) * 2, 0);

        // Dummy VAO for fullscreen quad vert shader
        glGenVertexArrays(1, &defocusVAO);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        mainMenu.Initialize("hotbox", MainMenuColor);
        mainMenu.IsTopLevel   = true;
        mainMenu.BGopacity    = 0.0f;
        mainMenu.FrostedBG    = true;
        mainMenu.HasHeaderBar = false;
        mainMenu.ExtraInput   = MainMenuInput;
        {
            sceneManager.Initialize("Scene Manager", SecondaryColor, &SM::SceneNodeNames);
            sceneManager.CenteredList = true;
            sceneManager.ExtraInput = SceneManagerInput;

            assetManager.Initialize("Asset Manager", TertieryColor);
            assetManager.ExtraInput = AssetManagerInput;
            {
                meshes.Initialize("Meshes    (1)", TertieryColor, &AM::MeshNames);
                meshes.ExtraInput = MeshesInput;

                lights.Initialize("Lights    (2)", TertieryColor, &AM::LightNames);
                lights.ExtraInput = LightsInput;

                materials.Initialize("Materials (3)", TertieryColor);

                textures.Initialize("Textures  (4)", TertieryColor);
            }
            assetManager.AddSubMenu(&meshes);
            assetManager.AddSubMenu(&lights);
            assetManager.AddSubMenu(&materials);
            assetManager.AddSubMenu(&textures);
            assetManager.BGopacity    = 0.0f;
            assetManager.FrostedBG    = true;
            assetManager.HasHeaderBar = false;
        
            statistics.Initialize("Statistics", SecondaryColor);
            engine.Initialize("Engine",         SecondaryColor);
            qk.Initialize("Qk",                 SecondaryColor);

            debug.Initialize("Debug", SecondaryColor, &MN_DebugItems);
            debug.ExtraInput = DebugInput;   
        }
        
        mainMenu.AddSubMenu(&sceneManager);
        mainMenu.AddSubMenu(&assetManager);
        mainMenu.AddSubMenu(&statistics);
        mainMenu.AddSubMenu(&engine);
        mainMenu.AddSubMenu(&debug);
        mainMenu.AddSubMenu(&qk);

        activemenu   = &mainMenu;
        toplevelmenu = &mainMenu;
    }

    void Resize(int width, int height)
    {
        titleHeight = Text::CalculateMaxTextAscent("A", TitleBarTextScaling) + TitleBarPaddingYPX * 2;
        itemHeightPX = Text::CalculateMaxTextAscent("A", ItemTextScaling) + ItemPaddingYPX * 2;
        centerX = Engine::GetWindowSize().x / 2.0f;
        centerY = Engine::GetWindowSize().y / 2.0f;
        
        rectShader->Use();
        rectShader->SetMatrix4("projection", AM::OrthoProjMat4);
        gradientRectShader->Use();
        gradientRectShader->SetMatrix4("projection", AM::OrthoProjMat4);
        frostedRectShader->Use();
        frostedRectShader->SetMatrix4("projection", AM::OrthoProjMat4);
        circleShader->Use();
        circleShader->SetMatrix4("projection", AM::OrthoProjMat4);
        colorWheelShader->Use();
        colorWheelShader->SetMatrix4("projection", AM::OrthoProjMat4);
    }

    void ReloadShaders()
    {
        rectShader->Reload();
        gradientRectShader->Reload();
        frostedRectShader->Reload();
        circleShader->Reload();
        colorWheelShader->Reload();
        Resize(Engine::GetWindowSize().x, Engine::GetWindowSize().y);
    }
}