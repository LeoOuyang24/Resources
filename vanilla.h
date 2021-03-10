#ifndef VANILLA_H_INCLUDED
#define VANILLA_H_INCLUDED

#include <iostream>
#include <tuple>
#include <math.h>
#include <sstream>
#include <vector>
#include <array>
#include <stdlib.h>
#include <memory>
#include <stack>
#include <unordered_map>

class Named
{
    std::string name = "";
public:
    Named(std::string nameTag);
    std::string getName() const;
};

class IDed
{
    int id = 0;
public:
    IDed(int ID);
    int getID();
};

class NumberManager
{
protected:

    std::vector<double> numbers; //numbers are ordered from smallest to greatest
public:
    NumberManager();
    virtual void addNumber(double number);
    virtual int findNumber(double code);
    virtual void removeNumber(double number);
    ~NumberManager();

};


class Point
{
public:
    double x,y;
    Point(double xCoord = 0, double yCoord = 0)
    {
        x = xCoord;
        y = yCoord;
    }
};

class Line
{

public:
    Point a,b ;
    double radians;
    Line(const Point& x,const Point&y)
    {
        a =x;
        b = y;
        radians = atan2((a.y - b.y),(a.x - b.x));
    }
};


void printPoint(const Point& p);
class Shape
{
    Point center = {0,0};
protected:
    virtual void setCenter(double x, double y)
    {
        center = {x,y};
    }
public:
    Shape()
    {

    }
    const Point& getCenter() const
    {
        return center;
    }
    virtual bool intersects(Shape& shape)
    {
        return false;
    }
    virtual bool lineIntersects(Line& line)
    {
      return false;
    }
    virtual bool pointIn(Point& p)
    {
        return false;
    }
};

bool LineAndLineIntersect(const Line& l1, const Line& l2);
class Rect;
class Polygon : public Shape
{
    protected:
    std::vector <Line> lines;
    double minX,minY,maxX,maxY;
public:
    Polygon(std::initializer_list<Point> list)
    {
       load(list);

    }
    Polygon()
    {

    }
    void load(std::initializer_list<Point> list)
    {
         int size = list.size();
        double xSum = 0, ySum = 0;
        Point first = (*list.begin());
        minX = first.x;
        maxX = first.x;
        minY = first.y;
        maxY = first.y;
        Point prevPoint;
        int count = 0;
        for (std::initializer_list<Point>::iterator i = list.begin(); i != list.end(); i ++)
        {
            Point current = (*i);
            xSum += current.x;
            ySum += current.y;
            if (current.x < minX)
            {
                minX = current.x;
            }
            else if (current.x > maxX)
            {
                maxX = current.x;
            }
            if (current.y < minY)
            {
                minY = current.y;
            }
            else if (current.y >  maxY)
            {
                maxY = current.y;
            }

            if (count > 0)
            {
                lines.push_back(Line(prevPoint,current));
            }
            count ++;
            prevPoint = (*i);

        }


        setCenter(xSum/size, ySum/size);
    }

    double getMinX() const
    {
        return minX;
    }
    double getMaxX() const
    {
        return maxX;
    }
    double getMinY() const
    {
        return minY;
    }
    double getMaxY() const
    {
        return maxY;
    }
    virtual  Rect getBoundingRect() const;
    virtual const Line* intersects(const Polygon& other) const
    {
        const std::vector<Line> otherVec = other.getLines();
        int otherVecSize = otherVec.size();
      //  std::cout << linesSize << " " << otherVecSize << std::endl;
        for (int i = 0; i < otherVecSize; i ++)
        {  const Line* value = lineIntersects(otherVec[i]);
            if (value)
            {
                return value;
            }
        }
        return nullptr;
    }
    virtual const Line* lineIntersects(const Line& line) const
    {
        int size = lines.size();
        for (int j = 0; j < size; j ++)
            {
                if (LineAndLineIntersect(lines[j],line))
                {
                    return &line;
                }
            }
            return nullptr;
    }
    virtual void increment(double xDistance, double yDistance)
    {
        int size = lines.size();
        for (int i = 0; i < size; i ++)
        {
            lines[i].a.x += xDistance;
            lines[i].a.y += yDistance;
            lines[i].b.x += xDistance;
            lines[i].b.y += yDistance;
        }
        minX += xDistance;
        maxX += xDistance;
        minY += yDistance;
        maxY += yDistance;
    }
    const std::vector<Line>& getLines() const
    {
        return lines;
    }
};


class Rect : public Polygon
{
public:
    double x,y,w,h;
    Rect(double xCoord = 0, double yCoord = 0, double width = 0, double height = 0)
    {
        x = xCoord;
        y = yCoord;
        w = width;
        h = height;
        //setCenter(x + w/2, y + h/2);
        load({Point(xCoord,yCoord), Point(xCoord + width,yCoord),  Point(xCoord + width, yCoord + height),Point(xCoord, yCoord + height),Point(xCoord, yCoord)});

    }
    Rect getBoundingRect()const
    {
       //std::cout << x << "" << y << std::endl;;
        return Rect(*this);
    }
    void increment(double xDistance, double yDistance)
    {
        x += xDistance;
        y += yDistance;
        Polygon::increment(xDistance, yDistance);
    }

    using Polygon::intersects;
    bool intersects(Rect& r2)
    {
        if (this->x <= r2.x + r2.w && this->x + this->w >= r2.x && this->y <= r2.y + r2.h && this->y + this->h >= r2.y)
        {
            return true;
        }
        return false;
    }
};


void printRect(const Rect& r);
bool PointInRect(const Point& point, const Rect& rect); //returns true if the point is in the rect or on the borders

bool LineAndLineIntersect(const Point& a, const Point& b, const Point& c, const Point& d);

bool LineAndLineIntersect(const Line& l1, const Line& l2);

bool LineAndRectIntersect(const Rect& r, double x1, double y1, double x2, double y2 );

bool RectIntersect(const Rect& r1, const Rect& r2);

double PointDistance(const Point& p1, const Point& p2);

int convertTo1(double number); // a method that converts a number to 1 or -1 depending on its sign. If entry is 0, return 0;

class GradientNumber //a number that approaches a goal number at a given speed
{
    double goal = 0;
    double current = 0;
    double speed = 0;
public:
    GradientNumber(double a = 0, double b = 0, double velocity= 1);
    void load (double a,double b, double velocity); //alternate constructor so Gradient Number doesn't have to be initialized immediately
    double update(); //updates current and returns the value
    void setValue(double d);
    void setGoal(double d);
    void setSpeed(double s);
    double getValue();
    bool atTarget();

};

bool angleRange(float rad1, float rad2, float range); //returns true if angle rad1 is within range of rad2 (inclusive)
double randomDecimal(int places); //generates a random decimal between 0 and 1, not including 1. places is the number of decimal places
int findIntLength(int x);
std::string convert(double input);

std::string convert(int input); //takes an int and returns the int in string form

double convert(std::string input);

int charCount(std::string s, char c); //returns how many times c shows up in s;

std::string* divideString(std::string input); //divides a string into parts and puts them all into an array
bool rectIntersectWithinRange(const Rect& r1, double dist,const Rect& r2); //checks if a rectange 2dist larger on all dimension than r1 would intersect with r2
bool rectIntersectWithinRange(float x1, float y1, float w1, float h1, double dist, float x2, float y2, float w2, float h2);

double absMin(double x, double y);
double absMax(double x, double y);

void fastPrint(std::string str);

template <typename T>
class MinHeap //creates a min heap where each node consists of data and an int that is used to compare nodes
{
    typedef bool(*Comparer)(T& obj1, T& obj2); //given 2 objects, returns if they are equal or not
    struct MinHeapNode
    {
        std::pair<T,int> data;
    };
    std::vector<MinHeapNode*> nodes;
    bool valid(int index); //returns true if index is a valid index
    int getParent(int index);
    int getLeftChild(int index);
    int getRightChild(int index);
    void swap(int i1, int i2); //swaps the nodes at indicies i1 i2
    void sift(int index); //puts the item at index at the right spot
public:
    MinHeap();
    ~MinHeap();
    void add(T item, int val);
    int find(T item, int val, Comparer comp = nullptr); //find the index of an item based on its value. O(n), but sometimes logarithmic. returns -1 on failure to find
    int find(T item); //finds the index in O(n) time. -1 on failure;
    void update(T item, int oldVal, int val, Comparer comp = nullptr); //update an item with oldVal. If it doesn't exist, add it
    T peak(); //returns the value of the top most node.
    void pop(); //removes the top most node
    void print(); //prints the nodes
    int size();
};

template <typename T>
bool MinHeap<T>::valid(int index)
{
    return index >= 0 && index < nodes.size();
}

template <typename T>
int MinHeap<T>::getParent(int index)
{
    return (index - 1)/2;
}

template <typename T>
int MinHeap<T>::getLeftChild(int index)
{
    return 2*index + 1;
}

template <typename T>
int MinHeap<T>::getRightChild(int index)
{
    return 2*index + 2;
}

template <typename T>
void MinHeap<T>::swap(int i1, int i2)
{
    MinHeapNode* temp = nodes[i2];
    nodes[i2] = nodes[i1];
    nodes[i1] = temp;
}

template <typename T>
void MinHeap<T>::sift(int index)
{
    int parent = getParent(index);
    while (valid(parent) && nodes[index]->data.second < nodes[parent]->data.second) //sift up. If we don't need to sift up, this while loop never runs
    {
        swap(index, parent);
        index = parent;
        parent = getParent(index);
    }

    int leftChild = getLeftChild(index);
    int rightChild = getRightChild(index);
    while (valid(leftChild)) //if there are children
    {
        int current = nodes[index]->data.second;
        int leftVal = nodes[leftChild]->data.second;
        if (valid(rightChild)) //if there is a right child
            {
                int rightVal = nodes[rightChild]->data.second;
                if (current > rightVal || current > leftVal) //if the current node is bigger than either node
                {
                    if (rightVal > leftVal) //if right > left, swap index with right
                    {
                        swap(index, leftChild);
                        index = leftChild;
                    }
                    else //otherwise, swap with left
                    {
                        swap(index, rightChild);
                        index = rightChild;
                    }
                }
                else //if both children are bigger than the current node, we are good!
                {
                    break;
                }
            }
            else //if there is no right child
            {
                if (current > leftVal)
                {
                    swap(index,leftChild);
                }
                break; //if there is no right child and only a left child, we are done, since there can't possibly be anymore nodes
            }
        leftChild = getLeftChild(index);
        rightChild = getRightChild(index);
    }
}

template <typename T>
MinHeap<T>::MinHeap()
{

}

template <typename T>
MinHeap<T>::~MinHeap()
{
    int size = nodes.size();
    if (size > 0)
    {
        for (int i = 0; i < size; ++i)
        {
            delete nodes[i];
        }
        nodes.clear();
    }
}

template <typename T>
void MinHeap<T>::add(T item, int val)
{
    nodes.push_back(new MinHeapNode({{item, val}}) );
    int index = nodes.size() - 1;
    int parent = getParent(index); //index of parent
    while (nodes[parent]->data.second > val && valid(index))
    {
        swap(index,parent);
        index = parent;
        parent = getParent(index); //index of parent
    }

}

template <typename T>
int MinHeap<T>::find(T item)
{
    int size = nodes.size();
    for (int i = 0; i < size; ++i)
    {
        if (nodes[i]->data.first == item)
        {
            return i;
        }
    }
    return -1;
}

template <typename T>
int MinHeap<T>::find(T item, int val, Comparer equals)
{
    int index = 0;
    std::stack<int> dfs; //stack of indicies we missed and have to search through later.
    while (valid(index) || dfs.size() != 0)
    {
        int current = nodes[index]->data.second;
        if (current != val || (!equals && nodes[index]->data.first != item) || (equals && !equals(nodes[index]->data.first,item)))
        {
            int leftChild = getLeftChild(index);
            if (current > val || !valid(leftChild))//can't search here. All children will be even larger or there are no more children
            {
                if (dfs.size() > 0)
                {
                    index = dfs.top(); //find the next queued
                    dfs.pop();
                }
                else
                {
                    break; //can't find the object
                }

            }
            else
            {
                int rightChild = getRightChild(index);
                index = leftChild; //search the left child
                if (valid(rightChild)) //if the right child exists, make sure to search it at some point.
                {
                    dfs.push(rightChild);
                }
            }
        }
        else //found it!
        {
            return index;
        }
    }
    return -1;
}

template <typename T>
void MinHeap<T>::update(T item, int oldVal, int val, Comparer equals) //finding an element is typically linear but we can slightly shorten the search time if we know the old value
{
    int index = find(item, oldVal,equals);
    if (index != -1)
    {
        nodes[index]->data.second = val;
        sift(index);
    }
    else
    {
        add(item,val);
    }
}

template <typename T>
T MinHeap<T>::peak() //returns the value of the top most node.
{
    if (nodes.size() == 0)
    {
        throw std::logic_error ("Can't peak at empty MinHeap");
    }
    return nodes[0]->data.first;
}

template <typename T>
void MinHeap<T>::pop() //removes the top most node
{
    MinHeapNode* ptr = nodes[0];
    delete ptr;
    swap(nodes.size() - 1, 0);
    nodes.pop_back(); //destroy the formerly highest node
    int size = nodes.size();
    sift(0);
}

template <typename T>
void MinHeap<T>::print()
{
    int size = nodes.size();
    std::ostringstream stream;
    stream << "[";
    for (int i = 0; i < size; ++i)
    {
        stream << convert(nodes[i]->data.second);
        if (i < size - 1)
        {
            stream << ",";
        }
    }
    stream <<"]\n";
    fastPrint(stream.str());
}

template <typename T>
int MinHeap<T>::size()
{
    return nodes.size();
}
#endif // VANILLA_H_INCLUDED
