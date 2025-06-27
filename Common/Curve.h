#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "Shader.h"

using namespace std;

class Curve
{
public:
    Curve();
    inline void setControlPoints(vector <glm::vec3> controlPoints_in) { this->controlPoints = controlPoints_in; }
    void setShader(Shader* shader_in);
    virtual void generateCurve(int pointsPerSegment) = 0;
    void drawCurve(glm::vec4 color);
    int getNbCurvePoints() { return curvePoints.size(); }
    glm::vec3 getPointOnCurve(int i) { return curvePoints[i]; }
    void setupCurveGeometry(); 

protected:
    vector <glm::vec3> controlPoints;
    vector <glm::vec3> curvePoints;
    glm::mat4 M;
    GLuint VAO_Curve;
    GLuint VBO_Curve;
    Shader* shader;
};