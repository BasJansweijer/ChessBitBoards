#include "boardVisualizer.h"
#include "bitBoard.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <algorithm>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static int s_width = 600;
static int s_height = s_width;
static float borderPercent = 0.05;
static float s_squareSize = (s_width * (1 - borderPercent * 2)) / 8;

void resizeBoard(GLFWwindow *window, int newWidth, int newHeight)
{
    int size = std::min(newWidth, newHeight);
    s_width = size;
    s_height = size;
    s_squareSize = (size * (1 - borderPercent * 2)) / 8;

    ImGui::GetIO().DisplaySize = ImVec2((float)size, (float)size);
}

ImVec2 getSquarePos(int rank, int file)
{
    int row = 7 - rank;
    float border = borderPercent * s_width;
    return ImVec2(file * s_squareSize + border, row * s_squareSize + border);
}

void renderEmptyBoard()
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    ImU32 lightColor = IM_COL32(255, 235, 195, 255);
    ImU32 darkColor = IM_COL32(110, 30, 15, 255);
    ImU32 border = IM_COL32(70, 15, 5, 255);

    ImVec2 topLeft = ImVec2(0, 0);
    ImVec2 bottomRight = ImVec2(s_width, s_height);
    drawList->AddRectFilled(topLeft, bottomRight, border);

    // Draw 8x8 chessboard
    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            ImU32 color = ((rank + file) % 2 == 0) ? lightColor // White
                                                   : darkColor; // Black

            topLeft = getSquarePos(rank, file);
            bottomRight = ImVec2(topLeft.x + s_squareSize, topLeft.y + s_squareSize);

            drawList->AddRectFilled(topLeft, bottomRight, color);
        }
    }
}

void highlightBitBoard(bitboard bb)
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    ImU32 highlight = IM_COL32(50, 150, 5, 150);
    for (int rank = 7; rank >= 0; rank--)
    {
        for (int file = 0; file < 8; file++)
        {
            int bit = (bb >> (rank * 8 + file)) & 1;
            if (bit)
            {
                ImVec2 topLeft = getSquarePos(rank, file);
                ImVec2 bottomRight = {topLeft.x + s_squareSize, topLeft.y + s_squareSize};
                drawList->AddRectFilled(topLeft, bottomRight, highlight);
            }
        }
    }
}

void boardRenderGUI(std::function<void()> boardContentRenderer)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return;
    }

    // Create the window
    GLFWwindow *window = glfwCreateWindow(s_width, s_height, "Chess bitboard", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Window creation failed!" << std::endl;
        glfwTerminate();
        return;
    }

    // Make the OpenGL context current
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    glfwSetFramebufferSizeCallback(window, resizeBoard);
    glfwSetWindowAspectRatio(window, 1, 1);

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
        renderEmptyBoard();

        boardContentRenderer();

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

namespace chess::bitBoards
{
    void showBitboardGUI(bitboard bb)
    {
        boardRenderGUI([=]()
                       { highlightBitBoard(bb); });
    }
}