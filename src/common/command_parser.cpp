#include "command_parser.h"
#include "qk.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>

namespace PC
{
    std::vector<std::string> SplitByWS(const std::string& str)
    {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;

        bool opening = false;
        bool closing = false;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        return tokens;
    }

    std::vector<std::string> SplitByChar(const std::string& str, char splitter, bool stripWS)
    {
        std::vector<std::string> tokens;
        std::string cur;

        for (char c : str) {
            if (c == splitter) {
                if (!cur.empty()) {
                    tokens.push_back(cur);
                    cur.clear();
                }
            }
            else if (!std::isspace(c) || !stripWS) cur += c;
        }

        if (!cur.empty()) tokens.push_back(cur);

        return tokens;
    }

    std::vector<std::string> SplitByCharExeceptInside(const std::string& str, char splitter, char c1, char c2)
    {
        std::vector<std::string> tokens;
        std::string cur;
        bool inside = false;

        for (char c : str) {
            if (c == c1) {
                inside = true;
                cur += c;
            }
            else if (c == c2) {
                inside = false;
                cur += c;
            }
            else if (c == splitter && !inside) {
                if (!cur.empty()) {
                    tokens.push_back(cur);
                    cur.clear();
                }
            }
            else cur += c;
        }

        if (!cur.empty()) tokens.push_back(cur);

        return tokens;
    }

    bool StringEnclosedWith(const std::string& str, char c)
    {
        if (str[0] == c && str[str.length() - 1] == c) return true;
        return false;
    }

    bool StringEnclosedWith(const std::string& str, char c1, char c2)
    {
        if (str[0] == c1 && str[str.length() - 1] == c2) return true;
        return false;
    }

    bool StringStartsWith(const std::string& str, const std::string& start)
    {
        if (start.length() > str.length()) return false;

        for (int i = 0; i < start.length(); i++) {
            if (start[i] != str[i]) return false;
        }
        return true;
    }

    std::string KeepInsideChars(const std::string& str, char c1, char c2)
    {
        std::string inside = "";
        bool isInside      = false;
        bool justEntered   = false;
        
        for (int i = 0; i < str.length(); i++)
        {
            if (str[i] == c1) {
                isInside = true;
                justEntered = true;
            }
            if (str[i] == c2) {
                isInside = false;
            }
            if (isInside && !justEntered) inside += str[i];
            justEntered = false;
        }

        return inside;
    }

    std::string KeepStringByIndices(const std::string& str, int start, int end)
    {
        std::string continous = "";
        for (int i = start; i < end; i++)
        {
            continous += str[i];
        }

        return continous;
    }

    void RunTest(std::string String)
    {
        std::vector<std::string> split = SplitByCharExeceptInside(String, ' ', '(', ')');
        if (split.empty()) return;

        for (auto s : split) printf("%s\n", s.c_str());
        std::cout << "\n";

        std::string objectName = "";
        std::string meshName   = "";
        glm::vec3   position;

        if (split[0] == "add") {
            std::cout << "Add command started\n";

            for (size_t i = 0; i < split.size(); i++)
            {
                std::string cur  = split[i];
                if (cur == "-o") {
                    std::string next = split[i + 1];
                    if (StringEnclosedWith(next, '"')) {
                        objectName = KeepStringByIndices(next, 1, next.length() - 1);

                        std::cout << "Object name is: " << objectName << "\n";
                    } else std::cout << "Object name unspecified\n";
                }
                else if (cur == "-m") {
                    std::string next = split[i + 1];
                    if (StringEnclosedWith(next, '"')) {
                        meshName = KeepStringByIndices(next, 1, next.length() - 1);
                        std::cout << "Mesh name is: " << meshName << "\n";
                    } else std::cout << "Mesh name unspecified\n";
                }
                else if (cur == "-p") {
                    std::string next = split[i + 1];
                    if (StringStartsWith(next, "vec3"))
                    {
                        std::cout << "Parsing position\n";
                        if (StringEnclosedWith(next, '('), ')') {
                            std::string inside = KeepInsideChars(next, '(', ')');
                            std::cout << "XYZ Parenthesis found: " << inside << "\n";
                            
                            std::vector<std::string> xyz = SplitByChar(inside, ',', true);
                            if (xyz.size() == 3) {
                                float x = qk::TextToFloat(xyz[0]);
                                float y = qk::TextToFloat(xyz[1]);
                                float z = qk::TextToFloat(xyz[2]);

                                printf("XYZ found: %f - %f - %f\n", x, y, z);
                            }
                            else std::cout << "vec3 takes 3 arguments, dude\n";
                        }
                    }
                }
            }
        }

    }
}
