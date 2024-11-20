#pragma once

#include <vector>

namespace UI
{
    void Initialize();
    void Render();

    void DrawRect(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 Color, float Opacity = 1.0f);

    class Menu
    {
        public:
            std::string Title;
            int SelectedSubMenu = 0;
            int NumItems        = 0;
            bool IsTopLevel     = false;
            bool HasItems       = false;
            bool FadingAway     = false;
            std::vector<Menu> SubMenus;
            std::vector<std::string>* Items;
            Menu* parent;

            glm::vec3 ThemeColor;
            int MenuWidth;
            float BGopacity   = 1.0f;
            bool HasHeaderBar = true;
            bool CenteredList = false;

            void Render();
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol);
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* Items);
            void Input(int NumSubMenus);
            void AddSubMenu(Menu &menu);

            Menu() : Items(nullptr) { }
            Menu(std::string MenuTitle, glm::vec3 ThemeCol) : Items(nullptr) { Initialize(MenuTitle, ThemeCol); }
            Menu(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* Items) : Items(Items) { Initialize(MenuTitle, ThemeCol, Items); }
    };
}