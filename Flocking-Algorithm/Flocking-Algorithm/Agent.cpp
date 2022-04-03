#include <windows.h>
#pragma comment(lib, "user32.lib") 

#include "Agent.h"
#include "Environment.h"
#include <tuple>

#include <GL/glut.h>  // (or others, depending on the system in use)

bool agDebug = false;//true;

extern int drawMode;

float theta = -180;
bool rotOtherWay = false;

extern Environment* gEnv;

Agent::Agent() { 
  initialized = false;
  drawForce = true;
  drawVelocity = true;
  drawVR = false;
  isControlled = false;
  ori = 0;
  status = 1000;
  maxStatus = status;
  isAdversary = false;
  tailLength = 40;
} //empty constructor  

void Agent::Init(int _id, Vector3d _pos, Vector3d _vel, double _mass, 
                 double _maxVel, double _maxAccel, double _viewRadius){
  initialized = true;
  id = _id;
  pos = _pos;
  vel = _vel;
  mass = _mass;
  maxVel = _maxVel;
  maxAccel = _maxAccel;
  viewRadius = _viewRadius;
  //radius = 5;
  radius = 12;
  tailLength = 40;
  
  drawForce = true;
  drawVelocity = true;
  drawVR = false;
  isControlled = false;
  
  separationComponent = 30;
  cohesionComponent = 20;
  alignmentComponent = 20;
  ori = 0;
  status = 1000;
  maxStatus = status;
  isAdversary = false;

  lifespan = 0.f;
  AdvR = 1.f, AdvG = 0.f, AdvB = 0.f;
  BaseR = 0.f, BaseG = 0.f, BaseB = 1.f;
  FinR = 112, FinG = 128, FinB = 144;
}

Agent::Agent(const Agent& other) {
  initialized = other.initialized;
  id = other.id;
  pos = other.pos;
  vel = other.vel;
  separationComponent = other.separationComponent;
  cohesionComponent = other.cohesionComponent;
  alignmentComponent = other.alignmentComponent;
  radius = other.radius;
  mass = other.mass;
  maxVel = other.maxVel;
  maxAccel = other.maxAccel;
  viewRadius = other.viewRadius;
  gridCell = other.gridCell;
  drawForce = other.drawForce;
  drawVelocity = other.drawVelocity;
  drawVR = other.drawVR;
  tailLength = other.tailLength;
  isControlled = other.isControlled;
  ori = other.ori;
  status = other.status;
  maxStatus = other.maxStatus;
  isAdversary = other.isAdversary;
  lifespan = other.lifespan;
  AdvR = other.AdvR;
  AdvG = other.AdvG;
  AdvB = other.AdvB;
  BaseR = other.BaseR;
  BaseG = other.BaseG;
  BaseB = other.BaseB;
  FinR = other.FinR;
  FinG = other.FinG;
  FinB = other.FinB;
}

Vector3d Agent::GetEnvironmentalForce(double mag) {
  double closeEdgeDist = 1e6;
  Vector3d pushDir;
  //xresolution
  if( fabs(pos[0]-gEnv->xmin) < closeEdgeDist ) {
    pushDir = Vector3d(1,0,0);
    closeEdgeDist = fabs(pos[0]-gEnv->xmin);
  }
  if( fabs(pos[0]-gEnv->xmax) < closeEdgeDist ) {
    pushDir = Vector3d(-1,0,0);
    closeEdgeDist = fabs(pos[0]-gEnv->xmax);
  }
  
  if( fabs(pos[1]-gEnv->ymin) < closeEdgeDist ) {
    pushDir = Vector3d(0,1,0);
    closeEdgeDist = fabs(pos[1]-gEnv->ymin);
  }
  if( fabs(pos[1]-gEnv->ymax) < closeEdgeDist ) {
    pushDir = Vector3d(0,-1,0);
    closeEdgeDist = fabs(pos[1]-gEnv->ymax);
  }
  if( closeEdgeDist > viewRadius ) {
    mag = 0;
  }
  pushDir *= mag;
  return pushDir;
}

void Agent::Update(vector<Agent>& agents, double dt) {
  if( !initialized ) 
  {
    cout << "Agent::Update id: " << id << " HAS NOT BEEN INITIALIZED> " << endl;
    exit(-1);
  }

  //double separationComponent = 240;
  Vector3d separation(0,0,0);
  Vector3d cohesion(0,0,0);
  Vector3d alignment(0,0,0);
  Vector3d center(0,0,0);
  Vector3d force(0,0,0);

  Vector3d attractionPtForce(0,0,0);
  Vector3d obstacleForce(0,0,0);

  separationForce.reset();
  cohesionForce.reset();
  alignmentForce.reset();

  if( isControlled ) 
  {
    force = GetForceFromControl();
  }
  else {
    int numSeen = 0;
    if (agDebug)
    {
        cout << "Update for agent: " << id << " isAdversary: " << isAdversary << endl;
    }
    //generate the forces
    if( !isAdversary ) 
    { //generate flocking force rule
      double closestDistToAdv=1e6;
      int numAdv=0;
      for(int i=0; i<(int)agents.size(); i++) 
      {
          if (id == i)
          {
              continue; //skip self
          }

	    //see if within view radius
          //TODO: Collision for Adversary and Non-Adversary Agents, update Adversary Agent Color
	    double dist = (agents[i].GetPos()-pos).norm();
	    if( dist < viewRadius )
        {
	        if( agents[i].IsAdversary() && (dist<(radius+agents[i].GetRadius())) ) 
            {
	            numAdv++;
	            if( dist<closestDistToAdv ) closestDistToAdv = dist; 
	        }

            if (agDebug)
            {
                cout << "Agent: " << id << " is within range of agent: " << i << " dist= " << dist << " viewRadius= " << viewRadius << endl;
            }
            if (agDebug)
            {
                cout << "SEPDBG agent[" << i << "].GetPos() " << agents[i].GetPos() << " pos[" << id << "]" << pos << endl;
            }
            if (agDebug)
            {
                cout << "SEPDBG agent[" << i << "]: separation component: " << agents[i].GetPos() - pos << endl;
            }

	        //separation += (agents[i].GetPos()-pos).normalize()*(1.0-dist/viewRadius);
	        separation += (pos-agents[i].GetPos()).normalize()*(1.0-dist/viewRadius);
	        center += agents[i].GetPos();
	        alignment += agents[i].GetVel();
	        numSeen++;
	    }
      }//endfor i

      vector<Vector3d>& attractionPoints = gEnv->GetAttractionPoints();
      int closestAPIndex = -1;
      double closestAPDist = 1e6;
      for(int i = 0; i<(int)attractionPoints.size(); i++) 
      {
	        double thisDist = (attractionPoints[i]-pos).norm();
	        if(thisDist<closestAPDist) 
            {
	            closestAPDist = thisDist;
	            closestAPIndex = i;
	        }
      }

      if( closestAPIndex != -1 ) 
      {
	        Vector3d& apt = attractionPoints[closestAPIndex];
	        attractionPtForce = (apt - pos).normalize();
	        attractionPtForce *= 100.0;
      }

      if(numAdv>0) 
      {
	        status -= 5.0*numAdv;
	        if( status < 0 ) status = 0;
      }

    }
    else 
    { //generate advesarial force
      int closestIndex=0;
      double closeDist = 1e6;
      for(int i=0; i<(int)agents.size(); i++) 
      {
          if (id == i)
          {
              continue; //skip self
          }

	     //see if within view radius
	     double dist = (agents[i].GetPos()-pos).norm();
	    if( dist < viewRadius && dist < closeDist ) 
        {
	        closestIndex = i;
	        closeDist = dist;
	        numSeen++;
	    }
      }//endfor i

      force = maxAccel*(agents[closestIndex].GetPos()-pos).normalize();
    }
    
    obstacleForce = GetEnvironmentalForce(20);
    if( numSeen>0 && !isAdversary ) 
    { 
      //regular find final force
      center /= 1.0*numSeen;
      double distToCenter = (center-pos).norm();
      cohesion = (center-pos).normalize()*cohesionComponent*(distToCenter/viewRadius);
      //separation *= separationComponent;
      //alignment *= alignmentComponent;
      separation = separation * separationComponent;
      alignment = alignment.normalize() * alignmentComponent;

      if (agDebug)
      {
          cout << "component vectors for : " << id << endl;
          cout << "sep " << separation << endl;
          cout << "coh " << cohesion << endl;
          cout << "ali " << alignment << endl;
      }

      force = separation + cohesion + alignment + attractionPtForce + obstacleForce;


      if (force.norm() > maxAccel)
      {
          force.selfScale(maxAccel);
      }

      separationForce = separation;
      cohesionForce = cohesion;
      alignmentForce = alignment;

      //force = separation;// + cohesion + alignment;

      if (agDebug)
      {
          cout << "totalf " << force << endl;
      }

      force /= mass;
    }
    else if( numSeen>0 && isAdversary ) 
    { //adv. find final force
      force /= mass;
    }
    else 
    {
        if (agDebug)
        {
            cout << "No agents visible for agent:  " << id << endl;
        }
    }
  }

  if (agDebug)
  {
      cout << "origPos " << pos << endl;
      cout << "origVel " << vel << endl;
  }

  oldPos = pos;
  pastPos.push_back(pos);

  while (pastPos.size() > tailLength)
  {
      pastPos.erase(pastPos.begin());
  }

  pos = pos + vel*dt;
  vel = vel + force*dt;

  if (agDebug)
  {
      cout << "newPos " << pos << endl;
      cout << "newVel " << vel << endl;
  }

  if( vel.norm() > maxVel ) 
  {
    vel.selfScale(maxVel);
  }

  if (agDebug)
  {
      cout << "newVel(trunc) " << vel << endl;
  }

  if( vel.norm() > 0.1 ) 
  {
    ori = atan2( vel[1], vel[0] );
    //cout << "ORI " << ori << endl;
  }

  //Lifetime
  lifespan += 0.000000005;

  bool wrapWorld = false;
  if (wrapWorld)
  {
      bool updated = false;
      Vector3d pNew = gEnv->GetWrappedPosition(pos, updated);
      if (updated)
      {
          pos = pNew;
          pastPos.clear();
      }
  }

  bool cdWithAgents = true;
  if (cdWithAgents)
  {
      ResolveCollisionWithOtherAgents(agents);
  }

  bool cdWorld = true;
  if (cdWorld)
  {
      bool updated = false;
      Vector3d pNew = gEnv->GetValidPosition(pos, oldPos, radius, vel, updated);
      if (updated)
      {
          pos = pNew;
      }
  }
  
}

void Agent::SetControl(string control) {
  lastControl = control;
  timeInControl = 0;
}

Vector3d Agent::GetForceFromControl() {
  timeInControl++;
  Vector3d force(0,0,0);
  if( timeInControl < 2 ) {
    if( lastControl == "forward" ) {
      Vector3d dir = vel.normalize();
      force = dir * 10.0*maxAccel;
    }
    else if( lastControl == "back" ) {
      Vector3d dir = vel.normalize();
      force = dir * (-10.0*maxAccel);
    }
    else if( lastControl =="left" ) {
      Vector3d dir = vel.normalize();
      force[0] = dir[1];
      force[1] = -1*dir[0];
      force *= -10.0*maxAccel;
    }
    else if( lastControl == "right" ) {
      Vector3d dir = vel.normalize();
      force[0] = dir[1];
      force[1] = -1*dir[0];
      force *= 10*maxAccel;
    }
  }
  return force;
}

//const double TWOPI = 6.28318;
void drawCircle(double radius, int divisions, bool filled) 
{
  double deltaAng = TWOPI / divisions;
  double curAng = 0;
  if (!filled)
  {
      glBegin(GL_LINE_LOOP);
  }
  else
  {
      glBegin(GL_POLYGON);
  }
  for(int i=0; i<divisions; i++) 
  {
    double x = radius * cos(curAng);
    double y = radius * sin(curAng);
    if (agDebug)
    {
        cout << "Circle " << i << " x= " << x << " y= " << y << endl;
    }
    glVertex2f( x, y );
    curAng += deltaAng;
    if (agDebug)
    {
        cout << "curAng= " << curAng << " deltaAng= " << deltaAng << endl;
    }
  }
  glEnd();
}

void drawAgentAsCircle(double radius, int divisions, bool filled, double percStatus) 
{
  //drawOutline
    drawCircle(radius, divisions, false);
  //if( percStatus < 0.99 ) {
    drawCircle(percStatus*radius, divisions, filled);
  //}
}

void drawTriangle(double length, bool filled) 
{
    if (!filled)
    {
        glBegin(GL_LINE_LOOP);
    }
    
    else
    {
        glBegin(GL_TRIANGLES);
    }

  glVertex2f(length/2.0, 0);
  glVertex2f(-length/2.0, length/3.0);
  glVertex2f(-length/2.0, -length/3.0);
  glEnd();
}

void drawTriangleStatus(double length, double percStatus) 
{
  glBegin(GL_LINE_LOOP);
  glVertex2f(-length/2.0, length/3.0);
  glVertex2f(-length/2.0, -length/3.0);
  glVertex2f(-length/2.0-5, -length/3.0);
  glVertex2f(-length/2.0-5, length/3.0);
  glEnd();
  double del = percStatus * 2*length/3;
  glBegin(GL_POLYGON);
  glVertex2f(-length/2.0-5, length/3.0);
  glVertex2f(-length/2.0, length/3.0);
  glVertex2f(-length/2.0, length/3.0-del);
  glVertex2f(-length/2.0-5, length/3.0-del);
  glEnd();
}

void drawBodyFish(double xradius, double yradius)
{
    //local variables
    const float DEG2RAD = 3.14159 / 180.0;
    float degInRad;
    float x, y;

    //Draw Body
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; i++)
    {
        //convert degrees into radians
        degInRad = i * DEG2RAD;
        x = cos(degInRad) * xradius, y = sin(degInRad) * yradius;
        glVertex2f(x, y);
    }
    glEnd();
}

void drawFishTail()
{
    glBegin(GL_POLYGON);
    glVertex2f(-10.0, 4.0);
    glVertex2f(-10.0, -4.0);
    glVertex2f(-30.0, -8.0);
    glVertex2f(-15.0, -2.0);
    glVertex2f(-20.0, 9.0);
    glEnd();
}

void drawFishFins()
{
    glBegin(GL_TRIANGLES);
    glVertex2f(10.0, -2.0);
    glVertex2f(25.0, -2.0);
    glVertex2f(0.0, -12.0);
    glEnd();
}

void Agent::Draw() 
{
    if (!isAdversary)
    {
        BaseB -= lifespan;
        glColor3f(BaseR, BaseG, BaseB);
    }
    
    else
    {
        AdvR -= lifespan;
        glColor3f(AdvR, AdvG, AdvB);
    }
    
  /*
  glPointSize(5);
  glBegin(GL_POINTS);
  glVertex2f( pos.GetX(), pos.GetY() );
  glEnd();
  */
  if( drawMode == 1 ) 
  {
      glPushMatrix();

      glTranslatef(pos.GetX(), pos.GetY(), 0);
      glRotated(radToDeg(ori), 0, 0, 1);
      drawBodyFish(radius, radius / 2);

      glTranslatef(0.0, -1.0, 0.0);
      glColor3ub(FinR, FinG, FinB);
      drawFishTail();

      glTranslatef(-11.0, 2.0, 0.0);
      glColor3ub(FinR, FinG, FinB);
      drawFishFins();

      glPopMatrix();
  }
  else if( drawMode == 2 ) {
    glPushMatrix();
    glTranslatef(pos.GetX(), pos.GetY(), 0);
    glRotated( radToDeg(ori), 0,0,1);
    drawTriangle(2*radius, isControlled);
    drawTriangleStatus(2*radius, 1.0*status/maxStatus);
    glPopMatrix();
  }
  else if (drawMode == 3)
  {
      glPushMatrix();
      glTranslatef(pos.GetX(), pos.GetY(), 0);
      //drawCircle(radius, 10, isControlled);
      drawAgentAsCircle(radius, 10, isControlled, 1.0 * status / maxStatus);
      glPopMatrix();
  }
  else 
  {
    glPushMatrix();
    glTranslatef(pos.GetX(), pos.GetY(), 0);
    drawCircle(radius, 10, isControlled);
    glPopMatrix();
    glColor3f(0.8,0.8,0.8);
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    for(int i=0; i<(int)pastPos.size(); i++) {
      glVertex2f(pastPos[i][0],pastPos[i][1]);
    }
    glEnd();
  }

  if( drawForce ) 
  {
    glColor3f(1.0, 0.48, 0.0);
    glBegin(GL_LINES);
    glVertex2f( pos.GetX(),pos.GetY() );
    glVertex2f( pos.GetX()+separationForce.GetX(), pos.GetY()+separationForce.GetY() );
    glEnd();

    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex2f( pos.GetX(),pos.GetY() );
    glVertex2f( pos.GetX()+cohesionForce.GetX(), pos.GetY()+cohesionForce.GetY() );
    glEnd();

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f( pos.GetX(),pos.GetY() );
    glVertex2f( pos.GetX()+alignmentForce.GetX(), pos.GetY()+alignmentForce.GetY() );
    glEnd();
  }

  if( drawVelocity ) 
  {
    //cout << "drawVelocity" << endl;
    glColor3f(1.0, 1.0, 0.0);
    glBegin(GL_LINES);
    glVertex2f( pos.GetX(),pos.GetY() );
    glVertex2f( pos.GetX()+vel.GetX(), pos.GetY()+vel.GetY() );
    glEnd();
  }
  else 
  { 
      /*cout << "NO drawVelocity. " << endl;*/ 
  }
}

void Agent::ResolveCollisionWithOtherAgents(vector<Agent>& agents) 
{
  for(int i=0; i<agents.size(); i++) 
  {
      if (id == agents[i].GetID())
      {
          continue; //ignore self
      }
      else
      {
      double dist = (pos - agents[i].GetPos()).norm();
      if( dist < (radius + agents[i].GetRadius()) ) 
      {
	    double overlap = fabs( dist-(radius +agents[i].GetRadius()) );
	    Vector3d resolveDir = (pos-agents[i].GetPos()).normalize();
	    pos += (overlap/2)*resolveDir;
	    Vector3d& pos_i = agents[i].GetPos();
	    pos_i += (-overlap/2)*resolveDir;
      }//end if dist
    }
  }//endfor i<agents
}