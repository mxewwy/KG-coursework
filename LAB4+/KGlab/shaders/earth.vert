#version 120
varying vec2 v_texCoord;
varying vec3 v_normalWorld;
void main() {
    v_texCoord = gl_MultiTexCoord0.xy;
    v_normalWorld = normalize(gl_Normal);
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}