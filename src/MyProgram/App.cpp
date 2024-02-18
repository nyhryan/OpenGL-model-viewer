#include "App.h"

#include <gl/gl3w.h>
#include <glfw/glfw3.h>

#include <fmt/core.h>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <functional>
#include <iostream>

#include "helper.h"
#include "Model.h"
#include "Shader.h"
#include "Camera.h"

App::App(int w, int h)
{
    mScreenWidth = w;
    mScreenHeight = h;

    // GLFW related
    // ------------------------------------
    // init GLFW
    if (glfwInit() == GL_FALSE)
    {
        fmt::print(stderr, "[ERROR] Failed to initialize GLFW\n");
        return;
    }

    // set OpenGL version and profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window and set it to current context
    mWindow = glfwCreateWindow(w, h, "My Program", nullptr, nullptr);
    if (!mWindow)
    {
        fmt::print(stderr, "[ERROR] Failed to create GLFW window\n");
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(mWindow);
    const GLFWvidmode* vidMode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwSetWindowPos(mWindow, (vidMode->width - w) / 2, (vidMode->height - h) / 2);

    // change viewport whenever window size changes
    glfwSetFramebufferSizeCallback(mWindow,
        [](GLFWwindow* win, int w, int h)
        {
            glViewport(0, 0, w, h);
        });

    glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (glfwRawMouseMotionSupported())
        glfwSetInputMode(mWindow, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);


    // updates mWidth, mHeight whenever window size changes
    // glfwSetWindowSizeCallback(mWindow,
    //     [](GLFWwindow* win, int w, int h)
    //     {
    //         // access that App "instance" we passed from here
    //         App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(win));
    //         app->SetWidth(w);
    //         app->SetHeight(h);
    //     });

    // OpenGL stuffs
    // ------------------------------------
    // load OpenGL functions
    gl3wInit();

    glEnable(GL_DEPTH_TEST);

    // enable openGL debug output
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(helper::GLDebugMessageCallback, nullptr);

    // ETC
    // ------------------------------------
    // prints version
    GLint profile;
    glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
    std::string mode = profile & GL_CONTEXT_CORE_PROFILE_BIT ? "CORE " : "COMPAT ";
    fmt::print("[INFO] {}{} Loaded\n", mode, (const char*)glGetString(GL_VERSION));
}

App::~App()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void App::Run()
{
    mShader = std::make_shared<Shader>();
    mShader->AddShader(GL_VERTEX_SHADER, "resources/shader.vert");
    mShader->AddShader(GL_FRAGMENT_SHADER, "resources/shader.frag");
    mShader->Link();
    mShader->SetInt("shadowMap", 0);

    mDrawLightCubeShader = std::make_unique<Shader>();
    mDrawLightCubeShader->AddShader(GL_VERTEX_SHADER, "resources/lightcube.vert");
    mDrawLightCubeShader->AddShader(GL_FRAGMENT_SHADER, "resources/lightcube.frag");
    mDrawLightCubeShader->Link();

    mDebugDepthShader = std::make_unique<Shader>();
    mDebugDepthShader->AddShader(GL_VERTEX_SHADER, "resources/debug_depth.vert");
    mDebugDepthShader->AddShader(GL_FRAGMENT_SHADER, "resources/debug_depth.frag");
    mDebugDepthShader->Link();
    mDebugDepthShader->SetInt("depthMap", 0);

    mDepthShader = std::make_unique<Shader>();
    mDepthShader->AddShader(GL_VERTEX_SHADER, "resources/depth.vert");
    mDepthShader->AddShader(GL_FRAGMENT_SHADER, "resources/depth.frag");
    mDepthShader->Link();


    LoadData();

    mCamera = std::make_shared<Camera>(mScreenWidth, mScreenHeight);

    // pass this "App" instance to GLFW
    glfwSetWindowUserPointer(mWindow, reinterpret_cast<void*>(this));

    // ------------------------------------
    // register GLFW callbacks BEFORE ImGUI initialization
    glfwSetCursorPosCallback(mWindow,
        [](GLFWwindow* win, double x, double y)
        {
            App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(win));

            const std::shared_ptr<Camera> camera = app->GetCamera();

            if (app->GetIsImGUIMode())
            {
                camera->SetFirstCapture(true);
                return;
            }

            float xPos = static_cast<float>(x);
            float yPos = static_cast<float>(y);

            float w = static_cast<float>(app->GetScreenWidth());
            float h = static_cast<float>(app->GetScreenHeight());

            if (camera->GetFirstCapture())
            {
                camera->SetLastMouseX(xPos);
                camera->SetLastMouseY(yPos);
                camera->SetFirstCapture(false);
            }

            float xOffset = xPos - camera->GetLastMouseX();
            float yOffset = camera->GetLastMouseY() - yPos;

            camera->SetLastMouseX(xPos);
            camera->SetLastMouseY(yPos);

            camera->ProcessMouse(xOffset, yOffset);
        });

    glfwSetKeyCallback(mWindow,
        [](GLFWwindow* win, int key, int scancode, int action, int mods)
        {
            App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(win));

            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            {
                glfwSetWindowShouldClose(win, true);
            }
            if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
            {
                if (app->GetIsImGUIMode())
                {
                    app->SetIsImGUIMode(false);
                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else
                {
                    app->SetIsImGUIMode(true);
                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
            if (key == GLFW_KEY_F2 && action == GLFW_PRESS)
            {
                bool mode = app->GetIsDepthShaderDebugMode();
                app->SetIsDepthShaderDebugMode(!mode);
            }
            if (key == GLFW_KEY_F5 && action == GLFW_PRESS)
            {
                const std::shared_ptr<Shader> shader = app->GetShader();
                shader->Recompile();
                fmt::print("[INFO] Shader recopmiled\n");
            }
        });
    // ------------------------------------
    // ------------------------------------

    float dt = static_cast<float>(glfwGetTime());
    float lastFrame = 0.0f;
    float lastFpsTime = dt;
    int nbFrames = 0;
    // ------------------------------------

    const GLuint SHADOW_W = 1024, SHADOW_H = 1024;
    GLuint depthMap = 0;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_W, SHADOW_H, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    GLuint depthMapFBO = 0;
    glGenFramebuffers(1, &depthMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ImGUI stuffs
    // ------------------------------------
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

    ImGui_ImplGlfw_InitForOpenGL(mWindow, true);  // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
    // ------------------------------------


    while (!glfwWindowShouldClose(mWindow))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // calculate deltatime
        float currentFrame = static_cast<float>(glfwGetTime());
        nbFrames++;
        dt = currentFrame - lastFrame;
        dt *= 1000.0f;  // in milisec
        lastFrame = currentFrame;

        ProcessInput(dt);

        // ----------------------------------------------------
        // ImGui Start
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ----------------------------------------------------

        // print frametime and FPS
        {
            ImGui::SeparatorText("Info");
            static float fps;
            static float ms;
            if (currentFrame - lastFpsTime >= 0.5)
            {
                fps = 1000.0f / dt;
                lastFpsTime = currentFrame;
            }
            ImGui::Text("%.1f ms / %.1f FPS", dt, fps);
            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "<F1>");
            ImGui::SameLine();
            ImGui::Text("ImGUI control mode toggle");

            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "<F2>");
            ImGui::SameLine();
            ImGui::Text("Depth map view mode toggle");

            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "<F5>");
            ImGui::SameLine();
            ImGui::Text("Reload \"shader.vert, shader.frag\" and recompile");
        }

        // setup camera(position, Z direction ImGUI elements, projection/view matrix)
        ImGui::SeparatorText("Settings");

        CameraSetup();

        glm::vec3 camPos = mCamera->GetPos();
        glm::vec3 camZ = mCamera->GetZ();

        // variable to rotate necoarc model
        static float modelRotation = 0.0f;

        // draw lightcube
        // movable Light Cube ImGUI element
        static float lightPositionFloat[3] = { 5.0, 7.0, 3.0 };
        static float cubeSize = 0.1f;

        static float colors[3] = { 0.5f, 0.5f, 0.5f };
        static float planes[2] = { 1.0f, 20.0f };
        static float size = 10.0f;
        {
            if (ImGui::TreeNode("Light Settings"))
            {
                ImGui::Text("Light Cube Size");
                ImGui::SliderFloat("##Light Cube Size", &cubeSize, 0.01f, 0.5f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

                ImGui::Text("Light Position");
                ImGui::SliderFloat3("##Light Position", lightPositionFloat, -10.0f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
                ImGui::Text("Near/Far plane");
                ImGui::SliderFloat2("##Near/Far plane", planes, 0.01f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);


                ImGui::Text("Ortho Size");
                ImGui::SliderFloat("##Ortho Size", &size, 1.0f, 100.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);

                ImGui::Text("Light Color");
                ImGui::ColorPicker3("##Light Color", colors);
                ImGui::TreePop();
            }
        }

        float& near_plane = planes[0];
        float& far_plane = planes[1];
        glm::mat4 lightSpaceMatrix;
        // generate depthmap
        {
            glm::mat4 lightProjection, lightView;
            lightProjection = glm::ortho(-size, size, -size, size, near_plane, far_plane);
            lightView = glm::lookAt(glm::make_vec3(lightPositionFloat), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
            lightSpaceMatrix = lightProjection * lightView;

            // render scene from light's point of view
            mDepthShader->SetUniformMat4("lightSpaceMatrix", lightSpaceMatrix);
            mShader->SetUniformMat4("lightSpaceMatrix", lightSpaceMatrix);

            glViewport(0, 0, SHADOW_W, SHADOW_H);
            glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
            // glCullFace(GL_FRONT);
            glClear(GL_DEPTH_BUFFER_BIT);

            GLuint shaderId = mDepthShader->GetId();

            glm::mat4 model = glm::mat4(1.0);
            model = glm::rotate(model, glm::radians(modelRotation), glm::vec3(0.0, 1.0, 0.0));
            mDepthShader->SetUniformMat4("model", model);
            mModel->Draw(shaderId);

            model = glm::mat4(1.0);
            mDepthShader->SetUniformMat4("model", model);
            mFloorModel->Draw(shaderId);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            // glCullFace(GL_BACK);
        }

        glViewport(0, 0, mScreenWidth, mScreenHeight);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!mIsDepthShaderDebugMode)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);

            GLuint shaderId = mShader->GetId();

            glm::mat4 model = glm::mat4(1.0);
            model = glm::rotate(model, glm::radians(modelRotation), glm::vec3(0.0, 1.0, 0.0));
            mShader->SetUniformMat4("model", model);
            mModel->Draw(shaderId);

            model = glm::mat4(1.0);
            model = glm::scale(model, glm::vec3(3.0f));
            mShader->SetUniformMat4("model", model);
            mFloorModel->Draw(shaderId);

            mShader->SetUniformVec3("lightPos", glm::make_vec3(lightPositionFloat));

            mShader->SetFloat3("lightColor", colors);
            mDrawLightCubeShader->SetFloat3("lightColor", colors);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::make_vec3(lightPositionFloat));
            model = glm::scale(model, glm::vec3(cubeSize));

            mDrawLightCubeShader->Use();
            mDrawLightCubeShader->SetUniformMat4("model", model);
            mLightCubeModel->Draw(mDrawLightCubeShader->GetId());
        }
        else
        {
            mDebugDepthShader->SetFloat("near_plane", near_plane);
            mDebugDepthShader->SetFloat("far_plane", far_plane);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, depthMap);
            {
                static GLuint quadVAO = 0;
                GLuint quadVBO;
                if (quadVAO == 0)
                {
                    float quadVertices[] = {
                        // positions        // texture Coords
                        -1.0f,
                        1.0f,
                        0.0f,
                        0.0f,
                        1.0f,
                        -1.0f,
                        -1.0f,
                        0.0f,
                        0.0f,
                        0.0f,
                        1.0f,
                        1.0f,
                        0.0f,
                        1.0f,
                        1.0f,
                        1.0f,
                        -1.0f,
                        0.0f,
                        1.0f,
                        0.0f,
                    };
                    // setup plane VAO
                    glGenVertexArrays(1, &quadVAO);
                    glGenBuffers(1, &quadVBO);
                    glBindVertexArray(quadVAO);
                    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
                    glEnableVertexAttribArray(0);
                    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
                    glEnableVertexAttribArray(1);
                    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
                }
                glBindVertexArray(quadVAO);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
                glBindVertexArray(0);
            }
        }


        // model rotation speed imgui
        {
            static float modelRotationSpeed = 1.0f;
            ImGui::Text("Rotation Speed");
            ImGui::SliderFloat("##Rotation Speed", &modelRotationSpeed, 0.0f, 5.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
            modelRotation += modelRotationSpeed / dt;
            float dummy = 0;
            if (modelRotation > 360.0f)
                glm::modf(modelRotation, dummy);
        }


        // ----------------------------------------------------
        // ImGui End
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // ----------------------------------------------------

        glfwSwapBuffers(mWindow);
        glfwPollEvents();
    }
}

void App::LoadData()
{
    mModel = std::make_unique<Model>("resources/necoarc.obj");
    mFloorModel = std::make_unique<Model>("resources/floor.obj");
    mLightCubeModel = std::make_unique<Model>("resources/cube.obj");
}

void App::ProcessInput(float dt)
{
    if (mIsImGUIMode)
    {
        return;
    }

    if (glfwGetKey(mWindow, GLFW_KEY_W) == GLFW_PRESS)
    {
        mCamera->MoveForward(dt);
    }
    if (glfwGetKey(mWindow, GLFW_KEY_S) == GLFW_PRESS)
    {
        mCamera->MoveBackward(dt);
    }
    if (glfwGetKey(mWindow, GLFW_KEY_A) == GLFW_PRESS)
    {
        mCamera->MoveLeft(dt);
    }
    if (glfwGetKey(mWindow, GLFW_KEY_D) == GLFW_PRESS)
    {
        mCamera->MoveRight(dt);
    }
    if (glfwGetKey(mWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        mCamera->Descend(dt);
    }
    if (glfwGetKey(mWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        mCamera->Ascend(dt);
    }
}

void App::CameraSetup()
{
    static float moveSpeed = 0.01f;
    static float mouseSens = 0.1f;
    static int fov = static_cast<int>(mCamera->GetFov());

    glm::vec3 camPos = mCamera->GetPos();
    glm::vec3 camZ = mCamera->GetZ();

    mShader->SetUniformVec3("camPos", camPos);

    if (ImGui::TreeNode("Camera Settings"))
    {
        ImGui::Text("Camera Speed");
        ImGui::SliderFloat("##camera speed", &moveSpeed, 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);

        ImGui::Text("Mouse sens");
        ImGui::SliderFloat("##Mouse sens", &mouseSens, 0.001f, 0.1f, "%.2f", ImGuiSliderFlags_AlwaysClamp | ImGuiSliderFlags_Logarithmic);

        // show camera pos & camera's Z direction elements
        ImGui::BeginDisabled();
        {
            ImGui::Text("Camera Pos");
            ImGui::DragFloat3("##Camera Pos", glm::value_ptr(camPos), 1.0, -1.0, 1.0, "%.1f");
            ImGui::Text("Camera Z");
            ImGui::DragFloat3("##Camera Z", glm::value_ptr(camZ), 1.0, -1.0, 1.0, "%.1f");
        }
        ImGui::EndDisabled();

        ImGui::Text("FOV");
        ImGui::SliderInt("##FOV", &fov, 45, 120);

        ImGui::TreePop();
    }

    mCamera->SetMoveSpeed(moveSpeed);
    mCamera->SetMouseSensitivity(mouseSens);
    mCamera->SetFov(static_cast<float>(fov));

    glm::mat4 projection = glm::perspectiveFov(
        glm::radians(static_cast<float>(fov)),
        static_cast<float>(mScreenWidth),
        static_cast<float>(mScreenHeight),
        0.1f,
        100.0f);
    mShader->SetUniformMat4("projection", projection);
    mDrawLightCubeShader->SetUniformMat4("projection", projection);

    glm::mat4 view = glm::mat4(1.0);
    view = glm::lookAt(
        camPos,                     // camera pos
        camPos - camZ,              // target
        glm::vec3(0.0, 1.0, 0.0));  // up vector

    mShader->SetUniformMat4("view", view);
    mDrawLightCubeShader->SetUniformMat4("view", view);
}