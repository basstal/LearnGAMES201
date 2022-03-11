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

int SCR_WIDTH = 1920;
int SCR_HEIGHT = 1080;
static float fov = 45.0f, aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
int simulation_count = 0;
glm::vec3 s, v = glm::vec3(0), a = glm::vec3(0);
float t = 0.0f, cosThetaX = 0.0f, cosThetaY = 0.0f, cosThetaZ = 0.0f;
float Lx = 0.0f, Ly = 0.0f, Lz = 0.0f, cosAlpha = 0.0f, b = 0.0f, xe = 0.0f, ze = 0.0f;
/// initia parameters
int simulation_steps = 599;
float alpha = 60.78f, gamma = 8.321f, vm = 8.0f, cannonLength = 2.0f, yStart = 0.0f;
float g = 0.98f;
float deltaStep = 0.016f;

glm::vec3 targetPosition = glm::vec3(40, 7.5f, 4.5f);
glm::vec3 targetSize = glm::vec3(6, 8, 5);

static glm::vec3 cameraPos = glm::vec3(-4.257f, 20.925f, 40.411f);
static glm::vec3 cameraFront = glm::vec3(0.502f, -0.365f, -0.784f);
static float yaw = -57.400f;
static float pitch = -21.400f;
static glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
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

static void mouseCallback(GLFWwindow *window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = (float)xPos;
        lastY = (float)yPos;
        firstMouse = false;
    }
    float xOffset = (float)xPos - lastX;
    float yOffset = lastY - (float)yPos; // reversed since y-coordinates range from bottom to top
    lastX = (float)xPos;
    lastY = (float)yPos;

    xOffset *= cameraRotatorSensitivity;
    yOffset *= cameraRotatorSensitivity;

    yaw += xOffset;
    pitch += yOffset;
    if (pitch > 89.0f)
    {
        pitch = 89.0f;
    }
    if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}

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
    simulation_count = 0;
    t = 0;
    s = glm::vec3(xe, (yStart + cannonLength * cosAlpha), ze);
    projectile.clear();
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "ParticleKinematics", NULL, NULL);
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
    getProjectFilePath("C02_Kinematics/Shaders/ParticleKinematics.vert", vsPath);
    getProjectFilePath("C02_Kinematics/Shaders/ParticleKinematics.frag", fsPath);
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
        simulate(deltaStep);

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

static void switch_cursor(GLFWwindow *window)
{
    if (!bCursorOff)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouseCallback);
    }
    else
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetCursorPosCallback(window, nullptr);
        firstMouse = true;
    }
    bCursorOff = !bCursorOff;
}

void processInput(GLFWwindow *window)
{

    float currentFrame = (float)glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float deltaSpeed = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {

        cameraPos += deltaSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {

        cameraPos -= deltaSpeed * cameraFront;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {

        cameraPos -= glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f))) * deltaSpeed;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        bPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE && bPressed)
    {
        bPressed = false;
        switch_cursor(window);
    }
}

void draw_imgui(GLFWwindow *window)
{
    ImGui::Separator();
    if (bCursorOff)
    {
        ImGui::Text("Press P to release control of the camera, and show cursor.");
    }
    else
    {
        ImGui::Text("Press P or Button follows to take control of the camera");
        if (ImGui::Button("Posses camera") && !bCursorOff)
        {
            switch_cursor(window);
        }
    }
    ImGui::InputFloat("camera speed", (float *)&cameraSpeed);
    ImGui::SliderFloat("camera rotator sensitivity", (float *)&cameraRotatorSensitivity, 0.0, 1.0);
    ImGui::InputFloat3("cameraPos", glm::value_ptr(cameraPos));
    ImGui::InputFloat3("spherePos", glm::value_ptr(s));
    ImGui::InputFloat3("target_position", glm::value_ptr(targetPosition));
    glm::vec2 angleInDegrees = glm::vec2(yaw, pitch);
    ImGui::InputFloat2("cameraRotationAngle", glm::value_ptr(angleInDegrees));
    yaw = angleInDegrees.x;
    pitch = angleInDegrees.y;
    ImGui::InputFloat3("cameraFront", glm::value_ptr(cameraFront));
    ImGui::Separator();
    ImGui::SliderInt("simulation steps", &simulation_steps, 600, 1800);
    ImGui::SliderFloat("alpha", &alpha, 0, 89);
    ImGui::SliderFloat("gamma", &gamma, 0, 89);
    ImGui::SliderFloat("velocity", &vm, 2.5f, 10);
    ImGui::InputFloat("cannon length", &cannonLength);
    ImGui::InputFloat("start height", &yStart);
    ImGui::InputFloat("delta step", &deltaStep);
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
    cosAlpha = cos(glm::radians(alpha));
    b = cannonLength * cos(glm::radians(90 - alpha));
    Lx = b * cos(glm::radians(gamma));
    Ly = cannonLength * cosAlpha;
    Lz = b * sin(glm::radians(gamma));
    cosThetaX = Lx / cannonLength;
    cosThetaY = Ly / cannonLength;
    cosThetaZ = Lz / cannonLength;

    // These are the x and z coordinates of the very end of the cannon barrel
    // we'll use these as the initial x and z displacements
    xe = cannonLength * cos(glm::radians(90 - alpha)) * cos(glm::radians(gamma));
    ze = cannonLength * cos(glm::radians(90 - alpha)) * sin(glm::radians(gamma));

    if ((simulation_count < simulation_steps && !bSimulatePaused) || bNextStep)
    {
        bNextStep = false;
        simulation_count += 1;
        t += delta;
        s = glm::vec3(
            xe + vm * cosThetaX * t,
            (yStart + cannonLength * cosAlpha) + (vm * cosThetaY) * t - 0.5f * g * t * t,
            ze + vm * cosThetaZ * t);
        v = glm::vec3(
            vm * cosThetaX,
            (vm * cosThetaY) - g * t,
            vm * cosThetaZ);
        a = -g * glm::vec3(0, 1, 0);
    }
}

static unsigned int projectileVAO, projectileVBO;

void drawProjectile()
{
    shaderProgram->setMat4("model", glm::mat4(1.0f));
    projectile.push_back(s.x);
    projectile.push_back(s.y);
    projectile.push_back(s.z);
    if (projectileVAO == 0)
    {
        glGenVertexArrays(1, &projectileVAO);
        // link vertex attributes
        glBindVertexArray(projectileVAO);
        glBindVertexArray(0);
    }
    glBindVertexArray(projectileVAO);
    glGenBuffers(1, &projectileVBO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, projectileVBO);
    glBufferData(GL_ARRAY_BUFFER, projectile.size() * sizeof(float), &projectile[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glDrawArrays(GL_LINE_STRIP, 0, (GLsizei)projectile.size() / 3);
    glDeleteBuffers(1, &projectileVBO);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void drawCannon()
{
    glm::vec3 cannonPos = glm::vec3(Lx / 2, Ly / 2, Lz / 2);
    glm::vec3 scale = glm::vec3(1, cannonLength, 1);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, cannonPos);
    model = glm::rotate(model, glm::radians(-gamma), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(-alpha), glm::vec3(0, 0, 1));
    model = glm::scale(model, scale);
    shaderProgram->setMat4("model", model);
    renderCube();
}

void drawScene()
{
    aspectRatio = (float)SCR_WIDTH / SCR_HEIGHT;
    glm::mat4 projection = glm::perspective(glm::radians(fov), aspectRatio, 0.1f, 1000.0f);
    shaderProgram->setMat4("projection", projection);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    shaderProgram->setMat4("view", view);

    glBindTexture(GL_TEXTURE_2D, 0);
    glm::mat4 plane = glm::mat4(1.0f);
    plane = glm::scale(plane, glm::vec3(2));
    shaderProgram->setMat4("model", plane);
    renderPlane();

    glBindTexture(GL_TEXTURE_2D, cubeTexture);
    glm::mat4 trans = glm::mat4(1.0f);
    trans = glm::translate(trans, targetPosition);
    trans = glm::scale(trans, targetSize);
    shaderProgram->setMat4("model", trans);
    renderCube();

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, s);
    shaderProgram->setMat4("model", model);
    Icosphere sphere = Icosphere(1.0f, 3, false);
    sphere.draw();

    drawProjectile();
    drawCannon();
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