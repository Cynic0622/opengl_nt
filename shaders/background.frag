#version 330 core

// 使用统一的命名规范和结构体布局
struct Light {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

// 输入接口块
in VS_OUT {
    vec3 ViewNormal;
    vec3 FragPos;
    mat4 ViewMatrix;
    vec4 FragPosLightSpace;
} fs_in;

// Uniforms
layout(std140) uniform LightUniforms {
    Light light;
};
uniform vec3 background_color;
uniform sampler2D shadowMap;

// 输出
layout(location = 0) out vec4 FragColor;

// 常量
const float GAMMA = 2.2;
const float SHININESS = 64.0;

// 阴影计算函数优化
float CalculateShadow(vec4 fragPosLightSpace)
{
    // 执行透视除法
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
    // 变换到[0,1]范围
    projCoords = projCoords * 0.5 + 0.5;
    
    // 添加PCF实现软阴影
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += projCoords.z > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    
    // 处理超出阴影贴图范围的情况
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}

// 光照计算函数
vec3 CalculatePhongLighting(vec3 normal, vec3 fragPos, vec3 baseColor)
{
    vec3 viewLightDir = normalize((fs_in.ViewMatrix * vec4(light.direction, 0.0)).xyz);
    vec3 viewDir = normalize(-fragPos);
    vec3 halfwayDir = normalize(viewDir - viewLightDir);
    
    // 环境光
    vec3 ambient = light.ambient * baseColor;
    
    // 漫反射
    float diff = max(dot(normal, -viewLightDir), 0.0);
    vec3 diffuse = light.diffuse * diff * baseColor;
    
    // 镜面反射
    float spec = pow(max(dot(halfwayDir, normal), 0.0), SHININESS);
    vec3 specular = light.specular * spec;
    
    return ambient + diffuse + specular;
}

void main()
{
    // Gamma校正的输入颜色
    vec3 color = pow(background_color, vec3(GAMMA));
    
    // 标准化法线
    vec3 normal = normalize(fs_in.ViewNormal);
    
    // 计算光照
    vec3 lighting = CalculatePhongLighting(normal, fs_in.FragPos, color);
    
    // 计算阴影
    float shadow = CalculateShadow(fs_in.FragPosLightSpace);
    
    // 合并光照和阴影
    vec3 finalColor = lighting * (1.0 - shadow);
    
    // Gamma校正输出
    FragColor = vec4(pow(finalColor, vec3(1.0/GAMMA)), 1.0);
}