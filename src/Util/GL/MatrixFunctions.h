// MatrixFunctions.h

#pragma once

#include <glm/glm.hpp>
#include <OVR.h>

glm::mat4 makeChassisMatrix_glm(
    float chassisYaw,
    float chassisPitch,
    float chassisRoll,
    glm::vec3 chassisPos);

glm::mat4 makeMatrixFromPose(const ovrPosef& eyePose, float headSize=1.f);

OVR::Matrix4f makeOVRMatrixFromGlmMatrix(const glm::mat4& gm);
