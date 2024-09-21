#include <map>
#include <memory>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H
#include "../../lib/glad/glad.h"
#include "../../lib/glm/glm.hpp"

#include "text_renderer.h"
#include "../common/shader.h"
#include "../common/stat_counter.h"
#include "../engine/asset_manager.h"

namespace Text
{
    std::unique_ptr<Shader> _textShader;
    uint _textVAO, _textVBO;

    float globalTextScale = 1.0f;

    struct Character
    {
        uint TextureID;
        uint Advance;
        glm::ivec2 Size;
        glm::ivec2 Bearing;
    };

    std::map<char, Character> Characters;

    float CalculateTextWidth(std::string text, float scale)
    {
        float _scale = scale * globalTextScale;
        float width = 0;

        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            Character ch = Characters[*c];
            width += (ch.Advance >> 6) * _scale;
        }

        return width;
    }

    float CalculateMaxTextHeight(std::string text, float scale, bool includeDescent)
    {
        float _scale = scale * globalTextScale;

        float maxheight = 0;
        int maxAscent   = 0;
        int maxDescent  = 0;

        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            Character ch = Characters[*c];
            int ascent   = ch.Bearing.y;
            int descent  = ch.Size.y - ascent;

            maxAscent  = glm::max(maxAscent,  ascent);
            maxDescent = glm::max(maxDescent, descent);
        }
        
        if (includeDescent) return (maxAscent - maxDescent) * _scale;
        else return (maxAscent) * _scale;
    }

    void Render(std::string text, float x, float y, float scale)
    {
        Render(text, x, y, scale, glm::vec3(0.9f));
    }

    void Render(std::string text, float x, float y, float scale, glm::vec3 color)
    {
        float _scale = scale * globalTextScale;

        _textShader->Use();
        _textShader->SetVector3("color", color);

        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(_textVAO);

        std::string::const_iterator c;
        for (c = text.begin(); c != text.end(); c++)
        {
            Character ch = Characters[*c];

            float xpos = x + ch.Bearing.x * _scale;
            float ypos = y - (ch.Size.y - ch.Bearing.y) * _scale;

            float w = ch.Size.x * _scale;
            float h = ch.Size.y * _scale;

            float vertices[6][4] =
            {
                { xpos,     ypos + h,   0.0f, 0.0f },            
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }  
            };

            glBindTexture(GL_TEXTURE_2D, ch.TextureID);
            
            glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glDrawArrays(GL_TRIANGLES, 0, 6);
            x += (ch.Advance >> 6) * _scale;
        }

        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void SetGlobalTextScaling(float scaling)
    {
        globalTextScale = scaling;
    }

    float GetGlobalTextScaling()
    {
        return globalTextScale;
    }

    void Initialize()
    {
        _textShader = std::make_unique<Shader>("/../res/shaders/text");
        WindowResize();

        FT_Library library;
        if (FT_Init_FreeType(&library))
        {
            std::cout << "Couldn't initialize Freetype library :(" << std::endl;
        }

        std::string fontPath = Statistics::ProjectPath + "/../res/fonts/DroidSansMono.ttf";

        FT_Face face;
        if (FT_New_Face(library, fontPath.c_str(), 0, &face))
        {
            std::cout << "Failed to load font" << std::endl;
        }

        FT_Set_Pixel_Sizes(face, 0, 48);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        FT_GlyphSlot slot = face->glyph;

        std::string loadedChars = "";
        for (unsigned char c = 32; c < 128; c++)
        {
            loadedChars += c;

            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "Failed to load glyph: " + c;
                continue;
            }

            FT_Render_Glyph(slot, FT_RENDER_MODE_SDF);

            uint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
            
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            Character character =
            {
                texture,
                face->glyph->advance.x,
                glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            };
            Characters.insert(std::pair<char, Character>(c, character));
    
        }

        std::cout << "Loading font: " + fontPath + "\n";
        std::cout << "Loaded chars:" + loadedChars + "\n" << std::endl;

        FT_Done_Face(face);
        FT_Done_FreeType(library);
        
        glGenVertexArrays(1, &_textVAO);
        glGenBuffers(1, &_textVBO);

        glBindVertexArray(_textVAO);
        glBindBuffer(GL_ARRAY_BUFFER, _textVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 6, NULL, GL_DYNAMIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, false, sizeof(float) * 4, 0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void WindowResize()
    {
        _textShader->Use();
        _textShader->SetMatrix4("projection", AssetManager::OrthoProjMat4);
    }
}