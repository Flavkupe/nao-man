#include <iostream>
#include "Kinematics.h"

const float Kinematics::clip(const float value,
                             const float minValue, const float maxValue) {
    if (value > maxValue)
        return maxValue;
    else if (value < minValue)
        return minValue;
    else
        return value;
}


/**
 * This method will destructively clip the chain angles that are passed to it.
 * This means that it will modify the array that was passed by reference
 * and will return nothing. The old values will be lost.
 */
const void Kinematics::clipChainAngles(const ChainID chainID,
                                       float angles[]) {
    switch (chainID) {
    case LLEG_CHAIN:
        for (int i=0; i<LEG_JOINTS; i++) {
            angles[i] = clip(angles[i],
                             LEFT_LEG_BOUNDS[i][0],LEFT_LEG_BOUNDS[i][1]);
        }
        break;
    case RLEG_CHAIN:
        for (int i=0; i<LEG_JOINTS; i++) {
            angles[i] = clip(angles[i],
                             RIGHT_LEG_BOUNDS[i][0],RIGHT_LEG_BOUNDS[i][1]);
        }
        break;
    case LARM_CHAIN:
        for (int i=0; i<ARM_JOINTS; i++) {
            angles[i] = clip(angles[i],
                             LEFT_ARM_BOUNDS[i][0],LEFT_ARM_BOUNDS[i][1]);
        }
        break;
    case RARM_CHAIN:
        for (int i=0; i<ARM_JOINTS; i++) {
            angles[i] = clip(angles[i],
                             RIGHT_ARM_BOUNDS[i][0],RIGHT_ARM_BOUNDS[i][1]);
        }
        break;
    case HEAD_CHAIN:
    default:
        for (int i=0; i<HEAD_JOINTS; i++) {
            angles[i] = clip(angles[i],
                             HEAD_BOUNDS[i][0],HEAD_BOUNDS[i][1]);
        }
        break;
    }
}


const float Kinematics::getMinValue(const ChainID id, const int jointNumber) {
    switch (id) {
    case LARM_CHAIN:
        return LEFT_ARM_BOUNDS[jointNumber][0];
    case RARM_CHAIN:
        return RIGHT_ARM_BOUNDS[jointNumber][0];
    case LLEG_CHAIN:
        return LEFT_LEG_BOUNDS[jointNumber][0];
    case RLEG_CHAIN:
        return RIGHT_LEG_BOUNDS[jointNumber][0];
    case HEAD_CHAIN:
    default:
        return HEAD_BOUNDS[jointNumber][0];
    }
    return 0;
}


const float Kinematics::getMaxValue(const ChainID id, const int jointNumber) {
    switch (id) {
    case LARM_CHAIN:
        return LEFT_ARM_BOUNDS[jointNumber][1];
    case RARM_CHAIN:
        return RIGHT_ARM_BOUNDS[jointNumber][1];
    case LLEG_CHAIN:
        return LEFT_LEG_BOUNDS[jointNumber][1];
    case RLEG_CHAIN:
        return RIGHT_LEG_BOUNDS[jointNumber][1];
    case HEAD_CHAIN:
    default: // hack! I don't want to use exceptions in this code.
        return HEAD_BOUNDS[jointNumber][0];
    }
    return 0;
}


/* Perform forward kinematics on a set of angles.
   Returns an x,y,z point where the end of a limb would be with a given
   chainID and chain angles. The returned value is a 3 by 1 vector.
*/
const ublas::vector<float> Kinematics::forwardKinematics(const ChainID id,
                                                         const float angles[]) {
    ublas::vector<float> finalPoint(3);
    float x=0,y=0,z=0;

    // Variables for legs. These will save computation by storing certain useful
    // things only ones
    float HYP, HP, HR, KP, AP, AR, sinHYP, cosHYP, sinHP, cosHP, sinHR, cosHR,
        sinKP, cosKP, sinAP, cosAP, sinAR, cosAR, root2, sinAPplusKP, cosAPplusKP;

    // Variables for arms.
    float SP, SR, EY, ER, sinSP, cosSP, sinSR, cosSR, sinEY, cosEY, sinER, cosER;

    switch(id) {
    case LARM_CHAIN:
        /* Extract the correct angles from the chainAngles tuple.
           Sines and cosines get reused a lot, so calculate them once.
           # SP - shoulder pitch
           # SR - shoulder roll 
           # EY - elbow yaw
           # ER - elbow roll */
        SP = angles[0];
        SR = angles[1];
        EY = angles[2];
        ER = angles[3];
        sinSP = sin(SP);
        cosSP = cos(SP);
        sinSR = sin(SR);
        cosSR = cos(SR);
        sinEY = sin(EY);
        cosEY = cos(EY);
        sinER = sin(ER);
        cosER = cos(ER);

        /* These are precalculated functions that take some joint
           angles and return single coordinate components. Pretty sweet.
           If you want to see the derivation of these formulas, take a look
           at the method 'calcLinkTransforms'.
           The formulas were precalculated using Mathematica and the notebook
           in which this was done is in svn under nao_robocup/kinematicsTester */
        x = LOWER_ARM_LENGTH*sinER*sinEY*sinSP + cosSP*((UPPER_ARM_LENGTH + LOWER_ARM_LENGTH*cosER)*cosSR - LOWER_ARM_LENGTH*cosEY*sinER*sinSR);

        y = SHOULDER_OFFSET_Y + LOWER_ARM_LENGTH*cosEY*cosSR*sinER + (UPPER_ARM_LENGTH + LOWER_ARM_LENGTH*cosER)*sinSR;

        z = SHOULDER_OFFSET_Z + LOWER_ARM_LENGTH*cosSP*sinER*sinEY - (UPPER_ARM_LENGTH + LOWER_ARM_LENGTH*cosER)*cosSR*sinSP + LOWER_ARM_LENGTH*cosEY*sinER*sinSP*sinSR;

        break;

    case RARM_CHAIN:
        SP = angles[0];
        SR = angles[1];
        EY = angles[2];
        ER = angles[3];
        sinSP = sin(SP);
        cosSP = cos(SP);
        sinSR = sin(SR);
        cosSR = cos(SR);
        sinEY = sin(EY);
        cosEY = cos(EY);
        sinER = sin(ER);
        cosER = cos(ER);

        x = LOWER_ARM_LENGTH*sinER*sinEY*sinSP + cosSP* ((UPPER_ARM_LENGTH + LOWER_ARM_LENGTH*cosER)*cosSR - LOWER_ARM_LENGTH*cosEY*sinER*sinSR);

        y = - SHOULDER_OFFSET_Y + LOWER_ARM_LENGTH*cosEY*cosSR*sinER + (UPPER_ARM_LENGTH + LOWER_ARM_LENGTH*cosER)*sinSR;

        z = SHOULDER_OFFSET_Z + LOWER_ARM_LENGTH*cosSP*sinER*sinEY - (UPPER_ARM_LENGTH + LOWER_ARM_LENGTH*cosER)*cosSR*sinSP + LOWER_ARM_LENGTH*cosEY*sinER*sinSP*sinSR;

        break;

    case LLEG_CHAIN:
        HYP = angles[0];
        HP = angles[1];
        HR = angles[2];
        KP = angles[3];
        AP = angles[4];
        AR = angles[5];
        sinHYP = sin(HYP);
        cosHYP = cos(HYP);
        sinHP = sin(HP);
        cosHP = cos(HP);
        sinHR = sin(HR);
        cosHR = cos(HR);
        sinKP = sin(KP);
        cosKP = cos(KP);
        sinAP = sin(AP);
        cosAP = cos(AP);
        sinAR = sin(AR);
        cosAR = cos(AR);
        root2 = sqrt(2);
        sinAPplusKP = sin(AP + KP);
        cosAPplusKP = cos(AP + KP);

        // uglyyyyyyy
        x = .5f*(-cosHR*(root2*(cosHP*(THIGH_LENGTH + TIBIA_LENGTH*cosKP + FOOT_HEIGHT*cosAR*cosAPplusKP) - FOOT_HEIGHT*sinAR)*sinHYP + 2*cosHYP*sinHP*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP)) + root2*sinHYP*((THIGH_LENGTH + TIBIA_LENGTH*cosKP + FOOT_HEIGHT*cosHP*sinAR)*sinHR + TIBIA_LENGTH*sinHP*sinKP + FOOT_HEIGHT*cosAR*(cosKP*(sinAP*sinHP + cosAP*sinHR) + (cosAP*sinHP - sinAP*sinHR)*sinKP)) + 2*cosHYP*(FOOT_HEIGHT*sinAR*sinHP*sinHR - cosHP*(TIBIA_LENGTH*sinKP + FOOT_HEIGHT*cosAR*sinAPplusKP)));

        y = .5f*(2*HIP_OFFSET_Y + THIGH_LENGTH*sinHR + THIGH_LENGTH*cosHYP*sinHR + TIBIA_LENGTH*cosKP*sinHR + TIBIA_LENGTH*cosHYP*cosKP*sinHR - 2*FOOT_HEIGHT*cosHP*sinAR*sinHR*sin(HYP/2.f)*sin(HYP/2.f) - root2*FOOT_HEIGHT*sinAR*sinHP*sinHR*sinHYP - TIBIA_LENGTH*sinHP*sinKP + TIBIA_LENGTH*cosHYP*sinHP*sinKP + root2*TIBIA_LENGTH*cosHP*sinHYP*sinKP + cosHR*(FOOT_HEIGHT*(1 + cosHYP)*sinAR + (2*cosHP*sin(HYP/2.f)*sin(HYP/2.f) + root2*sinHP*sinHYP)*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP)) + FOOT_HEIGHT*cosAR*(cosKP*((-1 + cosHYP)*sinAP*sinHP + cosAP*(1 + cosHYP)*sinHR) - 2*(cos(HYP/2.)*cos(HYP/2.f))*sinAP*sinHR*sinKP - 2*cosAP*sinHP*(sin(HYP/2.f)*sin(HYP/2.f))*sinKP + root2*cosHP*sinHYP*sinAPplusKP));

        z = .5f*(-2*HIP_OFFSET_Z + FOOT_HEIGHT*cosAR*cosKP*sinAP*sinHP + FOOT_HEIGHT*cosAR*cosHYP*cosKP*sinAP*sinHP - THIGH_LENGTH*sinHR + THIGH_LENGTH*cosHYP*sinHR - TIBIA_LENGTH*cosKP*sinHR - FOOT_HEIGHT*cosAP*cosAR*cosKP*sinHR + TIBIA_LENGTH*cosHYP*cosKP*sinHR + FOOT_HEIGHT*cosAP*cosAR*cosHYP*cosKP*sinHR - root2*FOOT_HEIGHT*sinAR*sinHP*sinHR*sinHYP + TIBIA_LENGTH*sinHP*sinKP + FOOT_HEIGHT*cosAP*cosAR*sinHP*sinKP + TIBIA_LENGTH*cosHYP*sinHP*sinKP + FOOT_HEIGHT*cosAP*cosAR*cosHYP*sinHP*sinKP + FOOT_HEIGHT*cosAR*sinAP*sinHR*sinKP - FOOT_HEIGHT*cosAR*cosHYP*sinAP*sinHR*sinKP + cosHR*(FOOT_HEIGHT*(-1 + cosHYP)*sinAR + root2*sinHP*sinHYP*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP)) + cosHP*(FOOT_HEIGHT*(1 + cosHYP)*sinAR*sinHR - 2*cosHR*(cos(HYP/2.)*cos(HYP/2.))*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP) + root2*sinHYP*(TIBIA_LENGTH*sinKP + FOOT_HEIGHT*cosAR*sinAPplusKP)));
        break;

    case RLEG_CHAIN:
        HYP = angles[0];
        HP = angles[1];
        HR = angles[2];
        KP = angles[3];
        AP = angles[4];
        AR = angles[5];
        sinHYP = sin(HYP);
        cosHYP = cos(HYP);
        sinHP = sin(HP);
        cosHP = cos(HP);
        sinHR = sin(HR);
        cosHR = cos(HR);
        sinKP = sin(KP);
        cosKP = cos(KP);
        sinAP = sin(AP);
        cosAP = cos(AP);
        sinAR = sin(AR);
        cosAR = cos(AR);
        root2 = sqrt(2);
        sinAPplusKP = sin(AP + KP);
        cosAPplusKP = cos(AP + KP);

        x = .5*(-cosHR*(root2*(cosHP*(THIGH_LENGTH + TIBIA_LENGTH*cosKP + FOOT_HEIGHT*cosAR*cosAPplusKP) + FOOT_HEIGHT*sinAR)*sinHYP + 2*cosHYP*sinHP*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP)) + root2*sinHYP*(-(THIGH_LENGTH + TIBIA_LENGTH*cosKP - FOOT_HEIGHT*cosHP*sinAR)*sinHR + TIBIA_LENGTH*sinHP*sinKP + FOOT_HEIGHT*cosAR*(cosKP*(sinAP*sinHP - cosAP*sinHR) + (cosAP*sinHP + sinAP*sinHR)*sinKP)) + 2*cosHYP*(FOOT_HEIGHT*sinAR*sinHP*sinHR - cosHP*(TIBIA_LENGTH*sinKP + FOOT_HEIGHT*cosAR*sinAPplusKP)));

        y = .5*(-2*HIP_OFFSET_Y + THIGH_LENGTH*sinHR + THIGH_LENGTH*cosHYP*sinHR + TIBIA_LENGTH*cosKP*sinHR + TIBIA_LENGTH*cosHYP*cosKP*sinHR + 2*FOOT_HEIGHT*cosHP*sinAR*sinHR*sin(HYP/2.f)*sin(HYP/2.f) + root2*FOOT_HEIGHT*sinAR*sinHP*sinHR*sinHYP + TIBIA_LENGTH*sinHP*sinKP - TIBIA_LENGTH*cosHYP*sinHP*sinKP - root2*TIBIA_LENGTH*cosHP*sinHYP*sinKP + cosHR*(FOOT_HEIGHT*(1 + cosHYP)*sinAR - (2*cosHP*sin(HYP/2.)*sin(HYP/2.f) + root2*sinHP*sinHYP)*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP)) + FOOT_HEIGHT*cosAR*(cosKP*(-(-1 + cosHYP)*sinAP*sinHP + cosAP*(1 + cosHYP)*sinHR) - 2*(cos(HYP/2.)*cos(HYP/2.))*sinAP*sinHR*sinKP + 2*cosAP*sinHP*(sin(HYP/2.f)*sin(HYP/2.f))*sinKP - root2*cosHP*sinHYP*sinAPplusKP));

        z = 0.5*(-2*HIP_OFFSET_Z + FOOT_HEIGHT*cosAR*cosKP*sinAP*sinHP + FOOT_HEIGHT*cosAR*cosHYP*cosKP*sinAP*sinHP + THIGH_LENGTH*sinHR - THIGH_LENGTH*cosHYP*sinHR + TIBIA_LENGTH*cosKP*sinHR + FOOT_HEIGHT*cosAP*cosAR*cosKP*sinHR - TIBIA_LENGTH*cosHYP*cosKP*sinHR - FOOT_HEIGHT*cosAP*cosAR*cosHYP*cosKP*sinHR - root2*FOOT_HEIGHT*sinAR*sinHP*sinHR*sinHYP + TIBIA_LENGTH*sinHP*sinKP + FOOT_HEIGHT*cosAP*cosAR*sinHP*sinKP + TIBIA_LENGTH*cosHYP*sinHP*sinKP + FOOT_HEIGHT*cosAP*cosAR*cosHYP*sinHP*sinKP - FOOT_HEIGHT*cosAR*sinAP*sinHR*sinKP + FOOT_HEIGHT*cosAR*cosHYP*sinAP*sinHR*sinKP + cosHR*((FOOT_HEIGHT - FOOT_HEIGHT*cosHYP)*sinAR + root2*sinHP*sinHYP*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP)) + cosHP*(FOOT_HEIGHT*(1 + cosHYP)*sinAR*sinHR - 2*cosHR*(cos(HYP/2.f)*cos(HYP/2.f))*(THIGH_LENGTH + (TIBIA_LENGTH + FOOT_HEIGHT*cosAP*cosAR)*cosKP - FOOT_HEIGHT*cosAR*sinAP*sinKP) + root2*sinHYP*(TIBIA_LENGTH*sinKP + FOOT_HEIGHT*cosAR*sinAPplusKP)));

        break;

    case LANKLE_CHAIN:
        HYP = angles[0];
        HP = angles[1];
        HR = angles[2];
        KP = angles[3];
        sinHYP = sin(HYP);
        cosHYP = cos(HYP);
        sinHP = sin(HP);
        cosHP = cos(HP);
        sinHR = sin(HR);
        cosHR = cos(HR);
        sinKP = sin(KP);
        cosKP = cos(KP);
        root2 = sqrt(2);
            
        x = ((THIGH_LENGTH + TIBIA_LENGTH*cosKP)*sinHR*sinHYP)/root2 - .5*cosHR*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*(2*cosHYP*sinHP + root2*cosHP*sinHYP) - TIBIA_LENGTH*cosHP*cosHYP*sinKP + (TIBIA_LENGTH*sinHP*sinHYP*sinKP)/root2;

        y = .5*(2*HIP_OFFSET_Y + 2*(cos(HYP/2.f)*cos(HYP/2.f))*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*sinHR + root2*THIGH_LENGTH*cosHR*sinHP*sinHYP + root2*TIBIA_LENGTH*cosHR*cosKP*sinHP*sinHYP - TIBIA_LENGTH*sinHP*sinKP + TIBIA_LENGTH*cosHYP*sinHP*sinKP + cosHP*(2*cosHR*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*(sin(HYP/2.f)*sin(HYP/2.f)) + root2*TIBIA_LENGTH*sinHYP*sinKP));
            
        z = .5*(-2*HIP_OFFSET_Z - 2*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*sinHR*(sin(HYP/2.f)*sin(HYP/2.f)) + root2*THIGH_LENGTH*cosHR*sinHP*sinHYP + root2*TIBIA_LENGTH*cosHR*cosKP*sinHP*sinHYP + TIBIA_LENGTH*sinHP*sinKP + TIBIA_LENGTH*cosHYP*sinHP*sinKP + cosHP*(-2*cosHR*(cos(HYP/2.)*cos(HYP/2.))*(THIGH_LENGTH + TIBIA_LENGTH*cosKP) + root2*TIBIA_LENGTH*sinHYP*sinKP));

        break;

    case RANKLE_CHAIN:
    default:
        HYP = angles[0];
        HP = angles[1];
        HR = angles[2];
        KP = angles[3];
        sinHYP = sin(HYP);
        cosHYP = cos(HYP);
        sinHP = sin(HP);
        cosHP = cos(HP);
        sinHR = sin(HR);
        cosHR = cos(HR);
        sinKP = sin(KP);
        cosKP = cos(KP);
        root2 = sqrt(2);

        x = -(((THIGH_LENGTH + TIBIA_LENGTH*cosKP)*sinHR*sinHYP)/root2) - .5*cosHR*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*(2*cosHYP*sinHP + root2*cosHP*sinHYP) - TIBIA_LENGTH*cosHP*cosHYP*sinKP + (TIBIA_LENGTH*sinHP*sinHYP*sinKP)/root2;

        y = .5*(-2*HIP_OFFSET_Y + 2*(cos(HYP/2.)*cos(HYP/2.))*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*sinHR - root2*THIGH_LENGTH*cosHR*sinHP*sinHYP - root2*TIBIA_LENGTH*cosHR*cosKP*sinHP*sinHYP + TIBIA_LENGTH*sinHP*sinKP - TIBIA_LENGTH*cosHYP*sinHP*sinKP - cosHP*(2*cosHR*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*(sin(HYP/2.)*sin(HYP/2.)) + root2*TIBIA_LENGTH*sinHYP*sinKP));
            
        z = .5*(-2*HIP_OFFSET_Z + 2*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*sinHR*(sin(HYP/2.)*sin(HYP/2.)) + root2*THIGH_LENGTH*cosHR*sinHP*sinHYP + root2*TIBIA_LENGTH*cosHR*cosKP*sinHP*sinHYP + TIBIA_LENGTH*sinHP*sinKP + TIBIA_LENGTH*cosHYP*sinHP*sinKP + cosHP*(-2*cosHR*(cos(HYP/2.)*cos(HYP/2.))*(THIGH_LENGTH + TIBIA_LENGTH*cosKP) + root2*TIBIA_LENGTH*sinHYP*sinKP));

        break;
    }
    finalPoint(0) = x;
    finalPoint(1) = y;
    finalPoint(2) = z;

    return finalPoint;
}


const ublas::matrix<float> Kinematics::buildHeelJacobian(const ChainID chainID,
                                                         const float angles[]) {
    float HYP = angles[0];
    float HP = angles[1];
    float HR = angles[2];
    float KP = angles[3];
    float AP = angles[4];
    float AR = angles[5];

    float sinHYP = sin(HYP);
    float cosHYP = cos(HYP);
    float sinHP = sin(HP);
    float cosHP = cos(HP);
    float sinHR = sin(HR);
    float cosHR = cos(HR);
    float sinKP = sin(KP);
    float cosKP = cos(KP);
    float sinAP = sin(AP);
    float cosAP = cos(AP);
    float sinAR = sin(AR);
    float cosAR = cos(AR);
    float sinAPplusKP = sin(AP + KP);
    float cosAPplusKP = cos(AP + KP);
    float root2 = sqrt(2.0f);

    if (chainID == LLEG_CHAIN) {
        float j_1_1 = -.5*FOOT_HEIGHT*cosAR*(sinAP*(root2*cosKP*sinHR*sinHYP - cosHR*cosKP*(2*cosHYP*sinHP + root2*cosHP*sinHYP) - 2*cosHP*cosHYP*sinKP + root2*sinHP*sinHYP*sinKP) - cosAP*(root2*cosKP*sinHP*sinHYP + 2*cosHR*cosHYP*sinHP*sinKP - root2*sinHR*sinHYP*sinKP + cosHP*(-2*cosHYP*cosKP + root2*cosHR*sinHYP*sinKP)));

        float j_1_2 = .5*FOOT_HEIGHT*(cosAR*(2*cosHYP*sinHP*sinHR + root2*(cosHR + cosHP*sinHR)*sinHYP) + cosAP*sinAR*(cosHR*cosKP*(2*cosHYP*sinHP + root2*cosHP*sinHYP) - root2*sinHYP*(cosKP*sinHR + sinHP*sinKP)) - sinAR*(root2*cosKP*sinAP*sinHP*sinHYP - root2*sinAP*sinHR*sinHYP*sinKP + cosHR*sinAP*(2*cosHYP*sinHP + root2*cosHP*sinHYP)*sinKP - 2*cosHP*cosHYP*sinAPplusKP));

        float j_2_1 = .5*FOOT_HEIGHT*cosAR*(-sinAP*(cosKP*((1 + cosHYP)*sinHR + root2*cosHR*sinHP*sinHYP) + (-1 + cosHYP)*sinHP*sinKP) + cosAP*((-1 + cosHYP)*cosKP*sinHP - ((1 + cosHYP)*sinHR + root2*cosHR*sinHP*sinHYP)*sinKP) + cosHP*(root2*cosAPplusKP*sinHYP - 2*cosHR*(sin(HYP/2.f)*sin(HYP/2.f))*sinAPplusKP));

        float j_2_2 = .5*FOOT_HEIGHT*(cosAR*(cosHR*(1 + cosHYP) + sinHR*(cosHP*(-1 + cosHYP) - root2*sinHP*sinHYP)) + sinAR*(sinAP*(cosKP*(sinHP - cosHYP*sinHP - root2*cosHP*sinHYP) + (-cosHP*cosHR*(-1 + cosHYP) + sinHR + cosHYP*sinHR + root2*cosHR*sinHP*sinHYP)*sinKP) - cosAP*(cosKP*(sinHR + cosHYP*sinHR + root2*cosHR*sinHP*sinHYP) + (-1 + cosHYP)*sinHP*sinKP + cosHP*(-cosHR*(-1 + cosHYP)*cosKP + root2*sinHYP*sinKP))));

        float j_3_1 = .5*FOOT_HEIGHT*cosAR*(-sinAP*(cosKP*((-1 + cosHYP)*sinHR + root2*cosHR*sinHP*sinHYP) + (1 + cosHYP)*sinHP*sinKP) + cosAP*((1 + cosHYP)*cosKP*sinHP + (sinHR - cosHYP*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP) + cosHP*(root2*cosAPplusKP*sinHYP + 2*cosHR*(cos(HYP/2.f)*cos(HYP/2.f))*sinAPplusKP));

        float j_3_2 = .5*FOOT_HEIGHT*(cosAR*(cosHR*(-1 + cosHYP) + sinHR*(cosHP*(1 + cosHYP) - root2*sinHP*sinHYP)) + sinAR*(-sinAP*(cosKP*((1 + cosHYP)*sinHP + root2*cosHP*sinHYP) + (cosHP*cosHR*(1 + cosHYP) + sinHR - cosHYP*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP) + cosAP*(cosKP*(sinHR - cosHYP*sinHR - root2*cosHR*sinHP*sinHYP) - (1 + cosHYP)*sinHP*sinKP + cosHP*(cosHR*(1 + cosHYP)*cosKP - root2*sinHYP*sinKP))));

        ublas::matrix<float> jacobian(3,2);
        jacobian(0,0) = j_1_1;  jacobian(0,1) = j_1_2;
        jacobian(1,0) = j_2_1;  jacobian(1,1) = j_2_2;
        jacobian(2,0) = j_3_1;  jacobian(2,1) = j_3_2;
        return jacobian;
    }

    else if (chainID == RLEG_CHAIN) {

        float j_1_1 = .5*FOOT_HEIGHT*cosAR*(sinAP*(root2*cosKP*sinHR*sinHYP + cosHR*cosKP*(2*cosHYP*sinHP + root2*cosHP*sinHYP) + 2*cosHP*cosHYP*sinKP - root2*sinHP*sinHYP*sinKP) + cosAP*(root2*cosKP*sinHP*sinHYP + 2*cosHR*cosHYP*sinHP*sinKP + root2*sinHR*sinHYP*sinKP + cosHP*(-2*cosHYP*cosKP + root2*cosHR*sinHYP*sinKP)));

        float j_1_2 = .5*FOOT_HEIGHT*(cosAR*(2*cosHYP*sinHP*sinHR + root2*(-cosHR + cosHP*sinHR)*sinHYP) + cosAP*sinAR*(cosHR*cosKP*(2*cosHYP*sinHP + root2*cosHP*sinHYP) + root2*sinHYP*(cosKP*sinHR - sinHP*sinKP)) - sinAR*(root2*cosKP*sinAP*sinHP*sinHYP + root2*sinAP*sinHR*sinHYP*sinKP + cosHR*sinAP*(2*cosHYP*sinHP + root2*cosHP*sinHYP)*sinKP - 2*cosHP*cosHYP*sinAPplusKP));

        float j_2_1 = -.5*FOOT_HEIGHT*cosAR*(sinAP*(cosKP*((1 + cosHYP)*sinHR - root2*cosHR*sinHP*sinHYP) - (-1 + cosHYP)*sinHP*sinKP) + cosAP*((-1 + cosHYP)*cosKP*sinHP + ((1 + cosHYP)*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP) + cosHP*(root2*cosAPplusKP*sinHYP - 2*cosHR*(sin(HYP/2.f)*sin(HYP/2.f))*sinAPplusKP));

        float j_2_2 = .5*FOOT_HEIGHT*(cosAR*(cosHR*(1 + cosHYP) + sinHR*(cosHP - cosHP*cosHYP + root2*sinHP*sinHYP)) + sinAR*(sinAP*(cosKP*((-1 + cosHYP)*sinHP + root2*cosHP*sinHYP) + (cosHP*cosHR*(-1 + cosHYP) + sinHR + cosHYP*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP) + cosAP*(-cosKP*(sinHR + cosHYP*sinHR - root2*cosHR*sinHP*sinHYP) + (-1 + cosHYP)*sinHP*sinKP + cosHP*(-cosHR*(-1 + cosHYP)*cosKP + root2*sinHYP*sinKP))));

        float j_3_1 = .5*FOOT_HEIGHT*cosAR*(sinAP*(cosKP*((-1 + cosHYP)*sinHR - root2*cosHR*sinHP*sinHYP) - (1 + cosHYP)*sinHP*sinKP) + cosAP*((1 + cosHYP)*cosKP*sinHP + ((-1 + cosHYP)*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP) + cosHP*(root2*cosAPplusKP*sinHYP + 2*cosHR*(cos(HYP/2.f)*cos(HYP/2.0f))*sinAPplusKP));
    
        float j_3_2 = -.5*FOOT_HEIGHT*(cosAR*(cosHR*(-1 + cosHYP) - sinHR*(cosHP*(1 + cosHYP) - root2*sinHP*sinHYP)) + sinAR*(sinAP*(cosKP*((1 + cosHYP)*sinHP + root2*cosHP*sinHYP) + (cosHP*cosHR*(1 + cosHYP) + (-1 + cosHYP)*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP) + cosAP*(cosKP*(sinHR - cosHYP*sinHR + root2*cosHR*sinHP*sinHYP) + (1 + cosHYP)*sinHP*sinKP - cosHP*(cosHR*(1 + cosHYP)*cosKP - root2*sinHYP*sinKP))));

        ublas::matrix<float> jacobian(3,2);
        jacobian(0,0) = j_1_1;  jacobian(0,1) = j_1_2;
        jacobian(1,0) = j_2_1;  jacobian(1,1) = j_2_2;
        jacobian(2,0) = j_3_1;  jacobian(2,1) = j_3_2;
        return jacobian;
    }
    else
        throw "Wrong chain";
}
const ublas::matrix<float> Kinematics::buildLegJacobian(const ChainID chainID,
                                                        const float angles[]) {
    float HYP = angles[0];
    float HP = angles[1];
    float HR = angles[2];
    float KP = angles[3];

    float sinHYP = sin(HYP);
    float cosHYP = cos(HYP);
    float sinHP = sin(HP);
    float cosHP = cos(HP);
    float sinHR = sin(HR);
    float cosHR = cos(HR);
    float sinKP = sin(KP);
    float cosKP = cos(KP);
    float root2 = sqrt(2.0f);

    if (chainID == LLEG_CHAIN) {
        float j_1_1 = .5*(sinHP*(root2*cosHR*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*sinHYP + 2*TIBIA_LENGTH*cosHYP*sinKP) + cosHP*(-2*cosHR*cosHYP*(THIGH_LENGTH + TIBIA_LENGTH*cosKP) + root2*TIBIA_LENGTH*sinHYP*sinKP));

        float j_1_2 = .5*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*(2*cosHYP*sinHP*sinHR + root2*(cosHR + cosHP*sinHR)*sinHYP);

        float j_1_3 = .5*TIBIA_LENGTH*(root2*cosKP*sinHP*sinHYP + (2*cosHR*cosHYP*sinHP - root2*sinHR*sinHYP)*sinKP + cosHP*(-2*cosHYP*cosKP + root2*cosHR*sinHYP*sinKP));

        float j_2_1 = .5*(cosHR*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*((-1 + cosHYP)*sinHP + root2*cosHP*sinHYP) + TIBIA_LENGTH*(cosHP*(-1 + cosHYP) - root2*sinHP*sinHYP)*sinKP);

        float j_2_2 = .5*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*(cosHR*(1 + cosHYP) + sinHR*(cosHP*(-1 + cosHYP) - root2*sinHP*sinHYP));

        float j_2_3 = .5*TIBIA_LENGTH*(cosKP*((-1 + cosHYP)*sinHP + root2*cosHP*sinHYP) + (cosHP*cosHR*(-1 + cosHYP) - (1 + cosHYP)*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP);

        float j_3_1 = .5*(cosHR*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*((1 + cosHYP)*sinHP + root2*cosHP*sinHYP) + TIBIA_LENGTH*(cosHP*(1 + cosHYP) - root2*sinHP*sinHYP)*sinKP);

        float j_3_2 = .5*(THIGH_LENGTH + TIBIA_LENGTH*cosKP)*(cosHR*(-1 + cosHYP) + sinHR*(cosHP*(1 + cosHYP) - root2*sinHP*sinHYP));

        float j_3_3 = .5*TIBIA_LENGTH*(cosKP*((1 + cosHYP)*sinHP + root2*cosHP*sinHYP) + (cosHP*cosHR*(1 + cosHYP) + sinHR - cosHYP*sinHR - root2*cosHR*sinHP*sinHYP)*sinKP);

        ublas::matrix<float> jacobian(3,3);
        jacobian(0,0) = j_1_1;  jacobian(0,1) = j_1_2;  jacobian(0,2) = j_1_3;
        jacobian(1,0) = j_2_1;  jacobian(1,1) = j_2_2;  jacobian(1,2) = j_2_3;
        jacobian(2,0) = j_3_1;  jacobian(2,1) = j_3_2;  jacobian(2,2) = j_3_3;
        return jacobian;
    }

    else if (chainID == RLEG_CHAIN) {
        float j_1_1 = -THIGH_LENGTH*cosHR*(cosHP*cosHYP - (sinHP*sinHYP)/root2) - TIBIA_LENGTH*(cosHR*cosKP*(cosHP*cosHYP - (sinHP*sinHYP)/root2) + (-cosHYP*sinHP - (cosHP*sinHYP)/root2)*sinKP);

        float j_1_2 = -THIGH_LENGTH*((cosHR*sinHYP)/root2 - sinHR*(cosHYP*sinHP + (cosHP*sinHYP)/root2)) - TIBIA_LENGTH*cosKP*((cosHR*sinHYP)/root2 - sinHR*(cosHYP*sinHP + (cosHP*sinHYP)/root2));

        float j_1_3 = -TIBIA_LENGTH*(cosKP*(cosHP*cosHYP - (sinHP*sinHYP)/root2) - ((sinHR*sinHYP)/root2 + cosHR*(cosHYP*sinHP + (cosHP*sinHYP)/root2))*sinKP);

        float j_2_1 = -THIGH_LENGTH*cosHR*((-.5 + cosHYP*.5)*sinHP + (cosHP*sinHYP)/root2) - TIBIA_LENGTH*(cosHR*cosKP*((-.5 + cosHYP*.5)*sinHP + (cosHP*sinHYP)/root2) + (cosHP*(-.5 + cosHYP*.5) - (sinHP*sinHYP)/root2)*sinKP);

        float j_2_2 = -THIGH_LENGTH*(cosHR*(-.5 - cosHYP*.5) - sinHR*(-cosHP*(-.5 + cosHYP*.5) + (sinHP*sinHYP)/root2)) - TIBIA_LENGTH*cosKP*(cosHR*(-.5 - cosHYP*.5) - sinHR*(-cosHP*(-.5 + cosHYP*.5) + (sinHP*sinHYP)/root2));

        float j_2_3 = -TIBIA_LENGTH*(cosKP*((-.5 + cosHYP*.5)*sinHP + (cosHP*sinHYP)/root2) - ((-.5 - cosHYP*.5)*sinHR + cosHR*(-cosHP*(-.5 + cosHYP*.5) + (sinHP*sinHYP)/root2))*sinKP);

        float j_3_1 = -THIGH_LENGTH*cosHR*((-.5 - cosHYP*.5)*sinHP - (cosHP*sinHYP)/root2) - TIBIA_LENGTH*(cosHR*cosKP*((-.5 - cosHYP*.5)*sinHP - (cosHP*sinHYP)/root2) + (cosHP*(-.5 - cosHYP*.5) + (sinHP*sinHYP)/root2)*sinKP);

        float j_3_2 = -THIGH_LENGTH*(cosHR*(-.5 + cosHYP*.5) - sinHR*(-cosHP*(-.5 - cosHYP*.5) - (sinHP*sinHYP)/root2)) - TIBIA_LENGTH*cosKP*(cosHR*(-.5 + cosHYP*.5) - sinHR*(-cosHP*(-.5 - cosHYP*.5) - (sinHP*sinHYP)/root2));

        float j_3_3 = -TIBIA_LENGTH*(cosKP*((-.5 - cosHYP*.5)*sinHP - (cosHP*sinHYP)/root2) - ((-.5 + cosHYP*.5)*sinHR + cosHR*(-cosHP*(-.5 - cosHYP*.5) - (sinHP*sinHYP)/root2))*sinKP);

        ublas::matrix<float> jacobian(3,3);
        jacobian(0,0) = j_1_1;  jacobian(0,1) = j_1_2;  jacobian(0,2) = j_1_3;
        jacobian(1,0) = j_2_1;  jacobian(1,1) = j_2_2;  jacobian(1,2) = j_2_3;
        jacobian(2,0) = j_3_1;  jacobian(2,1) = j_3_2;  jacobian(2,2) = j_3_3;
        return jacobian;
    }
    else
        throw "Wrong chain";
}


const float Kinematics::distance(const ublas::vector<float> a,
                                 const ublas::vector<float> b) {
    const ublas::vector<float> difference = a - b;
    return sqrt(inner_prod(difference, difference));
}


const Kinematics::IKLegResult
Kinematics::dlsInverseKinematics(const ChainID chainID,
                                 const ublas::vector<float> &goal,
                                 const float startAngles[],
                                 const float maxError = .1,
                                 const float maxHeelError = .1) {
    // The goal of the heel coincides with where want to go, but the goal of
    // ankle is to be directly above the heel, so that we are stepping correctly
    ublas::vector<float> ankleGoal(3);
    ankleGoal.assign(goal);
    // the ankle should be FOOT_HEIGHT above what the final goal is
    ankleGoal(2) = ankleGoal(2) + FOOT_HEIGHT;

    float currentAngles[LEG_JOINTS];
    memcpy(currentAngles, startAngles, LEG_JOINTS*sizeof(float));

    // The optimization method hits a singularity if the leg is perfectly
    // straight, so we can virtually bend the knee .3 radians and go around
    // that.
    currentAngles[3] = .3;

    int iterations = 0;

    ublas::matrix<float> identity = ublas::identity_matrix <float> (3);
    ublas::matrix<float> dampenMatrix = identity*(dampFactor*dampFactor);

    float values[] = {0.0f,0.0f,0.0f};
    ublas::zero_vector<float> origin(3);

    ChainID ankleChainID = (chainID == LLEG_CHAIN ?
                            LANKLE_CHAIN : RANKLE_CHAIN);

    bool ankleSuccess = false;

    while (iterations < maxAnkleIterations) {
        iterations++;

        // Define the Jacobian that describes the linear approximation at the
        // current angle values.
        ublas::matrix<float> j = buildLegJacobian(chainID, currentAngles);
        ublas::matrix<float> j_t = trans(j);

        ublas::vector<float> currentAnklePosition =
            forwardKinematics(ankleChainID,
                              currentAngles);
        ublas::vector<float> e = ankleGoal - currentAnklePosition;

        // Check if we have gotten close enough
        const float dist_e = distance(e, origin);

        if (dist_e < maxError) {
            ankleSuccess = true;
            break;
        }

        ublas::matrix<float> temp = prod(j, j_t);
        temp += dampenMatrix;

        // Now we need to find a vector that we'll call 'result' such that
        // temp*f = e. Solve operations like this are a little tricky in
        // ublas (sigh).
        ublas::permutation_matrix <float> P(3);
        int singularRow = lu_factorize(temp, P);
        if (singularRow != 0) {
            // TODO: This case needs to be dealt with
            throw "oops";
        }
        ublas::vector<float> result(3);
        result.assign(e);
        lu_substitute(temp, P, result);
        // Now we multiply j_t by result and we get delta_theta
        ublas::vector<float> ankleDeltaTheta = prod(j_t, result);

        /*
        Matrix t2 = t1.invert();
        Matrix ankleDeltaTheta = j_t * 2;
        ankleDeltaTheta *= e;
        */

        currentAngles[1] += clip(ankleDeltaTheta(0),
                                 -maxDeltaTheta,
                                 maxDeltaTheta);
        currentAngles[2] += clip(ankleDeltaTheta(1),
                                 -maxDeltaTheta,
                                 maxDeltaTheta);
        currentAngles[3] += clip(ankleDeltaTheta(2),
                                 -maxDeltaTheta,
                                 maxDeltaTheta);
        clipChainAngles(chainID,currentAngles);
    }



    // Now we update the heel
    // We want it to move right below the ankle
    ublas::vector<float> currentAnklePosition =
        forwardKinematics(ankleChainID,currentAngles);
    ublas::vector<float> heelGoal(3);
    heelGoal.assign(currentAnklePosition);
    heelGoal(2) = heelGoal(2) - FOOT_HEIGHT;

    iterations = 0;
    bool heelSuccess = false;

    while (iterations < maxHeelIterations) {
        iterations++;
        ublas::matrix<float> jHeel = buildHeelJacobian(chainID, currentAngles);
        ublas::matrix<float> jHeel_t = trans(jHeel);

        ublas::vector<float> currentHeelPosition =
            forwardKinematics(chainID,currentAngles);
        ublas::vector<float> eHeel = heelGoal - currentHeelPosition;

        float dist_e_heel = distance(eHeel,origin);
        if (dist_e_heel < maxHeelError) {
            heelSuccess = true;
            break;
        }

        ublas::matrix<float> temp = prod(jHeel, jHeel_t);
        temp += dampenMatrix;
        ublas::permutation_matrix <float> P(3);
        int singularRow = lu_factorize(temp, P);
        if (singularRow != 0) {
            // TODO: This case needs to be dealt with
            throw "oops";
        }
        ublas::vector<float> result(3);
        result.assign(eHeel);
        lu_substitute(temp, P, result);
        // Now we multiply j_t by result and we get delta_theta
        ublas::vector<float> heelDeltaTheta = prod(jHeel_t, result);

        currentAngles[4] += clip(heelDeltaTheta(0),
                                 -maxDeltaTheta,
                                 maxDeltaTheta);
        currentAngles[5] += clip(heelDeltaTheta(1),
                                 -maxDeltaTheta,
                                 maxDeltaTheta);
        clipChainAngles(chainID,currentAngles);
    }


    // Do error calculations
    ublas::vector<float> e = ankleGoal - currentAnklePosition;
    ublas::vector<float> currentHeelPosition =
        forwardKinematics(chainID,currentAngles);

    ublas::vector<float> eHeel = heelGoal - currentHeelPosition;
    float dist_e = distance(e,origin);
    float dist_e_heel = distance(eHeel,origin);

    IKOutcome outcome;
    if (ankleSuccess && heelSuccess)
        outcome = SUCCESS;
    else
        outcome = STUCK;

    IKLegResult result;
    result.outcome = outcome;
    result.ankleError = dist_e;
    result.heelError = dist_e_heel;
    memcpy(result.angles,currentAngles,LEG_JOINTS*sizeof(float));
    return result;
}


int main() {
    // George's test code
    for (int i=0; i<10000; i++) {
        float startAngles[] = {0,0,0,0.2,0,0};
        ublas::vector<float> goal(3);
        goal(0) = 50;
        goal(1) = 50;
        goal(2) = -305;
        //bool jointMask[4] = {true,true,true,true};
        Kinematics::IKLegResult result =
            Kinematics::dlsInverseKinematics(Kinematics::LLEG_CHAIN,
                                             goal,
                                             startAngles,
                                             //jointMask,
                                             0.1f);

        /*
        for(int i=0; i<6; i++) {
            printf("%f ",result.angles[i]);
        }
        printf("\n");
        printf("error: %f\n",result.ankleError);
        */
    }
    return 0;
}
