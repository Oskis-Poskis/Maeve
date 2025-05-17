#include "box_ui.h"
#include "ui.h"
#include "../engine/render_engine.h"

#include <vector>
#include <iostream>
#include <glm/glm.hpp>

namespace BUI
{
    float myValue1 = 0.5f;
    float myValue2 = 0.5f;
    float myValue3 = 0.5f;
    std::string myString = "15.000";
    glm::vec3 myColor1({ 1.0f, 0.0f, 0.0f });
    glm::vec3 myColor2({ 1.0f, 1.0f, 0.0f });
    Grid_Item it1(glm::ivec2(125, 125), 10);
    Grid_Item it2(glm::ivec2( 20, 125), 10);
    Grid_Item it3(glm::ivec2( 20, 125), 10);
    Grid_Item it4(glm::ivec2(125,  20), 10);
    Grid_Item it5(glm::ivec2(125,  20), 10);
    Grid_Item it6(glm::ivec2(125, 125), 10);

    UI_Grid Grid;

    void Initialize()
    {
        it1.data     = &myColor1;
        it1.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data)  {
            glm::ivec2 center = (tr + bl) / 2;
            float      radius = std::abs(tr.x - bl.x) / 2.0f;
            UI::DrawColorWheel(center, myColor1, *static_cast<glm::vec3*>(data), radius, radius - 25);
        };

        it2.data     = &myValue1;
        it2.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data) {
            UI::DrawSlider(tr, bl, *static_cast<float*>(data));
        };

        it3.data     = &myValue2;
        it3.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data) {
            UI::DrawSlider(tr, bl, *static_cast<float*>(data));
        };

        it4.data     = &myString;
        it4.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data) {
            UI::DrawInputBox(tr, bl, *static_cast<std::string*>(data), 51, true);
        };

        it5.data     = &myValue3;
        it5.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data) {
            UI::DrawSlider(tr, bl, *static_cast<float*>(data), 84, true);
        };

        // it6.contributeExtents = false;
        // it6.data     = &myColor2;
        // it6.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data)  {
        //     glm::ivec2 center = (tr + bl) / 2;
        //     float      radius = std::abs(tr.x - bl.x) / 2.0f;
        //     UI::DrawColorWheel(center, myColor2, *static_cast<glm::vec3*>(data), radius, radius - 25);
        // };

        Grid.Items = {
            { it1, it2, it3 },
            { it4,          },
            { it5,          },
        };

        Grid.UpdateExtents();
    }

    void DrawTest()
    {
        Grid.Draw();
    }

    void UI_Grid::UpdateExtents()
    {
        InternalExtents.clear();
        InternalExtents.resize(Items.size());

        size_t max_cols = 0;
        for (const auto& row : Items)
            max_cols = std::max(max_cols, row.size());

        for (auto& row : InternalExtents)
            row.resize(max_cols, glm::ivec2(0));

        // First pass: calculate max width per col and max height per row
        std::vector<int> max_col_widths(max_cols, 0);
        std::vector<int> max_row_heights(Items.size(), 0);

        for (size_t row = 0; row < Items.size(); ++row) {
            for (size_t col = 0; col < Items[row].size(); ++col) {
                const Grid_Item& item = Items[row][col];
                int full_width  = item.maxExtent.x + item.padding * 2;
                int full_height = item.maxExtent.y + item.padding * 2;
                
                if (item.contributeExtents) {
                    max_col_widths[col]  = std::max(max_col_widths[col],  full_width);
                    max_row_heights[row] = std::max(max_row_heights[row], full_height);
                }
            }
        }

        ExteriorExtents = { 0, 0 };
        for (int w : max_col_widths)
            ExteriorExtents.x += w;

        for (int h : max_row_heights)
            ExteriorExtents.y += h;

        // Second pass: fill InternalExtents
        for (size_t row = 0; row < Items.size(); ++row) {
            for (size_t col = 0; col < Items[row].size(); ++col) {
                InternalExtents[row][col] = glm::ivec2(
                    max_col_widths[col],
                    max_row_heights[row]
                );
            }
        }
    }

    float value = 0.0f;
    void UI_Grid::Draw()
    {
        glm::ivec2 center = Engine::GetWindowSize() / 2;

        // UI::DrawRect(center + glm::ivec2(-4, 4),
        //             ExteriorExtents.x + 8,
        //             ExteriorExtents.y + 8,
        //             glm::vec3(0.2f));

        int y_offset = 0;
        for (size_t row = 0; row < Items.size(); ++row) {
            int x_offset = 0;

            for (size_t col = 0; col < Items[row].size(); ++col) {
                const Grid_Item& item = Items[row][col];
                const glm::ivec2& cell_extent = InternalExtents[row][col];

                // // Grid internal extents
                // UI::DrawRect(center + glm::ivec2(x_offset, y_offset),
                //             cell_extent.x,
                //             cell_extent.y,
                //             { (col + 1.0f) / Items[row].size(), (row + 1.0f) / Items.size(), 0.5f });

                // // Item inside grid cell
                // UI::DrawRect(center + glm::ivec2(x_offset + item.padding, y_offset - item.padding),
                //             item.maxExtent.x,
                //             item.maxExtent.y,
                //             glm::vec3(0.2f));

                item.Draw(center + glm::ivec2(x_offset + item.padding + item.maxExtent.x, y_offset - item.padding),
                          center + glm::ivec2(x_offset + item.padding, y_offset - item.padding - item.maxExtent.y));

                // UI::DrawSlider(center + glm::ivec2(x_offset + item.maxExtent.x + item.padding,  y_offset - item.padding),    
                //                center + glm::ivec2(x_offset + item.padding,                    -item.maxExtent.y + y_offset - item.padding), value);

                x_offset += cell_extent.x;
            }

            y_offset -= InternalExtents[row][0].y;
        }
    }
}