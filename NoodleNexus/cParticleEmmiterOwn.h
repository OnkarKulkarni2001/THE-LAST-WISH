#pragma once
#include "sharedThings.h"
#include <vector>

class cParticleEmmiterOwn {
public:
    struct sParticle {
        sMesh* particleMesh;
        glm::vec3 position;
        glm::vec3 velocity;
        float life = 0.0f;     // Remaining life in seconds
        float maxLife = 1.0f;
        bool active = false;   // Used instead of deleting

        sParticle();
    };

    static const int MAX_PARTICLES = 1000;
    std::vector<sParticle> particles;
    std::vector<glm::mat4> instanceModelMatrices;

    GLuint instanceVBO;
    cParticleEmmiterOwn();  // Constructor

    void UpdateParticles(float deltaTime);
    void EmitNumParticles(glm::vec3 position, glm::vec3 velocity, float lifetime, int numParticles);
    void InitializeParticles();
    void EmitParticle(const glm::vec3& position, const glm::vec3& velocity, float lifetime);
    void DrawParticles();
    void SetupInstancedRendering();
};
