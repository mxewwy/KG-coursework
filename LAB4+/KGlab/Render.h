#pragma once

void initRender();
void Render(double);

extern Shader earthShader;
extern Texture earthDayTex;
extern Texture earthCloudTex;
extern bool useEarthShader;
extern Shader backgroundShader;
extern Texture milkyWayTex;
extern bool useBackground;

void initEarthResources();
void drawEarthWithShaders();