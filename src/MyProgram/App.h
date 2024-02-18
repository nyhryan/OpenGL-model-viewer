#pragma once

#include <gl/gl3w.h>
#include <glfw/glfw3.h>

#include <fmt/core.h>

#include "Model.h"
#include "Shader.h"
#include "Camera.h"

class App
{
public:
    App(int w, int h);
    ~App();

public:
    void Run();

public:
    const std::shared_ptr<Camera>& GetCamera() const { return mCamera; }
    const GLFWwindow* GetWindow() const { return mWindow; }

    const int GetScreenWidth() const { return mScreenWidth; }
    const int GetScreenHeight() const { return mScreenHeight; }

    const bool GetIsImGUIMode() const { return mIsImGUIMode; }
    void SetIsImGUIMode(bool mode) { mIsImGUIMode = mode; }

    const bool GetIsDepthShaderDebugMode() const { return mIsDepthShaderDebugMode; }
    void SetIsDepthShaderDebugMode(bool mode) { mIsDepthShaderDebugMode = mode; }

    const std::shared_ptr<Shader>& GetShader() const { return mShader; }


private:
    void LoadData();
    void ProcessInput(float);

    void RenderScene();
    void CameraSetup();

private:
    GLFWwindow* mWindow = nullptr;
    int mScreenWidth;
    int mScreenHeight;
    bool mIsImGUIMode = false;
    bool mIsDepthShaderDebugMode = false;

    std::unique_ptr<Model> mModel;
    std::unique_ptr<Model> mFloorModel;
    std::unique_ptr<Model> mLightCubeModel;
    
    std::shared_ptr<Shader> mShader;

    std::unique_ptr<Shader> mDrawLightCubeShader;
    std::unique_ptr<Shader> mDepthShader;
    std::unique_ptr<Shader> mDebugDepthShader;

    std::shared_ptr<Camera> mCamera;
};