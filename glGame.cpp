#include "glGame.h"
#include "render.h"

bool rectPathIntersect(const glm::vec4& oldRect, const glm::vec4& newRect, const glm::vec4& collide)
{
    return  vecIntersect(newRect,collide) ||
            lineInVec({oldRect.x + oldRect.z,oldRect.y},{newRect.x + newRect.z, newRect.y},collide) ||
            lineInVec({newRect.x + newRect.z,newRect.y},{oldRect.x, oldRect.y + oldRect.a},collide)||
            lineInVec({oldRect.x,oldRect.y},{newRect.x, newRect.y}, collide) ||
            lineInVec({oldRect.x + oldRect.z, oldRect.y + oldRect.a},{newRect.x + newRect.z, newRect.y + newRect.a},collide);
}

Positional::Positional(const glm::vec2& point) : pos(point)
{

}

bool Positional::collides(const glm::vec4& box)
{
    return pointInVec(box,pos.x,pos.y,0);
}

glm::vec2 Positional::getPos() const
{
    return pos;
}

RectPositional::RectPositional(const glm::vec4& box) : Positional({box.x,box.y}), rect(box)
{

}

glm::vec2 RectPositional::getPos() const
{
    return {rect.x,rect.y};
}

const glm::vec4& RectPositional::getRect() const
{
    return rect;
}

bool RectPositional::collides(const glm::vec4& box)
{
    return vecIntersect(box,rect);
}

glm::vec2 RectPositional::getCenter() const
{
    return {rect.x + rect.z/2, rect.y + rect.a/2};
}

const int QuadTree::maxCapacity = 100;

QuadTree::QuadTree(const glm::vec4& rect)
{
    region = rect;
    for ( int i = 0; i < 4; i ++)
    {
        nodes[i] = nullptr;
    }
}

void QuadTree::clear()
{
    if (nodes[0])
    {
        for (int i = 0; i < 4; i ++)
        {
            delete nodes[i];
        }
    }
    vec.clear();
    std::cout << "Deleted!" << std::endl;
}

QuadTree::~QuadTree()
{
    clear();
}

void QuadTree::render(const glm::vec2& displacement)
{

    if (nodes[0])
    {
        drawLine(RenderProgram::lineProgram,{0,0,0},{{region.x-displacement.x,region.y + region.a/2-displacement.y,region.x + region.z-displacement.x, region.y + region.a/2-displacement.y},
             {region.x + region.z/2-displacement.x, region.y-displacement.y, region.x+region.z/2-displacement.x, region.y + region.a-displacement.y}});
        for (int i = 0; i < 4; i ++)
        {
            nodes[i]->render(displacement);
        }
    }
}

void QuadTree::add(std::shared_ptr<Positional>&& ptr)
{
    add(ptr);
}

void QuadTree::add(Positional& ptr)
{
    add(std::shared_ptr<Positional>(&ptr));
}

void QuadTree::add(std::shared_ptr<Positional>& obj)
{
    Positional* ptr = obj.get();
    if (nodes[0])
    {
        QuadTree* found = find(*ptr);
        if (found == this)
        {
            vec.push_back(obj);
        }
        else
        {
            if (found)//if found is not equal to null, add it wherever it's supposed to be
            {
                found->add(obj);
            }
        }
    }
    else
    {
        vec.push_back(obj);
        if (vec.size() > maxCapacity)
        {
            split();
            for (int i = vec.size()-1; i >= 0; i --)
            {
                Positional* ptr = vec[i].get();
                QuadTree* found = find(*ptr);
                //std::cout << found->region.x << " " << found->region.y  << std::endl;
                if (found && found != this)
                {
                    move(*this,*found,*ptr);
                }
            }
        }
    }
}

void QuadTree::getNearest(std::vector<std::shared_ptr<Positional>>& vec, Positional& obj)
{
    if (obj.collides(region))
    {
        int size = this->vec.size();
        for (int i = 0; i < size;i ++)
        {
            vec.push_back(vec[i]);
        }
        if (nodes[0])
        {
            for (int i = 0; i < 4; i ++)
            {
                nodes[i]->getNearest(vec, obj);
            }
        }
    }
}

void QuadTree::getNearest(std::vector<std::shared_ptr<Positional>>& vec, const glm::vec4& area)
{
    if (vecIntersect(area,region))
    {
        int size = this->vec.size();
        for (int i = 0; i < size;i ++)
        {
            Positional* ptr = this->vec[i].get();
            if (ptr->collides(area))
            {
                vec.push_back(vec[i]);
            }
        }
        if (nodes[0])
        {
            for (int i = 0; i < 4; i ++)
            {
                nodes[i]->getNearest(vec, area);
            }
        }
    }
}

void QuadTree::split()
{
    nodes[0] = new QuadTree({region.x,region.y,region.z/2,region.a/2});
    nodes[1] = new QuadTree({region.x + region.z/2, region.y, region.z/2, region.a/2});
    nodes[2] = new QuadTree({region.x, region.y + region.a/2, region.z/2, region.a/2});
    nodes[3] = new QuadTree({region.x + region.z/2, region.y + region.a/2, region.z/2, region.a/2});
}

int QuadTree::count()
{
    int answer = vec.size();
    if (nodes[0])
    {
        for (int i = 0; i < 4; i ++)
        {
            answer += nodes[i]->count();
        }
    }
    return answer;
}

QuadTree* QuadTree::find(Positional& obj)
{
    QuadTree* answer = nullptr;
    if (obj.collides(region))
    {
        if (nodes[0])
        {
            for (int i = 0; i < 4; i ++)
            {
                if (obj.collides(nodes[i]->region))
                {
                    if (answer) //obj collides with multiple children. Return this;
                    {
                        answer = this;
                        break;
                    }
                    else
                    {
                        answer = nodes[i];
                    }
                }
            }
            if (answer != this)
            {
                answer = answer->find(obj);
            }
        }
        else
        {
            answer = this;
        }
    }
    return answer;
}

bool QuadTree::contains(Positional& positional)
{
    int size = vec.size();
    for (int i = 0; i < size; i ++)
    {
        if (vec[i].get() == &positional)
        {
            return true;
        }
    }
    return false;
}

void QuadTree::move(QuadTree& t1, QuadTree& t2, Positional& obj)
{
    int size = t1.vec.size();
  //  int size2 = t2.vec.size();
    for (int i = 0; i < size; i ++)
    {
       // glm::vec2 x= t1.vec[i].get()->getPos();
        if (t1.vec[i].get() == &obj)
        {
            t2.add(t1.vec[i]);
            t1.vec.erase(t1.vec.begin() + i);
            return;
        }
    }
}

std::shared_ptr<Positional>* QuadTree::remove(Positional& obj)
{
    if (!obj.collides(region)) //if obj isn't even in the region, don't bother
    {
        return nullptr;
    }
    int size = vec.size();
    for (int i = 0; i < size; i ++) //see if obj is in this quadtree
    {
        if (vec[i].get() == &obj)
        {
            std::shared_ptr<Positional> ptr = vec[i];
            vec.erase(vec.begin() + i);
            return &ptr;
        }
    }
    if (nodes[0]) //if not in this quadtree, search children
    {
        for (int i = 0; i < 4; i ++)
        {
            std::shared_ptr<Positional>* ptr = nodes[i]->remove(obj);
            if (ptr)
            {
                return ptr;
            }
        }
    }
    return nullptr;
}

QuadTree* QuadTree::update(Positional& positional, QuadTree& expected)
{
    QuadTree* newTree = find(positional);
    if (newTree != nullptr)
    {
        if (&expected != newTree)
        {
            move(expected,*newTree,positional);
        }
    }
    else
    {
        std::cout << positional.getPos().x << " " << positional.getPos().y << " " << region.z << " " << region.a << std::endl;
        throw new std::invalid_argument("failed to update positional because positional wasn't found");
    }
    return newTree;

}

RawQuadTree::RawQuadTree(const glm::vec4& rect)
{
    region = rect;
    for ( int i = 0; i < 4; i ++)
    {
        nodes[i] = nullptr;
    }
}

void RawQuadTree::clear()
{
    if (nodes[0])
    {
        for (int i = 0; i < 4; i ++)
        {
            delete nodes[i];
        }
    }
   // std::cout << vec.size() << " Deleted!" << std::endl;
    vec.clear();
}

RawQuadTree::~RawQuadTree()
{
    clear();
}

void RawQuadTree::render(const glm::vec2& displacement)
{
    //drawRectangle(RenderProgram::lineProgram,{0,0,0},region,0);
    if (nodes[0])
    {
        PolyRender::requestLine({region.x-displacement.x,region.y + region.a/2-displacement.y,region.x + region.z-displacement.x, region.y + region.a/2-displacement.y},{0,0,0,1});
        PolyRender::requestLine({region.x + region.z/2-displacement.x, region.y-displacement.y, region.x+region.z/2-displacement.x, region.y + region.a-displacement.y},{0,0,0,1} );
        for (int i = 0; i < 4; i ++)
        {

            nodes[i]->render(displacement);
        }
    }
}

const int RawQuadTree::maxCapacity = 100;

void RawQuadTree::add(Positional& obj)
{
    if (nodes[0])
    {
        RawQuadTree* found = find(obj);
        if (found == this)
        {
            vec.push_back(&obj);
        }
        else
        {
            if (found)//if found is not equal to null, add it wherever it's supposed to be
            {
                found->add(obj);
            }
            else
            {
                throw std::logic_error("Couldn't add the obj anywhere!");
            }
        }
    }
    else
    {
        vec.push_back(&obj);
        if (vec.size() > maxCapacity)
        {
            split();
            for (int i = vec.size()-1; i >= 0; i --)
            {
                Positional* ptr = vec[i];
                RawQuadTree* found = find(*ptr);
                //std::cout << found->region.x << " " << found->region.y  << std::endl;
                if (found && found != this)
                {
                    move(*this,*found,*ptr);
                }
            }
        }
    }
}

void RawQuadTree::getNearestHelper(std::vector<Positional*>& vec, Positional& obj)
{
    if (obj.collides(region))
    {
        int size = this->vec.size();
        for (int i = 0; i < size;i ++)
        {
            vec.push_back(this->vec[i]);
        }
        if (nodes[0])
        {
            for (int i = 0; i < 4; i ++)
            {
                nodes[i]->getNearestHelper(vec, obj);
            }
        }
    }
}

positionalVec RawQuadTree::getNearest(Positional& obj)
{
    positionalVec vec;
    getNearestHelper(vec,obj);
    return vec;
}

void RawQuadTree::getNearestHelper(positionalVec& vec, const glm::vec4& area)
{
    if (vecIntersect(area,region))
    {
        int size = this->vec.size();
        //std::cout << size << std::endl;
        for (int i = 0; i < size;i ++)
        {
            Positional* ptr = this->vec[i];
            if (ptr->collides(area))
            {
                vec.push_back(ptr);
            }
        }
        if (nodes[0])
        {
            for (int i = 0; i < 4; i ++)
            {
                nodes[i]->getNearestHelper(vec, area);
            }
        }
    }
}

positionalVec RawQuadTree::getNearest(const glm::vec4& area)
{
    positionalVec vec;
    getNearestHelper(vec,area);
    return vec;
}

void RawQuadTree::getNearestHelper(std::vector<Positional*>& vec, const glm::vec2& center, double radius)
{
    if (pointInVec(region, center.x, center.y) || pointVecDistance(region,center.x,center.y) <= radius)
    {
        int size = this->vec.size();
        for (int i = 0; i < size;i ++)
        {
            Positional* ptr = this->vec[i];
            if (ptr->distance(center) <= radius)
            {
                vec.push_back(ptr);
            }
        }
        if (nodes[0])
        {
            for (int i = 0; i < 4; i ++)
            {
                nodes[i]->getNearestHelper(vec, center,radius);
            }
        }
    }
}

std::vector<Positional*> RawQuadTree::getNearest(const glm::vec2& center, double radius)
{
    std::vector<Positional*> vec;
    getNearestHelper(vec,center, radius);
    return vec;
}

void RawQuadTree::split()
{
    nodes[0] = new RawQuadTree({region.x,region.y,region.z/2,region.a/2});
    nodes[1] = new RawQuadTree({region.x + region.z/2, region.y, region.z/2, region.a/2});
    nodes[2] = new RawQuadTree({region.x, region.y + region.a/2, region.z/2, region.a/2});
    nodes[3] = new RawQuadTree({region.x + region.z/2, region.y + region.a/2, region.z/2, region.a/2});
}

int RawQuadTree::count()
{
    int answer = vec.size();
    if (nodes[0])
    {
        for (int i = 0; i < 4; i ++)
        {
            answer += nodes[i]->count();
        }
    }
    return answer;
}

int RawQuadTree::size()
{
    return vec.size();
}

RawQuadTree* RawQuadTree::find(Positional& obj)
{
    RawQuadTree* answer = nullptr;
    if (obj.collides(region))
    {
        if (nodes[0])
        {
            for (int i = 0; i < 4; i ++)
            {
                if (obj.collides(nodes[i]->region))
                {
                    if (answer) //obj collides with multiple children. Return this;
                    {
                        answer = this;
                        break;
                    }
                    else
                    {
                        answer = nodes[i];
                    }
                }
            }
            if (answer != this)
            {
                answer = answer->find(obj);
            }
        }
        else
        {
            answer = this;
        }
    }
    if (answer == nullptr)
    {

    }
    return answer;
}

bool RawQuadTree::contains(Positional& positional)
{
    int size = vec.size();
    for (int i = 0; i < size; i ++)
    {
        if (vec[i] == &positional)
        {
            return true;
        }
    }
    return false;
}

void RawQuadTree::move(RawQuadTree& t1, RawQuadTree& t2, Positional& obj)
{
    int size = t1.vec.size();
    for (int i = 0; i < size; i ++)
    {
        if (t1.vec[i] == &obj)
        {
            t2.add(*(t1.vec[i]));
            t1.vec.erase(t1.vec.begin() + i);
            return;
        }
    }
}

Positional* RawQuadTree::remove(Positional& obj)
{
    if (!obj.collides(region)) //if obj isn't even in the region, don't bother
    {
        return nullptr;
    }
    int size = vec.size();
    for (int i = 0; i < size; i ++) //see if obj is in this quadtree
    {
        if (vec[i] == &obj)
        {
            Positional* ptr = vec[i];
            vec.erase(vec.begin() + i);
            return ptr;
        }
    }
    if (nodes[0]) //if not in this quadtree, search children
    {
        for (int i = 0; i < 4; i ++)
        {
            Positional* ptr = nodes[i]->remove(obj);
            if (ptr)
            {
                return ptr;
            }
        }
    }
    return nullptr;
}

RawQuadTree* RawQuadTree::update(Positional& positional, RawQuadTree& expected)
{
    RawQuadTree* newTree = find(positional);
    if (newTree != nullptr)
    {
        if (&expected != newTree)
        {
            move(expected,*newTree,positional);
        }
    }
    else
    {
        std::cerr << positional.getPos().x << " " << positional.getPos().y << " " << region.x << " " << region.y << " " << region.z << " " << region.a << "\n";
        throw new std::invalid_argument("");
    }
    return newTree;

}

