#version 330 core

// 输入顶点属性
layout(location = 0) in vec3 aPos;         // 顶点位置
layout(location = 1) in vec3 aNormal;    // 顶点法线
layout(location = 2) in vec2 aTexCoord;      // 纹理坐标

// 输出到片段着色器的数据
out vec2 TexCoord;           // 纹理坐标
out vec3 Normal;             // 视图空间法线
out vec3 FragPos;            // 视图空间片段位置
out vec4 FragPosLightSpace;  // 光源空间片段位置

// Uniform 变量（每次绘制调用保持不变）
uniform mat4 projection;     // 投影矩阵
uniform mat4 view;           // 视图矩阵 
uniform mat4 model;          // 模型矩阵
uniform mat4 lightSpaceMatrix; // 光源空间变换矩阵（用于阴影映射）

void main()
{
    // 计算最终屏幕空间位置
    mat4 modelView = view * model;
    vec4 viewPos = modelView * vec4(aPos, 1.0);
    gl_Position = projection * viewPos;
    
    // 传递纹理坐标
    TexCoord = aTexCoord;
    
    // 计算视图空间法线（使用法线矩阵 - 模型视图矩阵的逆转置）
    // 注：对于非均匀缩放，这是保持法线垂直于表面的正确方法
    mat3 normalMatrix = transpose(inverse(mat3(modelView)));
    Normal = normalMatrix * aNormal;
    
    // 视图空间片段位置（用于光照计算）
    FragPos = viewPos.xyz;
    
    // 光源空间坐标（用于阴影映射）
    FragPosLightSpace = lightSpaceMatrix * model * vec4(aPos, 1.0);
}