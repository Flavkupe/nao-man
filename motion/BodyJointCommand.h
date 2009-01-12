
#ifndef _BodyJointCommand_h_DEFINED
#define _BodyJointCommand_h_DEFINED

/**
 * BodyBodyJointCommand is a class that holds a single command for a subset of
 * all body joints (excluding the head). All joints and time must be set during
 * instantiation and can't be changed after that. No setters in here.
 */

#include <vector>
#include "Kinematics.h"
using namespace Kinematics;

class BodyJointCommand
{
  public:
    BodyJointCommand(const float time, const std::vector<float> *body_joints,
		     const Kinematics::InterpolationType _type);
    BodyJointCommand(const float time, ChainID chainId,
		     const std::vector<float> *joints,
		     const Kinematics::InterpolationType _type);
    BodyJointCommand(const float time, const std::vector<float> *larm,
                                       const std::vector<float> *lleg,
                                       const std::vector<float> *rleg,
                                       const std::vector<float> *rarm,
		     const Kinematics::InterpolationType _type);
    BodyJointCommand(const BodyJointCommand &other);
    ~BodyJointCommand(void);

    const float getDuration() const { return duration; }
    const std::vector<float>* getJoints(ChainID chainID) const;
    const std::vector<float>* getLArmJoints(void) const { return larm_joints; }
    const std::vector<float>* getLLegJoints(void) const { return lleg_joints; }
    const std::vector<float>* getRLegJoints(void) const { return rleg_joints; }
    const std::vector<float>* getRArmJoints(void) const { return rarm_joints; }
    const Kinematics::InterpolationType getType() const { return type; }

    const bool conflicts(const BodyJointCommand &other) const;
    const bool conflicts(const float chainTimeRemaining[NUM_CHAINS]) const;

  private:
    const float duration;
    const Kinematics::InterpolationType type;
    const std::vector<float> *larm_joints;
    const std::vector<float> *lleg_joints;
    const std::vector<float> *rleg_joints;
    const std::vector<float> *rarm_joints;

};

#endif
