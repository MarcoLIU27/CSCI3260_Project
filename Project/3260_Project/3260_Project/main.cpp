/*
Student Information
Student ID: 1155141556 1155141497
Student Name: Liu Haoyu Yu zhihao
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"

#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/stb_image/stb_image.h"

#include "Shader.h"
#include "Texture.h"
#include <math.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <Cstdlib>
#include <Ctime>

#define STB_IMAGE_IMPLEMENTATION

// screen setting
const int SCR_WIDTH = 1200;
const int SCR_HEIGHT = 900;

GLuint programID;
GLuint vao[4];
GLuint vboID;
GLuint EBO;
GLuint indexBufferID;

float z_delta = 0.0f;
float x_delta = 0.0f;
float y_delta = 0.3f;
float z_current = 0.0f;
float x_current = 0.0f;
float z_random = 0.0f;
float x_random = 0.0f;
float r_delta = glm::radians(45.0f);
float delta = 0.2f;
int z_press_num = 0;
int x_press_num = 0;
int y_press_num = 0;
int rotate_num = 0;
int theme_tiger = 1;
int theme_ground = 1;
float zoom = 0.0f;
bool LEFT_BUTTON = false;
float sensitivity = 0.1;
float yaw = 0.0f;
float pitch = 0.0f;
bool firstMouse = true;
float lastX = 0.0f;
float lastY = 0.0f;
float view_y = 0.0f;
float intensity = 1.0f;



// struct for storing the obj file
struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

struct Model {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
};

Model planetobj;
Model spacecraftobj;
Model craftobj;
Model rockobj;

GLuint planettexture;
GLuint spacecrafttexture;
GLuint crafttexture;
GLuint rocktexture;


int* twoRandomNum()
{
    //generate two random int number between -2 and 2.
    int num[2];
    srand(time(NULL));
    num[0] = rand() % 9 - 4;
    srand(time(NULL)+1000);
    num[1] = rand() % 9 - 4;
    return num;
}

Model loadOBJ(const char* objPath)
{
    // function to load the obj file
    // Note: this simple function cannot load all obj files.

    struct V {
        // struct for identify if a vertex has showed up
        unsigned int index_position, index_uv, index_normal;
        bool operator == (const V& v) const {
            return index_position == v.index_position && index_uv == v.index_uv && index_normal == v.index_normal;
        }
        bool operator < (const V& v) const {
            return (index_position < v.index_position) ||
                (index_position == v.index_position && index_uv < v.index_uv) ||
                (index_position == v.index_position && index_uv == v.index_uv && index_normal < v.index_normal);
        }
    };

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_uvs;
    std::vector<glm::vec3> temp_normals;

    std::map<V, unsigned int> temp_vertices;

    Model model;
    unsigned int num_vertices = 0;

    std::cout << "\nLoading OBJ file " << objPath << "..." << std::endl;

    std::ifstream file;
    file.open(objPath);

    // Check for Error
    if (file.fail()) {
        std::cerr << "Impossible to open the file! Do you use the right path? See Tutorial 6 for details" << std::endl;
        exit(1);
    }

    while (!file.eof()) {
        // process the object file
        char lineHeader[128];
        file >> lineHeader;

        if (strcmp(lineHeader, "v") == 0) {
            // geometric vertices
            glm::vec3 position;
            file >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            // texture coordinates
            glm::vec2 uv;
            file >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            // vertex normals
            glm::vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            temp_normals.push_back(normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            // Face elements
            V vertices[3];
            for (int i = 0; i < 3; i++) {
                char ch;
                file >> vertices[i].index_position >> ch >> vertices[i].index_uv >> ch >> vertices[i].index_normal;
            }

            // Check if there are more than three vertices in one face.
            std::string redundency;
            std::getline(file, redundency);
            if (redundency.length() >= 5) {
                std::cerr << "There may exist some errors while load the obj file. Error content: [" << redundency << " ]" << std::endl;
                std::cerr << "Please note that we only support the faces drawing with triangles. There are more than three vertices in one face." << std::endl;
                std::cerr << "Your obj file can't be read properly by our simple parser :-( Try exporting with other options." << std::endl;
                exit(1);
            }

            for (int i = 0; i < 3; i++) {
                if (temp_vertices.find(vertices[i]) == temp_vertices.end()) {
                    // the vertex never shows before
                    Vertex vertex;
                    vertex.position = temp_positions[vertices[i].index_position - 1];
                    vertex.uv = temp_uvs[vertices[i].index_uv - 1];
                    vertex.normal = temp_normals[vertices[i].index_normal - 1];

                    model.vertices.push_back(vertex);
                    model.indices.push_back(num_vertices);
                    temp_vertices[vertices[i]] = num_vertices;
                    num_vertices += 1;
                }
                else {
                    // reuse the existing vertex
                    unsigned int index = temp_vertices[vertices[i]];
                    model.indices.push_back(index);
                }
            } // for
        } // else if
        else {
            // it's not a vertex, texture coordinate, normal or face
            char stupidBuffer[1024];
            file.getline(stupidBuffer, 1024);
        }
    }
    file.close();

    std::cout << "There are " << num_vertices << " vertices in the obj file.\n" << std::endl;
    return model;
}

void get_OpenGL_info() {
    // OpenGL information
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << std::endl;
    std::cout << "Renderer name: " << renderer << std::endl;
    std::cout << "OpenGL version: " << glversion << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

GLuint loadTexture(const char* texturePath) {
    // tell stb_image.h to flip the loaded texture on the y-axis.
    stbi_set_flip_vertically_on_load(true);
    // load the texture data into "data"
    int Width, Height, BPP;
    unsigned char* data = stbi_load(texturePath, &Width, &Height, &BPP, 0);
    GLenum format = 3;
    switch (BPP) {
    case 1: format = GL_RED; break;
    case 3: format = GL_RGB; break;
    case 4: format = GL_RGBA; break;
    }
    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);
    // "Bind" the newly created texture :
    // to indicate all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    // Give the image to OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, Width, Height, 0, GL_RGB,
        GL_UNSIGNED_BYTE, data);
    // OpenGL has now copied the data. Free our own version
    stbi_image_free(data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
        GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    std::cout << "Load " << texturePath << " successfully!" << std::endl;
    glBindTexture(GL_TEXTURE_2D, 0);
    return textureID;
}

void sendDataToOpenGL() {
    // planet
    planetobj = loadOBJ("resources/object/planet.obj");

    glGenVertexArrays(1, &vao[0]);
    glBindVertexArray(vao[0]);

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, planetobj.vertices.size() * sizeof(Vertex),
        &planetobj.vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planetobj.indices.size() * sizeof(unsigned int),
        &planetobj.indices[0], GL_STATIC_DRAW);

    // 1st attribute buffer : position
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    planettexture = loadTexture("resources/texture/earthTexture.bmp");

    // spacecraft
    spacecraftobj = loadOBJ("resources/object/spacecraft1.obj");

    glGenVertexArrays(1, &vao[1]);
    glBindVertexArray(vao[1]);

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, spacecraftobj.vertices.size() * sizeof(Vertex),
        &spacecraftobj.vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, spacecraftobj.indices.size() * sizeof(unsigned int),
        &spacecraftobj.indices[0], GL_STATIC_DRAW);

    // 1st attribute buffer : position
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    spacecrafttexture = loadTexture("resources/texture/spacecraftTexture.bmp");

    // craft
    craftobj = loadOBJ("resources/object/craft1.obj");

    glGenVertexArrays(1, &vao[2]);
    glBindVertexArray(vao[2]);

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, craftobj.vertices.size() * sizeof(Vertex),
        &craftobj.vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, craftobj.indices.size() * sizeof(unsigned int),
        &craftobj.indices[0], GL_STATIC_DRAW);

    // 1st attribute buffer : position
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    crafttexture = loadTexture("resources/texture/vehicleTexture.bmp");
    
    // rock
    rockobj = loadOBJ("resources/object/rock.obj");

    glGenVertexArrays(1, &vao[3]);
    glBindVertexArray(vao[3]);

    glGenBuffers(1, &vboID);
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glBufferData(GL_ARRAY_BUFFER, rockobj.vertices.size() * sizeof(Vertex),
        &rockobj.vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, rockobj.indices.size() * sizeof(unsigned int),
        &rockobj.indices[0], GL_STATIC_DRAW);

    // 1st attribute buffer : position
    glBindBuffer(GL_ARRAY_BUFFER, vboID);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, position) // array buffer offset
    );
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, // attribute
        2, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, uv) // array buffer offset
    );
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, // attribute
        3, // size
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        sizeof(Vertex), // stride
        (void*)offsetof(Vertex, normal) // array buffer offset
    );

    rocktexture = loadTexture("resources/texture/rockTexture.bmp");
}

bool checkStatus(
    GLuint objectID,
    PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
    PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
    GLenum statusType)
{
    GLint status;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE)
    {
        GLint infoLogLength;
        objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar* buffer = new GLchar[infoLogLength];

        GLsizei bufferSize;
        getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
        std::cout << buffer << std::endl;

        delete[] buffer;
        return false;
    }
    return true;
}

bool checkShaderStatus(GLuint shaderID) {
    return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
    return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
    std::ifstream meInput(fileName);
    if (!meInput.good()) {
        std::cout << "File failed to load ... " << fileName << std::endl;
        exit(1);
    }
    return std::string(
        std::istreambuf_iterator<char>(meInput),
        std::istreambuf_iterator<char>()
    );
}

void installShaders() {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* adapter[1];
    //adapter[0] = vertexShaderCode;
    std::string temp = readShaderCode("VertexShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(vertexShaderID, 1, adapter, 0);
    //adapter[0] = fragmentShaderCode;
    temp = readShaderCode("FragmentShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(fragmentShaderID, 1, adapter, 0);

    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);

    if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
        return;

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    if (!checkProgramStatus(programID))
        return;

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    glUseProgram(programID);

}

void initializedGL(void) {
    // run only once
    //TODO: set up the camera parameters	
    //TODO: set up the vertex shader and fragment shader
    glEnable(GL_DEPTH_TEST);
    installShaders();
    sendDataToOpenGL();
    
}

void paintGL(void) {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //specify the background color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);

    // planet
    glBindVertexArray(vao[0]);
    glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
    modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
    modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(0.0f, 0.0f, -15.0f));
    GLint modelTransformMatrixUniformLocation =
        glGetUniformLocation(programID, "modelTransformMatrix");
    glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1,
        GL_FALSE, &modelTransformMatrix[0][0]);

    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(0.0f, 0.0f, 30.0f + zoom),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));

    viewMatrix = glm::rotate(viewMatrix,
        glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    viewMatrix = glm::rotate(viewMatrix,
        glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    viewMatrix = glm::rotate(viewMatrix,
        glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    viewMatrix = glm::translate(viewMatrix, glm::vec3(0.0f, view_y, 0.0f));
    GLint viewMatrixUniformLocation =
        glGetUniformLocation(programID, "viewMatrix");
    glUniformMatrix4fv(viewMatrixUniformLocation, 1,
        GL_FALSE, &viewMatrix[0][0]);

    glm::mat4 projectionMatrix = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 100.0f);
    GLint projectionMatrixUniformLocation =
        glGetUniformLocation(programID, "projectionMatrix");
    glUniformMatrix4fv(projectionMatrixUniformLocation, 1,
        GL_FALSE, &projectionMatrix[0][0]);

    unsigned int slot = 0;
    GLuint TextureID = glGetUniformLocation(programID, "ourTexture");
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, planettexture);

    glUniform1i(TextureID, slot);

    // directional light
    GLint dirAmbientUniformLocation = glGetUniformLocation(programID, "dirLight.ambient");
    glm::vec3 dirAmbient(0.3f * intensity, 0.3f * intensity, 0.3f * intensity);
    glUniform3fv(dirAmbientUniformLocation, 1, &dirAmbient[0]);

    GLint dirPositionUniformLocation = glGetUniformLocation(programID, "dirLight.position");
    glm::vec3 dirPosition(-5.0f, -3.0f, -5.0f);
    glUniform4fv(dirPositionUniformLocation, 1, &dirPosition[0]);

    GLint dirDiffuseUniformLocation = glGetUniformLocation(programID, "dirLight.diffuse");
    glm::vec3 dirDiffuse(0.8f, 0.8f, 0.8f);
    glUniform3fv(dirDiffuseUniformLocation, 1, &dirDiffuse[0]);

    GLint dirSpecularUniformLocation = glGetUniformLocation(programID, "dirLight.specular");
    glm::vec3 dirSpecular(0.3f, 0.3f, 0.3f);
    glUniform3fv(dirSpecularUniformLocation, 1, &dirSpecular[0]);

    GLint eyePositionUniformLocation = glGetUniformLocation(programID, "eyePositionWorld");
    glm::vec3 eyePosition(0.0f, 0.0f, 20.0f + zoom);
    glUniform3fv(eyePositionUniformLocation, 1, &eyePosition[0]);

    //// point light
    //GLint pointAmbientUniformLocation = glGetUniformLocation(programID, "pointLight.ambient");
    //glm::vec3 pointAmbient(1.0f, 1.0f, 1.0f);
    //glUniform4fv(pointAmbientUniformLocation, 1, &pointAmbient[0]);

    //GLint pointPositionUniformLocation = glGetUniformLocation(programID, "pointLight.position");
    //glm::vec3 pointPosition(0.0f, 5.0f, 0.0f);
    //glUniform4fv(pointPositionUniformLocation, 1, &pointPosition[0]);

    //GLint pointDiffuseUniformLocation = glGetUniformLocation(programID, "pointLight.diffuse");
    //glm::vec3 pointDiffuse(0.8f, 0.8f, 0.8f);
    //glUniform3fv(pointDiffuseUniformLocation, 1, &pointDiffuse[0]);

    //GLint pointSpecularUniformLocation = glGetUniformLocation(programID, "pointLight.specular");
    //glm::vec3 pointSpecular(0.8f, 0.8f, 0.8f);
    //glUniform3fv(pointSpecularUniformLocation, 1, &pointSpecular[0]);

    glDrawElements(GL_TRIANGLES, planetobj.indices.size(),
        GL_UNSIGNED_INT, 0);

    // spacecraft
    glBindVertexArray(vao[1]);
    modelTransformMatrix = glm::mat4(1.0f);
    modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.1f, 0.1f, 0.1f));
    modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(0.0f, 0.0f, 40.0f));
    modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(x_current + x_press_num * x_delta * delta, 0.0f, 0.0f));
    modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(0.0f, 0.0f, z_current + z_press_num * z_delta * delta));
    modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(0.0f, y_press_num * y_delta, 0.0f));
    //modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(x_random, 0.0f, 0.0f));
    //modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(0.0f, 0.0f, z_random));
    //modelTransformMatrix = glm::rotate(modelTransformMatrix, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //modelTransformMatrix = glm::rotate(modelTransformMatrix, rotate_num * r_delta, glm::vec3(0.0f, 1.0f, 0.0f));

    modelTransformMatrixUniformLocation = glGetUniformLocation(programID, "modelTransformMatrix");
    glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1, GL_FALSE, &modelTransformMatrix[0][0]);

    TextureID = glGetUniformLocation(programID, "ourTexture");
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, spacecrafttexture);

    glUniform1i(TextureID, slot);

    glDrawElements(GL_TRIANGLES, spacecraftobj.indices.size(),
        GL_UNSIGNED_INT, 0);

    //craft
    glBindVertexArray(vao[2]);
    modelTransformMatrix = glm::mat4(1.0f);
    modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

    modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(-10.0f, 0.0f, -30.0f));
    modelTransformMatrixUniformLocation =
        glGetUniformLocation(programID, "modelTransformMatrix");
    glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1,
        GL_FALSE, &modelTransformMatrix[0][0]);

    TextureID = glGetUniformLocation(programID, "ourTexture");
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, crafttexture);
    glUniform1i(TextureID, slot);

    glDrawElements(GL_TRIANGLES, craftobj.indices.size(),
        GL_UNSIGNED_INT, 0); 

    //rock
    glBindVertexArray(vao[3]);
    modelTransformMatrix = glm::mat4(1.0f);
    modelTransformMatrix = glm::scale(modelTransformMatrix, glm::vec3(0.5f, 0.5f, 0.5f));

    modelTransformMatrix = glm::translate(modelTransformMatrix, glm::vec3(-20.0f, 0.0f, -20.0f));
    modelTransformMatrixUniformLocation =
        glGetUniformLocation(programID, "modelTransformMatrix");
    glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1,
        GL_FALSE, &modelTransformMatrix[0][0]);

    TextureID = glGetUniformLocation(programID, "ourTexture");
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, rocktexture);
    glUniform1i(TextureID, slot);

    glDrawElements(GL_TRIANGLES, rockobj.indices.size(),
        GL_UNSIGNED_INT, 0);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // Sets the mouse-button callback for the current window.	
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        LEFT_BUTTON = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        LEFT_BUTTON = false;
        firstMouse = true;
    }
}

void cursor_position_callback(GLFWwindow* window, double x, double y)
{
    // Sets the cursor position callback for the current window
    if (LEFT_BUTTON == true) {
        if (firstMouse == true)
        {
            lastX = x;
            lastY = y;
            firstMouse = false;
        }
        float xoffset = x - lastX;
        float yoffset = lastY - y;
        lastX = x;
        lastY = y;

        xoffset *= sensitivity;
        yoffset *= sensitivity;
        yaw += xoffset;
        pitch += yoffset;
        if (pitch > 30.0f)
            pitch = 30.0f;
        else if (pitch < -50.0f)
            pitch = -50.0f;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // Sets the scoll callback for the current window.
    if (zoom >= -15.0f && zoom <= 30.0f)
        zoom -= yoffset;
    else if (zoom >= 30.0f)
        zoom = 30.0f;
    else if (zoom <= -15.0f)
        zoom = -15.0f;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Sets the Keyboard callback for the current window.
    if (key == GLFW_KEY_1 && action == GLFW_PRESS)
    {
        theme_tiger = 1;
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS)
    {
        theme_tiger = 2;
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS)
    {
        theme_ground = 1;
    }
    if (key == GLFW_KEY_4 && action == GLFW_PRESS)
    {
        theme_ground = 2;
    }


    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        int* ran_num;
        float old_x_random = x_random;
        float old_z_random = z_random;
        x_delta = 0.25f;
        z_delta = 0.25f;

        do {
            ran_num = twoRandomNum(); // between -4 and 4
            int ran_num_x = *ran_num;
            int ran_num_z = *(ran_num + 1);
            x_random = ran_num_x * x_delta;
            z_random = ran_num_z * z_delta;
           
        } while (x_random == old_x_random && z_random == old_z_random);
    };

    if (key == GLFW_KEY_UP && action == GLFW_PRESS) {
        y_press_num += 1;
    }
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) {
        y_press_num -= 1;
    }
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        if (intensity <= 5.0f) {
            intensity += 0.5f;
        }
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        if (intensity >= 0.0f) {
            intensity -= 0.5f;
        }
    }

    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        x_current = x_current + x_press_num * x_delta * delta;
        z_current = z_current + z_press_num * z_delta * delta;
        x_press_num = -1;
        x_delta = cos(rotate_num * r_delta) * 1.0f;
        z_press_num = 1;
        z_delta = sin(rotate_num * r_delta) * 1.0f;
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        x_current = x_current + x_press_num * x_delta * delta;
        z_current = z_current + z_press_num * z_delta * delta;
        x_press_num = 1;
        x_delta = cos(rotate_num * r_delta) * 2.0f;
        z_press_num = -1;
        z_delta = sin(rotate_num * r_delta) * 2.0f;
    }
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
        rotate_num += 1;
    }
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
        rotate_num -= 1;
    }
    if (key == GLFW_KEY_R && action == GLFW_PRESS) {
        if (view_y <= 8.0f) {
            view_y += 1.0f;
        }
        else {
            view_y = 8.0f;
        }
    }
    if (key == GLFW_KEY_F && action == GLFW_PRESS) {
        if (view_y >= -8.0f) {
            view_y -= 1.0f;
        }
        else {
            view_y = -8.0f;
        }
    }
}


int main(int argc, char* argv[]) {
    GLFWwindow* window;

    /* Initialize the glfw */
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    /* glfw: configure; necessary for MAC */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    /* do not allow resizing */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2 - LIU Haoyu", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);

    /* Initialize the glew */
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    get_OpenGL_info();
    initializedGL();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        paintGL();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}