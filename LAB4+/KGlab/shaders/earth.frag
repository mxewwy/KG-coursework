#version 120
uniform sampler2D u_dayTex;
uniform sampler2D u_cloudTex;
uniform vec3 u_lightDirWorld;
uniform float u_lightIntensity;
uniform float u_ambientStrength;
varying vec2 v_texCoord;
varying vec3 v_normalWorld;

void main() {
    vec3 normal = normalize(v_normalWorld);
    float NdotL = max(0.0, dot(normal, u_lightDirWorld));
    float brightness = u_ambientStrength + NdotL * u_lightIntensity;
    brightness = clamp(brightness, 0.0, 1.0);
    
    vec4 dayColor = texture2D(u_dayTex, v_texCoord);
    vec4 cloudColor = texture2D(u_cloudTex, v_texCoord);
    
    vec3 earthColor = dayColor.rgb * brightness;
    float cloudIntensity = (cloudColor.r + cloudColor.g + cloudColor.b) / 3.0;
    cloudIntensity = clamp(cloudIntensity * 1.5, 0.0, 0.8);
    vec3 cloudIlluminated = vec3(1.0, 1.0, 1.0) * brightness;
    
    vec3 finalColor = mix(earthColor, cloudIlluminated, cloudIntensity);
    gl_FragColor = vec4(finalColor, 1.0);
}