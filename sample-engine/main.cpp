#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "shader.h"
#include "assimp/scene.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <iostream>
#include <random>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

class sceneRenderer {
    const aiScene* scene;
    unsigned int numVertices;
    float* vertices;
    unsigned int numIndices;
    unsigned int* indices;
    bool once = false;
    unsigned int VBO, VAO, EBO;

public:
    sceneRenderer(const aiScene* scene) {
        this->scene = scene;
    }

    void drawMesh(size_t ind) {
        if (!once) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, 1.0f);

            numVertices = scene->mMeshes[ind]->mNumVertices;
            vertices = new float[scene->mMeshes[ind]->mNumVertices * 6];
            for (size_t i = 0; i < numVertices; i++) {
                vertices[i * 6] = scene->mMeshes[ind]->mVertices[i][0] / 2;
                vertices[i * 6 + 1] = scene->mMeshes[ind]->mVertices[i][1] / 2;
                vertices[i * 6 + 2] = scene->mMeshes[ind]->mVertices[i][2] / 2;
                vertices[i * 6 + 3] = dis(gen);
                vertices[i * 6 + 4] = dis(gen);
                vertices[i * 6 + 5] = dis(gen);
            }

            numIndices = scene->mMeshes[ind]->mNumFaces;
            indices = new unsigned int[numIndices * 3];
            for (size_t i = 0; i < numIndices; i++) {
                indices[i * 3 + 2] = scene->mMeshes[ind]->mFaces[i].mIndices[0];
                indices[i * 3 + 1] = scene->mMeshes[ind]->mFaces[i].mIndices[1];
                indices[i * 3 + 0] = scene->mMeshes[ind]->mFaces[i].mIndices[2];
            }
        
            
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(VAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * numVertices * 6, vertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int ) * numIndices * 3, indices, GL_STATIC_DRAW);

            // position attribute
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            // color attribute
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            once = true;
        }
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, numIndices * 3, GL_UNSIGNED_INT, 0);
    }

};

int main()
{
    const aiScene* scene = aiImportFile("resources/Barbarian.fbx", aiProcessPreset_TargetRealtime_MaxQuality);
    sceneRenderer rendere(scene);
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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

    float vertices[] = {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
    };

    std::cout << sizeof(vertices);


    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.use();
        rendere.drawMesh(0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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