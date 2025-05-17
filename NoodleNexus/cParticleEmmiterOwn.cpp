#include "cParticleEmmiterOwn.h"
#include <glm/gtc/matrix_transform.hpp> 
#include <iostream>

extern cVAOManager* g_pMeshManager;
extern GLuint program;
void DrawMesh(sMesh* pCurMesh, glm::mat4 matModel, GLuint program, bool SetTexturesFromMeshInfo = true);
sMesh* g_pFindMeshByFriendlyName(std::string theNameToFind);
extern cParticleEmmiterOwn* g_myParticleEmmiter;
extern double deltaTime;

cParticleEmmiterOwn::cParticleEmmiterOwn() {
    particles.resize(MAX_PARTICLES);
    //for (int i = 0; i < MAX_PARTICLES; i++) {
    //    particles[i].active = false;
    //}
    //InitializeParticles();
    //sParticle particle;
    //particle.particleMesh = g_pFindMeshByFriendlyName("particle");
    //sModelDrawInfo drawInfo;

    //if (::g_pMeshManager->FindDrawInfoByModelName(particle.particleMesh->modelFileName, drawInfo)) {
    //    glBindVertexArray(drawInfo.VAO_ID);

    //    glGenBuffers(1, &instanceVBO);
    //    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    //    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    //    std::size_t vec4Size = sizeof(glm::vec4);
    //    GLsizei stride = sizeof(glm::mat4);

    //    GLint location = glGetAttribLocation(program, "instanceModelMatrix");

    //    for (int i = 0; i < 4; ++i) {
    //        glEnableVertexAttribArray(location + i); // Using attribute locations 4–7
    //        glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(i * vec4Size));
    //        glVertexAttribDivisor(location + i, 1); // Advance per instance
    //    }

    //    glBindVertexArray(0);
    //}
}

//void cParticleEmmiterOwn::UpdateParticles(float deltaTime) {
//    for (int i = 0; i < MAX_PARTICLES; i++) {
//        if (!particles[i].active) {
//            continue;
//        }
//
//        particles[i].life -= deltaTime;
//        if (particles[i].life <= 0.0f) {
//            particles[i].active = false;
//            continue;
//        }
//
//        particles[i].position += particles[i].velocity * deltaTime;
//    }
//}
void cParticleEmmiterOwn::UpdateParticles(float deltaTime)
{
    instanceModelMatrices.clear();  // Reset every frame

    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!particles[i].active)
            continue;

        particles[i].life -= deltaTime;
        if (particles[i].life <= 0.0f) {
            particles[i].active = false;
            continue;
        }

        // Update position based on velocity
        particles[i].position += particles[i].velocity * deltaTime;

        //std::cout << "ParticlePosition: (" << particles[i].position.x << ", " << particles[i].position.y << ", " << particles[i].position.z << ")" << std::endl;
        //EmitParticle(particles[i].position, particles[i].velocity, particles[i].life);

        // Add matrix for instanced rendering
        glm::mat4 model = glm::translate(glm::mat4(1.0f), particles[i].position);
        instanceModelMatrices.push_back(model);
        std::cout << "Instance count: " << instanceModelMatrices.size() << std::endl;

    }

    // Upload instance matrix data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    if (!instanceModelMatrices.empty()) {
        glBufferSubData(GL_ARRAY_BUFFER, 0, instanceModelMatrices.size() * sizeof(glm::mat4), instanceModelMatrices.data());
    }
    //DrawParticles();
}


//void cParticleEmmiterOwn::EmitParticle(const glm::vec3& position, const glm::vec3& velocity, float lifetime) {
//    for (int i = 0; i < MAX_PARTICLES; i++) {
//        if (!particles[i].active) {
//            float offsetRange = 0.5f;
//
//            float offsetX = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
//            float offsetY = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
//            float offsetZ = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
//
//            glm::vec3 randomOffset(offsetX, offsetY, offsetZ);
//            glm::vec3 finalVelocity = velocity + randomOffset;
//
//            particles[i].position = position;
//            particles[i].velocity = finalVelocity;
//            particles[i].life = lifetime;
//            particles[i].maxLife = lifetime;
//            particles[i].active = true;
//
//            sMesh* particleMesh = g_pFindMeshByFriendlyName("particle");
//            particles[i].particleMesh = particleMesh;
//            particles[i].particleMesh->positionXYZ = particles[i].position;
//
//            //DrawMesh(particles[i].particleMesh, glm::mat4(1.0f), program);
//
//            sModelDrawInfo meshToDrawInfo;
//            if (::g_pMeshManager->FindDrawInfoByModelName(particles[i].particleMesh->modelFileName, meshToDrawInfo))
//            {
//                // Found the model
//                glBindVertexArray(meshToDrawInfo.VAO_ID); 		// enable VAO(and everything else)
//                //https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElements.xhtml
//                glDrawElements(GL_TRIANGLES,
//                    meshToDrawInfo.numberOfIndices,
//                    GL_UNSIGNED_INT,
//                    (void*)0);
//
//                glBindVertexArray(0); 			//disable VAO(and everything else)
//            }
//
//            break;
//        }
//    }
//}
void cParticleEmmiterOwn::EmitNumParticles(glm::vec3 position, glm::vec3 velocity, float lifetime, int numParticles)
{
    // Emit the requested number of particles
    int emitted = 0;
    for (int i = 0; i < MAX_PARTICLES && emitted < numParticles; i++) {
        if (!particles[i].active) {
            float offsetRange = 0.5f;
            float offsetX = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
            float offsetY = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
            float offsetZ = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;

            glm::vec3 velocityWithSpread = velocity + glm::vec3(offsetX, offsetY, offsetZ);

            particles[i].position = position;
            particles[i].velocity = velocityWithSpread;
            particles[i].life = lifetime;
            particles[i].maxLife = lifetime;
            particles[i].active = true;

            particles[i].particleMesh = g_pFindMeshByFriendlyName("particle");
            particles[i].particleMesh->positionXYZ = particles[i].position;

            emitted++;
        }
    }

    //UpdateParticles(deltaTime);
}

void cParticleEmmiterOwn::InitializeParticles() {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        sParticle particle;
        particle.particleMesh = g_pFindMeshByFriendlyName("particle");
        particle.position = glm::vec3(0.0f);
        particle.velocity = glm::vec3(0.0f);
        particle.life = 0.0f;
        particle.maxLife = 0.0f;
        particle.active = false;

        particles[i] = particle;
        //particles.push_back(particle);
    }
    //sParticle particle;
    //particle.particleMesh = g_pFindMeshByFriendlyName("particle");
    //sModelDrawInfo drawInfo;

    //if (::g_pMeshManager->FindDrawInfoByModelName(particle.particleMesh->modelFileName, drawInfo)) {
    //    glBindVertexArray(drawInfo.VAO_ID);

    //    glGenBuffers(1, &g_myParticleEmmiter->instanceVBO);
    //    glBindBuffer(GL_ARRAY_BUFFER, g_myParticleEmmiter->instanceVBO);
    //    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    //    std::size_t vec4Size = sizeof(glm::vec4);
    //    GLsizei stride = sizeof(glm::mat4);

    //    GLint location = glGetAttribLocation(program, "instanceModelMatrix");

    //    for (int i = 0; i < 4; ++i) {
    //        glEnableVertexAttribArray(location + i); // Using attribute locations 4–7
    //        glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(i * vec4Size));
    //        glVertexAttribDivisor(location + i, 1); // Advance per instance
    //    }

    //    glBindVertexArray(0);
    //}
    SetupInstancedRendering();
}

void cParticleEmmiterOwn::EmitParticle(const glm::vec3& position, const glm::vec3& baseVelocity, float lifetime)
{
    //for (int i = 0; i < MAX_PARTICLES; i++) {
    //    if (!particles[i].active) {
    //        // Add some randomized offset to the velocity for natural spread
    //        float offsetRange = 0.5f;

    //        float offsetX = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
    //        float offsetY = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
    //        float offsetZ = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;

    //        glm::vec3 velocityWithSpread = baseVelocity + glm::vec3(offsetX, offsetY, offsetZ);

    //        // Initialize particle
    //        particles[i].position = position;
    //        particles[i].velocity = velocityWithSpread;
    //        particles[i].life = lifetime;
    //        particles[i].maxLife = lifetime;
    //        particles[i].active = true;

    //        // Set mesh pointer (shared across particles)
    //        particles[i].particleMesh = g_pFindMeshByFriendlyName("particle");

    //        particles[i].particleMesh->positionXYZ = particles[i].position;
    //        // Add instance matrix to render list
    //        //glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), position);
    //        //instanceModelMatrices.push_back(modelMatrix);

    //        break; // Emit only one particle per call
    //    }
    //}
    /*int emitted = 0;
    for (int i = 0; i < MAX_PARTICLES && emitted < numberToEmit; i++) {
        if (!particles[i].active) {
            float offsetRange = 0.5f;
            float offsetX = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
            float offsetY = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;
            float offsetZ = (static_cast<float>(rand()) / RAND_MAX) * offsetRange * 2.0f - offsetRange;

            glm::vec3 velocityWithSpread = baseVelocity + glm::vec3(offsetX, offsetY, offsetZ);

            particles[i].position = position;
            particles[i].velocity = velocityWithSpread;
            particles[i].life = lifetime;
            particles[i].maxLife = lifetime;
            particles[i].active = true;

            particles[i].particleMesh = g_pFindMeshByFriendlyName("particle");
            particles[i].particleMesh->positionXYZ = particles[i].position;

            emitted++;
        }
    }*/
}

void cParticleEmmiterOwn::DrawParticles()
{
    if (particles.empty() || instanceModelMatrices.empty())
        return;

    sMesh* particleMesh = g_pFindMeshByFriendlyName("particle");
    if (!particleMesh)
        return;

    sModelDrawInfo drawInfo;
    if (!g_pMeshManager->FindDrawInfoByModelName(particleMesh->modelFileName, drawInfo))
        return;

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    //glBufferSubData(GL_ARRAY_BUFFER, 0, instanceModelMatrices.size() * sizeof(glm::mat4), instanceModelMatrices.data());

    glUseProgram(program);

    glUniform1f(glGetUniformLocation(program, "bIsParticle"), (GLfloat)GL_TRUE);


    glBindVertexArray(drawInfo.VAO_ID);

    // Draw using instancing
    glDrawElementsInstanced(
        GL_TRIANGLES,
        drawInfo.numberOfIndices,
        GL_UNSIGNED_INT,
        (void*)0,
        static_cast<GLsizei>(instanceModelMatrices.size())
    );

    glBindVertexArray(0);

    glUniform1f(glGetUniformLocation(program, "bIsParticle"), (GLfloat)GL_FALSE);

}

cParticleEmmiterOwn::sParticle::sParticle()
{
    //this->particleMesh = g_pFindMeshByFriendlyName("particle");
    //sModelDrawInfo drawInfo;

    //if (::g_pMeshManager->FindDrawInfoByModelName(this->particleMesh->modelFileName, drawInfo)) {
    //    glBindVertexArray(drawInfo.VAO_ID);

    //    glGenBuffers(1, &g_myParticleEmmiter->instanceVBO);
    //    glBindBuffer(GL_ARRAY_BUFFER, g_myParticleEmmiter->instanceVBO);
    //    glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

    //    std::size_t vec4Size = sizeof(glm::vec4);
    //    GLsizei stride = sizeof(glm::mat4);

    //    GLint location = glGetAttribLocation(program, "instanceModelMatrix");

    //    for (int i = 0; i < 4; ++i) {
    //        glEnableVertexAttribArray(location + i); // Using attribute locations 4–7
    //        glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(i * vec4Size));
    //        glVertexAttribDivisor(location + i, 1); // Advance per instance
    //    }

    //    glBindVertexArray(0);
    //}
}

void cParticleEmmiterOwn::SetupInstancedRendering() {
    sMesh* particleMesh = g_pFindMeshByFriendlyName("particle");
    //sParticle& particle = particles[0];
    if (!particleMesh) return;

    sModelDrawInfo drawInfo;
    if (::g_pMeshManager->FindDrawInfoByModelName(particleMesh->modelFileName, drawInfo)) {
        glBindVertexArray(drawInfo.VAO_ID);

        glGenBuffers(1, &instanceVBO);
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
        glBufferData(GL_ARRAY_BUFFER, MAX_PARTICLES * sizeof(glm::mat4), nullptr, GL_DYNAMIC_DRAW);

        std::size_t vec4Size = sizeof(glm::vec4);
        GLsizei stride = sizeof(glm::mat4);
        GLint location = glGetAttribLocation(program, "instanceModelMatrix");

        for (int i = 0; i < 4; ++i) {
            glEnableVertexAttribArray(location + i);
            glVertexAttribPointer(location + i, 4, GL_FLOAT, GL_FALSE, stride, (void*)(i * vec4Size));
            glVertexAttribDivisor(location + i, 1);
        }

        glBindVertexArray(0);
    }
}
