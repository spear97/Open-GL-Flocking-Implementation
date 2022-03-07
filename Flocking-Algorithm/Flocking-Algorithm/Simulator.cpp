#include "Simulator.h"
#include <stdlib.h>

#include <GL/glut.h>  // (or others, depending on the system in use)
#include "Basic.h"

Simulator::Simulator() {
  for(int i=0; i<8; i++) {
    //pos
    double x = 50+200*(myRand()-0.5);
    double y = 50+200*(myRand()-0.5);
    double z = 0;
    AddMember(i,x,y); 
    /*
    Vector3d pos(x,y,z);
    //vel
    double vx = 20*(myRand()-0.5);
	double vy = 20 * (myRand() - 0.5);
    double vz = 0;
    Vector3d vel(vx,vy,vz);
	double mass = 4 + myRand() * 10; //random between 4 and 14
	double maxVel = 8 + myRand() * 5; //random between 18 and 28
    //double maxAccel = 20+drand48()*10; //random between 20 and 30
	double maxAccel = 20 + myRand() * 10; //random between 20 and 30
	double viewRadius = 40 + myRand() * 10; //random between 40 and 50
    //agents.push_back( Vector3d(x,y) );
    Agent a_i;
    a_i.Init(i,pos,vel,mass,maxVel,maxAccel,viewRadius);
    agents.push_back( a_i );
    */
  }
  controlAgentIndex = -1;
}

void Simulator::AddMember(int index, double x, double y) {
  int i = index;
  if( i<0 ) i = agents.size(); //if send in negative, set id to last in list
  double z = 0;
  Vector3d pos(x,y,z);
  //vel
  double vx = 20*(myRand()-0.5);
  double vy = 20 * (myRand() - 0.5);
  double vz = 0;
  Vector3d vel(vx,vy,vz);
  double mass = 4 + myRand() * 10; //random between 4 and 14
  double maxVel = 8 + myRand() * 5; //random between 18 and 28
  //double maxAccel = 20+drand48()*10; //random between 20 and 30
  double maxAccel = 20 + myRand() * 10; //random between 20 and 30
  double viewRadius = 40 + myRand() * 10; //random between 40 and 50
  //agents.push_back( Vector3d(x,y) );
  Agent a_i;
  a_i.Init(i,pos,vel,mass,maxVel,maxAccel,viewRadius);
  agents.push_back( a_i );
}

void Simulator::Draw() {
  for(int i=0; i<(int)agents.size(); i++) {
    agents[i].Draw();
  }
}

void Simulator::Update() {
  //double dt = 0.1; //0.05;
	double dt = 0.025;
  for(int i=0; i<(int)agents.size(); i++) {
    agents[i].Update(agents, dt);
  }


}

void Simulator::IncrementControllingAgent() {
  if( controlAgentIndex>=0 && controlAgentIndex<agents.size() ) {
    agents[controlAgentIndex].SetControlled(false);
  }
  controlAgentIndex++;
  if( controlAgentIndex>=agents.size() ) controlAgentIndex = controlAgentIndex % agents.size();
  agents[controlAgentIndex].SetControlled(true);
}

void Simulator::SendControl(string control) {
  if( controlAgentIndex>=0 && controlAgentIndex<agents.size() ) {
    agents[controlAgentIndex].SetControl(control);
  }
}

void Simulator::ToggleControlledAgent() {
  if( controlAgentIndex>=0 && controlAgentIndex<agents.size() ) {
    agents[controlAgentIndex].ToggleControlled();
  }
}
void Simulator::ToggleControlledAdversary() {
  
  if( controlAgentIndex>=0 && controlAgentIndex<agents.size() ) {
    agents[controlAgentIndex].ToggleAdversary();
  }
}

Vector3d Simulator::GetPointToFollow() {
  if( controlAgentIndex>=0 && controlAgentIndex<agents.size() ) {
    return agents[controlAgentIndex].GetPos();
  }
  else 
    return agents[0].GetPos();
}
