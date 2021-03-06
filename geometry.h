#ifndef GEOMETRY_H_INCLUDED
#define GEOMETRY_H_INCLUDED

#include <vector>
#include <map>
#include "glew.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"


void printRect(const glm::vec4& rect);
int loadShaders(const GLchar* source, GLenum shaderType );
double pointDistance(const glm::vec2& v1, const glm::vec2& v2);
glm::vec2 findMidpoint(const glm::vec4& v1); //midpoint of a line
bool vecIntersect(const glm::vec4& vec1,const glm::vec4& vec2);
glm::vec4 vecIntersectRegion(const glm::vec4& vec1, const glm::vec4& vec2); //returns the region of two colliding rects. Any given dimension will be 0 if there is no intersection in that direction.
bool vecInside(const glm::vec4& vec1, const glm::vec4& vec2); //like vecIntersect but doesn't return true if the rect's only overlap is a line.
double vecDistance(const glm::vec4& vec1, const glm::vec4& vec2); //gets the distance between two rects. 0 if they are intersecting
bool vecContains(glm::vec4 smallerRect, glm::vec4 biggerRect); //returns true if biggerRect contains smallerRect
bool pointInVec(const glm::vec4& vec1, double x, double y, double angle = 0); //angle of vec1 is by default 0
bool pointInVec(const glm::vec4& vec1, const glm::vec2& point, double angle = 0);
double pointVecDistance(const glm::vec4& vec, float x, float y); //shortest distance from the point to the rectangle. 0 if the point is in the rect
glm::vec2 closestPointOnVec(const glm::vec4& vec, const glm::vec2& point); //returns the point on vec that is the closest distance to point. Returns point if point is in vec
double pointLineDistance(const glm::vec4& line, const glm::vec2& point); //rotates line until is is parallel to the x-axis, then computes distance from point to line
bool lineInLine(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2);
glm::vec2 lineLineIntersect(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2); //returns the point at which two lines intersect. Returns {0,0} if there is no intersection. Returns a1 if the two lines are the same. The intersection is based on the line segments not the hypothetical infinite lines
glm::vec2 lineLineIntersectExtend(const glm::vec2& a1, const glm::vec2& a2, const glm::vec2& b1, const glm::vec2& b2); //returns the point at which two lines intersect. Returns {0,0} if there is no intersection. Returns a1 if the two lines are the same. The intersection is based on the hypothetical infinite lines rather than the provided line segments
bool lineInVec(const glm::vec2& p1, const glm::vec2& p2, const  glm::vec4& r1, double angle = 0); //angle of r1 is by default 0
bool pointInTriangle (const glm::vec2 a, const glm::vec2& b, const glm::vec2& c, const glm::vec2& p);
glm::vec4 absoluteValueRect(const glm::vec4& rect); //given a rect with at least one negative dimension, converts it into a regular rect with positive dimensions. A rect with already positive dimensions will return itself
glm::vec2 pairtoVec(const std::pair<double,double>& pear); //converts a pair to a vec2
glm::vec2 rotatePoint(const glm::vec2& p, const glm::vec2& rotateAround, double angle); //angle in radians


#endif // GEOMETRY_H_INCLUDED
