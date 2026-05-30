#pragma once
#include <string>

class Texture {
    unsigned int texId = 0;
public:
    Texture() {}
    ~Texture();
    void LoadTexture(const std::string& filename);
    void Bind();
    void Bind(int unit);
    unsigned int getTexId() const { return texId; }
};