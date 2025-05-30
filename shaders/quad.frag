#version 330 core
// 输入顶点属性
in vec2 TexCoord;           // 纹理坐标

uniform sampler2D textureSampler;  // 纹理采样器

void main() {
    // 采样纹理
    vec3 sampledColor = texture(textureSampler, TexCoord).rgb;
    
    // 应用伽马校正
    // sampledColor.rgb = pow(sampledColor.rgb, vec3(1.0 / 2.2));
    
    // 输出颜色
    gl_FragColor = vec4(sampledColor, 1.0);
    // gl_FragColor = vec4(1.0); // 输出颜色
}