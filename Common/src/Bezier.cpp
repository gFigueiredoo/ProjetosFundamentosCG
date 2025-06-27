#include "Bezier.h"

Bezier::Bezier() : speed_(0.01f), follow_trajectory_(false)
{
    M = glm::mat4(
        -1.0f,  3.0f, -3.0f, 1.0f,
         3.0f, -6.0f,  3.0f, 0.0f,
        -3.0f,  3.0f,  0.0f, 0.0f,
         1.0f,  0.0f,  0.0f, 0.0f
    );
}

void Bezier::generateCurve(int pointsPerSegment)
{
    if (controlPoints.size() < 4)
    {
        curvePoints.clear();
        return;
    }

    curvePoints.clear();
    float step = 1.0f / (float)pointsPerSegment;

    for (int i = 0; i <= (int)controlPoints.size() - 4; i += 3)
    {
        glm::vec3 P0 = controlPoints[i];
        glm::vec3 P1 = controlPoints[i + 1];
        glm::vec3 P2 = controlPoints[i + 2];
        glm::vec3 P3 = controlPoints[i + 3];

        glm::mat4x3 G_mat(P0, P1, P2, P3);

        for (float t = 0.0; t <= 1.0; t += step)
        {
            glm::vec4 T_vec(t*t*t, t*t, t, 1.0f);
            
            glm::vec3 p = G_mat * M * T_vec; 

            curvePoints.push_back(p);
        }
    }
    setupCurveGeometry();
}