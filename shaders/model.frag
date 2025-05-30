#version 330 core

struct Light {
    vec3 direction;  // 光源方向（视图空间）
    vec3 ambient;    // 环境光颜色
    vec3 diffuse;    // 漫反射光颜色
    vec3 specular;   // 镜面反射光颜色
};

// 从顶点着色器接收的输入
in vec2 TexCoord;           // 纹理坐标
in vec3 Normal;             // 视图空间法线
in vec3 FragPos;            // 视图空间片段位置
in vec4 FragPosLightSpace;  // 光源空间片段位置

// Uniform 变量
uniform sampler2D myTextureSampler;  // 颜色纹理
uniform sampler2D shadowMap;         // 阴影贴图
uniform Light light;                 // 光源属性
uniform int object_id;               // 对象ID（0=篮球，1=花瓶）

// 输出数据（多渲染目标）
layout(location = 0) out vec3 FragColor;     // 主颜色输出
// layout(location = 1) out vec3 UVOutput;      // UV坐标输出
// layout(location = 2) out vec3 NormalOutput;  // 视图空间法线输出


void main()
{
    vec3 texColor = texture(myTextureSampler, TexCoord).rgb;
    
    // 输出UV坐标
    // UVOutput = vec3(TexCoord, 0.0);
    
    // 输出归一化法线
    vec3 normalizedNormal = normalize(Normal);
    // NormalOutput = normalizedNormal;

    
    FragColor = texColor; // 输出最终颜色
}