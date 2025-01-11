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
            Menu() : Items(nullptr) { }
            Menu(std::string MenuTitle, glm::vec3 ThemeCol) : Items(nullptr) { Initialize(MenuTitle, ThemeCol); }
            Menu(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* Items) : Items(Items) { Initialize(MenuTitle, ThemeCol, Items); }

            std::string Title;
            int SelectedSubMenu = 0;
            int NumItems        = 0;
            bool IsTopLevel     = false;
            bool HasItems       = false;
            std::vector<Menu> SubMenus;
            std::vector<std::string>* Items;
            std::function<void()> ExtraInput;
            Menu* parent;

            glm::vec3 ThemeColor;
            float BGopacity = 1.0f;
            int  MenuWidth;
            int  MaxItemsUntilWrap = 24;
            bool HiearchyView = false;
            bool HasHeaderBar = true;
            bool CenteredList = false;

            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol);
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* Items);

            void Render();
            void AddSubMenu(Menu &menu);
            int  GetNumColumns();
            int  GetActiveColumn();

        private:
            void Input(int NumSubMenus);
            int WrapIndex(int index, int size, bool negate = false);
    };
}