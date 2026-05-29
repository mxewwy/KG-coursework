#version 120

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

uniform sampler2D dayTexture;
uniform sampler2D nightTexture;
uniform sampler2D normalMap;   // опционально, можно не использовать
uniform vec3 lightDir;         // направление на солнце (должно быть нормализовано)
uniform vec3 viewPos;
uniform float time;

void main() {
    vec3 norm = normalize(Normal);
    vec3 light = normalize(lightDir);
    
    // Диффузное освещение (коэффициент 0..1)
    float diff = max(dot(norm, light), 0.0);
    
    // Смягчаем переход на границе (опционально: можно применить smoothstep)
    float brightness = diff;
    
    // Дневная и ночная текстуры
    vec3 dayColor = texture2D(dayTexture, TexCoord).rgb;
    vec3 nightColor = texture2D(nightTexture, TexCoord).rgb;
    
    // Смешивание: дневная там, где diff > 0, иначе ночная
    vec3 color = mix(nightColor, dayColor, brightness);
    
    // Небольшой спекулярный блик (солнечное отражение)
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-light, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.4) * spec;
    
    gl_FragColor = vec4(color + specular, 1.0);
}