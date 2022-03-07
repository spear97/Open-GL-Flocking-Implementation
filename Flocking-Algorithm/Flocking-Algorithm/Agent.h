#ifndef AGENT
#define AGENT
#include "Vector.h"
using namespace mathtool;
#include <vector>
using namespace std;

class Agent {
public:
  Agent();
  Agent(const Agent& other);

  void Init(int _id, Vector3d _pos, Vector3d _vel, double _mass, double _maxVel, double _maxAccel, double _viewRadius);
  Vector3d& GetPos() { return pos; }
  Vector3d& GetVel() { return vel; }
  double GetRadius() { return radius; }
  int GetID() { return id; }

  void Update(vector<Agent>& agents, double dt);
  void Draw();

  void SetControlled(bool ic) { isControlled = ic; }
  void ToggleControlled() { isControlled = !isControlled; }
  void SetControl(string control);

  bool IsAdversary() { return isAdversary; }
  void ToggleAdversary() { isAdversary = !isAdversary; }
  
  Vector3d GetForceFromControl();
  void ResolveCollisionWithOtherAgents(vector<Agent>& agents);
  Vector3d GetEnvironmentalForce(double mag);
 
private:
  int id;
  Vector3d pos;
  Vector3d oldPos;
  Vector3d vel;
  //Vector3d accel;
  Vector3d separationForce;
  Vector3d cohesionForce;
  Vector3d alignmentForce;
  Vector3d obstacleForce;
  vector<Vector3d> pastPos;
  int tailLength;
  
  double separationComponent;
  double cohesionComponent;
  double alignmentComponent;

  double radius;
  double mass;
  double maxVel;
  double maxAccel;
  double viewRadius;

  bool initialized;

  pair<int,int> gridCell;

  bool drawForce;
  bool drawVelocity;
  bool drawVR;


  /////////////////////////
  double ori;
  /////////////////////////
  double status;
  double maxStatus;
  /////////////////////////
  bool isControlled;
  string lastControl;
  int timeInControl;
  /////////////////////////
  bool isAdversary; 
};

#endif
