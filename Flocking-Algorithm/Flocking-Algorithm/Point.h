#ifndef POINT
#define POINT

class Point {
   public:
      Point(double _x=0, double _y=0, double _z=0) { x = _x; y = _y; z=_z;}
      
      double GetX() { return x; }
      double GetY() { return y; }
      double GetZ() { return z; }

      double x, y, z;
};

#endif
