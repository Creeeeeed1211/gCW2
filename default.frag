#version 430

in vec3 v2fColor;
in vec3 v2fNormal;
in vec2 v2fTexCoord; // 接收纹理坐标

layout(location = 0) out vec3 oColor;
layout(location = 2) uniform vec3 uLightDir;
layout(location = 3) uniform vec3 uLightDiffuse;
layout(location = 4) uniform vec3 uSceneAmbient;
layout(location = 5) uniform sampler2D uTexture;
layout(location = 6) uniform bool useTexture;

void main() {
    vec3 normal = normalize(v2fNormal); // Normalize the normal
    float nDotL = max(0.0, dot(normal, uLightDir)); // Calculate diffuse intensity
    vec3 diffuse = uLightDiffuse * nDotL; // Diffuse component

    // Choose between texture or vertex color
    vec3 baseColor = useTexture ? texture(uTexture, v2fTexCoord).rgb : v2fColor;

    oColor = (uSceneAmbient + diffuse) * baseColor; // Final color output
}
