#pragma once

namespace UI
{
    void Initialize();
    void Render();

    void DrawRect(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 color);

    class Menu
    {
        public:
            std::string Title;
            glm::vec3 ThemeColor;
            int MenuWidth;
            int ActiveSubMenu = 0;

            void Render();
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol);
            void Input(int NumSubMenus);
    };
}