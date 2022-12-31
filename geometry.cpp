#define _USE_MATH_DEFINES
#include <math.h>
#include <iostream>

#include "geometry.h"


void printRect(const glm::vec4& rect)
{
    std::cout << rect.x << " " << rect.y << " " << rect.z << " " << rect.a << std::endl;
}


float pointDistance(const glm::vec2& v1, const glm::vec2& v2)
{
    return sqrt(pow(v1.x - v2.x,2) + pow(v1.y - v2.y,2));
}

glm::vec2 findMidpoint(const glm::vec4& v1)
{
    return {(v1.x + v1.z)/2, (v1.y + v1.a)/2};
}

bool vecIntersect(const glm::vec4& vec1,const glm::vec4& vec2)
{
    if (vec1.z < 0 || vec1.a < 0 || vec2.z < 0 || vec2.a < 0) //if a dimension is negative, we will have to rearrange where the corners are
    {
        glm::vec4 rect1 = vec1, rect2 = vec2;
        if (vec1.z < 0)
        {
            rect1.x += vec1.z;
            rect1.z *= -1;
        }
        if (vec1.a < 0)
        {
            rect1.y += vec1.a;
            rect1.a *= -1;
        }
        if (vec2.z < 0)
        {
            rect2.x += vec2.z;
            rect2.z *= -1;
        }
        if (vec2.a < 0)
        {
            rect2.y += vec2.a;
            rect2.a *= -1;
        }
        return vecIntersect(rect1,rect2);
    }
    bool b1 = vec1.x <= vec2.x + vec2.z;
    bool b2 = vec1.x + vec1.z>= vec2.x;
    bool b3 = vec1.y <= vec2.y + vec2.a;
    bool b4 = vec1.y + vec1.a >= vec2.y;
    return (vec1.x <= vec2.x + vec2.z && vec1.x + vec1.z>= vec2.x && vec1.y <= vec2.y + vec2.a && vec1.y + vec1.a >= vec2.y);
}
bool vecIntersect(const glm::vec4& vec1,const glm::vec4& vec2, float angle1, float angle2)
{
  /*This function does a ghetto version of Separating Axis Theorem. It basically takes one rect to be an axis and transforms the other rect to that
  rect's local space, then tests if that rect's bounding box collides with the axis rect. It does thsi for both rects and if they both collide, then
  there is collision between the two rects.
  */
   /* if (angle1 == 0 && angle2 == 0)
    {
        return vecIntersect(vec1,vec2);
    }
    glm::vec2 center = {vec2.x + vec2.z/2, vec2.y + vec2.a/2};
    glm::vec2 topLeft = rotatePoint({vec2.x, vec2.y}, center, angle2);
    glm::vec2 topRight = rotatePoint({vec2.x + vec2.z, vec2.y}, center, angle2);
    glm::vec2 botLeft = rotatePoint({vec2.x, vec2.y + vec2.a}, center, angle2);
    glm::vec2 botRight = rotatePoint({vec2.x + vec2.z, vec2.y + vec2.a}, center,angle2);



    //Basically just see if any sides intersect
    return (lineInVec(topLeft, topRight, vec1, angle1) || //top side
            lineInVec(topLeft, botLeft, vec1, angle1) || //left side
            lineInVec(botLeft, botRight, vec1, angle1) || //bot side
            lineInVec(topRight, botRight, vec1, angle1));*/
    auto lambda = [](const glm::vec4& vec, float angle, const glm::vec4& axis, float axisAngle){
        //"lambda" takes "vec" and transforms it to "axis'" space and checks if its bounding box collides with "axis".
        float slope = sqrt(pow(vec.z/2,2)+pow(vec.a/2,2)); //Basically the distance from "vec's" center to a vertex
        float theta = atan2(vec.a,vec.z); //angle of center to vertex.
        glm::vec2 center = rotatePoint(glm::vec2(vec.x + vec.z/2, vec.y + vec.a/2),glm::vec2(axis.x + axis.z/2, axis.y + axis.a/2),-axisAngle); //the center of "vec", rotated around the center of "axis"
                                                                                                                                                //we use negative "axisAngle" because we are reverting the rotation of "axis"

        //the way we represent our tranlsated rectangle is as a dispalcement from the center
        //"angle"  +- "theta" is the angle of the vertice to center. We don't know if it's + or - theta so we take the bigger of the two
        //we subtract "axisAngle" because rotating "vec" around "axis" also rotates it by "axisAngle"
        float rightLeft = slope*std::max(abs(cos(angle + theta - axisAngle)),abs(cos(angle  - theta - axisAngle))); //once all rotations are considered, "rightLeft" represents the displacement from the center for the furthest right and left of the bounding box
        float upDown = slope*std::max(abs(sin(angle + theta - axisAngle)),abs(sin(angle  - theta - axisAngle)));    //same thing but for up and down

        return axis.x + axis.z > center.x - rightLeft && axis.x < center.x + rightLeft &&
                axis.y + axis.a > center.y - upDown && axis.y < center.y + upDown;
    };
    return lambda(vec1,angle1,vec2,angle2) && lambda(vec2,angle2,vec1,angle1);
}

glm::vec4 vecIntersectRegion(const glm::vec4& vec1, const glm::vec4& vec2) //returns the region of two colliding rects
{
    glm::vec4 answer= {0,0,0,0};

    if (vec1.y + vec1.a >= vec2.y && vec1.y <= vec2.y + vec2.a)
    {
        answer.y = std::max(vec1.y, vec2.y);
        answer.a = std::min(vec2.y + vec2.a, vec1.y + vec1.a) - answer.y;
    }
    if (vec1.x + vec1.z >= vec2.x && vec1.x <= vec2.x + vec2.z)
    {
        answer.x = std::max(vec1.x, vec2.x);
        answer.z = std::min(vec2.x + vec2.z, vec1.x + vec1.z) - answer.x;
    }
    return answer;
}

bool vecInside(const glm::vec4& vec1, const glm::vec4& vec2)
{
    glm::vec4 region = vecIntersectRegion(vec1,vec2);
    return region.z > 0 && region.a > 0;
}

double vecDistance(const glm::vec4& rect1, const glm::vec4& rect2)
{
    glm::vec2 comp = {0,0}; //the horizontal and vertical components of the distance
    glm::vec4 region = vecIntersectRegion(rect1,rect2);
    if (region.z == 0) //there is no horizontal overlap. This means that the rect are left and right of each other but with no intersection
    {
        double right = std::min(rect1.x + rect1.z, rect2.x + rect2.z);
        double left =  std::max(rect1.x, rect2.x);
        comp.x = std::max(right,left) - std::min(right,left);
    }
    if (region.a == 0) //same as before but for vertical overlap
    {
        double down = std::min(rect1.y + rect1.a, rect2.y + rect2.a);
        double up =  std::max(rect1.y, rect2.y);
        comp.y = std::max(up,down) - std::min(up,down);
    }
    return sqrt(pow(comp.x,2) + pow(comp.y,2));
}

bool vecContains(glm::vec4 r1, glm::vec4 r2)
{
    return (r1.x >= r2.x && r1.x + r1.z <= r2.x + r2.z && r1.y >= r2.y && r1.y + r1.a <= r2.y + r2.a);
}

bool pointInVec(const glm::vec4& vec1, double x, double y, double angle)
{
    glm::vec2 center = {vec1.x + vec1.z/2, vec1.y+vec1.a/2};
    glm::vec2 rotated = rotatePoint({x,y},center,-angle);
    return rotated.x >= vec1.x && rotated.x <= vec1.x + vec1.z && rotated.y >= vec1.y && rotated.y <= vec1.y +vec1.a;
}

bool pointInVec(const glm::vec4& vec1, const glm::vec2& point, double angle)
{
    return pointInVec(vec1,point.x,point.y,angle);
}

double pointVecDistance(const glm::vec4& vec, float x, float y, float angle)
{
    if (angle != 0)
    {
        glm::vec2 rotated = rotatePoint({x,y},{vec.x + vec.z/2, vec.y + vec.a/2},-angle);
        return pointVecDistance(vec,rotated.x,rotated.y,0);
    }
    return sqrt(pow(std::min(vec.x + vec.z,std::max(vec.x, x)) - x,2) + pow(std::min(vec.y + vec.a,std::max(vec.y, y)) - y,2));
}

glm::vec2 closestPointOnVec(const glm::vec4& vec, const glm::vec2& point) //returns the point on vec that is the closest distance to point. Returns point if point is in vec
{
    return {std::min(vec.x + vec.z,std::max(vec.x, point.x)),std::min(vec.y + vec.a,std::max(vec.y, point.y))};
}


bool inLine(const glm::vec2& point, const glm::vec2& point1, const glm::vec2& point2) //line 1 and line 2 are the points of the line
{
    if (point1.x == point2.x)
    {
        return point1.x == point.x && point.y >= std::min(point1.y,point2.y) && point.y <= std::max(point1.y,point2.y);
    }
    float slope = (point2.y - point1.y)/(point2.x - point1.x);
    float yInt = point1.y - slope*point1.x;
    return slope*point.x + yInt == point.y && point.x >= std::min(point1.x,point2.x) && point.x <= std::max(point1.x,point2.x);
}

double pointLineDistance(const glm::vec4& line, const glm::vec2& point)
{
    glm::vec2 a = {line.x,line.y};
    glm::vec2 b = {line.z,line.a};
    double abDist = (pointDistance(a,b));
    double bpDist = (pointDistance(b,point));
    double apDist = pointDistance(a,point);
    double bAngle =  acos((pow(apDist,2) - ( pow(abDist,2)+pow(bpDist,2) ))/(-2*abDist*bpDist)); //angle pba
    double aAngle = acos((pow(bpDist,2) - ( pow(abDist,2)+pow(apDist,2) ))/(-2*abDist*apDist)); //angle pab
    if (aAngle > M_PI/2 || bAngle > M_PI/2) //if either angle is obtuse, the shortest distance is the distance between point and one of the endpoints
    {
        return std::min(bpDist,apDist);
    }
    return sin(bAngle)*bpDist;

}

glm::vec3 bezierNums(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2) //useful helper function for workign with line segments
{
    //https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line_segment
    return {
    (a1.x - a2.x)*(b1.y - b2.y) - (a1.y - a2.y)*(b1.x - b2.x), //the common demonator, 0 if the lines are parallel
    (a1.x - b1.x)*(b1.y - b2.y) - (a1.y - b1.y)*(b1.x - b2.x), //t's Numerator. this/denominator = t
    (a1.x - b1.x)*(a1.y - a2.y) - (a1.y - b1.y)*(a1.x - a2.x) //u's numerator
    };
}

glm::vec2 sortFunc(const glm::vec2& p1, const glm::vec2& p2){
//sorting function helps us find the "most top left" point. p1 if they are the same point.
//using this sort function is effectively teh same as comparing y coordinates for 2 vertical lines and comparing x coordinates for all other coincident lines
//very useful for line segment functions
        if (p1.x < p2.x)
        {
            return p1;
        }
        else if (p2.x > p1.x)
        {
            return p2;
        }
        else
        {
            return (p1.y <= p2.y ? p1 : p2);
        }
}
bool lineInLine(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2)
{
    //https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line_segment
    glm::vec3 consts = bezierNums(a1,a2,b1,b2);
    int denom = consts.x;
    int tNum = consts.y; //numerator for t
    //as of right now, I can't for the life of me convince myself that t AND u are necessary, since the existicen of t implies the existence of u. Might change later
    if (denom == 0)
    {
        if (tNum == 0) //this means they are the same line (coincident lines)
        {
            glm::vec2 aMost = sortFunc(a1,a2);
            glm::vec2 bMost = sortFunc(b1,b2);

            return (sortFunc(aMost,bMost) == aMost && sortFunc(bMost,a1 + a2 - aMost) == bMost) || //a1 + a2 - aMost returns the other point that's not aMost
                   (sortFunc(aMost,bMost) == bMost && sortFunc(aMost, b1 + b2 - bMost) == aMost); //this basically finds if 2nd most point is between the other line
        }
        else //if denom == 0 and tNum is not 0, that means the segments don't collide
        {
            return false;
        }
    }
    else if ((float)tNum/denom >= 0 && (float)tNum/denom <= 1) //as per the wiki page, 't', or tNum/denom in our case, has to be between 0 and 1
    {
        return true;
    }
    return false; //the last case is that tNum/denom is not between 0 and 1, and they are both not 0. This means there is no intersection.
}

bool lineInLineExtend(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2)
{
    if (a1.x == a2.x && b1.x ==b2.x)
    {
        return b1.x == a1.x;
    }
    float slope1 = (a1.y - a2.y)/(a1.x - a2.x);
    float slope2 = (b1.y - b2.y)/(b1.x - b2.x);
    if (slope1 == slope2)
    {
        return (a1.y - slope1*a1.x) == (a2.y - slope1*a2.x); //if the two slopes are the same, they still intersect if they have the same y-intercept, since they'd be the same line
    }
    return true; //if slopes are different, they definitely intersect somewhere
}

glm::vec3 lineLineIntersect(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2)
{
    glm::vec2 intersect = {0,0};
    bool intersected = lineInLine(a1,a2,b1,b2);
    if (intersected)
    {
        glm::vec3 bezier = bezierNums(a1,a2,b1,b2);
        float t = bezier.y/bezier.x;
        if (bezier.x == 0 && bezier.y == 0) //coincident lines
        {
            auto invSort = [](const glm::vec2& p1, const glm::vec2& p2) //literally just returns the opposite of sortFunc
            {
                return sortFunc(p1,p2) == p1 ? p2 : p1;
            };
            intersect = invSort(sortFunc(a1,a2),sortFunc(b1,b2)); //this should get the 2nd most sort point, which should also be the first point of intersection
        }
        else if (t >= 0 && t <= 1)
        {
            intersect = {t*(a2.x - a1.x) + a1.x,t*(a2.y - a2.y)};
        }
    }
    return glm::vec3(intersect,intersected);
}

glm::vec2 lineLineIntersectExtend(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2) //returns the point at which two lines intersect. Returns {0,0} if there is no intersection. Returns a1 if the two lines are the same. The intersection is based on the line segments not the hypothetical infinite lines
{
    if (a1.x == a2.x && b1.x == b2.x)
    {
        if (a1.x != b1.x)
        {
            return {0,0};
        }
        else
        {
            return a1;
        }
    }
    else
    {
        if (a1.x == a2.x)
        {
            double slope = (b1.y - b2.y)/(b1.x - b2.x);
            return {a1.x,b1.y + slope*(a1.x - b1.x)};
        }
        else if (b1.x == b2.x)
        {
            double slope = (a1.y - a2.y)/(a1.x - a2.x);
            return {b1.x,a1.y + slope*(b1.x - a1.x)};
        }
        else
        {
            double slope1 = (a1.y - a2.y)/(a1.x - a2.x);
            double slope2 = (b1.y - b2.y)/(b1.x - b2.x);

            if (slope1 == slope2)
            {
                return {0,0};
            }

            double yInt1 = a1.y - slope1*a1.x;
            double yInt2 = b1.y - slope2*b1.x;

            double x= (yInt1 - yInt2)/(slope2 - slope1);
            return {x, slope2*x + yInt2};
        }
    }
}

bool lineInVec(const glm::vec2& point1,const glm::vec2& point2, const glm::vec4& r1, double angle)
                                        //given points p1 and p2, with p1 having the lesser x value, this draws a line between the 2 points and checks to
{                                        //see if that line intersects with any of the sides of r1.
    glm::vec2 center = {r1.x + r1.z/2, r1.y + r1.a/2};

    glm::vec2 p1 = rotatePoint(point1,center,-angle); //slightly more efficient to rotate the line than all 4 points of the rectangle
    glm::vec2 p2 = rotatePoint(point2,center,-angle);

    glm::vec2 topLeft = {r1.x, r1.y};
    glm::vec2 topRight = {r1.x + r1.z, r1.y};
    glm::vec2 botLeft = {r1.x, r1.y + r1.a};
    glm::vec2 botRight = {r1.x + r1.z, r1.y + r1.a};

    return lineInLine(topLeft,topRight,p1,p2) ||
            lineInLine(topLeft,botLeft,p1,p2) ||
            lineInLine(topRight,botRight,p1,p2) ||
            lineInLine(botLeft,botRight,p1,p2) ||
            pointInVec(r1,p1,0);
}

bool pointInTriangle (const glm::vec2 a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& p)
{
 int denominator = ((b.y - c.y)*(a.x - c.x) + (c.x - b.x)*(a.y - c.y));
 double num1 = ((b.y - c.y)*(p.x - c.x) + (c.x - b.x)*(p.y - c.y)) / denominator;
 double num2 = ((c.y - a.y)*(p.x - c.x) + (a.x - c.x)*(p.y - c.y)) / denominator;
 double num3 = 1 - num1 - num2;

 return (0 <= num1 && num1 <= 1 && 0 <= num2 && num2 <= 1 && 0 <= num3 && num3 <= 1) ||
        (inLine(p,a,b) || inLine(p,b,c) || inLine(p,a,c));
}

glm::vec4 absoluteValueRect(const glm::vec4& rect) //a better name would be absoluteValueRect or absRect
{
    glm::vec4 fixed = rect;
    if (rect.z < 0 && rect.x > rect.x + rect.z)
    {
        fixed.x = rect.x + rect.z;
        fixed.z = abs(rect.z);
    }
    if (rect.a < 0 && rect.y > rect.y + rect.a)
    {
        fixed.y = rect.y + rect.a;
        fixed.a = -1*rect.a;
    }
    return fixed;
}

glm::vec2 pairtoVec(const std::pair<double,double>& pear)
{
    return {pear.first, pear.second};
}

glm::vec2 rotatePoint(const glm::vec2& p, const glm::vec2& rotateAround, double angle)
{
    glm::vec2 point = {p.x - rotateAround.x,p.y-rotateAround.y};//distances between target and pivot point
    return {point.x*cos(angle)-point.y*sin(angle)+rotateAround.x, point.x*sin(angle) + point.y*cos(angle)+rotateAround.y};
}

glm::vec4 moveRect(const glm::vec4& rect, const glm::vec4& wall, const glm::vec2& move, float rotation, float wallRotation)
{
    glm::vec4 finalRect = rect + glm::vec4(move.x,move.y,0,0);
    glm::vec4 vertRect = {rect.x, rect.y + move.y, rect.z, rect.a};
    if (vecIntersect(vertRect, wall,rotation,wallRotation)) //result if rect only moved vertically.
    {
        if (rect.y + rect.a < wall.y)
        {
            finalRect.y = wall.y - rect.a - 1;
        }
        else
        {
            finalRect.y = wall.y + wall.a+ 1;
        }
    }
    glm::vec4 horizRect = {rect.x + move.x, rect.y, rect.z,rect.a};
    if (vecIntersect(horizRect,wall,rotation,wallRotation)) //result if rect only moved horizontally.
        {
            if (rect.x + rect.z < wall.x)
            {
                finalRect.x = wall.x - rect.z - 1;
            }
            else
            {
                finalRect.x = wall.x + wall.z + 1;
            }
        }
    return finalRect;
}
