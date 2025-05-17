#include <vector>
#include <functional>

#include <glm/glm.hpp>

#pragma once

namespace BUI
{
    void Initialize();
    void DrawTest();

    struct Grid_Item {
        glm::ivec2 maxExtent;
        glm::ivec2 minExtent;
        int        padding;
        bool contributeExtents = true;

        void* data = nullptr;
        std::function<void(glm::ivec2 topRight, glm::ivec2 bottomLeft, void* data)> drawFunc = nullptr;

        Grid_Item() = default;
        Grid_Item(glm::ivec2 MaxExtent, int padding) : maxExtent(MaxExtent), padding(padding) {}

        void Draw(glm::ivec2 TopRight, glm::ivec2 BottomLeft) const {
            if (drawFunc && data) drawFunc(TopRight, BottomLeft, data);
        }
    };

    struct UI_Grid {
        glm::ivec2                           Position;

        std::vector<std::vector<Grid_Item>>  Items;
        std::vector<std::vector<glm::ivec2>> InternalExtents;
        glm::ivec2                           ExteriorExtents;

        void UpdateExtents();
        void Draw();
    };
}