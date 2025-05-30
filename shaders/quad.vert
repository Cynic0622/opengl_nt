#version 330 core

// 输入顶点属性
layout(location = 0) in vec3 aPos;         // 顶点位置
layout(location = 1) in vec2 aTexCoord;    // 纹理坐标

// 输出到片段着色器的数据
out vec2 TexCoord; // 纹理坐标

void main() {
    gl_Position = vec4(aPos, 1.0); // 设置顶点位置
    TexCoord = aTexCoord;          // 传递纹理坐标
}