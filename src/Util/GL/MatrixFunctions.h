// MatrixFunctions.h

#pragma once

#include <glm/glm.hpp>

#ifdef USE_OCULUSSDK
#include <OVR.h>
#endif

glm::mat4 makeChassisMatrix_glm(
    float chassisYaw,
    float chassisPitch,
    float chassisRoll,
    glm::vec3 chassisPos);

#ifdef USE_OCULUSSDK
glm::mat4 makeMatrixFromPose(const ovrPosef& eyePose, float headSize=1.f);

OVR::Matrix4f makeOVRMatrixFromGlmMatrix(const glm::mat4& gm);
#endif
