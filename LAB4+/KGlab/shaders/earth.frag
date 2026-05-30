#version 120

varying vec2 TexCoord;
varying vec3 Normal;
varying vec3 FragPos;

uniform sampler2D dayTexture;
uniform sampler2D nightTexture;
uniform vec3 lightDir;      // направление на солнце (в видовом пространстве)
uniform vec3 viewPos;       // позиция камеры (в видовом пространстве)

void main() {
    vec3 norm = normalize(Normal);
    vec3 light = normalize(lightDir);
    float diff = max(dot(norm, light), 0.0);
    
    // Дневная и ночная текстуры
    vec3 dayColor = texture2D(dayTexture, TexCoord).rgb;
    vec3 nightColor = texture2D(nightTexture, TexCoord).rgb;
    
    // Смешивание
    vec3 color = mix(nightColor, dayColor, diff);
    
    // Спекулярное отражение
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-light, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
    vec3 specular = vec3(0.5) * spec;
    
    gl_FragColor = vec4(color + specular, 1.0);
}