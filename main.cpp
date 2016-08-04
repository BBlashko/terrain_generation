/* Author: Brett A. Blashko
 * ID: V00759982
 */

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL/SOIL.h>
#include <stdio.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>
#include "camera.hpp"

#define MESH_X_VERTICES_SIZE 128
#define MESH_Z_VERTICES_SIZE 128

//(MESH_X_SIZE + 1) * MESH_Z_SIZE * (NUM_POINTS_PER_VERTEX + NUM_COLOR_POINTS + NUM_UV)
#define MESH_VERTICES_COUNT (MESH_X_VERTICES_SIZE * MESH_Z_VERTICES_SIZE * (3 + 3 + 2 + 2))

#define MESH_INDICES_COUNT (((MESH_X_VERTICES_SIZE + MESH_Z_VERTICES_SIZE - 1) * MESH_Z_VERTICES_SIZE) - 1)

#define SHADER_POSITION "vertexPosition"
#define SHADER_COLOR    "vertexColor"
#define SHADER_TEXCOORD "vertexTexcoord"
#define SHADER_HEIGHTMAP "vertexHeightmap"
#define SHADER_CURRENT_OBJECT "vertexCurrentObject"


//==============================================================================
// SHADER LOADER
//==============================================================================
std::string StringFromFile(const char* filename)
{
    std::ifstream fs(filename);
    if (!fs)
    {
        return "";
    }

    std::string s(
    std::istreambuf_iterator<char>{fs},
    std::istreambuf_iterator<char>{});

    return s;
}

GLuint loadShaders(const char* vs_filename, const char* fs_filename)
{
    const char* version = "#version 400\n";
    GLint status;
    //Compile vertex shader
    std::string vs_source = StringFromFile(vs_filename);
    const char* vs_strings[] = { version, vs_source.c_str() };
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 2, vs_strings, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint logLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength + 1);
        glGetShaderInfoLog(vertexShader, logLength, NULL, log.data());
        fprintf(stderr, "Error compiling vertex shader: %s\n", log.data());
        glfwTerminate();
        exit(1);
    }

    // Compile fragment shader
    std::string fs_source = StringFromFile(fs_filename);
    const char* fs_strings[] = { version, fs_source.c_str() };
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 2, fs_strings, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        GLint logLength;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength + 1);
        glGetShaderInfoLog(fragmentShader, logLength, NULL, log.data());
        fprintf(stderr, "Error compiling fragment shader: %s\n", log.data());
        glfwTerminate();
        exit(1);
    }

    //link shader program
    printf("%s\n", "Linking the shader program.");
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor");
    glLinkProgram(shaderProgram);

    glDetachShader(shaderProgram, vertexShader);
    glDetachShader(shaderProgram, fragmentShader);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

//==============================================================================
// WINDOW INITIALIZATION
//==============================================================================

GLFWwindow* initializeWindow()
{
    if (!glfwInit())
    {
        fprintf(stderr, "GLFW failed to initialize\n");
    }

    glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    GLFWwindow* window = glfwCreateWindow(1200, 800,
                                          "BBlashko --- Real Time Rendering",
                                          NULL, NULL);
    glfwMakeContextCurrent(window);
    return window;
}

//==============================================================================
// UPDATE FPS TRACKER
//==============================================================================

void update_fps (GLFWwindow* window) {
    static double previous_seconds = glfwGetTime ();
    static int frame_count;
    double current_seconds = glfwGetTime ();
    double elapsed_seconds = current_seconds - previous_seconds;

    if (elapsed_seconds > 0.25) {
        previous_seconds = current_seconds;
        double fps = (double)frame_count / elapsed_seconds;
        char tmp[128];
        sprintf (tmp, "BBlashko --- RealTimeRendering @ fps: %.2f", fps);
        glfwSetWindowTitle (window, tmp);
        frame_count = 0;
    }
    frame_count++;
}

//==============================================================================
// TERRAIN MESH
//==============================================================================

void generate_mesh_vertex_buffer(GLfloat (&vertexBuffer)[MESH_VERTICES_COUNT])
{
    float x = 0.5f - MESH_X_VERTICES_SIZE / 2.0f;
    float z_init = 0.f - MESH_Z_VERTICES_SIZE / 2.0f;

    float u_translate = abs(x);
    float v_translate = abs(z_init);

    for (int i = 0; i < MESH_VERTICES_COUNT; i)
    {
        for (int j = z_init; j < MESH_Z_VERTICES_SIZE / 2.0f; j++)
        {
            //MESH_X_VERTICES_SIZE position
            vertexBuffer[i++] = x;
            vertexBuffer[i++] = 0.0f;
            vertexBuffer[i++] = (float)(j) + 0.5f;

            //MESH_X_VERTICES_SIZE color
            vertexBuffer[i++] = 1.0f;
            vertexBuffer[i++] = 1.0f;
            vertexBuffer[i++] = 1.0f;

            //MESH_X_VERTICES_SIZE uv
            vertexBuffer[i++] = (float) (u_translate + x) / (float) ((MESH_X_VERTICES_SIZE - 1) / (MESH_X_VERTICES_SIZE / 8));
            vertexBuffer[i++] = (float) (v_translate + j) / (float) ((MESH_Z_VERTICES_SIZE - 1) / (MESH_Z_VERTICES_SIZE / 8));

            //MESH_X_VERTICES_SIZE uv for heightmap
            vertexBuffer[i++] = (float) (u_translate + x) / (float) ((MESH_X_VERTICES_SIZE - 1));
            vertexBuffer[i++] = (float) (v_translate + j) / (float) ((MESH_Z_VERTICES_SIZE - 1));
        }
        x++;
    }
}

void generate_mesh_index_buffer(GLint (&indexBuffer)[MESH_INDICES_COUNT])
{
    int i = 0;
    for (int x = 0; x < MESH_X_VERTICES_SIZE; x++)
    {
        for (int z = 0; z < MESH_Z_VERTICES_SIZE;  z++)
        {
            indexBuffer[i++] = (x * (MESH_Z_VERTICES_SIZE)) + z;
            indexBuffer[i++] = ((x + 1) * (MESH_Z_VERTICES_SIZE)) + z;
        }
        indexBuffer[i++] = -1;
    }
}

//==============================================================================
// PERLIN NOISE
//==============================================================================

float rand_func()
{
    return ((float) std::rand())/((float) RAND_MAX);
}

float interpolate(float x, float y, float alpha)
{
    return (x * (1.0f - alpha)) + (alpha * y);
}

float blend(float t)
{
    float t3 = t * t * t;
    return 6 * t * t * t3 - 15 * t * t3 + 10 * t3;
}

void generate_base_noise(GLfloat (&baseNoise)[MESH_X_VERTICES_SIZE][MESH_Z_VERTICES_SIZE], int width, int height)
{
    //random seed
    std::srand(0);
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            baseNoise[i][j] = rand_func();
        }
    }
}

void generate_smooth_noise(GLfloat (&baseNoise)[MESH_X_VERTICES_SIZE][MESH_Z_VERTICES_SIZE], int width, int height, int octave,
                               GLfloat (&smoothNoise)[MESH_X_VERTICES_SIZE * MESH_Z_VERTICES_SIZE])
{
    int period = pow(2, octave);
    float frequency = 1.0f / period;
    int n = 0;
    for (int i = 0; i < width; i++)
    {
        int leftSample = (i / period) * period;
        int rightSample = (leftSample + period) % width;
        float dx_blend = (i - leftSample) * frequency;

        for (int j = 0; j < height; j++)
        {
            int topSample = (j / period) * period;
            int bottomSample = (topSample + period) % height;
            float dy_blend = (j - topSample) * frequency;


            float fx = blend(dx_blend);
            float fy = blend(dy_blend);

            float top = interpolate(baseNoise[leftSample][topSample],
                                    baseNoise[rightSample][topSample],
                                    fx);

            float bottom = interpolate(baseNoise[leftSample][bottomSample],
                                    baseNoise[rightSample][bottomSample],
                                    fx);


            smoothNoise[n++] = interpolate(top, bottom, fy);
        }
    }
}

void generate_perlin_noise(GLfloat (&perlinNoise)[MESH_X_VERTICES_SIZE * MESH_Z_VERTICES_SIZE], int width, int height, int octaveCount)
{
    GLfloat baseNoise[MESH_X_VERTICES_SIZE][MESH_Z_VERTICES_SIZE];
    generate_base_noise(baseNoise, MESH_X_VERTICES_SIZE, MESH_Z_VERTICES_SIZE);

    GLfloat smoothNoise[octaveCount][MESH_X_VERTICES_SIZE * MESH_Z_VERTICES_SIZE];

    float persistance = 0.5f;
    float amplitude = 1.0f;
    float totalAmplitude = 0.0f;

    for (int i = 0; i < octaveCount; i++)
    {
        generate_smooth_noise(baseNoise, MESH_X_VERTICES_SIZE, MESH_Z_VERTICES_SIZE, i, smoothNoise[i]);
    }

    for (int octave = octaveCount - 1; octave >= 0; octave--)
    {
        amplitude *= persistance;
        totalAmplitude += amplitude;

        for (int i = 0; i < width * height; i++)
        {
            perlinNoise[i] += smoothNoise[octave][i] * amplitude;
        }
    }

    for (int i = 0; i < width * height; i++)
    {
        perlinNoise[i] /= totalAmplitude;

    }
}

//===========================================================================================
// SKY BOX
//===========================================================================================

void generate_skybox_vertices(GLfloat (&skyboxVertices)[36 * 5])
{
    float radius = 0.5f - MESH_X_VERTICES_SIZE / 2.0f;

    GLfloat cube_with_texture_vertices[] = {
        //X       Y       Z         U       V

        -radius,   radius,   -radius,
        -radius,   -radius,  -radius,
        radius,    -radius,  -radius,
        radius,    -radius,  -radius,
        radius,    radius,   -radius,
        -radius,   radius,   -radius,

        -radius,   -radius,  radius,
        -radius,   -radius,   -radius,
        -radius,    radius,   -radius,
        -radius,    radius,   -radius,
        -radius,    radius,  radius,
        -radius,   -radius,  radius,

       //top face
       radius,   -radius,   -radius,
        radius,   -radius,   radius,
        radius,   radius,  radius,
        radius,   radius,  radius,
       radius,   radius,  -radius,
       radius,   -radius,  -radius,

       //bottom face
       -radius,  -radius,  radius,
       -radius,  radius,  radius,
        radius,  radius,   radius,
        radius,  radius,   radius,
       radius,  -radius,   radius,
       -radius,  -radius,  radius,

       //back face
        -radius,   radius,  -radius,
       radius,   radius,  -radius,
       radius,    radius,  radius,
       radius,    radius,  radius,
        -radius,    radius,  radius,
        -radius,   radius,  -radius,

        //front face
       -radius,   -radius,   -radius,
        -radius,   -radius,   radius,
        radius,   -radius,   -radius,
        radius,    -radius,   -radius,
       -radius,    -radius,   radius,
       radius,   -radius,   radius,
    };
    std::copy(std::begin(cube_with_texture_vertices), std::end(cube_with_texture_vertices), std::begin(skyboxVertices));
}


//==============================================================================
// MAIN
//==============================================================================

int main()
{
    GLFWwindow* window = initializeWindow();
    //initialize drawing
    glewExperimental = GL_TRUE;
    glewInit();

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    GLuint program = loadShaders("scene.vert", "scene.frag");
    glUseProgram(program);

    //Enable zBuffering
    glEnable(GL_DEPTH_TEST);

    //camera initializations
    Camera camera(window);

    //Model matrix for camera
    GLint model = glGetUniformLocation(program, "model");
    GLint view = glGetUniformLocation(program, "view");
    GLint projection = glGetUniformLocation(program, "proj");
    GLint positionAttrib = glGetAttribLocation(program, SHADER_POSITION);
    GLint colorAttrib = glGetAttribLocation(program, SHADER_COLOR);
    GLint texcoordAttrib = glGetAttribLocation(program, SHADER_TEXCOORD);
    GLint heightmapAttrib = glGetAttribLocation(program, SHADER_HEIGHTMAP);
    GLint currentObject = glGetUniformLocation(program, SHADER_CURRENT_OBJECT);

    glUniform1f(currentObject, 0);

    //generate perlin noise ****************************************************
    GLfloat perlinNoise[MESH_X_VERTICES_SIZE * MESH_Z_VERTICES_SIZE];
    memset(perlinNoise, 0, sizeof(perlinNoise));
    generate_perlin_noise(perlinNoise, MESH_X_VERTICES_SIZE, MESH_Z_VERTICES_SIZE, 5);

    //instantiate all textures *************************************************
    GLuint textureIDs[7];
    glGenTextures(7, textureIDs);

    GLuint tex = glGetUniformLocation(program, "tex");
    GLuint texWater = glGetUniformLocation(program, "texWater");
    GLuint texSand = glGetUniformLocation(program, "texSand");
    GLuint texGrass = glGetUniformLocation(program, "texGrass");
    GLuint texSnow= glGetUniformLocation(program, "texSnow");
    GLuint texMountain = glGetUniformLocation(program, "texMountain");
    GLuint texSkybox = glGetUniformLocation(program, "texSkybox");
    glUniform1i(tex, 0);
    glUniform1i(texWater, 1);
    glUniform1i(texSand, 2);
    glUniform1i(texGrass, 3);
    glUniform1i(texMountain, 4);
    glUniform1i(texSnow, 5);
    glUniform1i(texSkybox, 6);

    glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, MESH_X_VERTICES_SIZE, MESH_Z_VERTICES_SIZE, 0, GL_RED, GL_FLOAT, perlinNoise);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);


    int width, height;
    //water texture ************************************************************
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
    unsigned char* image = SOIL_load_image("Textures/water.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);

    //sand texture *************************************************************
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureIDs[2]);
    image = SOIL_load_image("Textures/sand.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);

    //grass texture ************************************************************
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, textureIDs[3]);
    image = SOIL_load_image("Textures/grass.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);

    //mountain texture *********************************************************
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, textureIDs[4]);
    image = SOIL_load_image("Textures/mountain.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);

    //snow texture
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, textureIDs[5]);
    image = SOIL_load_image("Textures/snow.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glGenerateMipmap(GL_TEXTURE_2D);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);

    //skybox texture
    std::vector<const GLchar*> faces;
    faces.push_back("Textures/Skybox/right.png");
    faces.push_back("Textures/Skybox/left.png");
    faces.push_back("Textures/Skybox/top.png");
    faces.push_back("Textures/Skybox/bottom.png");
    faces.push_back("Textures/Skybox/front.png");
    faces.push_back("Textures/Skybox/back.png");

    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureIDs[6]);
    for(GLuint i = 0; i < faces.size(); i++)
    {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGBA);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    SOIL_free_image_data(image);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, 10.0f);

    //Generate terrain mesh ****************************************************

    GLuint vao_terrain_mesh;
    glGenVertexArrays(1, &vao_terrain_mesh);
    glBindVertexArray(vao_terrain_mesh);

    //set vertices vbo *********************************************************
    GLfloat vertexBuffer[MESH_VERTICES_COUNT];
    generate_mesh_vertex_buffer(vertexBuffer);

    GLuint vbo_terrain_mesh_vertices;
    glGenBuffers(1, &vbo_terrain_mesh_vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_terrain_mesh_vertices);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexBuffer), vertexBuffer,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionAttrib);
    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE,
                          10 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(colorAttrib);
    glVertexAttribPointer(colorAttrib, 3, GL_FLOAT, GL_FALSE,
                          10 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(texcoordAttrib);
    glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE,
                        10 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));

    glEnableVertexAttribArray(heightmapAttrib);
    glVertexAttribPointer(heightmapAttrib, 2, GL_FLOAT, GL_FALSE,
                        10 * sizeof(GLfloat), (void*)(8 * sizeof(GLfloat)));

    //set indices vbo***********************************************************
    GLint indexBuffer[MESH_INDICES_COUNT];
    generate_mesh_index_buffer(indexBuffer);

    GLuint vbo_terrain_mesh_indicies;
    glGenBuffers(1, &vbo_terrain_mesh_indicies);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_terrain_mesh_indicies);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexBuffer), indexBuffer,
               GL_STATIC_DRAW);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(-1);

    //generate skybox **********************************************************
    GLfloat skyboxVertices[36 * (3 + 2)];
    memset(skyboxVertices, 0, sizeof(skyboxVertices));
    generate_skybox_vertices(skyboxVertices);

    GLuint vao_skybox;
    glGenVertexArrays(1, &vao_skybox);
    glBindVertexArray(vao_skybox);

    GLuint vbo_skybox;
    glGenBuffers(1, &vbo_skybox);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_skybox);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices,
                 GL_STATIC_DRAW);

    glEnableVertexAttribArray(positionAttrib);
    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE,
                          3 * sizeof(GLfloat), 0);

    // glEnableVertexAttribArray(texcoordAttrib);
    // glVertexAttribPointer(texcoordAttrib, 2, GL_FLOAT, GL_FALSE,
    //                     5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));

    //update, render loop ****************************************************************

    while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
           && glfwWindowShouldClose(window) == 0)
    {
        //clear the color and draw the background color.
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //update the fps in the window
        update_fps(window);
        //update camera (consider redoing camera header file... should make cpp)
        camera.update_camera_from_inputs(window);
        glm::mat4 projection4 = camera.getPerspectiveMatrix();
        glm::mat4 view4 = camera.getViewMatrix();
        glm::mat4 model4 = glm::mat4(1.0f);
        glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(view4));
        glUniformMatrix4fv(projection, 1, GL_FALSE, glm::value_ptr(projection4));
        glUniformMatrix4fv(model, 1, GL_FALSE, glm::value_ptr(model4));

        //Draw Everything
        //Draw mesh
        glUniform1f(currentObject, 0);
        glBindVertexArray(vao_terrain_mesh);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glDrawElements(GL_TRIANGLE_STRIP, MESH_INDICES_COUNT, GL_UNSIGNED_INT, 0);

        //Drawskybox (draw last)
        glDepthFunc(GL_LEQUAL);
        glUniform1f(currentObject, 1);
        glBindVertexArray(vao_skybox);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glDepthFunc(GL_LESS);


        //end with this
        glfwPollEvents();
        glfwSwapBuffers(window);

    }
    glDeleteTextures(6, textureIDs);
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao_skybox);
    glDeleteVertexArrays(1, &vao_terrain_mesh);
    glDeleteBuffers(1, &vbo_skybox);
    glDeleteBuffers(1, &vbo_terrain_mesh_vertices);
    glDeleteBuffers(1, &vbo_terrain_mesh_indicies);

    glfwDestroyWindow(window);
    glfwTerminate();
}
