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
#include "../../ui/box_ui.h"
#include "../../ui/text_renderer.h"

namespace LightManipulation
{
    glm::vec3      chosenColor;
    float          chosenSaturation = 1.0f;
    float          chosenValue = 1.0f;
    std::string    chosenIntensity = "";
    
    BUI::Grid_Item colorWheel ({125, 125}, 10);
    BUI::Grid_Item saturationSlider({ 25, 125}, 10);
    BUI::Grid_Item valueSlider({ 25, 125}, 10);
    BUI::Grid_Item intensityField({ 125, 25}, 10);
    
    BUI::UI_Grid   grid;

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

            SM::SceneNode* node  = SM::SceneNodes[SM::GetSelectedIndex()];
            SM::Light*     light = SM::GetLightFromNode(node);

            grid.Draw(qk::WorldToScreen(light->GetPosition()));

            glm::vec3 newColor = qk::HSVToRGB(glm::vec3(qk::RGBToHSV(chosenColor).r, chosenSaturation, chosenValue));
            light->SetColor(newColor);

            try {
                // Only update the intensity if the InputText is a valid number
                float newIntensity = std::stof(InputText);  // Convert string to float
                newIntensity = std::round(newIntensity * 1000.0f) / 1000.0f;
                light->SetIntensity(newIntensity);
            } catch (const std::invalid_argument& e) {
                // Handle invalid input (e.g., non-numeric text)
                // Optionally, you could reset the input text or leave it unchanged
                // std::cerr << "Invalid intensity value: " << InputText << std::endl;
            } catch (const std::out_of_range& e) {
                // Handle out-of-range values (e.g., too large or too small)
                // std::cerr << "Intensity value out of range: " << InputText << std::endl;
            }
        }
    }

    void Initialize()
    {
        colorWheel.data     = &chosenColor;
        colorWheel.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data)  {
            glm::ivec2 center = (tr + bl) / 2;
            float      radius = std::abs(tr.x - bl.x) / 2.0f;
            
            SM::SceneNode* node  = SM::SceneNodes[SM::GetSelectedIndex()];
            SM::Light*     light = SM::GetLightFromNode(node);
            
            UI::DrawColorWheel(center, light->GetColor(), *static_cast<glm::vec3*>(data), radius, radius - 25);
        };

        saturationSlider.data = &chosenSaturation;
        saturationSlider.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data)  {
            SM::SceneNode* node  = SM::SceneNodes[SM::GetSelectedIndex()];
            SM::Light*     light = SM::GetLightFromNode(node);

            glm::vec3 originalColor = light->GetColor();
            glm::vec3 originalHSV   = qk::RGBToHSV(originalColor);

            
            UI::DrawSlider(tr, bl, originalHSV.y, *static_cast<float*>(data), GLFW_KEY_1, false, true, glm::vec3(1.0f), qk::HSVToRGB({originalHSV.x, 1.0f, 1.0f}));
        };
        
        valueSlider.data = &chosenValue;
        valueSlider.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data)  {
            SM::SceneNode* node  = SM::SceneNodes[SM::GetSelectedIndex()];
            SM::Light*     light = SM::GetLightFromNode(node);
            
            glm::vec3 originalColor = light->GetColor();
            glm::vec3 originalHSV   = qk::RGBToHSV(originalColor);
            
            UI::DrawSlider(tr, bl, originalHSV.z, *static_cast<float*>(data), GLFW_KEY_2, false, true, glm::vec3(0.0f), glm::vec3(1.0f));
        };

        intensityField.data = &InputText;
        intensityField.drawFunc = [&](glm::ivec2 tr, glm::ivec2 bl, void* data)  {
            SM::SceneNode* node  = SM::SceneNodes[SM::GetSelectedIndex()];
            SM::Light*     light = SM::GetLightFromNode(node);
            
            UI::DrawInputBox(tr, bl, *static_cast<std::string*>(data), GLFW_KEY_3, true);
        };
        
        grid.Items = {
            { colorWheel, saturationSlider, valueSlider },
            { intensityField                            },
        };
        grid.UpdateExtents();

        Engine::RegisterEditorDrawUIFunction(DrawLightColorGizmo);
    }   
}
