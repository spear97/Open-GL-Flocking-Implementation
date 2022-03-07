#ifndef ENVIRONMENT
#define ENVIRONMENT

#include "Point.h"
#include "Vector.h"
using namespace mathtool;
#include <iostream>
#include <string>
#include <vector>
using namespace std;

class Cell {
public:
  Cell();
  void Init(int _i, int _j, Point _center, double _dx, double _dy, bool _blocked);
  void Draw();
  Vector3d GetCenter() { return Vector3d(center.GetX(),center.GetY(),0); }
  bool IsCollision(Vector3d pNew, double radius);

  //this function will set maximums for the cell and return values
  //like whether or not the x/y is in collision, the
  //min/max values, and which condition is violated
  void EdgeCheck(Vector3d pNew, double radius, bool& flipX, bool& flipY, double& txmin, double& txmax, double& tymin, double& tymax, int& xcondition, int& ycondition);

  int xi;
  int xj;
  double dx;
  double dy;
  Point center;
  Point minPt;
  Point maxPt;
  bool blocked;
  bool isCol;
};

class Environment {
public:
  Environment();
  Environment(string _inputFile, double _xmin, double _xmax, double _ymin, double _ymax );
  void Draw();

  pair<int,int> GetCellIndices(Vector3d p);
  pair<int,int> GetClosestBlocked(pair<int,int> curCellInd, Vector3d pos, double radius);
  Vector3d GetWrappedPosition(Vector3d p, bool& updated);
  Vector3d GetValidPosition(Vector3d p, Vector3d oldP, double radius, Vector3d& vel, bool& updated);
  void MakeEmptyEnv();

  void AddAttractionPoint(double tx, double ty);

  vector<Vector3d>& GetAttractionPoints() { return attractionPts; }

  Cell** cells;

  double xmin, xmax, ymin, ymax; 
  double dx, dy;
  int rows, columns;
  string inputFile;

  vector<Vector3d> attractionPts;

};

#endif
