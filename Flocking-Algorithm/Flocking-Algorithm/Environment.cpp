#include <windows.h>
#pragma comment(lib, "user32.lib") 

#include "Environment.h"
#include <fstream>

#include <GL/glut.h>  // (or others, depending on the system in use)

///////////////////////////////////////////////////////////
Cell::Cell() 
{  
  blocked = false;
  isCol = false;
}
void Cell::Init(int _i, int _j, Point _center, double _dx, double _dy, bool _blocked) 
{
  xi = _i;
  xj = _j;
  dx = _dx;
  dy = _dy;
  center = _center;
  minPt.x = center.x-dx/2.0;
  minPt.y = center.y-dy/2.0;
  maxPt.x = center.x+dx/2.0;
  maxPt.y = center.y+dy/2.0;
  blocked = _blocked;
  isCol = false;
}

void Cell::Draw() 
{
  glLineWidth(1);
  if( blocked ) 
  {
      if (isCol)
      {
          glColor3f(0.6, 0.9, 0.6);
      }
      
      else
      {
          glColor3f(0.6, 0.6, 0.6);
      }
    glBegin(GL_POLYGON);
  }

  else 
  {
    glColor3f(0.6, 0.6, 0.6);
    glBegin(GL_LINE_LOOP);
  }
  glVertex2f(minPt.x, minPt.y);
  glVertex2f(maxPt.x, minPt.y);
  glVertex2f(maxPt.x, maxPt.y);
  glVertex2f(minPt.x, maxPt.y);
  glEnd();
}

bool Cell::IsCollision(Vector3d pNew, double radius) 
{
    bool collision = (pNew[0] < (center.GetX() + dx / 2 + radius)) 
                    && (pNew[0] > (center.GetX() - dx / 2 - radius)) 
                    && (pNew[1] < (center.GetY() + dy / 2 + radius)) 
                    && (pNew[1] > (center.GetY() - dy / 2 - radius));
  if(collision)
  {
    cout << "pNew=" << pNew << " radius=" << radius << " dx= " << dx << " dy=" << dy << endl;
    cout << "condition1=" << (center.GetX()+dx+radius) << endl;
    cout << "condition2=" << (center.GetX()-dx-radius) << endl;
    cout << "condition3=" << (center.GetY()+dy+radius) << endl;
    cout << "condition4=" << (center.GetY()-dy-radius) << endl;
    isCol = true;
    return true;
  }
  return false;
}


//this function will set maximums for the cell and return values
//like whether or not the x/y is in collision, the
//min/max values, and which condition is violated
void Cell::EdgeCheck(Vector3d p, double radius, bool& flipX, bool& flipY, double& txmin, double& txmax, double& tymin, double& tymax, int& xcondition, int& ycondition) 
{
  txmin = center.GetX()-dx/2-radius;
  txmax = center.GetX()+dx/2+radius;
  tymax = center.GetY()+dy/2+radius;
  tymin = center.GetY()-dy/2-radius;
  xcondition = -1;
  ycondition = -1;
  flipX = false;
  flipY = false;

  if( p[0]>txmin && p[0]<txmax ) 
  {
    flipX = true;
    if( fabs(p[0]-txmax) < fabs(p[0]-txmin)) xcondition = 1;
    else xcondition = 0; 
  }

  if( p[1]>tymin && p[1]<txmax ) 
  {
    flipY = true;

    if (fabs(p[1] - tymax) < fabs(p[1] - tymin))
    {
        ycondition = 1;
    }

    else ycondition = 0;
  }
}

///////////////////////////////////////////////////////////
Environment::Environment() {/*Constructor*/}

Environment::Environment(string _inputFile, double _xmin, double _xmax, double _ymin, double _ymax ) 
{
  inputFile = _inputFile;

  xmin = _xmin;
  xmax = _xmax;
  ymin = _ymin;
  ymax = _ymax;
  cout << " Environment boundaries: [" << xmin << ", " << xmax << " | " << ymin << ", " << ymax << "]"<< endl;

  cout << "Attempting to load environment file: " << inputFile << endl;
  ifstream infile;
  infile.open(inputFile.c_str());

  if( !infile ) 
  {
    cout << "Could NOT open infile: " << inputFile << endl;
	//exit(-1);
  }
  if(infile >> rows) 
  {
    cout << "There will be " << rows << " rows ";
  }
  if(infile >> columns) 
  {
    cout << "There will be " << columns << " rows ";
  }
  cells = new Cell*[rows];
  for(int i=0; i<rows; i++) 
  {
    cells[i] = new Cell[columns];
  }
  dx = (_xmax-_xmin)/columns;
  dy = (_ymax-_ymin)/rows;
  string colData;
  string tmpStr;
  getline(infile, tmpStr); //just to get rid of carriage return
  int rowIndex=0;
  while( getline(infile, colData) ) 
  {
    //cout << rowIndex << " going to process '"<<colData<<"'"<<endl;
    for(int i=0; i<colData.length(); i++) 
    {
      Cell& cell = cells[rowIndex][i];
      bool isBlocked = (colData.at(i)=='x'||colData.at(i)=='X')? true: false;
      Point center(xmin+ dx/2+dx*i,ymin+ dy/2+dy*rowIndex);
      //cout << "Initializing cell: " << rowIndex << ", " << i << " center: " << center.GetX() << " " << center.GetY() << " dx " << dx << " dy " << dy << " isBlocked " << isBlocked << endl;
      cell.Init(rowIndex,i,center,dx,dy,isBlocked);
    }
    rowIndex++;
  }
  infile.close();
}

void drawAttractionPt(double radius, int divisions, bool filled) 
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
    glVertex2f( x, y );
    curAng += deltaAng;
  }
  glEnd();
}

void Environment::Draw() 
{
  for(int i=0; i<rows; i++) 
  {
      for (int j = 0; j < columns; j++)
      {
          cells[i][j].Draw();
      }
  }

  for(int i=0; i<attractionPts.size(); i++) 
  {
    Vector3d& tv = attractionPts[i];
    glPushMatrix();
    glTranslatef(tv[0],tv[1],0);
    drawAttractionPt(10, 4, true);
    glPopMatrix();
  }

}

pair<int,int> Environment::GetCellIndices(Vector3d p) 
{
  //cout << "p= " << p << " [" << xmin << "," << xmax << "|" << ymin << "," << ymax << "]"<< endl;
  int i = int((p[0]-xmin)/dx);
  int j = int((p[1]-ymin)/dy);

  //cout << "1i " << int((p[0]-xmin)/(xmax-xmin)) << " j " << j << endl;
  if (i < 0)
  {
      i = 0;
  }

  if (j < 0)
  {
      j = 0;
  }

  if (i >= columns)
  {
      i = columns - 1;
  }

  if (j >= rows)
  {
      j = rows - 1;
  }

  //cout << "2i " << i << " j " << j << endl;
  return make_pair(j,i);
}

pair<int,int> Environment::GetClosestBlocked(pair<int,int> curCellInd, Vector3d pos, double radius) 
{
  int numXToOffset = int(dx/radius + 0.5) + 1;
  int numYToOffset = int(dy/radius + 0.5) + 1;
  int xIndexStart = curCellInd.second-numXToOffset;
  int yIndexStart = curCellInd.first-numYToOffset;

  if (xIndexStart < 0)
  {
      xIndexStart = 0;
  }

  if (yIndexStart < 0)
  {
      yIndexStart = 0;
  }

  int xIndexEnd = curCellInd.second+numXToOffset;
  int yIndexEnd = curCellInd.first + numYToOffset;

   if (xIndexEnd >= columns)
   {
       xIndexEnd = columns - 1;
   }

   if (yIndexEnd >= rows)
   {
       yIndexEnd = rows - 1;
   }

  //cout << "GetClosestBlocked xStart " << xIndexStart << " xEnd " << xIndexEnd << " yStart " << yIndexStart << " yEnd " << yIndexEnd << endl;
  pair<int,int> closestCellIndex(-1,-1);
  double closeDist = 1e6;
  for(int j=yIndexStart; j<=yIndexEnd; j++) 
  {
    for(int i=xIndexStart; i<=xIndexEnd; i++) 
    {
      if(cells[j][i].blocked) 
      {
	    double dist = (cells[j][i].GetCenter()-pos).norm();
	    if( dist < closeDist ) 
        {
	        closeDist = dist;
	        closestCellIndex = make_pair(j,i);
	    }
      }
    }//endfor i
  }//endfor j
  return closestCellIndex;
}

Vector3d Environment::GetWrappedPosition(Vector3d p, bool& updated) 
{
  Vector3d pNew(p);
  if( pNew.GetX()>xmax ) 
  {
    pNew.SetX( pNew.GetX() - (xmax-xmin) );
    updated=true;
  }
  else if( pNew.GetX()<xmin ) 
  {
    //pNew[0] += (xmax-xmin);
	pNew.SetX(pNew.GetX() + (xmax - xmin));
    updated=true;
  }

  if( pNew.GetY()>ymax ) 
  {
    //pNew[1] -= (ymax-ymin);
	pNew.SetY(pNew.GetY() - (ymax - ymin));
    updated=true;
  }
  else if( pNew.GetY()<ymin ) 
  {
	  //pNew[1] += (ymax - ymin);
	  pNew.SetY(pNew.GetY() + (ymax - ymin));
      updated=true;
  }
  return pNew;
}

Vector3d Environment::GetValidPosition(Vector3d p, Vector3d oldP, double radius, Vector3d& vel, bool& updated) {
  Vector3d pNew(p);
  if( pNew.GetX()+radius>xmax ) 
  {
    //backoff by half and call again
    /*
    double limX = xmax-radius;
    double del = fabs(p[0]-oldP[0]);
    double t = 1- (p[0]-limX)/del;
    pNew = oldP + (0.99*t)*(p-oldP);
    vel[0] *=-1;
    */
    pNew = oldP;
    pNew[0] = xmax - radius;
    vel[0] *=-1;
    updated=true;
  }
  else if( pNew.GetX()-radius<xmin ) 
  {
    //backoff by half and call again
    /*
    double limX = xmin+radius;
    double del = fabs(p[0]-oldP[0]);
    double t = 1- (p[0]-limX)/del;
    pNew = oldP + t*(p-oldP);
    */
    pNew = oldP;
    pNew[0] = xmin + radius;
    vel[0] *=-1;
    updated=true;
  }

  if( pNew.GetY()+radius>ymax ) 
  {
    //backoff by half and call again
    /*
    double limY = ymax-radius;
    double del = fabs(p[1]-oldP[1]);
    double t = 1- (p[1]-limY)/del;
    pNew = oldP + (0.99*t)*(p-oldP);
    */
    pNew[1] = ymax - radius;
    vel[1] *=-1;
    updated=true;
  }
  else if( (pNew.GetY()-radius)<ymin ) 
  {
    //backoff by half and call again
    /*
    double limY = ymin+radius;
    double del = fabs(p[1]-oldP[1]);
    double t = 1- (p[1]-limY)/del;
    pNew = oldP + t*(p-oldP);
    */
    pNew[1] = ymin + radius;
    vel[1] *=-1;
    updated=true;
  }

  //return pNew;
  //////////////////////////////////////////////////////////////
  //find current cell
  //find nearest blocked cell
  //if p is in collision set to oldP and reflect
  pair<int,int> curCellInd = GetCellIndices(pNew);
  //cout << "Point pNew: " << pNew << " in cell: " << curCellInd.first << "," << curCellInd.second << endl;
  pair<int,int> closestBlocked = GetClosestBlocked(curCellInd, pNew, radius);
  //cout << "Point pNew: " << pNew << " closest blocked cell: " << closestBlocked.first << "," << closestBlocked.second << endl;

  if( closestBlocked.first != -1 && closestBlocked.second != -1 ) 
  {
    //here is where the position may be updated
    //check if collision with the cell
    int j = closestBlocked.first;
    int i = closestBlocked.second;
    Cell& cell = cells[j][i];
    //top part of cell
    if( cell.IsCollision(pNew, radius) ) 
    {
      //found collision
      //cout << "found collision!!! oldVel=" << vel << endl;
      pNew = oldP;
      updated=true;
      bool flipX=false;
      bool flipY=false;
      double txmin, txmax, tymin, tymax; 
      int tcondX, tcondY;
      cell.EdgeCheck(pNew,radius, flipX, flipY, txmin, txmax, tymin, tymax, tcondX, tcondY);
      if( flipX ) 
      {
        vel[0] *= -1.0;
        if (tcondX == 1)
        {
            pNew[0] = txmax;
        }

        else
        {
            pNew[0] = txmin;
        }

      }

      if( flipY ) 
      {
	    vel[1] *= -1.0;
        if (tcondY == 1)
        {
            pNew[1] = tymax;
        }

        else
        {
            pNew[1] = tymin;
        }
      }
      //cout << " new vel = " << vel << endl;
    }
  }

  return pNew;
}

void Environment::MakeEmptyEnv() 
{
  for(int j=0; j<rows; j++) 
  {
    for(int i=0; i<columns; i++) 
    {
      cells[j][i].blocked = false;
    }
  }
}


void Environment::AddAttractionPoint(double tx, double ty)
{
  Vector3d aP(tx,ty,0);
  attractionPts.push_back(aP);
}
