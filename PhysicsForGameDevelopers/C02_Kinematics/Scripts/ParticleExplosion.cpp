#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <math.h>
#include <Shader.h>
#include <others/Icosphere.h>
#include <Utils.h>

float randInRange(float min, float max)
{
    return (float)(rand() / (float)(RAND_MAX)) * (max - min) + min;
}

typedef struct _TParticle
{
    float x;       // x coordinate of the particle
    float y;       // y coordinate of the particle
    float vi;      // initial velocity
    float angle;   // initial trajectory (direction)
    int life;      // duration in milliseconds
    int r;         // red component of particle's color
    int g;         // green component of particle's color
    int b;         // blue component of particle's color
    int time;      // keeps track of the effect's time
    float gravity; // gravity factor
    bool active;   // indicates whether this particle is active or dead
} TParticle;

#define _MAXPARTICLES 50

typedef struct _TParticleExplosion
{
    TParticle p[_MAXPARTICLES]; // list of particles  making up this effect
    int v0;                     // initial velocity, or strength, of the effect
    int x;                      // initial x location
    int y;                      // initial y location
    bool active;                // indicates whether this effect is active or dead
} TParticleExplosion;

int SCR_WIDTH = 1920;
int SCR_HEIGHT = 1080;
static float fov = 45.0f, aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
int simulation_steps = 599, simulation_count = 0;

//---------------------------------------------------------------------------//
// Now define the variables required for this simulation
//---------------------------------------------------------------------------//
TParticleExplosion Explosion;
int xc = (int)randInRange(-(float)SCR_WIDTH / 4, (float)SCR_WIDTH / 4);   // x coordinate of the effect
int yc = (int)randInRange(-(float)SCR_HEIGHT / 4, (float)SCR_HEIGHT / 4); // y coordinate of the effect
int v0 = 10;                                                              // initial velocity
int duration = 10000;                                                     // life in milliseconds
float gravity = -9.8f;                                                    // gravity factor (acceleration)
float angle = 999;                                                        // indicates particles' direction
// glm::vec3 s, v = glm::vec3(0), a = glm::vec3(0);
// float t = 0.0f, cosThetaX = 0.0f, cosThetaY = 0.0f, cosThetaZ = 0.0f;
// float Lx = 0.0f, Ly = 0.0f, Lz = 0.0f, cosAlpha = 0.0f, b = 0.0f, xe = 0.0f, ze = 0.0f;
// /// initia parameters
// float alpha = 60.78f, gamma = 8.321f, vm = 8.0f, cannonLength = 2.0f, yStart = 0.0f;
// float g = 0.98f;
// float deltaStep = 0.016f;

glm::vec3 targetPosition = glm::vec3(40, 7.5f, 4.5f);
glm::vec3 targetSize = glm::vec3(6, 8, 5);

// static glm::vec3 cameraPos = glm::vec3(-4.257f, 20.925f, 40.411f);
// static glm::vec3 cameraFront = glm::vec3(0.502f, -0.365f, -0.784f);
static float yaw = -57.400f;
static float pitch = -21.400f;
// static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;
static float cameraRotatorSensitivity = 0.1f;
static float cameraSpeed = 10;
static bool firstMouse = true;
static float lastX = 0, lastY = 0;
static bool bCursorOff = false;
static bool bPressed, bNextStep, bSimulatePaused;
const float PI = 3.14159265359f;
std::vector<float> projectile = std::vector<float>();

static std::shared_ptr<Shader> shaderProgram;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
bool updateParticleState(TParticle *p, int dtime);
static void renderCircle(glm::vec2 pos, glm::vec3 color, glm::vec3 radius = glm::vec3(4));

/////////////////////////////////////////////////////////////////////
/*     This function creates a new particle explosion effect.
    x,y:        starting point of the effect
    Vinit:      a measure of how fast the particles will be sen(it's actually the initial velocity of the particles)
    life:       life of the particles in milliseconds; part fade and die out as they approach their specified life
    gravity:    the acceleration due to gravity, which co rate at which particles will fall as they fly
    angle:      initial trajectory angle of the particles,specify 999 to create a particle explosion that emits particles in all directions; otherwise, 0 right, 90 up, 180 left, etc...
*/
void createParticleExplosion(int x, int y, int Vinit, int life, float gravity, float angle)
{
    int m;
    float f;

    Explosion.active = true;
    Explosion.x = x;
    Explosion.y = y;
    Explosion.v0 = Vinit;

    for (int i = 0; i < _MAXPARTICLES; ++i)
    {
        Explosion.p[i].x = 0;
        Explosion.p[i].y = 0;
        Explosion.p[i].vi = randInRange((float)Vinit / 2, (float)Vinit);
        if (angle < 999)
        {
            m = randInRange(0, 1) == 0 ? -1 : 1;
            Explosion.p[i].angle = -angle + m * randInRange(0, 10);
        }
        else
        {
            Explosion.p[i].angle = randInRange(0, 360);
        }

        f = (float)randInRange(0.8f, 1.0f);
        Explosion.p[i].life = (int)randInRange(0.0f, life * f);
        Explosion.p[i].r = (int)randInRange(225.0f, 255.0f);
        Explosion.p[i].g = (int)randInRange(85.0f, 115.0f);
        Explosion.p[i].b = (int)randInRange(15.0f, 45.0f);
        Explosion.p[i].time = 0;
        Explosion.p[i].active = true;
        Explosion.p[i].gravity = gravity;
    }
}

//---------------------------------------------------------------------------//
// Draws the particle system and updates the state of each particle.
// Returns false when all of the particles have died out.
//---------------------------------------------------------------------------//
bool drawParticleExplosion(void)
{
    bool finished = true;
    float r;

    if (Explosion.active)
    {
        for (int i = 0; i < _MAXPARTICLES; ++i)
        {
            TParticle particle = Explosion.p[i];
            if (particle.active)
            {
                finished = false;
                // Calculate a color scale factor to fade the particle's color as its life expires
                r = ((float)particle.life - particle.time) / (float)particle.life;
                renderCircle(glm::vec2(particle.x + Explosion.x, particle.y + Explosion.y), glm::vec3(particle.r, particle.g, particle.b) / glm::vec3(255));
                if (!bSimulatePaused || bNextStep)
                {
                    bNextStep = false;
                    Explosion.p[i].active = updateParticleState(&(Explosion.p[i]), 10);
                }
            }
        }
    }
    if (finished)
    {
        Explosion.active = false;
    }
    return !finished;
}

//---------------------------------------------------------------------------//
/*  This is generic function to update the state of a given particle.
    p:          pointer to a particle structure
    dtime:      time increment in milliseconds to advance the state of the particle
    If the total elapsed time for this particle has exceeded the particle's set life, then this function returns FALSE, indicating that the particle should expire.    
*/
bool updateParticleState(TParticle *p, int dtime)
{
    float t;
    p->time += dtime;
    t = (float)p->time / 1000.0f;
    p->x = p->vi * cos(glm::radians(p->angle)) * t;
    p->y = p->vi * sin(glm::radians(p->angle)) * t + (p->gravity * t * t) * 0.5f;
    if (p->time >= p->life)
    {
        return false;
    }
    return true;
}

// static void mouseCallback(GLFWwindow *window, double xPos, double yPos)
// {
//     if (firstMouse)
//     {
//         lastX = (float)xPos;
//         lastY = (float)yPos;
//         firstMouse = false;
//     }
//     float xOffset = (float)xPos - lastX;
//     float yOffset = lastY - (float)yPos; // reversed since y-coordinates range from bottom to top
//     lastX = (float)xPos;
//     lastY = (float)yPos;

//     xOffset *= cameraRotatorSensitivity;
//     yOffset *= cameraRotatorSensitivity;

//     yaw += xOffset;
//     pitch += yOffset;
//     if (pitch > 89.0f)
//     {
//         pitch = 89.0f;
//     }
//     if (pitch < -89.0f)
//     {
//         pitch = -89.0f;
//     }
//     glm::vec3 direction;
//     direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
//     direction.y = sin(glm::radians(pitch));
//     direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
//     cameraFront = glm::normalize(direction);
// }

void draw_imgui(GLFWwindow *window);
void drawProjectile();
void drawCannon();
void drawScene();
void simulate(float delta);
static void renderCube();
static void renderPlane();
static unsigned int cubeTexture;

void resetSimulation()
{

    createParticleExplosion(xc, yc, v0, duration, gravity, angle);

    // simulation_count = 0;
    // t = 0;
    // s = glm::vec3(xe, (yStart + cannonLength * cosAlpha), ze);
    // projectile.clear();
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ParticleExplosion", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    cubeTexture = loadImage("C02_Kinematics/Pictures/constant_acceleration_kinematic_formulas.png", false);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    std::string vsPath, fsPath;
    getProjectFilePath("C02_Kinematics/Shaders/ParticleExplosion.vert", vsPath);
    getProjectFilePath("C02_Kinematics/Shaders/ParticleExplosion.frag", fsPath);
    shaderProgram = std::make_shared<Shader>(vsPath.c_str(), fsPath.c_str(), nullptr);
    shaderProgram->use();

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwGetFramebufferSize(window, &SCR_WIDTH, &SCR_HEIGHT);

    resetSimulation();
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            ImGui::Begin("Editor");
            // Exceptionally add an extra assert here for people confused about initial Dear ImGui setup
            // Most ImGui functions would normally just crash if the context is missing.
            IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context. Refer to examples app!");
            draw_imgui(window);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawScene();
        // simulate(deltaStep);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    if (width > 0 && height > 0)
    {
        SCR_WIDTH = width;
        SCR_HEIGHT = height;
        glViewport(0, 0, width, height);
    }
}

// static void switch_cursor(GLFWwindow *window)
// {
//     if (!bCursorOff)
//     {
//         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
//         glfwSetCursorPosCallback(window, mouseCallback);
//     }
//     else
//     {
//         glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
//         glfwSetCursorPosCallback(window, nullptr);
//         firstMouse = true;
//     }
//     bCursorOff = !bCursorOff;
// }

void processInput(GLFWwindow *window)
{

    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float deltaSpeed = cameraSpeed * deltaTime;
    // if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    // {

    //     cameraPos += deltaSpeed * cameraFront;
    // }
    // if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    // {

    //     cameraPos -= deltaSpeed * cameraFront;
    // }
    // if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    // {

    //     cameraPos -= glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaSpeed;
    // }
    // if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    // {
    //     cameraPos += glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaSpeed;
    // }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    // if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    // {
    //     bPressed = true;
    // }
    // if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && bPressed)
    // {
    //     bPressed = false;
    //     switch_cursor(window);
    // }
}

void draw_imgui(GLFWwindow *window)
{
    ImGui::Separator();
    // if (bCursorOff)
    // {
    //     ImGui::Text("Press P to release control of the camera, and show cursor.");
    // }
    // else
    // {
    //     ImGui::Text("Press P or Button follows to take control of the camera");
    //     if (ImGui::Button("Posses camera") && !bCursorOff)
    //     {
    //         switch_cursor(window);
    //     }
    // }
    // ImGui::InputFloat("camera speed", (float *)&cameraSpeed);
    // ImGui::SliderFloat("camera rotator sensitivity", (float *)&cameraRotatorSensitivity, 0.0, 1.0);
    // ImGui::InputFloat3("target_position", glm::value_ptr(targetPosition));
    // glm::vec2 angleInDegrees = glm::vec2(yaw, pitch);
    // pitch = angleInDegrees.y;
    // yaw = angleInDegrees.x;
    // ImGui::InputFloat2("cameraRotationAngle", glm::value_ptr(angleInDegrees));
    // ImGui::InputFloat3("cameraPos", glm::value_ptr(cameraPos));
    // ImGui::InputFloat3("cameraFront", glm::value_ptr(cameraFront));
    ImGui::Separator();
    ImGui::SliderInt("simulation steps", &simulation_steps, 600, 1800);
    ImGui::SliderInt("xc", &xc, -SCR_WIDTH / 2, SCR_WIDTH / 2);
    ImGui::SliderInt("yc", &yc, -SCR_HEIGHT / 2, SCR_HEIGHT / 2);
    ImGui::SliderInt("velocity", &v0, 0, 100);
    ImGui::SliderInt("duration", &duration, 1000, 500000);
    ImGui::InputFloat("gravity", &gravity);
    ImGui::InputFloat("angle", &angle);
    if (ImGui::Button("Restart"))
    {
        resetSimulation();
    }
    if (ImGui::Button(bSimulatePaused ? "Continue" : "Pause"))
    {
        bSimulatePaused = !bSimulatePaused;
    }
    if (ImGui::Button("Step"))
    {
        bNextStep = true;
    }
}

void simulate(float delta)
{
    // cosAlpha = cos(glm::radians(alpha));
    // b = cannonLength * cos(glm::radians(90 - alpha));
    // Lx = b * cos(glm::radians(gamma));
    // Ly = cannonLength * cosAlpha;
    // Lz = b * sin(glm::radians(gamma));
    // cosThetaX = Lx / cannonLength;
    // cosThetaY = Ly / cannonLength;
    // cosThetaZ = Lz / cannonLength;

    // // These are the x and z coordinates of the very end of the cannon barrel
    // // we'll use these as the initial x and z displacements
    // xe = cannonLength * cos(glm::radians(90 - alpha)) * cos(glm::radians(gamma));
    // ze = cannonLength * cos(glm::radians(90 - alpha)) * sin(glm::radians(gamma));

    // if ((simulation_count < simulation_steps && !bSimulatePaused) || bNextStep)
    // {
    //     bNextStep = false;
    //     simulation_count += 1;
    //     t += delta;
    //     s = glm::vec3(
    //         xe + vm * cosThetaX * t,
    //         (yStart + cannonLength * cosAlpha) + (vm * cosThetaY) * t - 0.5f * g * t * t,
    //         ze + vm * cosThetaZ * t);
    //     v = glm::vec3(
    //         vm * cosThetaX,
    //         (vm * cosThetaY) - g * t,
    //         vm * cosThetaZ);
    //     a = -g * glm::vec3(0, 1, 0);
    // }
}

static std::vector<glm::vec2> positions, uv;
static std::vector<unsigned int> indices;

static unsigned int circleVAO, circleVBO, circleEBO, indexCount;

static void renderCircle(glm::vec2 pos, glm::vec3 color, glm::vec3 radius)
{
    glm::mat4 model;
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    model = glm::scale(model, radius);
    shaderProgram->setVec3("color", color);
    shaderProgram->setMat4("model", model);
    if (circleVAO == 0)
    {
        glGenVertexArrays(1, &circleVAO);
        glBindVertexArray(circleVAO);

        glGenBuffers(1, &circleVBO);
        glGenBuffers(1, &circleEBO);
        // glGenBuffers(1, &lightParamsBO);
        // glGenBuffers(1, &lightPositionsBO);
        // glGenBuffers(1, &lightColorsBO);
        // glGenBuffers(1, &modelBO);

        indices.clear();
        positions.clear();
        uv.clear();
        // normals.clear();

        const unsigned int X_SEGMENTS = 4;
        const unsigned int Y_SEGMENTS = 4;
        const float PI = 3.14159265359f;
        for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
        {
            for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                // float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                glm::vec2 point(xPos, yPos);
                positions.push_back(point);
                uv.push_back(glm::vec2(xSegment, ySegment));
                // normals.push_back(point);
            }
        }

        bool oddRow = false;
        for (int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = (unsigned int)indices.size();

        std::vector<float> data;
        for (int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            // data.push_back(positions[i].z);
            // data.push_back(normals[i].x);
            // data.push_back(normals[i].y);
            // data.push_back(normals[i].z);
            data.push_back(uv[i].x);
            data.push_back(uv[i].y);
        }
        glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, circleEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        GLsizei stride = (2 + 2) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (void *)(2 * sizeof(float)));
        // glEnableVertexAttribArray(2);
        // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void *)(6 * sizeof(float)));

        // vector<glm::vec3> lightParams;
        // for (unsigned int i = 0; i < NR_LIGHTS; ++i)
        // {
        //     lightParams.push_back(glm::vec3(lightRadius[i]));
        // }
        // glBindBuffer(GL_ARRAY_BUFFER, lightParamsBO);
        // glBufferData(GL_ARRAY_BUFFER, lightParams.size() * sizeof(glm::vec3), &lightParams[0], GL_STATIC_DRAW);
        // glEnableVertexAttribArray(3);
        // glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        // glVertexAttribDivisor(3, 1);

        // glBindBuffer(GL_ARRAY_BUFFER, lightPositionsBO);
        // glBufferData(GL_ARRAY_BUFFER, lightPositions.size() * sizeof(glm::vec3), &lightPositions[0], GL_STATIC_DRAW);
        // glEnableVertexAttribArray(4);
        // glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        // glVertexAttribDivisor(4, 1);

        // glBindBuffer(GL_ARRAY_BUFFER, lightColorsBO);
        // glBufferData(GL_ARRAY_BUFFER, lightColors.size() * sizeof(glm::vec3), &lightColors[0], GL_STATIC_DRAW);
        // glEnableVertexAttribArray(5);
        // glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
        // glVertexAttribDivisor(5, 1);

        // vector<glm::mat4> modelMatrices;
        // for (unsigned int i = 0; i < NR_LIGHTS; ++i)
        // {
        //     glm::vec3 lightPosition = lightPositions[i];
        //     float radius = lightRadius[i];
        //     glm::mat4 model = glm::mat4(1.0f);
        //     model = glm::translate(model, lightPosition);
        //     model = glm::scale(model, glm::vec3(radius));
        //     modelMatrices.push_back(model);
        // }

        // std::size_t vec4Size = sizeof(glm::vec4);
        // glBindBuffer(GL_ARRAY_BUFFER, modelBO);
        // glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);
        // glEnableVertexAttribArray(6);
        // glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)0);
        // glEnableVertexAttribArray(7);
        // glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(vec4Size));
        // glEnableVertexAttribArray(8);
        // glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(vec4Size * 2));
        // glEnableVertexAttribArray(9);
        // glVertexAttribPointer(9, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void *)(vec4Size * 3));
        // glVertexAttribDivisor(6, 1);
        // glVertexAttribDivisor(7, 1);
        // glVertexAttribDivisor(8, 1);
        // glVertexAttribDivisor(9, 1);
    }

    glBindVertexArray(circleVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

// static unsigned int projectileVAO, projectileVBO;

// void drawProjectile()
// {
//     shaderProgram->setMat4("model", glm::mat4(1.0f));
//     projectile.push_back(s.x);
//     projectile.push_back(s.y);
//     projectile.push_back(s.z);
//     if (projectileVAO == 0)
//     {
//         glGenVertexArrays(1, &projectileVAO);
//         // link vertex attributes
//         glBindVertexArray(projectileVAO);
//         glBindVertexArray(0);
//     }
//     glBindVertexArray(projectileVAO);
//     glGenBuffers(1, &projectileVBO);
//     // fill buffer
//     glBindBuffer(GL_ARRAY_BUFFER, projectileVBO);
//     glBufferData(GL_ARRAY_BUFFER, projectile.size() * sizeof(float), &projectile[0], GL_STATIC_DRAW);
//     glEnableVertexAttribArray(0);
//     glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
//     glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)projectile.size() / 3);
//     glDeleteBuffers(1, &projectileVBO);
//     glBindBuffer(GL_ARRAY_BUFFER, 0);
//     glBindVertexArray(0);
// }

void drawScene()
{
    // glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, -3.0f);
    // glm::vec3 target_pos = glm::vec3(0.0f, 0.0f, 0.0f);
    // glm::mat4 view = glm::lookAt(camera_pos, target_pos, glm::vec3(0.0f, 1.0f, 0.0f));
    aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
    glm::mat4 view = glm::ortho(-(float)SCR_WIDTH / 2, (float)SCR_WIDTH / 2, -(float)SCR_HEIGHT / 2, (float)SCR_HEIGHT / 2, -1.0f, 100.0f);
    shaderProgram->setMat4("view", view);

    // glBindTexture(GL_TEXTURE_2D, 0);
    // glm::mat4 plane = glm::mat4(1.0f);
    // plane = glm::scale(plane, glm::vec3(2));
    // shaderProgram->setMat4("model", plane);
    // renderPlane();

    // glBindTexture(GL_TEXTURE_2D, cubeTexture);
    // glm::mat4 trans = glm::mat4(1.0f);
    // trans = glm::translate(trans, targetPosition);
    // trans = glm::scale(trans, targetSize);
    // shaderProgram->setMat4("model", trans);
    // renderCube();

    drawParticleExplosion();

    // glm::mat4 model = glm::mat4(1.0f);
    // model = glm::translate(model, s);
    // shaderProgram->setMat4("model", model);
    // Icosphere sphere = Icosphere(1.0f, 3, false);
    // sphere.draw();

    // drawProjectile();
    // drawCannon();
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
static unsigned int cubeVAO = 0;
static unsigned int cubeVBO = 0;

static void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,  // top-left
            // front face
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,  // bottom-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,   // top-right
            -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,  // top-left
            -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-right
            // right face
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-left
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top-right
            1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom-right
            1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,   // top-left
            1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // bottom-left
            // bottom face
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,  // top-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom-left
            -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,  // bottom-right
            -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,  // top-right
            1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,   // bottom-right
            -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // top-left
            -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f   // bottom-left
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

static unsigned int planeVAO = 0, planeVBO = 0;
static void renderPlane()
{
    if (!planeVAO)
    {
        float planeVertices[] = {
            // positions            // normals         // texcoords
            25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
            -25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,

            25.0f, -0.5f, 25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 0.0f,
            -25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 0.0f, 25.0f,
            25.0f, -0.5f, -25.0f, 0.0f, 1.0f, 0.0f, 25.0f, 25.0f};
        // plane VAO
        glGenVertexArrays(1, &planeVAO);
        glGenBuffers(1, &planeVBO);
        glBindVertexArray(planeVAO);
        glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
        glBindVertexArray(0);
    }
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}