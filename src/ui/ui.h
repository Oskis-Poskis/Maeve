#pragma once

#include <string>
#include <vector>
#include <functional>

namespace UI
{
    void Initialize();
    void Render();
    void Eject();

    void DrawSlider(glm::ivec2 TopRight, glm::ivec2 BottomLeft, float StartValue, float& OutValue, int HotKey = 84, bool Horizontal = false, bool Gradient = false, glm::vec3 Color1 = glm::vec3(1.0f), glm::vec3 Color2 = glm::vec3(0.0f));
    void DrawColorWheel(glm::ivec2 Center, glm::vec3 StartColor, glm::vec3& OutColor, int OuterRadius, int InnerRadius);
    void DrawInputBox(glm::ivec2 TopRight, glm::ivec2 BottomLeft, std::string& OutText, int HotKey = 51, bool OnlyNumbers = false, bool OnlyIntegers = false);
    
    void DrawCircle(glm::ivec2 Center, glm::vec3 Color, int OuterRadius, int InnerRadius = 0, float Opacity = 1.0f);
    void DrawQuad(glm::ivec2 Center, int Radius, glm::vec3 Color, float Opacity = 1.0f);
    void DrawRect(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 Color, float Opacity = 1.0f);
    void DrawRect(glm::ivec2 TopLeft, int width, int height, glm::vec3 Color, float Opacity = 1.0f);
    void DrawRectGradient(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 Color1, glm::vec3 Color2);
    void DrawRectBlurred(glm::ivec2 TopRight, glm::ivec2 BottomLeft, glm::vec3 Tint = glm::vec3(1.0f));
    bool isRectHovered(glm::ivec2 BottomLeft, glm::ivec2 TopRight);

    class Menu
    {
        public:
            Menu() : Items(nullptr) {}
            Menu(std::string MenuTitle, glm::vec3 ThemeCol) : Items(nullptr) { Initialize(MenuTitle, ThemeCol); };
            Menu(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* items) : Items(items) { Initialize(MenuTitle, ThemeCol, items); };

            std::string Title;
            int SelectedSubMenu = 0;
            int NumItems        = 0;
            bool IsTopLevel     = false;
            bool HasItems       = false;
            std::vector<Menu*> SubMenus;
            std::vector<std::string>* Items = nullptr;
            std::function<void()> ExtraInput;
            Menu* parent;

            glm::vec3 ThemeColor;
            float BGopacity = 1.0f;
            int  MenuWidth;
            int  MaxItemsUntilWrap = 24;
            bool FrostedBG    = false;
            bool HiearchyView = false;
            bool HasHeaderBar = true;
            bool CenteredList = false;

            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol);
            void Initialize(std::string MenuTitle, glm::vec3 ThemeCol, std::vector<std::string>* Items);

            void Render();
            void AddSubMenu(Menu* menu);
            int  GetNumColumns();
            int  GetActiveColumn();

        private:
            void Input(int NumSubMenus);
            int WrapIndex(int index, int size, bool negate = false);
    };
}