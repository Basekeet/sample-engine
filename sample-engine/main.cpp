#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include "shader.h"
#include "assimp/scene.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <random>
#include <vector>
#include <queue>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

typedef unsigned int ui32;
typedef float f32;
typedef size_t szt;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


class sceneRenderer {
    const aiScene* scene;

    ui32 numMeshes;

    ui32* numVertices;
    f32** vertices;

    ui32* numIndices;
    ui32** indices;
   
    ui32* VBO;
    ui32* VAO;
    ui32* EBO;

    ui32 texture;

    Shader* shader;
public:
    sceneRenderer(const aiScene* scene, Shader* shader) {
        this->shader = shader;

        this->scene = scene;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);

        numMeshes = scene->mNumMeshes;

        numVertices = new ui32[numMeshes];
        vertices = new f32 * [numMeshes];
        numIndices = new ui32[numMeshes];
        indices = new ui32 * [numMeshes];

        VBO = new ui32[numMeshes];
        VAO = new ui32[numMeshes];
        EBO = new ui32[numMeshes];

        glGenVertexArrays(numMeshes, VAO);
        glGenBuffers(numMeshes, VBO);
        glGenBuffers(numMeshes, EBO);

        for (size_t i = 0; i < numMeshes; i++) {
            numVertices[i] = scene->mMeshes[i]->mNumVertices;
            vertices[i] = new float[scene->mMeshes[i]->mNumVertices * 8];
            for (size_t j = 0; j < numVertices[i]; j++) {
                vertices[i][j * 8] = scene->mMeshes[i]->mVertices[j][0];
                vertices[i][j * 8 + 1] = scene->mMeshes[i]->mVertices[j][1];
                vertices[i][j * 8 + 2] = scene->mMeshes[i]->mVertices[j][2];
                vertices[i][j * 8 + 3] = 0;
                vertices[i][j * 8 + 4] = 0;
                vertices[i][j * 8 + 5] = 0;
                vertices[i][j * 8 + 6] = scene->mMeshes[i]->mTextureCoords[0][j].x;
                vertices[i][j * 8 + 7] = scene->mMeshes[i]->mTextureCoords[0][j].y;
            }

            numIndices[i] = scene->mMeshes[i]->mNumFaces;
            indices[i] = new unsigned int[numIndices[i] * 3];
            for (size_t j = 0; j < numIndices[i]; j++) {
                indices[i][j * 3 + 0] = scene->mMeshes[i]->mFaces[j].mIndices[0];
                indices[i][j * 3 + 1] = scene->mMeshes[i]->mFaces[j].mIndices[1];
                indices[i][j * 3 + 2] = scene->mMeshes[i]->mFaces[j].mIndices[2];
            }

            glBindVertexArray(VAO[i]);

            glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices[i] * 8, vertices[i], GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * numIndices[i] * 3, indices[i], GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
        }

        
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

        int width, height, nrChannels;

        unsigned char* data = stbi_load("C:\\Users\\Uer\\Desktop\\sample-engine\\x64\\Debug\\resources\\barbarian_texture.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);

        shader->use();
        shader->setInt("ourTexture", 0);
    }

    void drawMeshRec(aiNode* node, glm::mat4 trans) {
        aiMatrix4x4 lt = node->mTransformation;
        lt.Transpose();
        glm::mat4 localTrans;
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                localTrans[i][j] = lt[i][j];
            }
        }
        
        trans = trans * localTrans;
        unsigned int transformLoc = glGetUniformLocation(shader->ID, "transform");

        for (size_t i = 0; i < node->mNumMeshes; i++) {
            
            glUniformMatrix4fv(transformLoc, 1, GL_FALSE, (float*)&trans);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture);
            shader->use();
            glBindVertexArray(VAO[node->mMeshes[i]]);
            glDrawElements(GL_TRIANGLES, numIndices[node->mMeshes[i]] * 3, GL_UNSIGNED_INT, 0);
        }

        for (size_t i = 0; i < node->mNumChildren; i++) {
            drawMeshRec(node->mChildren[i], trans);
        }
    }

    void drawMesh() {
        shader->use();
        drawMeshRec(scene->mRootNode, glm::scale(glm::rotate(glm::mat4(1.0f), (float)glfwGetTime() / 10, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(0.5f, 0.5f, 0.5f)));
    }
};

int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader ourShader("resources/shader.vs", "resources/shader.fs"); 

    const aiScene* scene = aiImportFile("resources/Barbarian.fbx", aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    sceneRenderer rendere(scene, &ourShader);

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rendere.drawMesh();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}