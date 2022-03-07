#ifndef SIMULATOR
#define SIMULATOR

#include "Vector.h"
using namespace mathtool;

#include "Agent.h"

#include <vector>
using namespace std;

class Simulator {
   public:
      Simulator(); 

      void AddMember(int index, double x, double y);

      void Draw();
      void Update();

      void IncrementControllingAgent();
      void SendControl(string control);

      void ToggleControlledAdversary();
      void ToggleControlledAgent();

      Vector3d GetPointToFollow();
   private:
      vector<Agent> agents;
      int controlAgentIndex;
};

#endif
