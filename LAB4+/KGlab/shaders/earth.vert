#version 120

attribute vec3 aPos;
attribute vec2 aTexCoord;
attribute vec3 aNormal;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

void main() {
    TexCoord = aTexCoord;
    // ѕреобразуем нормаль в видовое пространство
    Normal = gl_NormalMatrix * aNormal;
    vec4 pos = gl_ModelViewMatrix * vec4(aPos, 1.0);
    FragPos = pos.xyz;
    gl_Position = gl_ProjectionMatrix * pos;
}