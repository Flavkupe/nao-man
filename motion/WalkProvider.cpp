#include "WalkProvider.h"
using Kinematics::LLEG_CHAIN;
using Kinematics::RLEG_CHAIN;

WalkProvider::WalkProvider()
    : MotionProvider(),
      walkParameters(310.0f, 0.0f),
      controller(new PreviewController()),
      left(LLEG_CHAIN,&walkParameters),
      right(RLEG_CHAIN,&walkParameters),
      stepGenerator(&walkParameters)
{

}

WalkProvider::~WalkProvider() {

}

void WalkProvider::requestStop() {

}

void WalkProvider::calculateNextJoints() {
    //Tick the step generator (ensure we have preview values ready)
    const list<float>* zmp_ref = stepGenerator.tick();

    //Tick the controller (input: ZMPref, sensors -- out: CoM x, y)
    float reference_zmp = 0.0f; //dummy
    float com_x = 0.0f;
    float com_y = controller->tick(zmp_ref);
    //cout << "Com y: " << com_y << endl;

    //Tick each leg (in: CoM x,y balance mode, out: joint angles)
    vector<float> lleg_results = left.tick(com_x,com_y);
    vector<float> rleg_results = right.tick(com_x,com_y);

    //Return the joints for the legs
    setNextChainJoints(LLEG_CHAIN,lleg_results);
    setNextChainJoints(RLEG_CHAIN,rleg_results);
}

void WalkProvider::setMotion(const float x, const float y, const float theta) {

}

