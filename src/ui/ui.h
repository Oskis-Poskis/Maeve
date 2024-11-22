#pragma once

#include <vector>

namespace UI
{
    void Initialize();
    void Render();

    void DrawRect(glm::vec2 TopRight, glm::vec2 BottomLeft, glm::vec3 Color, float Opacity = 1.0f);
    void DrawRect(glm::vec2 TopLeft, int width, int height, glm::vec3 Color, float Opacity = 1.0f);

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
            int MaxItemsUntilWrap = 24;
            float BGopacity       = 1.0f;
            bool HasHeaderBar     = true;
            bool CenteredList     = false;

            void Render();
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol);
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* Items);
            void AddSubMenu(Menu &menu);
            int GetNumColumns();
            int GetActiveColumn();

            Menu() : Items(nullptr) { }
            Menu(std::string MenuTitle, glm::vec3 ThemeCol) : Items(nullptr) { Initialize(MenuTitle, ThemeCol); }
            Menu(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* Items) : Items(Items) { Initialize(MenuTitle, ThemeCol, Items); }

        private:
            void Input(int NumSubMenus);
            int WrapIndex(int index, int size, bool negate = false);
    };
}