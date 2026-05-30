#include "Render.h"
#include "MyShaders.h"
#include "ObjLoader.h"
#include "Texture.h"
#include "GUItextRectangle.h"

#ifndef MAX
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#define _USE_MATH_DEFINES
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cmath>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>

#include "debout.h"
#include "MyOGL.h"
#include "Light.h"
#include "Camera.h"
#include "Vector3.h"

extern OpenGL gl;
Light light;
Camera camera;

// -----------------------------------------------------------------
// Ресурсы для Земли
// -----------------------------------------------------------------
Shader earthShader;
Texture earthDayTex;
Texture earthCloudTex;
bool useEarthShader = true;

// -----------------------------------------------------------------
// Ресурсы для фона (Milky Way)
// -----------------------------------------------------------------
Shader backgroundShader;
Texture milkyWayTex;
bool useBackground = true;

// -----------------------------------------------------------------
// Класс орбиты
// -----------------------------------------------------------------
class Orbit {
public:
    double a, e, i, Omega, omega, nu;
    static constexpr double MU = 3.986004418e14;
    Orbit(double a = 7e6, double e = 0.0, double i = 0.0,
        double Omega = 0.0, double omega = 0.0, double nu = 0.0)
        : a(a), e(e), i(i), Omega(Omega), omega(omega), nu(nu) {
    }
    double period() const { return 2.0 * M_PI * sqrt(a * a * a / MU); }
    double meanMotion() const { return sqrt(MU / (a * a * a)); }
    double eccentricAnomaly() const {
        double E = 2.0 * atan2(sqrt((1.0 - e) / (1.0 + e)) * sin(nu / 2.0), cos(nu / 2.0));
        return E;
    }
    double meanAnomaly() const {
        double E = eccentricAnomaly();
        return E - e * sin(E);
    }
    double radius() const { return a * (1.0 - e * e) / (1.0 + e * cos(nu)); }
    double speed() const {
        double r = radius();
        return sqrt(MU * (2.0 / r - 1.0 / a));
    }
    Vector3 toEci() const {
        double r = radius();
        double xOrb = r * cos(nu), yOrb = r * sin(nu);
        double x1 = xOrb * cos(omega) - yOrb * sin(omega);
        double y1 = xOrb * sin(omega) + yOrb * cos(omega);
        double z1 = 0.0;
        double x2 = x1;
        double y2 = y1 * cos(i) - z1 * sin(i);
        double z2 = y1 * sin(i) + z1 * cos(i);
        double x = x2 * cos(Omega) - y2 * sin(Omega);
        double y = x2 * sin(Omega) + y2 * cos(Omega);
        double z = z2;
        return Vector3(x, y, z);
    }
    void advanceTime(double dt) {
        double n = meanMotion();
        double M = meanAnomaly() + n * dt;
        double E = M;
        for (int i = 0; i < 10; ++i) E = M + e * sin(E);
        nu = 2.0 * atan2(sqrt(1.0 + e) * sin(E / 2.0), sqrt(1.0 - e) * cos(E / 2.0));
    }
};

// -----------------------------------------------------------------
// Зона покрытия (исправленная для масштаба сцены)
// -----------------------------------------------------------------
class CoverageZone {
public:
    static constexpr double EARTH_RADIUS_SCENE = 5.0;

    static std::vector<Vector3> computeBoundary(const Vector3& satPosScaled, int seg = 64) {
        std::vector<Vector3> boundary;
        double R = EARTH_RADIUS_SCENE;
        double r = satPosScaled.length();
        if (r <= R) return boundary;
        double theta = acos(R / r);
        Vector3 dir = satPosScaled.normalize();
        Vector3 u = (fabs(dir.x()) < 0.9) ? Vector3(1, 0, 0) : Vector3(0, 1, 0);
        Vector3 v = (dir ^ u).normalize();
        Vector3 w = (v ^ dir).normalize();
        for (int i = 0; i <= seg; ++i) {
            double az = 2.0 * M_PI * i / seg;
            Vector3 perp = v * cos(az) + w * sin(az);
            Vector3 bdir = dir * cos(theta) + perp * sin(theta);
            boundary.push_back(bdir * R);
        }
        return boundary;
    }

    static void drawWireframe(const std::vector<Vector3>& boundary, const Vector3& satPosScaled) {
        if (boundary.empty()) return;
        glDisable(GL_LIGHTING);
        glColor3f(0, 1, 0);
        glLineWidth(2);
        glBegin(GL_LINE_LOOP);
        for (auto& p : boundary) glVertex3d(p.x(), p.y(), p.z());
        glEnd();
        glBegin(GL_LINES);
        for (auto& p : boundary) {
            glVertex3d(satPosScaled.x(), satPosScaled.y(), satPosScaled.z());
            glVertex3d(p.x(), p.y(), p.z());
        }
        glEnd();
        glEnable(GL_LIGHTING);
    }
};

// -----------------------------------------------------------------
// Спутник
// -----------------------------------------------------------------
class Satellite {
public:
    Orbit orbit;
    ObjModel model;
    bool isVisible;
    Vector3 cachedPos;
    Satellite(const Orbit& orb) : orbit(orb), isVisible(true) {}
    void loadModel(const std::string& fname) { model.LoadModel(fname.c_str()); }
    void update(double dt) {
        orbit.advanceTime(dt);
        cachedPos = orbit.toEci();
    }
    void draw(double scale = 0.5) {
        if (!isVisible) return;
        glPushMatrix();
        glTranslated(cachedPos.x() / 1e5, cachedPos.y() / 1e5, cachedPos.z() / 1e5);
        glScaled(scale, scale, scale);
        model.Draw();
        glPopMatrix();
    }
    Vector3 getScaledPosition() const {
        return Vector3(cachedPos.x() / 1e5, cachedPos.y() / 1e5, cachedPos.z() / 1e5);
    }
};

// -----------------------------------------------------------------
// Глобальные переменные
// -----------------------------------------------------------------
bool texturing = true, lightning = true, alpha = false;
Orbit currentOrbit(7e6, 0.1, 0.2, 0.5, 0.3, 0.0);
Satellite sat(currentOrbit);
double simTime = 0.0;
bool showCoverage = true;
bool showSatInfo = true;
GuiTextRectangle satInfoText;

// -----------------------------------------------------------------
// Отрисовка орбиты (trace)
// -----------------------------------------------------------------
void drawOrbitTrace(const Orbit& orbit, int segments = 360) {
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glColor3f(0.7f, 0.7f, 1.0f);
    glLineWidth(1.5f);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i <= segments; ++i) {
        double nu = 2.0 * M_PI * i / segments;
        Orbit tempOrbit = orbit;
        tempOrbit.nu = nu;
        Vector3 pos = tempOrbit.toEci();
        glVertex3d(pos.x() / 1e5, pos.y() / 1e5, pos.z() / 1e5);
    }
    glEnd();
    glEnable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
}

// -----------------------------------------------------------------
// Инициализация ресурсов Земли
// -----------------------------------------------------------------
void initEarthResources() {
    earthDayTex.LoadTexture("textures/earth_day.jpg");
    earthCloudTex.LoadTexture("textures/earth_clouds.jpg");
    earthShader.VshaderFileName = "shaders/earth.vert";
    earthShader.FshaderFileName = "shaders/earth.frag";
    earthShader.LoadShaderFromFile();
    earthShader.Compile();
}

// -----------------------------------------------------------------
// Инициализация фона
// -----------------------------------------------------------------
void initBackground() {
    milkyWayTex.LoadTexture("textures/milky_way.jpg");
    backgroundShader.VshaderFileName = "shaders/background.vert";
    backgroundShader.FshaderFileName = "shaders/background.frag";
    backgroundShader.LoadShaderFromFile();
    backgroundShader.Compile();
}

// -----------------------------------------------------------------
// Отрисовка фона
// -----------------------------------------------------------------
void drawBackground() {
    if (!useBackground) return;
    glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    backgroundShader.UseShader();
    milkyWayTex.Bind(0);
    glUniform1iARB(glGetUniformLocationARB(backgroundShader.program, "u_texture"), 0);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluSphere(quad, 100.0, 64, 64);
    gluDeleteQuadric(quad);
    backgroundShader.DontUseShaders();
    glPopAttrib();
}

// -----------------------------------------------------------------
// Отрисовка Земли с освещением
// -----------------------------------------------------------------
void drawEarthWithShaders() {
    if (!useEarthShader) {
        glDisable(GL_TEXTURE_2D);
        glColor3f(0.2f, 0.3f, 0.8f);
        GLUquadric* quad = gluNewQuadric();
        gluSphere(quad, 5.0, 64, 64);
        gluDeleteQuadric(quad);
        glEnable(GL_TEXTURE_2D);
        return;
    }
    earthShader.UseShader();
    GLfloat lightDirWorld[3] = { (float)light.x(), (float)light.y(), (float)light.z() };
    float len = sqrt(lightDirWorld[0] * lightDirWorld[0] + lightDirWorld[1] * lightDirWorld[1] + lightDirWorld[2] * lightDirWorld[2]);
    if (len > 0.0f) {
        lightDirWorld[0] /= len; lightDirWorld[1] /= len; lightDirWorld[2] /= len;
    }
    else {
        lightDirWorld[0] = 1.0f; lightDirWorld[1] = 0.0f; lightDirWorld[2] = 0.0f;
    }
    glUniform3fvARB(glGetUniformLocationARB(earthShader.program, "u_lightDirWorld"), 1, lightDirWorld);
    glUniform1fARB(glGetUniformLocationARB(earthShader.program, "u_lightIntensity"), 1.8f);
    glUniform1fARB(glGetUniformLocationARB(earthShader.program, "u_ambientStrength"), 0.3f);
    earthDayTex.Bind(0);
    earthCloudTex.Bind(1);
    glUniform1iARB(glGetUniformLocationARB(earthShader.program, "u_dayTex"), 0);
    glUniform1iARB(glGetUniformLocationARB(earthShader.program, "u_cloudTex"), 1);
    GLUquadric* quad = gluNewQuadric();
    gluQuadricTexture(quad, GL_TRUE);
    gluQuadricNormals(quad, GLU_SMOOTH);
    gluSphere(quad, 5.0, 128, 128);
    gluDeleteQuadric(quad);
    earthShader.DontUseShaders();
}

// -----------------------------------------------------------------
// Обработчики клавиш
// -----------------------------------------------------------------
void switchModes(OpenGL* sender, KeyEventArg arg) {
    char key = LOWORD(MapVirtualKeyA(arg.key, MAPVK_VK_TO_CHAR));
    switch (key) {
    case 'L': lightning = !lightning; break;
    case 'T': texturing = !texturing; break;
    case 'A': alpha = !alpha; break;
    case 'M': useEarthShader = !useEarthShader; break;
    case 'B': useBackground = !useBackground; break;
    case 'C': showCoverage = !showCoverage; break;
    case 'I': showSatInfo = !showSatInfo; break;
    }
}

void handleOrbitInput(OpenGL* sender, KeyEventArg arg) {
    int key = arg.key;
    double step_a = 1e5, step_e = 0.01, step_i = 0.05;
    double step_O = 0.1, step_w = 0.1, step_n = 0.05;
    if (key == VK_UP) {
        currentOrbit.a += step_a;
    }
    else if (key == VK_DOWN) {
        currentOrbit.a = MAX(6.5e6, currentOrbit.a - step_a);
    }
    else if (key == VK_LEFT) {
        currentOrbit.e = MAX(0.0, currentOrbit.e - step_e);
    }
    else if (key == VK_RIGHT) {
        currentOrbit.e = MIN(0.99, currentOrbit.e + step_e);
    }
    else {
        char ch = LOWORD(MapVirtualKeyA(key, MAPVK_VK_TO_CHAR));
        switch (ch) {
        case '+': case '=': currentOrbit.i = MIN(M_PI, currentOrbit.i + step_i); break;
        case '-': currentOrbit.i = MAX(0.0, currentOrbit.i - step_i); break;
        case '[': currentOrbit.Omega += step_O; break;
        case ']': currentOrbit.Omega -= step_O; break;
        case ';': currentOrbit.omega += step_w; break;
        case '\'': currentOrbit.omega -= step_w; break;
        case '9': currentOrbit.nu += step_n; break;
        case '0': currentOrbit.nu -= step_n; break;
        }
    }
    currentOrbit.nu = fmod(currentOrbit.nu, 2.0 * M_PI);
    if (currentOrbit.nu < 0) currentOrbit.nu += 2.0 * M_PI;
    sat.orbit = currentOrbit;
}

// -----------------------------------------------------------------
// Инициализация
// -----------------------------------------------------------------
void initRender() {
    initShadersFunctions();
    sat.loadModel("models/satellite.obj");
    sat.orbit = currentOrbit;
    initEarthResources();
    initBackground();
    satInfoText.setSize(300, 120);
    gl.KeyDownEvent.reaction(handleOrbitInput);
    gl.KeyDownEvent.reaction(switchModes);
    gl.WheelEvent.reaction([&](OpenGL* sender, MouseWheelEventArg arg) { camera.Zoom(sender, arg); });
    gl.MouseMovieEvent.reaction([&](OpenGL* sender, MouseEventArg arg) { camera.MouseMovie(sender, arg); });
    gl.MouseLeaveEvent.reaction([&](OpenGL* sender, MouseEventArg arg) { camera.MouseLeave(sender, arg); });
    gl.MouseLdownEvent.reaction([&](OpenGL* sender, MouseEventArg arg) { camera.MouseStartDrag(sender, arg); });
    gl.MouseLupEvent.reaction([&](OpenGL* sender, MouseEventArg arg) { camera.MouseStopDrag(sender, arg); });
    gl.MouseMovieEvent.reaction([&](OpenGL* sender, MouseEventArg arg) { light.MoveLight(sender, arg); });
    gl.KeyDownEvent.reaction([&](OpenGL* sender, KeyEventArg arg) { light.StartDrug(sender, arg); });
    gl.KeyUpEvent.reaction([&](OpenGL* sender, KeyEventArg arg) { light.StopDrug(sender, arg); });
    camera.setPosition(0, 0, 50);
    camera.caclulateCameraPos();
}

// -----------------------------------------------------------------
// Основной рендер
// -----------------------------------------------------------------
void Render(double delta_time) {
    simTime += delta_time;
    sat.update(delta_time);
    if (gl.isKeyPressed('F'))
        light.SetPosition(camera.x(), camera.y(), camera.z());
    camera.SetUpCamera();
    light.SetUpLight();
    if (lightning) glEnable(GL_LIGHTING);
    else glDisable(GL_LIGHTING);
    if (texturing) glEnable(GL_TEXTURE_2D);
    else glDisable(GL_TEXTURE_2D);
    if (alpha) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else {
        glDisable(GL_BLEND);
    }
    glEnable(GL_NORMALIZE);
    drawBackground();
    drawEarthWithShaders();
    drawOrbitTrace(currentOrbit, 360);
    glColor3f(0.8f, 0.8f, 0.8f);
    sat.draw(0.5);
    if (showCoverage) {
        auto boundary = CoverageZone::computeBoundary(sat.getScaledPosition(), 128);
        CoverageZone::drawWireframe(boundary, sat.getScaledPosition());
    }
    light.DrawLightGizmo();
}