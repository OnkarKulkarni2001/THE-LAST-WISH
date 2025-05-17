#include "Health.h"
#include "cSoundManager.h"


extern cSoundManager* g_pSoundManager;
extern cPlayer* pPlayerReference;
extern GLFWwindow* window;
extern cParticleEmitter_2* g_pParticles;


glm::vec3 WorldToScreen(const glm::vec3& worldPos, const glm::mat4& view, const glm::mat4& proj, int screenWidth, int screenHeight);

cHealth::cHealth()
{
}

cHealth::~cHealth()
{
}

void cHealth::Interact()
{
    if (pPlayerReference->Health < 75.0f) {
        pPlayerReference->Health = 75.0f;
        
        //glm::vec3 minVel = glm::vec3(-0.5f, -10.0f, -0.5f); // slight X and Z variation
        //glm::vec3 maxVel = glm::vec3(0.5f, -15.0f, 0.5f);
        //g_pParticles->SetInitalVelocity(minVel, maxVel);
        //g_pParticles->SetSourceLocation(glm::vec3(0.0f, 50.0f, 0.0f));
    }
    else {
        pPlayerReference->Health = pPlayerReference->Health;
    }
	pInteractableMesh->bIsVisible = false;
	pInteractableTrigger->bIsVisible = false;
    g_pSoundManager->PlaySoundEffect("HealthPickup");
}

void DrawInteractUI(glm::vec3& objectPos, glm::vec3& playerPos,
    glm::mat4& view, glm::mat4& proj,
    int screenWidth, int screenHeight) {

    float distance = glm::length(playerPos - objectPos);
    if (distance < 3.0f) {
        glm::vec3 screen = WorldToScreen(objectPos + glm::vec3(0, 1.6f, 0), view, proj, screenWidth, screenHeight);

        if (screen.z > 0.0f) 
        {
            ImVec2 pos(screen.x, screen.y);
            const char* text = "PRESS [E] TO INTERACT";

            // Calculate size
            ImVec2 textSize = ImGui::CalcTextSize(text);
            ImVec2 padding(20.0f, 14.0f);

            ImVec2 doubledPadding;
            doubledPadding.x = padding.x;
            doubledPadding.x += padding.x;
            doubledPadding.y = padding.y;
            doubledPadding.y += padding.y;

            ImVec2 windowSize;
            windowSize.x = textSize.x;
            windowSize.x += doubledPadding.x;
            windowSize.y = textSize.y;
            windowSize.y += doubledPadding.y;

            ImVec2 topLeft;
            topLeft.x = pos.x;
            topLeft.x -= windowSize.x * 0.5f;
            topLeft.y = pos.y;
            topLeft.y -= windowSize.y;

            ImVec2 bottomRight;
            bottomRight.x = topLeft.x;
            bottomRight.x += windowSize.x;
            bottomRight.y = topLeft.y;
            bottomRight.y += windowSize.y;



            // Colors
            ImDrawList* drawList = ImGui::GetBackgroundDrawList();
            ImU32 bgColor = IM_COL32(0, 0, 0, 200);              // Darker background
            ImU32 borderColor = IM_COL32(255, 255, 255, 120);    // Subtle white border

            drawList->AddRectFilled(topLeft, bottomRight, bgColor, 3.0f);
            drawList->AddRect(topLeft, bottomRight, borderColor, 3.0f, 0, 1.5f);

            // UI Text
            ImGui::SetNextWindowPos(pos, ImGuiCond_Always, ImVec2(0.5f, 1.0f));
            ImGui::Begin("RE_InteractHint", nullptr,
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoInputs);

            //if (reFont)
            //    ImGui::PushFont(reFont);

            ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.9f, 1.0f), "%s", text);

            //if (reFont)
            //    ImGui::PopFont();

            ImGui::End();
        }
    }
}
