#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>


// GLAD 必须在 GLFW 之前包含
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM 数学库
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

// 项目定义的头文件
#include "shader.hpp"
#include "objLoader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "cnpy/cnpy.h"

// 窗口和视口尺寸
int window_width = 1024;
int window_height = 1024;
int viewport_width = 1024;
int viewport_height = 1024;

// 渲染配置
bool enable_output = false;  // 设置为 true 将保存渲染结果到文件
int total_frame = 2000;      // 渲染帧数
glm::vec3 background_color = glm::vec3(0.3f, 0.3f, 0.3f);
int object_id = 0;           // 0 表示篮球；1 表示花瓶

// 输出路径
std::string uv_output_dir = "output/uv/%04d.npy";
std::string screenshot_output_dir = "output/frame/%04d.png";
std::string camera_extrinsics_output_dir = "output/extrinsics/%04d.npy";

// 光照定义
struct Light {
	glm::vec3 direction;  // 标准化的方向
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

// 初始化背景（地面）顶点数据
// void initBackground(unsigned int& bgVAO, unsigned int& bgVBO) {
//     // 顶点数据包含：坐标 - 法线 - UV
//     float vertices_basketball[] = {
//         // 底面
//         -2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  0.0f, 0.0f,
//          2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  0.08f, 0.0f,
//          2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.04f, 0.08f,
//          2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.04f, 0.08f,
//         -2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.08f, 0.0f,
//         -2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  0.0f, 0.0f
//     };
//
//     float vertices_vase[] = {
//         // 底面
//         -2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  1.0f, 0.0f,
//          2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  0.8f, 0.0f,
//          2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.8f, 0.15f,
//          2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.8f, 0.15f,
//         -2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  1.0f, 0.15f,
//         -2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  1.0f, 0.0f
//     };
//
//     glGenVertexArrays(1, &bgVAO);
//     glGenBuffers(1, &bgVBO);
//     glBindVertexArray(bgVAO);
//
//     // 填充缓冲区
//     glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
//     if (object_id == 0) {
//         glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_basketball), vertices_basketball, GL_STATIC_DRAW);
//     }
//     else if (object_id == 1) {
//         glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_vase), vertices_vase, GL_STATIC_DRAW);
//     }
//
//     // 链接顶点属性
//     // 坐标
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
//     // UV
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(6 * sizeof(float)));
//     // 法线
//     glEnableVertexAttribArray(2);
//     glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(3 * sizeof(float)));
//
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
// }
//
// // 保存视图法线图像
// void saveImageViewNormal(const char* filename) {
//     GLfloat* image_view_normal_data = new GLfloat[viewport_height * viewport_width * 3];
//     glReadBuffer(GL_COLOR_ATTACHMENT2);
//     glReadPixels(0, 0, viewport_width, viewport_height, GL_RGB, GL_FLOAT, image_view_normal_data);
//
//     // 反转像素（因为OpenGL和图像坐标系不同）
//     for (int j = 0; j * 2 < viewport_height; ++j) {
//         int x = j * viewport_width * 3;
//         int y = (viewport_height - 1 - j) * viewport_width * 3;
//         for (int i = viewport_width * 3; i > 0; --i) {
//             std::swap(image_view_normal_data[x], image_view_normal_data[y]);
//             ++x;
//             ++y;
//         }
//     }
//
//     // 保存为NPY格式
//     std::vector<float> data;
//     for (int j = 0; j < viewport_height; j++) {
//         for (int i = 0; i < viewport_width; i++) {
//             int t = j * viewport_width * 3 + i * 3;
//             data.push_back(image_view_normal_data[t]);
//             data.push_back(image_view_normal_data[t + 1]);
//             data.push_back(image_view_normal_data[t + 2]);
//         }
//     }
//
//     // 使用cnpy库保存为npy格式
//     cnpy::npy_save(filename, &data[0], { (unsigned long)viewport_height, (unsigned long)viewport_width, 3 }, "w");
//     delete[] image_view_normal_data;
// }
//
// // 保存相机外参
// glm::vec3 saveCameraExtrinsics(const char* filename, glm::mat4 model, glm::vec3 cameraPos) {
//     glm::vec3 extrinsics = normalize(glm::vec3(model * glm::vec4(cameraPos, 1.0)));
//     float data[] = { extrinsics.x, extrinsics.y, extrinsics.z };
//     cnpy::npy_save(filename, &data[0], { 3 }, "w");
//     return extrinsics;
// }
//
// // 保存UV坐标数据
// void saveImageUV(const char* filename) {
//     GLfloat* image_uv_data = new GLfloat[viewport_height * viewport_width * 3];
//
//     glReadBuffer(GL_COLOR_ATTACHMENT1);
//     glReadPixels(0, 0, viewport_width, viewport_height, GL_RGB, GL_FLOAT, image_uv_data);
//
//     // 反转像素
//     for (int j = 0; j * 2 < viewport_height; ++j) {
//         int x = j * viewport_width * 3;
//         int y = (viewport_height - 1 - j) * viewport_width * 3;
//         for (int i = viewport_width * 3; i > 0; --i) {
//             std::swap(image_uv_data[x], image_uv_data[y]);
//             ++x;
//             ++y;
//         }
//     }
//
//     // 保存为NPY格式
//     std::vector<float> data;
//     for (int j = 0; j < viewport_height; j++) {
//         for (int i = 0; i < viewport_width; i++) {
//             int t = j * viewport_width * 3 + i * 3;
//             data.push_back(image_uv_data[t]);
//             data.push_back(image_uv_data[t + 1]);
//         }
//     }
//
//     cnpy::npy_save(filename, &data[0], { (unsigned long)viewport_height, (unsigned long)viewport_width, 2 }, "w");
//     delete[] image_uv_data;
// }
//
// // 保存截图
// void saveScreenshot(const char* filename) {
//     GLubyte* data = new GLubyte[viewport_height * viewport_width * 3];
//
//     glReadBuffer(GL_COLOR_ATTACHMENT0);
//     glReadPixels(0, 0, viewport_width, viewport_height, GL_RGB, GL_UNSIGNED_BYTE, data);
//
//     // 反转像素
//     for (int j = 0; j * 2 < viewport_height; ++j) {
//         int x = j * viewport_width * 3;
//         int y = (viewport_height - 1 - j) * viewport_width * 3;
//         for (int i = viewport_width * 3; i > 0; --i) {
//             std::swap(data[x], data[y]);
//             ++x;
//             ++y;
//         }
//     }
//
//     // 使用stb_image_write保存为PNG
//     int saved = stbi_write_png(filename, viewport_width, viewport_height, 3, data, 0);
//     delete[] data;
// }
//
// // 错误回调函数
void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// 窗口大小改变回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;
    glViewport(0, 0, width, height);
}

int main(void) {
    // 设置GLFW错误回调
    glfwSetErrorCallback(error_callback);

    // 初始化GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // 配置GLFW
    glfwWindowHint(GLFW_SAMPLES, 4);                           // 4x抗锯齿
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);             // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);       // 向前兼容
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
        object_id == 0 ? "Basketball Display" : "Vase Display", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 设置窗口大小变化回调
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 加载OpenGL函数指针（通过GLAD）
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 设置输入模式
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);


    // 加载并编译着色器
    Shader modelShader("shaders/model.vert", "shaders/model.frag");
    // Shader backgroundShader("shaders/background.vert", "shaders/background.frag");
    // Shader simpleDepthShader("shaders/shadow_depth.vert", "shaders/shadow_depth.frag");

    // 加载纹理
    int width, height, nrChannels;
    unsigned char* data;
    std::string texturePath = object_id == 0 ?
        "models/basketball/NBA BASKETBALL DIFFUSE.jpg" :
        "models/vase/Vase-obj_0.jpg";
	// std::cout << texturePath.c_str() << std::endl;
    data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
    if (data)
    {
        printf("Texture width: %d, height: %d, channels: %d\n", width, height, nrChannels);
    }
    else
    {
		printf("Failed to load texture\n");
		return -1;
    }

    // 创建纹理对象
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(data);

    // 加载模型
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    
    std::string modelPath = object_id == 0 ?
        "models/basketball/basketball.obj" :
        "models/vase/Vase-obj2.obj";
    
    loadOBJ(modelPath.c_str(), vertices, uvs, normals);
    long vertices_size = vertices.size();
	if (vertices_size == 0) {
		printf("Failed to load model: %s\n", modelPath.c_str());
		return -1;
	}
    printf("Vertices: %zu, UVs: %zu, Normals: %zu\n", vertices.size(), uvs.size(), normals.size());

    // 创建VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    // 创建顶点缓冲对象
    unsigned int vertexbuffer, uvbuffer, normalbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);

    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    
    // 设置顶点属性
    glBindVertexArray(VAO);
    
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBindVertexArray(0);
    
    // 设置帧缓冲对象用于输出
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    // 创建颜色输出纹理
    unsigned int color;
    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, viewport_width, viewport_height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    
    // 创建渲染缓冲对象用于深度和模板测试
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport_width, viewport_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    // 创建UV输出纹理
    unsigned int image_uv;
    glGenTextures(1, &image_uv);
    glBindTexture(GL_TEXTURE_2D, image_uv);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, viewport_width, viewport_height, 0, GL_RGB, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, image_uv, 0);
    
    // 创建视图法线输出纹理
    unsigned int view_normal;
    glGenTextures(1, &view_normal);
    glBindTexture(GL_TEXTURE_2D, view_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, viewport_width, viewport_height, 0, GL_RGB, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, view_normal, 0);
    
    // 指定绘制缓冲
    unsigned int DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    // // 初始化背景
    // unsigned int bgVAO, bgVBO;
    // initBackground(bgVAO, bgVBO);
    //
    // 配置光照
    Light light;
    light.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, 0));
    light.ambient = glm::vec3(0.1, 0.1, 0.1);
    light.diffuse = glm::vec3(0.6, 0.6, 0.6);
    light.specular = glm::vec3(0.3, 0.3, 0.3);
    
    // // 创建阴影映射帧缓冲
    // unsigned int depthMapFBO;
    // glGenFramebuffers(1, &depthMapFBO);
    // glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    //
    // // 创建深度贴图
    // unsigned int depthMap;
    // glGenTextures(1, &depthMap);
    // glBindTexture(GL_TEXTURE_2D, depthMap);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewport_width, viewport_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    // glDrawBuffer(GL_NONE);
    // glReadBuffer(GL_NONE);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    // 设置投影矩阵
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)viewport_width / (float)viewport_height, 1.0f, 4000.0f);
    
    // 设置模型矩阵
    glm::vec3 myRotationAxis(1, 0, 0);
    glm::mat4 translateMatrix;
    if (object_id == 0) {
        translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 230.0f, 0.0f));
    }
    else {
        translateMatrix = glm::mat4(1.0f);
    }
    
    glm::mat4 rotateMatrix = glm::rotate(-1.570796f, myRotationAxis);
    glm::vec3 yAxis(0, 1, 0);
    
    // 启用深度测试
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glClearColor(background_color.r, background_color.g, background_color.b, 1.0f);
    //
    // // 创建输出目录
    // std::filesystem::create_directories("output/uv");
    std::filesystem::create_directories("output/frame");
    // std::filesystem::create_directories("output/extrinsics");
    //
    // 主渲染循环
    char file_name[256];
    for (int z = 0; z < total_frame && !glfwWindowShouldClose(window); z++) {
        // 更新模型矩阵 - 围绕Y轴旋转相机
        glm::mat4 yrotateMatrix = glm::rotate(6.283184f / total_frame * z, yAxis);
        // float dis = 1500 - z * 0.25;
        // glm::vec4 camerapos = yrotateMatrix * glm::vec4(dis * 0.5f, dis * 0.866, 0, 1);


		glm::vec4 camerapos = glm::vec4(0, 0, 100, 1);
        glm::mat4 View = glm::lookAt(
            glm::vec3(camerapos),  // 相机位置
            glm::vec3(0, 0, 0),    // 看向原点
            glm::vec3(0, 1, 0)     // 上方向为y轴
        );
    
    // 光源空间矩阵
    float light_distance = 120;
    glm::mat4 lightProjection = glm::ortho(-2000.0f, 2000.0f, -2000.0f, 2000.0f, 1.0f, 4000.0f);
    glm::vec3 lightPosition = glm::vec3(1200, 1200, 0);
    glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    
    // 设置视口
    glViewport(0, 0, viewport_width, viewport_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    //     // 第一步 - 计算深度图（从光源角度）
    //     glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    //     glClear(GL_DEPTH_BUFFER_BIT);
    //
    //     simpleDepthShader.use();
    //     simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    //
    //     // 背景地面
    //     glm::mat4 BackgroundModel = glm::mat4(1.0f);
    //     simpleDepthShader.setMat4("model", BackgroundModel);
    //     glBindVertexArray(bgVAO);
    //     glDrawArrays(GL_TRIANGLES, 0, 6);
    //     glBindVertexArray(0);
    //
    // 物体（篮球或花瓶）
    // glm::mat4 myScalingMatrix;
    // if (object_id == 0) {
    //     myScalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f));
    // }
    // else {
    //     myScalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.9f, 0.9f, 0.9f));
    // }
    //
    // glm::mat4 Model = translateMatrix * rotateMatrix * myScalingMatrix;
    // // simpleDepthShader.setMat4("model", Model);
    // glBindVertexArray(VAO);
    // glDrawArrays(GL_TRIANGLES, 0, vertices_size);
    // glBindVertexArray(0);
    // //
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //
    //     // 第二步 - 渲染场景（从相机角度）
    if (enable_output) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glDrawBuffers(3, DrawBuffers);
    }
    
    glViewport(0, 0, viewport_width, viewport_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4 model = glm::mat4(1.f);
    // 绘制背景
    modelShader.use();
    modelShader.setMat4("view", View);
    modelShader.setMat4("projection", Projection);
    modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    modelShader.setVec3("light.direction", light.direction);
    modelShader.setVec3("light.ambient", light.ambient);
    modelShader.setVec3("light.diffuse", light.diffuse);
    modelShader.setVec3("light.specular", light.specular);
    modelShader.setMat4("model", model);
    modelShader.setInt("object_id", object_id);
    
    // 绑定纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    modelShader.setInt("myTextureSampler", 0);
    
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, depthMap);
    // ourShader.setInt("shadowMap", 1);
    //
    //     // 绘制背景
    //     glBindVertexArray(bgVAO);
    //     glDrawArrays(GL_TRIANGLES, 0, 6);
    //     glBindVertexArray(0);
    //
    // 绘制物体
    // modelShader.setMat4("model", Model);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    modelShader.setInt("myTextureSampler", 0);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices_size);
    glBindVertexArray(0);
    
    //     // 输出渲染结果
    //     if (enable_output) {
    //         // 截图
    //         snprintf(file_name, sizeof(file_name), screenshot_output_dir.c_str(), z);
    //         saveScreenshot(file_name);
    //
    //         // UV坐标
    //         snprintf(file_name, sizeof(file_name), uv_output_dir.c_str(), z);
    //         saveImageUV(file_name);
    //
    //         // 相机外参
    //         snprintf(file_name, sizeof(file_name), camera_extrinsics_output_dir.c_str(), z);
    //         saveCameraExtrinsics(file_name, Model, glm::vec3(camerapos));
    //     }
    //
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // 交换缓冲区
    glfwSwapBuffers(window);
    glfwPollEvents();
    //
    // 检查ESC键是否被按下
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        break;
    }
    }
    
    // 清理资源
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &texture);
    glDeleteTextures(1, &image_uv);
    glDeleteTextures(1, &color);
    glDeleteTextures(1, &view_normal);
    // glDeleteFramebuffers(1, &depthMapFBO);
    // glDeleteTextures(1, &depthMap);
    // glDeleteBuffers(1, &bgVBO);
    glDeleteVertexArrays(1, &VAO);
    // glDeleteVertexArrays(1, &bgVAO);
    
    // 关闭窗口并终止GLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}