#pragma once

#include "MyShaders.h"
#include "Texture.h"
#include "ObjLoader.h"

void initRender();
void Render(double);

extern Shader earthShader;
extern Texture earthDayTex;
extern Texture earthNightTex;
extern Texture earthCloudTex;
extern ObjModel earthModel;
extern bool useEarthShader;
extern Shader backgroundShader;
extern Texture milkyWayTex;
extern bool useBackground;

void initBackground();
void drawBackground();

void initEarthResources();
void drawEarthWithShaders();