#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>


// GLAD ������ GLFW ֮ǰ����
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM ��ѧ��
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>

// ��Ŀ�����ͷ�ļ�
#include "shader.hpp"
// #include "objLoader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#include "cnpy/cnpy.h"
#include "model.hpp"
#include "camera.hpp"

// ���ں��ӿڳߴ�
int window_width = 1024;
int window_height = 1024;
int viewport_width = 1024;
int viewport_height = 1024;

// ��Ⱦ����
bool enable_output = false;  // ����Ϊ true ��������Ⱦ������ļ�
int total_frame = 200;      // ��Ⱦ֡��
glm::vec3 background_color = glm::vec3(0.3f, 0.3f, 0.3f);
int object_id = 1;           // 0 ��ʾ����1 ��ʾ��ƿ
Camera camera(glm::vec3(0, 0, 15), glm::vec3(0, 1, 0), -90.0f, 0.0f);;
float lastX = window_width / 2.0f;
float lastY = window_height / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// ���·��
std::string uv_output_dir = "output/mars8/uv/%04d.npy";
std::string screenshot_output_dir = "output/mars8/frame/%04d.png";
std::string camera_extrinsics_output_dir = "output/mars8/extrinsics/%04d.npy";

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// ���ն���
struct Light {
	glm::vec3 direction;  // ��׼���ķ���
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
};

float quad_vertices[] = {
    -1.f, 1.f, 0.f, 0.f, 1.f,
    -1.f, -1.f, 0.f, 0.f, 0.f,
    1.f, -1.f, 0.f, 1.f, 0.f,
    1.f, -1.f, 0.f, 1.f, 0.f,
    1.f, 1.f, 0.f, 1.f, 1.f,
    -1.f, 1.f, 0.f, 0.f, 1.f
};

// ��ʼ�����������棩��������
// void initBackground(unsigned int& bgVAO, unsigned int& bgVBO) {
//     // �������ݰ��������� - ���� - UV
//     float vertices_basketball[] = {
//         // ����
//         -2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  0.0f, 0.0f,
//          2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  0.08f, 0.0f,
//          2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.04f, 0.08f,
//          2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.04f, 0.08f,
//         -2500.0f, 0.0f,  2500.0f,  0.0f, 2500.0f, 0.0f,  0.08f, 0.0f,
//         -2500.0f, 0.0f, -2500.0f,  0.0f, 2500.0f, 0.0f,  0.0f, 0.0f
//     };
//
//     float vertices_vase[] = {
//         // ����
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
//     // ��仺����
//     glBindBuffer(GL_ARRAY_BUFFER, bgVBO);
//     if (object_id == 0) {
//         glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_basketball), vertices_basketball, GL_STATIC_DRAW);
//     }
//     else if (object_id == 1) {
//         glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_vase), vertices_vase, GL_STATIC_DRAW);
//     }
//
//     // ���Ӷ�������
//     // ����
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)0);
//     // UV
//     glEnableVertexAttribArray(1);
//     glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(6 * sizeof(float)));
//     // ����
//     glEnableVertexAttribArray(2);
//     glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(3 * sizeof(float)));
//
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
// }
//

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// ������ͼ����ͼ��
void saveImageViewNormal(const char* filename) {
    GLfloat* image_view_normal_data = new GLfloat[viewport_height * viewport_width * 3];
    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glReadPixels(0, 0, viewport_width, viewport_height, GL_RGB, GL_FLOAT, image_view_normal_data);

    // ��ת���أ���ΪOpenGL��ͼ������ϵ��ͬ��
    for (int j = 0; j * 2 < viewport_height; ++j) {
        int x = j * viewport_width * 3;
        int y = (viewport_height - 1 - j) * viewport_width * 3;
        for (int i = viewport_width * 3; i > 0; --i) {
            std::swap(image_view_normal_data[x], image_view_normal_data[y]);
            ++x;
            ++y;
        }
    }

     // ����ΪNPY��ʽ
     std::vector<float> data;
     for (int j = 0; j < viewport_height; j++) {
         for (int i = 0; i < viewport_width; i++) {
             int t = j * viewport_width * 3 + i * 3;
             data.push_back(image_view_normal_data[t]);
             data.push_back(image_view_normal_data[t + 1]);
             data.push_back(image_view_normal_data[t + 2]);
         }
     }

    // ʹ��cnpy�Ᵽ��Ϊnpy��ʽ
    cnpy::npy_save(filename, &data[0], { (unsigned long)viewport_height, (unsigned long)viewport_width, 3 }, "w");
    delete[] image_view_normal_data;
}

// ����������
glm::vec3 saveCameraExtrinsics(const char* filename, glm::mat4 model, glm::vec3 cameraPos) {
    glm::vec3 extrinsics = normalize(glm::vec3(model * glm::vec4(cameraPos, 1.0)));
    float data[] = { extrinsics.x, extrinsics.y, extrinsics.z };
    cnpy::npy_save(filename, &data[0], { 3 }, "w");
    return extrinsics;
}

// ����UV��������
void saveImageUV(const char* filename) {
    GLfloat* image_uv_data = new GLfloat[viewport_height * viewport_width * 3];

    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(0, 0, viewport_width, viewport_height, GL_RGB, GL_FLOAT, image_uv_data);

    // ��ת����
    for (int j = 0; j * 2 < viewport_height; ++j) {
        int x = j * viewport_width * 3;
        int y = (viewport_height - 1 - j) * viewport_width * 3;
        for (int i = viewport_width * 3; i > 0; --i) {
            std::swap(image_uv_data[x], image_uv_data[y]);
            ++x;
            ++y;
        }
    }

    // ����ΪNPY��ʽ
    std::vector<float> data;
    for (int j = 0; j < viewport_height; j++) {
        for (int i = 0; i < viewport_width; i++) {
            int t = j * viewport_width * 3 + i * 3;
            data.push_back(image_uv_data[t]);
            data.push_back(image_uv_data[t + 1]);
        }
    }

    cnpy::npy_save(filename, &data[0], { (unsigned long)viewport_height, (unsigned long)viewport_width, 2 }, "w");
    delete[] image_uv_data;
}

// �����ͼ
void saveScreenshot(const char* filename) {
    GLubyte* data = new GLubyte[viewport_height * viewport_width * 3];

    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, viewport_width, viewport_height, GL_RGB, GL_UNSIGNED_BYTE, data);

    // ��ת����
    for (int j = 0; j * 2 < viewport_height; ++j) {
        int x = j * viewport_width * 3;
        int y = (viewport_height - 1 - j) * viewport_width * 3;
        for (int i = viewport_width * 3; i > 0; --i) {
            std::swap(data[x], data[y]);
            ++x;
            ++y;
        }
    }

    // ʹ��stb_image_write����ΪPNG
    int saved = stbi_write_png(filename, viewport_width, viewport_height, 3, data, 0);
    delete[] data;
}

// // ����ص�����
void error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// ���ڴ�С�ı�ص�
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    viewport_width = width;
    viewport_height = height;
    glViewport(0, 0, width, height);
}

int main(void) {
    // ����GLFW����ص�
    glfwSetErrorCallback(error_callback);

    // ��ʼ��GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // ����GLFW
    // glfwWindowHint(GLFW_SAMPLES, 4);                           // 4x�����
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);             // OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);       // ��ǰ����
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // ��������
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
        object_id == 0 ? "Basketball Display" : "Vase Display", NULL, NULL);

    if (window == NULL) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    // ���ô��ڴ�С�仯�ص�

    // ����OpenGL����ָ�루ͨ��GLAD��
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glViewport(0, 0, window_width, window_height);
    // glfwSetCursorPosCallback(window, mouse_callback);
    // glfwSetScrollCallback(window, scroll_callback);

    // ��������ģʽ
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

    stbi_set_flip_vertically_on_load(true);
    // ���ز�������ɫ��
    Shader modelShader("shaders/model.vert", "shaders/model.frag");
	Shader quadShader("shaders/quad.vert", "shaders/quad.frag");
    // Shader backgroundShader("shaders/background.vert", "shaders/background.frag");
    // Shader simpleDepthShader("shaders/shadow_depth.vert", "shaders/shadow_depth.frag");

    // ��������
 //    int width, height, nrChannels;
 //    unsigned char* data;
 //    std::string texturePath = object_id == 0 ?
 //        "models/basketball/NBA BASKETBALL DIFFUSE.jpg" :
 //        "models/vase/Vase-obj_0.jpg";
	// texturePath = "models/planet/mars.png";
	// // std::cout << texturePath.c_str() << std::endl;
 //    data = stbi_load(texturePath.c_str(), &width, &height, &nrChannels, 0);
 //    if (data)
 //    {
 //        printf("Texture width: %d, height: %d, channels: %d\n", width, height, nrChannels);
 //    }
 //    else
 //    {
	// 	printf("Failed to load texture\n");
	// 	return -1;
 //    }
  //   for (int i = 0; i < 10; ++i)
  //   {
		// std::cout << data[i * 3] << " " << data[i * 3 + 1] << " " << data[i * 3 + 2] << std::endl;
  //   }

    // �����������
    // unsigned int texture;
    // glGenTextures(1, &texture);
    // glBindTexture(GL_TEXTURE_2D, texture);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // stbi_image_free(data);

    // ����ģ��
 //    std::vector<glm::vec3> vertices;
 //    std::vector<glm::vec2> uvs;
 //    std::vector<glm::vec3> normals;
 //    
 //    std::string modelPath = object_id == 0 ?
 //        "models/basketball/basketball.obj" :
 //        "models/vase/Vase-obj2.obj";
	// modelPath = "models/planet/planet.obj";
 //    loadOBJ(modelPath.c_str(), vertices, uvs, normals);
 //    long vertices_size = vertices.size();
	// if (vertices_size == 0) {
	// 	printf("Failed to load model: %s\n", modelPath.c_str());
	// 	return -1;
	// }
 //    printf("Vertices: %zu, UVs: %zu, Normals: %zu\n", vertices.size(), uvs.size(), normals.size());

    // ����VAO
    // unsigned int model_VAO;
    // glGenVertexArrays(1, &model_VAO);
    // // �������㻺�����
    // unsigned int vertexbuffer, uvbuffer, normalbuffer;
    // glGenBuffers(1, &vertexbuffer);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    //
    // glGenBuffers(1, &uvbuffer);
    // glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    // glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);
    //
    // glGenBuffers(1, &normalbuffer);
    // glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    // glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    //
    // // ���ö�������
    // glBindVertexArray(model_VAO);
    //
    // glEnableVertexAttribArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //
    // glEnableVertexAttribArray(1);
    // glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    // glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //
    // glEnableVertexAttribArray(2);
    // glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    // glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    //
    // glBindVertexArray(0);
    std::string modelPath = "models/planet/planet.obj";
	// modelPath = "models/basketball/basketball.obj";
    // modelPath = "models/rock/rock.obj";
    // modelPath = "models/Vase/Vase-obj.obj";
    Model mars(modelPath.c_str());
    // std::cout << mars.textures_loaded.size();
    
    
    // ����֡��������������
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    // ������ɫ�������
    unsigned int color;
    glGenTextures(1, &color);
    glBindTexture(GL_TEXTURE_2D, color);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, viewport_width, viewport_height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    
    // ������Ⱦ�������������Ⱥ�ģ�����
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, viewport_width, viewport_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    
    // ����UV�������
    unsigned int image_uv;
    glGenTextures(1, &image_uv);
    glBindTexture(GL_TEXTURE_2D, image_uv);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, viewport_width, viewport_height, 0, GL_RGB, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, image_uv, 0);
    
    // ������ͼ�����������
    unsigned int view_normal;
    glGenTextures(1, &view_normal);
    glBindTexture(GL_TEXTURE_2D, view_normal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, viewport_width, viewport_height, 0, GL_RGB, GL_FLOAT, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, view_normal, 0);
    
	// ���֡��������Ƿ�����
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Framebuffer not complete!" << std::endl;
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

    // ָ�����ƻ���
    unsigned int DrawBuffers[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
	glDrawBuffers(3, DrawBuffers);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int quad_VAO, quad_buffer;
    glGenVertexArrays(1, &quad_VAO);
    glGenBuffers(1, &quad_buffer);
    glBindVertexArray(quad_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindBuffer(GL_ARRAY_BUFFER, 0);

    //
    // // ��ʼ������
    // unsigned int bgVAO, bgVBO;
    // initBackground(bgVAO, bgVBO);
    //
    // ���ù���
    Light light;
    light.direction = glm::normalize(glm::vec3(-1.0f, -1.0f, 0));
    light.ambient = glm::vec3(0.6, 0.6, 0.6);
    light.diffuse = glm::vec3(0.6, 0.6, 0.6);
    light.specular = glm::vec3(0.6, 0.6, 0.6);
    
    // // ������Ӱӳ��֡����
    // unsigned int depthMapFBO;
    // glGenFramebuffers(1, &depthMapFBO);
    // glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    //
    // // ���������ͼ
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
    
    
    // ����ģ�;���
    // glm::vec3 myRotationAxis(1, 0, 0);
    // glm::mat4 translateMatrix;
    // if (object_id == 0) {
    //     translateMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 230.0f, 0.0f));
    // }
    // else {
    //     translateMatrix = glm::mat4(1.0f);
    // }
    //
    // glm::mat4 rotateMatrix = glm::rotate(-1.570796f, myRotationAxis);
    // glm::vec3 yAxis(0, 1, 0);
    
    
    
    // �������Ŀ¼
    std::filesystem::create_directories("output/mars8/uv");
    std::filesystem::create_directories("output/mars8/frame");
    std::filesystem::create_directories("output/mars8/extrinsics");
    
    
    // ����Ⱦѭ��
    char file_name[256];
    for (int z = 0; z < total_frame && !glfwWindowShouldClose(window); z++){
        // ����ģ�;��� - Χ��Y����ת���
        glm::vec3 yAxis = glm::vec3(0, 1, 0);
		glm::vec3 xAxis = glm::vec3(1, 0, 0);
        glm::mat4 yrotateMatrix = glm::rotate((float)glfwGetTime() * 0.5f, yAxis);
        glm::mat4 xrotateMatrix = glm::rotate((float)glfwGetTime() * 0.5f, xAxis);
        glm::vec3 camerapos(0, 0, 0);
	    // if (z < total_frame / 2)
        if (true)
	    {  
	       camerapos = yrotateMatrix * glm::vec4(0, 0, 15, 1);  
		   camera.Position = camerapos;
	       camera.Front = glm::normalize(-camera.Position); // ���ʼ�ն���ԭ��  
	    }  
	    else  
	    {  
	       camerapos = xrotateMatrix * glm::vec4(0, 0, 15, 1);  
	       camera.Position = glm::vec3(camerapos);  
	       camera.Front = glm::normalize(-camera.Position); // ���ʼ�ն���ԭ��  
	    }
        
        // float dis = 1500 - z * 0.25;
        // glm::vec4 camerapos = yrotateMatrix * glm::vec4(dis * 0.5f, dis * 0.866, 0, 1);
        
		// camera = Camera (glm::vec3(camerapos), glm::vec3(0, 1, 0), -90.0f, 0.0f);
        // float currentFrame = static_cast<float>(glfwGetTime());
        // deltaTime = currentFrame - lastFrame;
        // lastFrame = currentFrame;
        // processInput(window);
        // camerapos = glm::vec4(0, 0, 15, 1);
        // camera = Camera (glm::vec3(camerapos), glm::vec3(0, 1, 0), -90.0f, 0.0f);
        // glm::mat4 View = glm::lookAt(
        //     glm::vec3(camerapos),  // ���λ��
        //     glm::vec3(0, 0, 0),    // ����ԭ��
        //     glm::vec3(0, 1, 0)     // �Ϸ���Ϊy��
        // );

		
        // ����ͶӰ����
        glm::mat4 Projection = glm::perspective(glm::radians(45.0f), (float)window_width / (float)window_height, 1.0f, 4000.0f);
        // ��Դ�ռ����
        float light_distance = 1200;
        glm::mat4 lightProjection = glm::ortho(-2000.0f, 2000.0f, -2000.0f, 2000.0f, 1.0f, 4000.0f);
        glm::vec3 lightPosition = glm::vec3(1200, 1200, 0);
        glm::mat4 lightView = glm::lookAt(lightPosition, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightSpaceMatrix = lightProjection * lightView;
    
        
        
    //
    //     // ��һ�� - �������ͼ���ӹ�Դ�Ƕȣ�
    //     glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    //     glClear(GL_DEPTH_BUFFER_BIT);
    //
    //     simpleDepthShader.use();
    //     simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
    //
    //     // ��������
    //     glm::mat4 BackgroundModel = glm::mat4(1.0f);
    //     simpleDepthShader.setMat4("model", BackgroundModel);
    //     glBindVertexArray(bgVAO);
    //     glDrawArrays(GL_TRIANGLES, 0, 6);
    //     glBindVertexArray(0);
    //
    // ���壨�����ƿ��
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
    //     // �ڶ��� - ��Ⱦ������������Ƕȣ�
        // if (enable_output) {
        //     glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        //     glDrawBuffers(3, DrawBuffers);
        // }
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(background_color.r, background_color.g, background_color.b, 1.0f);
		glDrawBuffers(3, DrawBuffers);
        glViewport(0, 0, viewport_width, viewport_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // ������Ȳ���
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
	    glm::mat4 model = glm::mat4(1.f);

        // ���Ʊ���
        modelShader.use();
        modelShader.setMat4("view", camera.GetViewMatrix());
        modelShader.setMat4("projection", Projection);
        modelShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
        modelShader.setVec3("light.direction", light.direction);
        modelShader.setVec3("light.ambient", light.ambient);
        modelShader.setVec3("light.diffuse", light.diffuse);
        modelShader.setVec3("light.specular", light.specular);
        modelShader.setMat4("model", model);
        modelShader.setInt("object_id", object_id);

        mars.Draw(modelShader);
    
        // ������
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture);
        // modelShader.setInt("myTextureSampler", 0);
    
    // glActiveTexture(GL_TEXTURE1);
    // glBindTexture(GL_TEXTURE_2D, depthMap);
    // ourShader.setInt("shadowMap", 1);
    //
    //     // ���Ʊ���
    //     glBindVertexArray(bgVAO);
    //     glDrawArrays(GL_TRIANGLES, 0, 6);
    //     glBindVertexArray(0);
    //
    // ��������
    // modelShader.setMat4("model", Model);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture);
        // modelShader.setInt("myTextureSampler", 0);
    
        // glBindVertexArray(model_VAO);
        //
        // glDrawArrays(GL_TRIANGLES, 0, vertices_size);
        // glBindVertexArray(0);
    
        // �����Ⱦ���
        if (enable_output) {
            // ��ͼ
            snprintf(file_name, sizeof(file_name), screenshot_output_dir.c_str(), z);
            saveScreenshot(file_name);
            
            // UV����
            snprintf(file_name, sizeof(file_name), uv_output_dir.c_str(), z);
            saveImageUV(file_name);
    
            // ������
            snprintf(file_name, sizeof(file_name), camera_extrinsics_output_dir.c_str(), z);
            saveCameraExtrinsics(file_name, model, glm::vec3(camera.Position));
        }
    
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        // �����ӿ�
        glViewport(0, 0, viewport_width, viewport_height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glClearColor(background_color.r, background_color.g, background_color.b, 1.0f);
        
        // glClear(GL_COLOR_BUFFER_BIT);
        quadShader.use();
		glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(quad_VAO);
		glBindTexture(GL_TEXTURE_2D, color);
        
        glDisable(GL_DEPTH_TEST);
        quadShader.setInt("textureSampler", 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
    
        // ����������
        glfwSwapBuffers(window);
        glfwPollEvents();
    
        // ���ESC���Ƿ񱻰���
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }
    }
    
    // ������Դ
    // glDeleteBuffers(1, &vertexbuffer);
    // glDeleteBuffers(1, &uvbuffer);
    // glDeleteBuffers(1, &normalbuffer);
    // glDeleteRenderbuffers(1, &rbo);
    // glDeleteFramebuffers(1, &fbo);
    // glDeleteTextures(1, &texture);
    // glDeleteTextures(1, &image_uv);
    // glDeleteTextures(1, &color);
    // glDeleteTextures(1, &view_normal);
    // glDeleteFramebuffers(1, &depthMapFBO);
    // glDeleteTextures(1, &depthMap);
    // glDeleteBuffers(1, &bgVBO);
    // glDeleteVertexArrays(1, &model_VAO);
    // glDeleteVertexArrays(1, &bgVAO);
    
    // �رմ��ڲ���ֹGLFW
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}