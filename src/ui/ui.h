#pragma once

namespace UI
{
    void Initialize();
    void Render();
    void WindowResize();

    class Menu
    {
        public:
            std::string Title;
            glm::vec3 ThemeColor;
            int MenuWidth;
            int ActiveSubMenu = 0;

            void Render();
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol);
        
        private:
            void Input(int NumSubMenus);
    };
}