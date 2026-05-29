#version 120

attribute vec3 aPos;
attribute vec2 aTexCoord;
attribute vec3 aNormal;

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    TexCoord = aTexCoord;
    // Преобразуем нормаль в мировое пространство (без учёта масштаба)
    Normal = mat3(transpose(inverse(model))) * aNormal;
    FragPos = vec3(model * vec4(aPos, 1.0));
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}