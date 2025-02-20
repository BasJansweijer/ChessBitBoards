#include "boardVisualizer.h"
#include "bitBoard.h"

#include <iostream>
#include <GLFW/glfw3.h>
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

static GLuint kingsTexture = 0;
static GLuint pawnsTexture = 0;
static GLuint knightsTexture = 0;
static GLuint rooksTexture = 0;
static GLuint bishopsTexture = 0;
static GLuint queensTexture = 0;
static bool loadedTextures = false;

// Function to load a PNG image and create a texture (GLuint)
GLuint loadPNGTexture(const char *filename)
{
    int width, height, nrChannels;

    // Load the image using stb_image
    unsigned char *data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);            // Generate texture ID
    glBindTexture(GL_TEXTURE_2D, textureID); // Bind texture

    // Upload the image data to OpenGL
    GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA; // Depending on image channels
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Set texture parameters (e.g., filtering, wrapping)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Free the image data after loading into OpenGL
    stbi_image_free(data);

    return textureID; // Return the texture ID (GLuint)
}

void loadPieces()
{
    kingsTexture = loadPNGTexture("assets/kings.png");
    pawnsTexture = loadPNGTexture("assets/pawns.png");
    knightsTexture = loadPNGTexture("assets/knights.png");
    rooksTexture = loadPNGTexture("assets/rooks.png");
    bishopsTexture = loadPNGTexture("assets/bishops.png");
    queensTexture = loadPNGTexture("assets/queens.png");
}

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

void renderPieceType(bitboard locations, GLuint texture, bool isWhite, float textureSpacing)
{
    ImVec2 uv_min;
    ImVec2 uv_max;

    float scaleMult = 0.75f;
    float pieceSize = s_squareSize * scaleMult;
    float xOffset = pieceSize * textureSpacing;
    if (isWhite)
    {
        uv_min = {0.0f, 0.0f};
        uv_max = {0.5f, 1.0f};
    }
    else
    {
        xOffset *= -1;
        uv_min = {0.5f, 0.0f};
        uv_max = {1.0f, 1.0f};
    }
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();

    for (int rank = 0; rank < 8; rank++)
    {
        for (int file = 0; file < 8; file++)
        {
            int bit = (locations >> (rank * 8 + file)) & 1;
            if (bit)
            {
                ImVec2 topLeft = getSquarePos(rank, file);
                // center on the square (and add color specific offset)
                topLeft.x += (s_squareSize - pieceSize) / 2 + xOffset;
                topLeft.y += (s_squareSize - pieceSize) / 2;

                ImVec2 bottomRight = {topLeft.x + pieceSize, topLeft.y + pieceSize};
                drawList->AddImage(texture, topLeft, bottomRight, uv_min, uv_max);
            }
        }
    }
}

void renderPieces(const chess::BoardState &board)
{
    // Spacing to account for differing white spaces between white and black piece in the png
    float pawnSpacing = 0.07f;
    float rooksSpacing = 0.055f;
    float knightSpacing = 0.04f;
    float bishopSpacing = 0.035f;
    float kingSpacing = 0.04f;
    float queenSpacing = 0.02f;

    // white pieces
    renderPieceType(board.getWhitePawns(), pawnsTexture, true, pawnSpacing);
    renderPieceType(board.getWhiteRooks(), rooksTexture, true, rooksSpacing);
    renderPieceType(board.getWhiteKnights(), knightsTexture, true, knightSpacing);
    renderPieceType(board.getWhiteBishops(), bishopsTexture, true, bishopSpacing);
    renderPieceType(board.getWhiteQueens(), queensTexture, true, queenSpacing);
    renderPieceType(board.getWhiteKing(), kingsTexture, true, kingSpacing);

    // black pieces
    renderPieceType(board.getBlackPawns(), pawnsTexture, false, pawnSpacing);
    renderPieceType(board.getBlackRooks(), rooksTexture, false, rooksSpacing);
    renderPieceType(board.getBlackKnights(), knightsTexture, false, knightSpacing);
    renderPieceType(board.getBlackBishops(), bishopsTexture, false, bishopSpacing);
    renderPieceType(board.getBlackQueens(), queensTexture, false, queenSpacing);
    renderPieceType(board.getBlackKing(), kingsTexture, false, kingSpacing);
}

void renderImgTest()
{
    ImDrawList *drawList = ImGui::GetBackgroundDrawList();
    ImVec2 topLeft = getSquarePos(0, 0);
    ImVec2 bottomRight = {topLeft.x + s_squareSize, topLeft.y + s_squareSize};
    std::cout << kingsTexture << std::endl;
    drawList->AddImage(kingsTexture, topLeft, bottomRight, {0.0f, 0.0f}, {1.0f, 1.0f});
}

static int clickedFile, clickedRank;

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY); // Get cursor position

        int width, height;
        glfwGetWindowSize(window, &width, &height); // Get window size

        // Flip Y-axis to match OpenGL's coordinate system
        mouseY = height - mouseY;

        double borderW = borderPercent * width;
        clickedRank = (mouseY - borderW) / s_squareSize;
        clickedFile = (mouseX - borderW) / s_squareSize;
    }
}

void boardRenderGUI(std::function<void()> boardContentRenderer, const std::string &windowName)
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed!" << std::endl;
        return;
    }

    // Create the window
    GLFWwindow *window = glfwCreateWindow(s_width, s_height, windowName.c_str(), NULL, NULL);
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

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    glfwSetMouseButtonCallback(window, mouse_button_callback);

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
    void showBitboardGUI(bitboard bb, const std::string &windowName)
    {
        boardRenderGUI([=]()
                       { highlightBitBoard(bb); }, windowName);
    }
}

void boardRenderer(const chess::BoardState &board, bitboard highlights)
{
    highlightBitBoard(highlights);
    renderPieces(board);
}

namespace chess
{

    void showBoardGUI(const BoardState &board, bitboard highlights, const std::string &windowName)
    {
        bool piecesLoaded = false;

        auto renderMethod = [&]() mutable
        {
            // prevent loading pieces again on each update
            if (!piecesLoaded)
                loadPieces();

            piecesLoaded = true;
            if (bitBoards::inBounds(clickedRank, clickedFile))
            {
                highlights = 0;
                square clicked = clickedFile + clickedRank * 8;
                for (auto mv : board.legalMoves())
                {
                    if (mv.from != clicked)
                        continue;
                    highlights |= 1ULL << mv.to;
                }
            }

            boardRenderer(board, highlights);
        };
        boardRenderGUI(renderMethod, windowName);
    }
}