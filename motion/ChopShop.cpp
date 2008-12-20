#include "ChopShop.h"


ChopShop::ChopShop (Sensors *s, float motionFrameLength)
	: finalJoints(),
	  diffPerChop(22),
	  sensors(s),
	  FRAME_LENGTH_S(motionFrameLength),
	  choppedJoints(),
	  nextChain()


{

}

// Breaks command into FRAME_LENGTH_S size pieces,
// adds it to the queue
queue<vector<vector<float> > > ChopShop::chopCommand(const BodyJointCommand *command){
	// Checks type of body command and sends to
	// appropriate function
//  if (command->getType() == INTERPOLATION_LINEAR){
	cout << "Chopping command!!!" << endl;
	return chopLinear(command);
//  }

//  else if (command->getType() == INTERPOLATION_SMOOTH){
	//  return chopSmooth(command);
//  }


}

// Smooth interpolation motion
queue<vector<vector<float> > > ChopShop::chopSmooth(const BodyJointCommand *command){
	queue<vector<vector<float> > > a;
	return a;
}

// Linear interpolation motion
// Maybe allocate stuff on heap?
queue<vector<vector<float> > > ChopShop::chopLinear(const BodyJointCommand *command){


 	while (!choppedJoints.empty()){
 		choppedJoints.pop();
 	}
	diffPerChop.clear();

	// Get number of chops according to duration
	float numChops = command->getDuration()/FRAME_LENGTH_S;
	// Get current angles



	vector<float> currentJoints = sensors->getBodyAngles();
	vector<float> currentJointErrors = sensors->getBodyAngleErrors();

	for (int i=0; i<NUM_JOINTS ; i++){
		currentJoints[i] = currentJoints[i]-currentJointErrors[i];
	}

	// Add final joints for all chains
	addFinalJoints(command, &currentJoints);


	//Get diff per chop from current to final
	for ( int joint_id=0; joint_id < NUM_JOINTS ;++joint_id){
		diffPerChop.push_back( (finalJoints.at(joint_id) -
								currentJoints.at(joint_id)) /
							   numChops);
		
	}
	finalJoints.clear();
	// @JGM need to add vector for each chain!
    // Make vectors of new joints



	chopThat(numChops, &currentJoints);
	return choppedJoints;
}

void ChopShop::chopThat(float numChops, vector<float> *currentJoints) {
	cout << "chopthatshit" << endl;
	float nextVal(0);

	for (int num_chopped=1; num_chopped<=numChops; num_chopped++ ) {
		vector<vector<float> > nextChopped(5);

		int lastChainJoint = 0;
		for (int chain=0,joint=0; chain<NUM_CHAINS; chain++) {
			lastChainJoint += chain_lengths[chain];
			for ( ; joint < lastChainJoint ; joint++){
				nextVal = currentJoints->at(joint)+ diffPerChop.at(joint)*num_chopped;
				nextChopped.at(chain).push_back(nextVal);
			}
		}
		choppedJoints.push(nextChopped);
	}


} // END CHOP THAT


void ChopShop::addFinalJoints(const BodyJointCommand *command,
								  vector<float>* currentJoints) {
	vector<float>::const_iterator currentStart = currentJoints->begin();
	vector<float>::const_iterator currentEnd = currentJoints->begin();

	for (int chain=0; chain < NUM_CHAINS;chain++) {
		// First, get chain joints from command
		nextChain = command->getJoints((ChainID)chain);

		// Set the end iterator
		currentEnd += chain_lengths[chain];

		// If the next chain is not queued (empty), add current joints
		if ( nextChain == 0 ||
			 nextChain->empty()) {
			finalJoints.insert(finalJoints.end(), currentStart, currentEnd );
		}else {
			// Add each chain of joints to the final joints
			finalJoints.insert( finalJoints.end(),
								nextChain->begin(),
								nextChain->end() );
		}
		// Set the start iterator into the right position for the
		// next chain
		currentStart += chain_lengths[chain];

	}

}
