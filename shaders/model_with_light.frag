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
layout(location = 1) out vec3 UVOutput;      // UV坐标输出
layout(location = 2) out vec3 NormalOutput;  // 视图空间法线输出

// 阴影计算函数
// float ShadowCalculation(vec4 fragPosLightSpace)
// {
//     // 执行透视除法
//     vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
//     // 变换到[0,1]范围
//     projCoords = projCoords * 0.5 + 0.5;
    
//     // 获取最近的深度值
//     float closestDepth = texture(shadowMap, projCoords.xy).r;
        
//     // 当前片段的深度
//     float currentDepth = projCoords.z;
    
//     // 阴影偏移值 - 根据对象类型调整
//     float bias = object_id == 0 ? 0.075 : 0.01;
    
//     // 计算阴影值（0=无阴影，1=完全阴影）
//     float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
    
//     // 如果在光源视锥外，不产生阴影
//     if(projCoords.z > 1.0)
//         shadow = 0.0;
        
//     return shadow;
// }

void main()
{
    // 伽马校正值
    const float gamma = 2.2;
    
    // 采样纹理并应用伽马校正
    vec3 texColor = texture(myTextureSampler, TexCoord).rgb;
    // texColor = pow(texColor, vec3(gamma)); // 从 sRGB 转到线性空间
    
    // 输出UV坐标
    UVOutput = vec3(TexCoord, 0.0);
    
    // 输出归一化法线
    vec3 normalizedNormal = normalize(Normal);
    NormalOutput = normalizedNormal;
    
    // 基于物理的光照计算 (PBR简化版)
    
    // 1. 环境光照
    vec3 ambient = light.ambient;
    
    // // 2. 漫反射 - 使用法线和光照方向
    vec3 lightDir = normalize(-light.direction); // 视图空间中的光源方向
    float diff = max(dot(normalizedNormal, lightDir), 0.0);
    vec3 diffuse = light.diffuse * diff;
    
    // // 3. 镜面反射 - 使用Blinn-Phong模型
    vec3 viewDir = normalize(-FragPos); // 视图方向
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalizedNormal, halfwayDir), 0.0), 32.0);
    vec3 specular = light.specular * spec;
    
    // 4. 计算阴影
    // float shadow = ShadowCalculation(FragPosLightSpace);
    
    // 5. 合并所有光照组件
    // vec3 lighting = (ambient + (diffuse + specular) * (1.0 - shadow)) * texColor;
    vec3 lighting = (ambient + diffuse + specular) * texColor;
    // vec3 lighting = texColor; // 纹理颜色与光照颜色相乘
    // lighting = clamp(lighting, 0.0, 1.0); // 确保颜色在[0,1]范围内
    FragColor = lighting; // 输出最终颜色
    // 应用逆伽马校正，准备输出
    // FragColor = pow(lighting, vec3(1.0/gamma)); // 从线性空间转到sRGB
}