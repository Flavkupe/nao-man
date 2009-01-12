#include "WalkingLeg.h"


WalkingLeg::WalkingLeg(ChainID id,
                       const WalkingParameters *walkP)
    :state(SUPPORTING),lastState(SUPPORTING),lastDiffState(SUPPORTING),
     frameCounter(0),
     cur_dest(EMPTY_STEP),swing_src(EMPTY_STEP),
     chainID(id), walkParams(walkP),
     goal(ufvector3(3)),last_goal(ufvector3(3)),
     leg_sign(id == LLEG_CHAIN ? 1 : -1),
     leg_name(id == LLEG_CHAIN ? "left" : "right") {
#ifdef DEBUG_WALKING_LOCUS_LOGGING
    char filepath[100];
    sprintf(filepath,"/tmp/%s_locus_log.xls",leg_name.c_str());
    locus_log  = fopen(filepath,"w");
    fprintf(locus_log,"time\tgoal_x\tgoal_y\tgoal_z\tstate\n");
#endif
#ifdef DEBUG_WALKING_DEST_LOGGING
    char filepath2[100];
    sprintf(filepath2,"/tmp/%s_dest_log.xls",leg_name.c_str());
    dest_log  = fopen(filepath2,"w");
    fprintf(dest_log,"time\tdest_x\tdest_y\tsrc_x\tsrc_y\tstate\n");
#endif
    for ( unsigned int i = 0 ; i< LEG_JOINTS; i++) lastJoints[i]=0.0f;
}

WalkingLeg::~WalkingLeg(){
#ifdef DEBUG_WALKING_LOCUS_LOGGING
    fclose(locus_log);
#endif
#ifdef DEBUG_WALKING_DEST_LOGGING
    fclose(dest_log);
#endif
}

vector <float> WalkingLeg::tick(boost::shared_ptr<Step> step,
                                boost::shared_ptr<Step> _swing_src,
                                ublas::matrix<float> fc_Transform){
    //cout << "In leg" << chainID << " got target (" x
    //     << dest_x << "," <<dest_y << ")" <<endl;
    cur_dest = step;
    swing_src = _swing_src;

    //ublas::vector<float> dest_f = CoordFrame3D::vector3D(cur_dest->x,cur_dest->y);
    //ublas::vector<float> dest_c = prod(fc_Transform,dest_f);
    //float dest_x = dest_c(0);
    //float dest_y = dest_c(1);
    //cout << "FC Transform" << fc_Transform <<endl;
    //cout <<"Dest_c: " << dest_f<<endl;
    vector<float> result(6);
    switch(state){
    case SUPPORTING:
        result  = supporting(fc_Transform);
        break;
    case SWINGING:
        if(step->type == REGULAR_STEP)
            result  =  swinging(fc_Transform);
        else{
            //cout << "It's an Irregular step, so we are not swinging:" <<endl;
            result = supporting(fc_Transform);
        }
        break;
    case DOUBLE_SUPPORT:
        //In dbl sup, we have already got the final target after swinging in
        //mind, so we actually want to keep the target as the "source"
        cur_dest = swing_src;
        result  = supporting(fc_Transform);
        break;
    case PERSISTENT_DOUBLE_SUPPORT:
        result  = supporting(fc_Transform);
        break;
    default:
        throw "Invalid SupportMode passed to WalkingLeg::tick";
    }

    debugProcessing();

    last_goal = goal;
    frameCounter++;
    //Decide if it's time to switch states
    if ( shouldSwitchStates())
        switchToNextState();

    lastState=state;
    return result;
}


vector <float> WalkingLeg::swinging(ublas::matrix<float> fc_Transform){//(float dest_x, float dest_y) {
    ublas::vector<float> dest_f = CoordFrame3D::vector3D(cur_dest->x,cur_dest->y);


    ublas::vector<float> src_f = CoordFrame3D::vector3D(swing_src->x,swing_src->y);


    ublas::vector<float> dest_c = prod(fc_Transform,dest_f);
    ublas::vector<float> src_c = prod(fc_Transform,src_f);

    //float dest_x = dest_c(0);
    //float dest_y = dest_c(1);

    static float dist_to_cover_x = 0;
    static float dist_to_cover_y = 0;

     if(firstFrame()){
         //cout << "Current destination" << cur_dest->x<< ","<<cur_dest->y << endl;
         //cout << "Last destination" << swing_src->x<< ","<<swing_src->y << endl;
         dist_to_cover_x = cur_dest->x - swing_src->x;
         dist_to_cover_y = cur_dest->y - swing_src->y;

         //cout <<"Distance to cover x"<<dist_to_cover_x<<endl;
         //cout <<"Distance to cover y"<<dist_to_cover_y<<endl;
     }


    //There are two attirbutes to control - the height off the ground, and
    //the progress towards the goal.

    //HORIZONTAL PROGRESS:
    float percent_complete =
        frameCounter/static_cast<float>(walkParams->singleSupportFrames);

    //cout <<"Percent incomplete" << percent_incomplete <<endl;
    //cout << "percent complete" << percent_complete<<endl;
    //Then we can express the destination as the proportionate distance to cover
    float dest_x = src_f(0) + percent_complete*dist_to_cover_x;
    float dest_y = src_f(1) + percent_complete*dist_to_cover_y;


    ublas::vector<float> target_f = CoordFrame3D::vector3D(dest_x,dest_y);
    //cout << "New Dest (x,y) "<<dest_x <<","<<dest_y <<endl;
    ublas::vector<float> target_c = prod(fc_Transform, target_f);

    float target_c_x = target_c(0);
    float target_c_y = target_c(1);

    //ELEVATION PROGRESS:
    // the swinging leg will follow a trapezoid in 3-d. The trapezoid has
    // three stages: going up, a level stretch, going back down to the ground
    static int stage;
    if (firstFrame()) stage = 0;
    float x,y;
    float heightOffGround = 0.0f;

    if (stage == 0) { // we are rising
        // we want to raise the foot up for the first third of the step duration
        heightOffGround = walkParams->stepHeight*
            static_cast<float>(frameCounter) /
            ((walkParams->singleSupportFrames/3));
        if (frameCounter > walkParams->singleSupportFrames/3.)
            stage++;

    }
    else if (stage == 1) { // keep it level
        heightOffGround = walkParams->stepHeight;

        if (frameCounter > 2.* walkParams->singleSupportFrames/3)
            stage++;
    }
    else {// stage 2, set the foot back down on the ground
        heightOffGround = max(0.0f,
                              walkParams->stepHeight*
                              static_cast<float>(walkParams->singleSupportFrames
                                                 -frameCounter)/
                              (walkParams->singleSupportFrames/3));
    }

    goal(0) = target_c_x;
    goal(1) = target_c_y;
    goal(2) = -walkParams->bodyHeight + heightOffGround;

    IKLegResult result = Kinematics::dls(chainID,goal,lastJoints);
    memcpy(lastJoints, result.angles, LEG_JOINTS*sizeof(float));
    return vector<float>(result.angles, &result.angles[LEG_JOINTS]);
}

vector <float> WalkingLeg::supporting(ublas::matrix<float> fc_Transform){//float dest_x, float dest_y) {
    /**
       this method calculates the angles for this leg when it is on the ground
       (i.e. the leg on the ground in single support, or either leg in double
       support).
       We calculate the goal based on the comx,comy from the controller,
       and the given parameters using inverse kinematics.
     */
    ublas::vector<float> dest_f = CoordFrame3D::vector3D(cur_dest->x,cur_dest->y);
    ublas::vector<float> dest_c = prod(fc_Transform,dest_f);
    float dest_x = dest_c(0);
    float dest_y = dest_c(1);

    float physicalHipOffY = 0;
    goal(0) = dest_x; //targetX for this leg
    goal(1) = dest_y;  //targetY
    goal(2) = -walkParams->bodyHeight;         //targetZ

    //calculate the new angles
    IKLegResult result = Kinematics::dls(chainID,goal,lastJoints);
    memcpy(lastJoints, result.angles, LEG_JOINTS*sizeof(float));
    return vector<float>(result.angles, &result.angles[LEG_JOINTS]);
}


void WalkingLeg::startLeft(){
    if(chainID == LLEG_CHAIN){
        //we will start walking first by swinging left leg (this leg), so
        //we want double, not persistent, support
        setState(DOUBLE_SUPPORT);
    }else{
        setState(PERSISTENT_DOUBLE_SUPPORT);
    }

}
void WalkingLeg::startRight(){
    if(chainID == LLEG_CHAIN){
        //we will start walking first by swinging right leg (not this leg), so
        //we want persistent double support
        setState(PERSISTENT_DOUBLE_SUPPORT);
    }else{
        setState(DOUBLE_SUPPORT);
    }

}


//transition function of the fsa
//returns the next logical state to enter
SupportMode WalkingLeg::nextState(){
    switch(state){
    case SUPPORTING:
        return DOUBLE_SUPPORT;
    case SWINGING:
        return PERSISTENT_DOUBLE_SUPPORT;
    case DOUBLE_SUPPORT:
        return SWINGING;
    case PERSISTENT_DOUBLE_SUPPORT:
        return SUPPORTING;
    default:
        throw "Non existent state";
        return PERSISTENT_DOUBLE_SUPPORT;
    }
}

//Decides if time is up for the current state
bool WalkingLeg::shouldSwitchStates(){
    switch(state){
    case SUPPORTING:
        return frameCounter >= walkParams->singleSupportFrames;
    case SWINGING:
        return frameCounter >= walkParams->singleSupportFrames;
    case DOUBLE_SUPPORT:
        return frameCounter >= walkParams->doubleSupportFrames;
    case PERSISTENT_DOUBLE_SUPPORT:
        return frameCounter >= walkParams->doubleSupportFrames;
    }

    throw "Non existent state";
    return false;
}

void WalkingLeg::switchToNextState(){
    setState(nextState());
}

void WalkingLeg::setState(SupportMode newState){
    state = newState;
    lastDiffState = state;
    frameCounter = 0;
}


void WalkingLeg::debugProcessing(){

#ifdef DEBUG_WALKING_STATE_TRANSITIONS
    if (firstFrame()){
        if(chainID == LLEG_CHAIN){
            cout<<"Left leg "
        }else{
            cout<<"Right leg "
        }
      if(state == SUPPORTING)
          cout <<"switched into single support"<<endl;
      else if(state== DOUBLE_SUPPORT || state == PERSISTENT_DOUBLE_SUPPORT)
          cout <<"switched into double support"<<endl;
      else if(state == SWINGING)
          cout << "switched into swinging."<<endl;
    }
#endif

#ifdef DEBUG_WALKING_GOAL_CONTINUITY
    ufvector3 diff = goal - last_goal;
    #define GTHRSH 5



    if(diff(0) > GTHRSH || diff(1) > GTHRSH ||diff(2) > GTHRSH ){
        if(chainID == LLEG_CHAIN){
            cout << "Left leg ";
        }else
            cout << "Right leg ";
        cout << "noticed a big jump from last frame"<< diff<<endl;
        cout << "  from: "<< last_goal<<endl;
        cout << "  to: "<< goal<<endl;

    }
#endif

#ifdef DEBUG_WALKING_LOCUS_LOGGING
    static float ttime= 0.0f;
    fprintf(locus_log,"%f\t%f\t%f\t%f\t%d\n",ttime,goal(0),goal(1),goal(2),state);
    ttime += 0.02f;
#endif
#ifdef DEBUG_WALKING_DEST_LOGGING
    static float stime= 0.0f;
    fprintf(dest_log,"%f\t%f\t%f\t%f\t%f\t%d\n",stime,
            cur_dest->x,cur_dest->y,
            swing_src->x,swing_src->y,state);
    stime += 0.02f;
#endif

}
