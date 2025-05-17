    //  CHANGES YOU WILL NEED TO DO IN theMain.cpp
    //  Add below lines at the top of theMain.cpp
    //  GLuint program;
    //  GLFWwindow* window;
    //  void RenderImGui();
    // 
    //  Change this line in int main() GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    //  To window = glfwCreateWindow(640, 480, "OpenGL Triangle", NULL, NULL);
    // 
    //  Add below lines to int main just after creating the OpenGL window mentioned above
    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
    //  IMGUI_CHECKVERSION();
    //  ImGui::CreateContext();
    //  //ImGui_ImplGlfw_InitForOpenGL(imguiWindow, true);
    //  ImGui_ImplGlfw_InitForOpenGL(window, true);
    //  ImGui_ImplOpenGL3_Init("#version 330");
    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------  
    //  
    //  Change this line GLuint program = pShaderManager->getIDFromFriendlyName("shader01");
    //  To program = pShaderManager->getIDFromFriendlyName("shader01");
    //  
    //  Add below lines in the while loop
    //  
    //  Handle async IO stuff
    //  Change below lines
    //  handleKeyboardAsync(window);
    //  handleMouseAsync(window);
    //  
    //  To
    //  if (!ImGui::GetIO().WantCaptureKeyboard) {
    //      handleKeyboardAsync(window);
    //  }
    //  if (!ImGui::GetIO().WantCaptureMouse) {
    //      handleMouseAsync(window);
    //  }
    // 
    //  Add below line just below the Handle async IO Stuff
    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
    //  ImGui_ImplOpenGL3_NewFrame();
    //  ImGui_ImplGlfw_NewFrame();
    //  ImGui::NewFrame();
    //  RenderImGui();
    //  ImGui::Render();
    //  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
    //  
    //  Add below lines outside while loop after the loop ends
    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
    //  ImGui_ImplOpenGL3_Shutdown();
    //  ImGui_ImplGlfw_Shutdown();
    //  ImGui::DestroyContext();
    //  --------------------------------------------------------------------------IMGUI--------------------------------------------------------------------
    //

#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "sMesh.h"
#include "cVAOManager/cVAOManager.h"
#include "cLightManager.h"
#include <string>
#include <cstring>
#include <iostream>

void SaveDataToFile(const std::string& filename);
void LoadDataFromFile(const std::string& filename);
sMesh* g_pFindMeshByFriendlyName(std::string theNameToFind);
sMesh* myMesh = nullptr;
extern std::vector<sMesh*> g_vecMeshesToDraw;
extern cVAOManager* g_pMeshManager;
extern GLuint program;
extern GLFWwindow* window;
extern cLightManager* g_pLightManager;
extern bool g_bShowDebugSpheres;
extern unsigned int g_selectedLightIndex;

static char modelFilePath[256] = "assets/models/.ply";
static char friendlyName[128] = "";
static glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
static glm::vec3 rotation = glm::vec3(0.0f);
static char textureFile[128] = ".bmp";
static float blendRatio = 1.0f;
static float scale = 1.0f;
static bool doNotLight = false;
sModelDrawInfo meshInfo;

extern glm::vec3 rightHandOffset;
extern glm::vec3 gunRot;

void RenderImGui()
{
    static char meshName[128] = "";
    static bool dockspaceOpen = true;

    // Get current window size
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    //// Set next window position and size (dock to right)
    //ImGui::SetNextWindowPos(ImVec2(width * 0.8f, 0)); // Right-side dock (70% OpenGL space)
    //ImGui::SetNextWindowSize(ImVec2(width * 0.2f, height)); // 30% width, full height
    ////ImGui::SetNextWindowSize(ImVec2(350, 500), ImGuiCond_FirstUseEver);
    ImVec2 windowPadding = ImVec2(10, 10);

    // Position at top-right corner with padding
    ImGui::SetNextWindowPos(ImVec2(width - windowPadding.x, windowPadding.y), ImGuiCond_Always, ImVec2(1.0f, 0.0f));

    // Automatically resize based on contents
    ImGui::SetNextWindowBgAlpha(0.9f); // Optional: semi-transparent background
    ImGui::Begin("Mesh Manager", &dockspaceOpen, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui::BeginTabBar("MeshTabs"))
    {
        // First tab - Add New Mesh
        if (ImGui::BeginTabItem("Add New Mesh"))
        {
            ImGui::InputText("Model File", modelFilePath, IM_ARRAYSIZE(modelFilePath));
            ImGui::InputText("Unique Name", friendlyName, IM_ARRAYSIZE(friendlyName));
            ImGui::DragFloat3("Position", &position[0], 1.0f);
            ImGui::DragFloat3("Rotation", &rotation[0], 1.0f);
            ImGui::InputText("Texture", textureFile, IM_ARRAYSIZE(textureFile));
            ImGui::SliderFloat("Blend Ratio", &blendRatio, 0.0f, 1.0f);
            ImGui::DragFloat("Scale", &scale, 0.1f, 0.01f, 50.0f);
            ImGui::Checkbox("No Lighting", &doNotLight);

            if (ImGui::Button("Add Mesh"))
            {
                ::g_pMeshManager->LoadModelIntoVAO_Async(modelFilePath, meshInfo, program);
                sMesh* newMesh = new sMesh();
                newMesh->modelFileName = modelFilePath;
                newMesh->uniqueFriendlyName = friendlyName;
                newMesh->positionXYZ = position;
                newMesh->rotationEulerXYZ = rotation;
                newMesh->textures[0] = textureFile;
                newMesh->blendRatio[0] = blendRatio;
                newMesh->uniformScale = scale;
                newMesh->bDoNotLight = doNotLight;

                ::g_vecMeshesToDraw.push_back(newMesh);
                std::cout << "Mesh " << friendlyName << " added to scene.\n";
            }

            ImGui::EndTabItem();
        }

        // Second tab - Mesh Editor
        if (ImGui::BeginTabItem("Mesh Editor"))
        {
            ImGui::InputText("Mesh Name", meshName, IM_ARRAYSIZE(meshName));
            if (ImGui::Button("Find Mesh")) {
                myMesh = g_pFindMeshByFriendlyName(meshName);
            }

            if (myMesh) {
                ImGui::DragFloat3("Position", &myMesh->positionXYZ[0], 0.1f);
                ImGui::DragFloat3("Rotation", &myMesh->rotationEulerXYZ[0], 1.0f);
                ImGui::DragFloat("Scale", &myMesh->uniformScale, 0.01f, 0.01f, 10.0f);
                ImGui::ColorEdit4("Color RGBA", &myMesh->objectColourRGBA[0]);
                ImGui::Checkbox("Override Color", &myMesh->bOverrideObjectColour);
                ImGui::Checkbox("Wireframe", &myMesh->bIsWireframe);
                ImGui::Checkbox("Visible", &myMesh->bIsVisible);
                ImGui::Checkbox("No Lighting", &myMesh->bDoNotLight);
                ImGui::DragFloat("Transparency", &myMesh->alphaTransparency, 0.01f, 0.0f, 1.0f);
            }

            ImGui::DragFloat3("Gun Pos Offset", &rightHandOffset[0], 0.1f);
            ImGui::DragFloat3("Gun Rotation", &gunRot[0], 1.0f);



            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Lights"))
        {
            static int selectedLight = 0; // Stores the index of the current light
            ::g_selectedLightIndex = selectedLight;

            // Select which light to edit
            ImGui::Text("Select Light:");
            ImGui::SliderInt("Light Index", &selectedLight, 0, cLightManager::NUMBEROFLIGHTS - 1);

            // Get reference to the selected light
            cLightManager::sLight& light = ::g_pLightManager->theLights[selectedLight];

            ImGui::Separator();
            ImGui::Text("Light Properties");

            // Light Position
            ImGui::DragFloat3("Position", &light.position.x, 0.1f, -100000.0f, 100000.0f);

            // Light Color
            ImGui::ColorEdit3("Diffuse Color", &light.diffuse.x);

            ////// Attenuation
            //ImGui::DragFloat("Linear Attenuation", &light.atten.y, 0.001f, 0.00000000001f, 1.0f, "%.009f");
            //ImGui::DragFloat("Quadratic Attenuation", &light.atten.z, 0.0001f, 0.000001f, 0.0001f, "%.100f");

            // Linear Attenuation — very low intensity
            ImGui::DragFloat("Linear Attenuation", &light.atten.y, 0.000001f, 0.0f, 1.0f, "%.7f");

            // Quadratic Attenuation — for smooth far falloff
            ImGui::DragFloat("Quadratic Attenuation", &light.atten.z, 0.00000001f, 0.0f, 1.0f, "%.10f");

            // Light Type (Point Light, Spotlight, Directional)
            const char* lightTypes[] = { "Point Light", "Spotlight", "Directional" };
            int currentLightType = static_cast<int>(light.param1.x);

            if (ImGui::Combo("Light Type", &currentLightType, lightTypes, IM_ARRAYSIZE(lightTypes))) {
                light.param1.x = static_cast<float>(currentLightType);
            }

            // Spotlight-Specific Controls
            if (currentLightType == 1) // Spotlight
            {
                ImGui::DragFloat3("Direction", &light.direction.x, 0.01f, -1.0f, 1.0f);
                ImGui::DragFloat("Inner Cone Angle", &light.param1.y, 0.1f, 0.0f, 90.0f);
                ImGui::DragFloat("Outer Cone Angle", &light.param1.z, 0.1f, 0.0f, 90.0f);
            }

            if (currentLightType == 2) // directional light
            {
                ImGui::DragFloat3("Direction", &light.direction.x, 0.01f, -1.0f, 1.0f);
            }

            // Light Toggle (On/Off)
            bool isLightOn = (light.param2.x > 0.5f);
            if (ImGui::Checkbox("Light On", &isLightOn)) {
                light.param2.x = isLightOn ? 1.0f : 0.0f;
            }

            ImGui::Checkbox("Show Light Borders", &::g_bShowDebugSpheres);

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();

        if (ImGui::Button("Save")) {
            SaveDataToFile("saveFile");
        }
        if (ImGui::Button("Load")) 
        {
            LoadDataFromFile("saveFile");
        }
    }

    ImGui::End();
}

inline float Saturate(float x) {
    return (x < 0.0f) ? 0.0f : (x > 1.0f ? 1.0f : x);
}

#include <random>

void ShowControlsUIBottomRight()
{
    // Get viewport size
    ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

    // Define static size
    ImVec2 windowSize(300, 220);

    // Smoother Bobbing animation: using a slower sine wave for smoother oscillation
    float time = ImGui::GetTime();
    float bobbingOffset = sinf(time * 0.5f) * 2.0f; // slower oscillation with smaller amplitude (up to ±2)

    // Calculate final position with smoother bobbing
    ImVec2 windowPos(
        viewportSize.x - windowSize.x - 20,
        viewportSize.y - windowSize.y - 20 + bobbingOffset
    );

    // Set size and animated position
    ImGui::SetNextWindowSize(windowSize);
    ImGui::SetNextWindowPos(windowPos, ImGuiCond_Always);

    // Stylized theme (Resident Evil–style)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.05f, 0.05f, 0.05f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.9f, 0.1f, 0.1f, 0.9f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 6));

    ImGui::Begin("Controls", nullptr,
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoTitleBar
    );

    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "ZOMBIE SHOOTER - CONTROLS");
    ImGui::Separator();

    ImGui::Text("Move:     "); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.2f, 1.0f), "W A S D");
    ImGui::Text("Run:      "); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.2f, 1.0f), "Left Shift");
    ImGui::Text("Aim:      "); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.2f, 1.0f), "Hold Right Mouse");
    ImGui::Text("Zoom Map: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.2f, 1.0f), "Scroll Wheel");
    ImGui::Text("Interact:      "); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.2f, 1.0f), "E");
    ImGui::Text("Objectives: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.2f, 1.0f), "I");

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 0.6f), "Stay quiet. Stay sharp.");

    ImGui::End();

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(2);
}

void RenderMinimap(GLuint textureID, int screenWidth, int screenHeight) {
    // Position and size for the minimap
    float minimapRadius = 120.0f;  // Increased radius for a bigger minimap
    ImVec2 minimapCenter(35.0f, 35.0f);  // Top-left corner with padding

    // Set position for the minimap (top-left corner with some padding)
    ImVec2 pos(minimapCenter.x /*- minimapRadius*/, minimapCenter.y /*- minimapRadius*/);  // Center position for the circle
    ImVec2 size(minimapRadius * 2.0f, minimapRadius * 2.0f);  // Diameter of the circle

    // Set the next window position and size
    ImGui::SetNextWindowPos(pos, ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);

    // Begin the minimap window
    ImGui::Begin("Minimap", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);

    // Create a circular clipping region
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Draw a circular clipping region to limit rendering to the circle
    drawList->AddCircleFilled(ImVec2(pos.x + minimapRadius, pos.y + minimapRadius), minimapRadius, IM_COL32(0, 0, 0, 255));  // Circle mask to hide corners

    // Apply the texture inside the circle
    drawList->PushClipRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), true);  // Clip to the bounding box

    // Render the texture inside the circular area
    ImTextureID imguiTextureID = reinterpret_cast<ImTextureID>(textureID);

    // We need to adjust the texture coordinates to fit inside the circular mask
    // This ensures the texture fits within the circle without distortion
    drawList->AddImage(imguiTextureID, pos, ImVec2(pos.x + size.x, pos.y + size.y));

    // Render the circular clipping region (texture will be masked to fit inside the circle)
    drawList->PopClipRect();

    ImGui::End();
}
