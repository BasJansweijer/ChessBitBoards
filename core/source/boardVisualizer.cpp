#include "boardVisualizer.h"
#include "bitBoard.h"

#include <iostream>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

namespace chess::bitBoards
{

    void showBitboardGUI(bitboard bb)
    {
        int width = 480;
        int height = 480;

        // Initialize GLFW
        if (!glfwInit())
        {
            std::cerr << "GLFW initialization failed!" << std::endl;
            return;
        }

        // Create the window
        GLFWwindow *window = glfwCreateWindow(width, height, "Chess bitboard", NULL, NULL);
        if (window == NULL)
        {
            std::cerr << "Window creation failed!" << std::endl;
            glfwTerminate();
            return;
        }

        // Make the OpenGL context current
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
        ImGui_ImplOpenGL3_Init();

        // Set initial display size for ImGui
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
        io.DisplaySize = ImVec2((float)fbWidth, (float)fbHeight);

        int i = 0;
        // Main rendering loop
        while (!glfwWindowShouldClose(window))
        {
            // wait and handle events
            glfwWaitEvents();

            ImGui_ImplOpenGL3_NewFrame();

            // Start a new frame
            ImGui::NewFrame();

            // Your ImGui code goes here
            ImGui::ShowDemoWindow(); // This shows a demo window, you can replace this with your custom GUI code

            // Rendering
            ImGui::Render();
            glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // Swap buffers
            glfwSwapBuffers(window);
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();
    }
}