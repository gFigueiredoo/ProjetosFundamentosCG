#pragma once
#include "Curve.h"

class Bezier : public Curve
{
public:
    Bezier();
    void generateCurve(int pointsPerSegment) override;
    void setSpeed(float s) { speed_ = s; }
    float getSpeed() const { return speed_; }
    void setFollowTrajectory(bool f) { follow_trajectory_ = f; }
    bool getFollowTrajectory() const { return follow_trajectory_; }

private:
    float speed_;
    bool follow_trajectory_;
};