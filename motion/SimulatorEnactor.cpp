#include "SimulatorEnactor.h"

#include <iostream>

void SimulatorEnactor::run() {
    std::cout << "SimulatorEnactor::run()" << std::endl;
    AL::ALMotionProxy *motionProxy = AL::ALMotionProxy::getInstance();
    motionProxy->setBodyStiffness(0.85f, 0.1f);

    while (running) {

        vector <float> result = switchboard->getNextJoints();

        //for (int i=0;

        // Get the angles we want to go to this frame from the switchboard
        motionProxy->postGotoBodyAngles(switchboard->getNextJoints(),
                                        MOTION_FRAME_LENGTH_S,
                                        AL::ALMotionProxy::INTERPOLATION_LINEAR);

        // TODO: This is probably wrong!!!!1!ONE
        // We probably want to sleep webots time and this sleeps real time.
        usleep(static_cast<useconds_t>(MOTION_FRAME_LENGTH_uS));
    }
}

