#include <iostream>
#include <iomanip>
#include <sstream>  

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "light_manipulation.h"
#include "../render_engine.h"
#include "../scene_manager.h"
#include "../../common/qk.h"
#include "../../common/input.h"
#include "../../ui/ui.h"
#include "../../ui/text_renderer.h"

namespace  LightManipulation
{
    bool colorGizmoOpen = false;
    std::string InputText = "";
    void DrawLightColorGizmo()
    {
        if (Input::KeyPressed(GLFW_KEY_C) && Input::GetInputContext() != Input::TextInput)
        {
            if (Input::GetInputContext() == Input::Menu || 
                Input::GetInputContext() == Input::Transforming || 
                SM::SceneNodes.empty()) return;

            SM::SceneNode* node = SM::SceneNodes[SM::GetSelectedIndex()];
            if (node->GetType() != SM::NodeType::Light_) return;
            SM::Light* light = SM::GetLightFromNode(node);

            colorGizmoOpen = !colorGizmoOpen;
            Input::SetInputContext(colorGizmoOpen ? Input::PopupMenu : Input::Game);

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(3) << light->GetIntensity();
            InputText = oss.str();
        }
        if (Input::KeyPressed(GLFW_KEY_G) && Input::GetInputContext() != Input::TextInput && Input::GetInputContext() != Input::Menu) {
            colorGizmoOpen = false;
            Input::SetInputContext(Input::Transforming);
        }

        if (colorGizmoOpen)
        {
            if (Input::GetInputContext() != Input::PopupMenu && Input::GetInputContext() != Input::TextInput)
            {
                colorGizmoOpen = false;
                return;
            }

            SM::SceneNode* node = SM::SceneNodes[SM::GetSelectedIndex()];
            SM::Light* light = SM::GetLightFromNode(node);

            float InnerRadius = 55.0f;
            float OuterRadius = 75.0f;
            int slider_width = 20;
            glm::vec2 screen_pos = qk::WorldToScreen(light->GetPosition());
            glm::ivec2 slider_center = screen_pos + glm::vec2(OuterRadius + slider_width / 2 + 20, 0);

            glm::ivec2 topRight = slider_center + glm::ivec2(slider_width / 2, OuterRadius);
            glm::ivec2 bottomLeft = slider_center - glm::ivec2(slider_width / 2, OuterRadius);

            glm::vec3 originalColor = light->GetColor();
            glm::vec3 originalHSV = qk::RGBToHSV(originalColor);

            float newSaturation = originalHSV.y;
            float newValue = originalHSV.z;
            glm::vec3 selectedColor = originalColor;

            UI::DrawColorWheel(screen_pos, originalColor, selectedColor, OuterRadius, InnerRadius);

            UI::DrawSlider(topRight, bottomLeft, newSaturation, GLFW_KEY_1, true, glm::vec3(1.0f), qk::HSVToRGB({originalHSV.x, 1.0f, 1.0f}));

            glm::ivec2 offset(40, 0);
            UI::DrawSlider(topRight + offset, bottomLeft + offset, newValue, GLFW_KEY_2, true, glm::vec3(0.0f), glm::vec3(1.0f));

            glm::vec3 newHue = qk::RGBToHSV(selectedColor);
            glm::vec3 newColor = qk::HSVToRGB(glm::vec3(newHue.x, newSaturation, newValue));

            glm::ivec2 slider_center_below = screen_pos - glm::vec2(0, OuterRadius + slider_width / 2 + 20);

            // glm::vec3 bgCol = glm::vec3(0.1f);
            // Text::RenderCenteredBG("(R)", screen_pos.x, slider_center.y + OuterRadius + 15, 0.5f, glm::vec3(0.95f), bgCol);
            // Text::RenderCenteredBG("(1)", slider_center.x, slider_center.y + OuterRadius + 15, 0.5f, glm::vec3(0.95f), bgCol);
            // Text::RenderCenteredBG("(2)", slider_center.x + offset.x, slider_center.y + OuterRadius + 15, 0.5f, glm::vec3(0.95f), bgCol);
            // Text::RenderCenteredBG("(3)", slider_center.x, slider_center_below.y - Text::CalculateMaxTextAscent("(3)", 0.5f) / 2.0f, 0.5f, glm::vec3(0.95f), bgCol);

            glm::ivec2 topRightBelow = slider_center_below + glm::ivec2(OuterRadius, slider_width / 2);
            glm::ivec2 bottomLeftBelow = slider_center_below - glm::ivec2(OuterRadius, slider_width / 2);

            float intensity = light->GetIntensity();
            UI::DrawInputBox(topRightBelow, bottomLeftBelow, InputText, GLFW_KEY_3, true);

            try {
                // Only update the intensity if the InputText is a valid number
                float newIntensity = std::stof(InputText);  // Convert string to float
                newIntensity = std::round(newIntensity * 1000.0f) / 1000.0f;

                light->SetIntensity(newIntensity);  // Set the light intensity to the new value
            } catch (const std::invalid_argument& e) {
                // Handle invalid input (e.g., non-numeric text)
                // Optionally, you could reset the input text or leave it unchanged
                // std::cerr << "Invalid intensity value: " << InputText << std::endl;
            } catch (const std::out_of_range& e) {
                // Handle out-of-range values (e.g., too large or too small)
                // std::cerr << "Intensity value out of range: " << InputText << std::endl;
            }

            light->SetColor(newColor);
        }
    }

    void Initialize()
    {
        Engine::RegisterEditorDrawUIFunction(DrawLightColorGizmo);
    }   
}