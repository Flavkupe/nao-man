

#include <boost/shared_ptr.hpp>

#include "synchro.h"
#include "Motion.h"
#include "_motionmodule.h"
#include "SimulatorEnactor.h"

#ifdef NAOQI1
Motion::Motion (ALPtr<ALMotionProxy> _proxy,shared_ptr<Synchro> _synchro, Sensors *s)
#else
Motion::Motion (ALMotionProxy * _proxy,shared_ptr<Synchro> _synchro, Sensors *s)
#endif
    : Thread(_synchro, "MotionCore"),
      switchboard(s),
      enactor(new SimulatorEnactor(&switchboard)),
      interface(&switchboard)
{
  //Breaks NaoQi1.0
#ifndef NAOQI1
  set_motion_interface(this);
  c_init_motion();
#endif
}

Motion::~Motion() {
    delete enactor;
}

int Motion::start() {
    // Start the enactor thread
    enactor->start();
    switchboard.start();

    return Thread::start();
x}

void Motion::stop() {
    switchboard.stop();
    enactor->stop();
    Thread::stop();
}

void Motion::run(){
    cout <<"Motion::run"<<endl;
    switchboard.run();
}
