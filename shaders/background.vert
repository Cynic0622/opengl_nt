#version 330 core

// 输入顶点数据
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;

// 输出接口块，与片段着色器匹配
out VS_OUT {
    vec3 ViewNormal;
    vec3 FragPos;
    mat4 ViewMatrix;
    vec4 FragPosLightSpace;
} vs_out;

// Uniforms
layout(std140) uniform Matrices {
    mat4 model;
    mat4 view;
    mat4 projection;
    mat4 lightSpaceMatrix;
};

void main() {
    // 计算视图模型矩阵
    mat4 viewModel = view * model;
    
    // 计算最终位置
    gl_Position = projection * viewModel * vec4(aPos, 1.0);
    
    // 计算观察空间法线矩阵
    mat4 normalMatrix = transpose(inverse(viewModel));
    
    // 填充输出接口块
    vs_out.ViewNormal = (normalMatrix * vec4(aNormal, 0.0)).xyz;
    vs_out.FragPos = (viewModel * vec4(aPos, 1.0)).xyz;
    vs_out.ViewMatrix = viewModel;
    vs_out.FragPosLightSpace = lightSpaceMatrix * vec4(aPos, 1.0);
}