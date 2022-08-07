#include "SDL.h"

#include "glGame.h"
#include "components.h"
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

bool Positional::collidesLine(const glm::vec4& line)
{
    return pointLineDistance(line,pos)==0;
}

glm::vec4 Positional::getBoundingRect() const
{
    return {pos.x,pos.y,0,0};
}

glm::vec2 Positional::getPos() const
{
    return pos;
}

glm::vec2 Positional::getCenter() const
{
    return pos;
}

RectPositional::RectPositional(const glm::vec4& box, float newTilt) : Positional({box.x,box.y}), rect(box)
{
    setTilt(newTilt);
}

glm::vec4 RectPositional::getBoundingRect() const
{
    if (tilt == 0)
    {
        return rect;
    }
    else
    {
        glm::vec2 center = {rect.x + rect.z/2, rect.y + rect.a/2};
        glm::vec2 topLeft =rotatePoint({rect.x,rect.y},center,tilt),
                botLeft = rotatePoint({rect.x,rect.y + rect.a},center,tilt),
                topRight = rotatePoint({rect.x + rect.z,rect.y},center,tilt),
                botRight = rotatePoint({rect.x + rect.z, rect.y + rect.a},center,tilt);
        glm::vec2 xy = glm::vec2(std::min(botRight.x,std::min(topRight.x,std::min(topLeft.x,botLeft.x))),
                                 std::min(botRight.y,std::min(topRight.y,std::min(topLeft.y,botLeft.y))));
        return glm::vec4(xy.x,xy.y,
                         std::max(botRight.x,std::max(topRight.x,std::max(topLeft.x,botLeft.x))) - xy.x, //yes I know that I can calculate the dimensions based on xy.x and xy.y without finding the largest x or y but do I care???
                         std::max(botRight.y,std::max(topRight.y,std::max(topLeft.y,botLeft.y))) - xy.y);
    }
}

glm::vec2 RectPositional::getPos() const
{
    return {rect.x,rect.y};
}

const glm::vec4& RectPositional::getRect() const
{
    return rect;
}

float RectPositional::getTilt() const
{
    return tilt;
}

void RectPositional::setTilt(float newTilt) //clamp to -M_PI - +M_PI, just like C++ trig functions
{
    tilt = fmod(newTilt,2*M_PI) + 2*M_PI*(newTilt < 0);
    if (tilt >M_PI)
    {
        tilt -= 2*M_PI;
    }
}

bool RectPositional::collidesLine(const glm::vec4& line)
{
    return lineInVec({line.x,line.y},{line.z,line.a},rect,tilt);
}

bool RectPositional::collides(const glm::vec4& box)
{
    return vecIntersect(box,rect,tilt,0);
}

glm::vec2 RectPositional::getCenter() const
{
    return {rect.x + rect.z/2, rect.y + rect.a/2};
}

void RectPosCamera::init(const std::shared_ptr<RectPositional>& followee_, int w, int h)
{
    RenderCamera::init(w,h);
    setFollowee(followee_);
}

void RectPosCamera::setFollowee(const std::shared_ptr<RectPositional>& followee_)
{
    followee = followee_;
}

void RectPosCamera::update()
{
    if (followee.lock().get())
    {
        recenter(followee.lock().get()->getCenter());
    }
}

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
            nodes[i] = nullptr;
        }
    }
   // std::cout << vec.size() << " Deleted!" << std::endl;
    vec.clear();
}

QuadTree::~QuadTree()
{
    clear();
}

void QuadTree::render(const glm::vec2& displacement)
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

void QuadTree::render(RenderCamera& camera)
{
      if (nodes[0])
    {
        for (int i = 0; i < 4; i ++)
        {
            nodes[i]->render(camera);
        }
    }
    else
    {
        PolyRender::requestRect(camera.toScreen(region),{0,0,0,1},false,0,1);
    }
}


void QuadTree::add(PosWrapper& obj)
{
    if (nodes[0])
    {
        QuadTree* found = find(*(obj.get()));
        if (found == this)
        {
            vec.emplace_back(&obj);
        }
        else
        {
            if (found)//if found is not equal to null, add it wherever it's supposed to be
            {
                found->add(obj);
            }
            else //otherwise, it's in the parent nor its children
            {
                throw std::logic_error("QuadTree::add: Couldn't add the obj anywhere!");
            }
        }
    }
    else
    {

        vec.emplace_back(&obj);
        //vec.emplace_back((new Positional({1,1})));
        if (vec.size() > maxCapacity)
        {

            split();
            for (int i = vec.size()-1; i >= 0; i --)
            {
                Positional* ptr = vec[i]->get();
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

std::shared_ptr<Positional>& QuadTree::add(Positional& pos)
{
    SharedWrapper* ptr=new SharedWrapper(&pos);
    add(*(ptr));
    return ptr->ptr;
}

void QuadTree::getNearestHelper(positionalVec& vec, Positional& obj)
{
    if (obj.collides(region))
    {
        int size = this->vec.size();
        for (int i = 0; i < size;i ++)
        {
            vec.push_back(this->vec[i]->get());
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

positionalVec QuadTree::getNearest(Positional& obj)
{
    positionalVec vec;
    getNearestHelper(vec,obj);
    return vec;
}

void QuadTree::getNearestHelper(positionalVec& vec, const glm::vec4& area)
{
    if (vecIntersect(area,region))
    {
        int size = this->vec.size();
        //std::cout << size << std::endl;
        for (int i = 0; i < size;i ++)
        {
            Positional* ptr = this->vec[i]->get();
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

positionalVec QuadTree::getNearest(const glm::vec4& area)
{
    positionalVec vec;
    getNearestHelper(vec,area);
    return vec;
}

void QuadTree::getNearestHelper(positionalVec& vec, const glm::vec2& center, double radius)
{
    if (pointInVec(region, center.x, center.y) || pointVecDistance(region,center.x,center.y) <= radius)
    {
        int size = this->vec.size();
        for (int i = 0; i < size;i ++)
        {
            Positional* ptr = this->vec[i]->get();
            if (ptr && ptr->distance(center) <= radius)
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

positionalVec QuadTree::getNearest(const glm::vec2& center, double radius)
{
    positionalVec vec;
    getNearestHelper(vec,center, radius);
    return vec;
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
    int answer = size();
    if (nodes[0])
    {
        for (int i = 0; i < 4; i ++)
        {
            answer += nodes[i]->count();
        }
    }
    return answer;
}

int QuadTree::size()
{
    return vec.size();
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
        if (vec[i]->get() == &positional)
        {
            return true;
        }
    }
    return false;
}

void QuadTree::move(QuadTree& t1, QuadTree& t2, Positional& obj)
{
    int size = t1.vec.size();
    for (int i = 0; i < size; i ++)
    {
        if (t1.vec[i]->get() == &obj)
        {
            t2.add((*t1.vec[i].release()));
            t1.vec.erase(t1.vec.begin() + i);
            return;
        }
    }
}

bool QuadTree::remove(Positional& obj, PositionalCompare func)
{
    if (!obj.collides(region)) //if obj isn't even in the region, don't bother
    {
        return false ;
    }
    int size = vec.size();
    for (int i = 0; i < size; i ++) //see if obj is in this quadtree
    {
        if ((func && func(obj,*vec[i]->get())) || vec[i]->get() == &obj)
        {
            vec.erase(vec.begin() + i);
            return true;
        }
    }
    if (nodes[0]) //if not in this quadtree, search children
    {
        for (int i = 0; i < 4; i ++)
        {
            if (nodes[i]->remove(obj, func)) //if one of the children found it, remove and we're done!
            {
                return true;
            }
        }
    }
    return false;
}

void QuadTree::map(void (*fun)(const Positional& pos))
{
    if (fun)
    {
        int size = vec.size();
        for (int i = 0; i < size; ++i)
        {
            fun(*(vec[i]->get()));
        }
        if (nodes[0])
        {
            for (int i = 0; i < 4; ++i)
            {
             //   printRect(nodes[i]->getRect());
                nodes[i]->map(fun);
               // printRect(nodes[i]->getRect());
            }
        }
    }
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
        std::cerr << "Positional out of bounds: " << positional.getPos().x << " " << positional.getPos().y << " " << region.x << " " << region.y << " " << region.z << " " << region.a << "\n";
      //  throw new std::invalid_argument("Cant find positional!");
    }
    return newTree;

}

BiTree::BiTreeScore BiTree::calculateScore(const BiTreeElement& element)
{
   return {element.node->vertDimen.x,element.rect.x,element.positional};//node.vertDimen.x + (element.rect.x - this->region.x)/(this->region.z);
}

BiTree::BiTree(const glm::vec4& rect) : region(rect), head({{0,rect.a}})
{

}

BiTree::BiTreeStorage::iterator BiTree::insert(const BiTreeElement& oldElement, BiTreeNode& node)
{
    auto element = oldElement;
    element.node = &node;

    BiTreeScore score = this->calculateScore(element); //calculating the score requires that we iterate all the way down to the node we want to insert into
    auto it = (this->elements.insert({score,element})).first;
    updateNode(it,node);
    return it;
}

void BiTree::insert(Positional& wrap)
{
    if (vecIntersect(wrap.getBoundingRect(),region))
    {
        processElement([this](const BiTreeElement& element) mutable {
                        insert(element,*element.node); //REFACTOR: slightly slow to insert only to remove an element in the event of a split
                        if (element.node->size > maxNodeSize && element.node->vertDimen.y >= 2 && element.node->bigSize < .5*maxNodeSize) //exceeding max capacity, time to split
                        {
                            //std::cout << node.bigSize << " " << node.size << " " << node.vertDimen.y<<"\n";
                            split(*element.node);
                        }
                       },wrap,wrap.getBoundingRect());
    }
}

BiTree::BiTreeStorage::iterator BiTree::remove(const BiTreeStorage::iterator& it)
{
    if (it == this->elements.end())
    {
        return it;
    }
    auto node = it->second.node;
    node->size--;
    bool isStart = node->start == it;
    auto removed = this->elements.erase(it);
    if (isStart)
    {
        node->start = node->size == 0 ? this->elements.end() : removed; //update starting iterator
    }
    return removed;
}

void BiTree::split(BiTreeNode& node)
{
    float split = node.vertDimen.x + node.vertDimen.y/2; //y coordinate of the split
    node.nodes[0] = new BiTreeNode({{node.vertDimen.x,node.vertDimen.y/2},0,node.start}); //new nodes are born! .
    node.nodes[1] = new BiTreeNode({{split,node.vertDimen.y/2},0,this->elements.end()});

    int count = 0;
    auto it= node.start;
    while (count < node.size)
    {
        if (it->second.rect.y < split && it->second.rect.y + it->second.rect.a > split) //if element belongs in both children, split it
        {

            float botHeight = it->second.rect.y + it->second.rect.a - split;
            insert({it->second.positional,glm::vec4(it->second.rect.x,split,it->second.rect.z,botHeight)},*node.nodes[1]); //insert the half that belongs in the new bottom child
            it->second.rect.a -= botHeight; //the top half has the same score as the old element, so we just modify its height
            updateNode(it,*node.nodes[0]); //and update the node it belongs in
            ++it;
        }
        else //otherwise, insert based on where the element is relative to the split
        {
            if (it->second.rect.y >= split) //if element belongs in the new bottom child
            {
              insert(it->second,*node.nodes[1]);
              it = remove(it);
            }
            else
            {
                updateNode(it,*node.nodes[0]);
                ++it; //if the element does not belong in the new bottom child, we don't have to remove it because its score remains the same
            }
        }
        count ++;
    }
}

unsigned int BiTree::size()
{
    return elements.size();
}

unsigned int BiTree::countNodes()
{
    unsigned int count = 0;
    processNode([&count,this](BiTreeNode& node){count ++;
    },head,true);
    return count;
}

glm::vec4 BiTree::getRegion()
{
    return region;
}

void BiTree::updateNode(BiTreeStorage::iterator& it, BiTreeNode& node)
{
    if (node.size == 0 || it->first < node.start->first)
    {
        node.start = it;
    }
    if (it->second.rect.a >= node.vertDimen.y/2)
    {
        node.bigSize++;
    }
    node.size++;
}

void BiTree::resetNode(BiTreeNode& node)
{
    node.bigSize = 0;
    node.size = 0;
    node.start = elements.end();
    node.vertDimen = {region.y,region.a};
    node.nodes[0] = nullptr;
    node.nodes[1] = nullptr;
}

BiTree::BiTreeStorage::iterator BiTree::remove(Positional& wrap)
{
    return processElement([this](const BiTreeElement& element){

                    BiTreeScore score = calculateScore(element); //once we've accurately calculated the score, we remove the element
                   auto found = this->elements.find(score);
                   return remove(found);
                   },wrap,wrap.getBoundingRect(),head);
    //return this->elements.end();
}

void BiTree::showNodes(RenderCamera* camera)
{
    processNode([camera,this](BiTreeNode& node){
                if (node.nodes[0])
                {
                    float mid = node.vertDimen.x + node.vertDimen.y/2;
                    PolyRender::requestLine(glm::vec4(region.x,mid, region.x + region.z,mid),glm::vec4(1,1,0,1),0,1,camera);
                }
                },head,false);
}

void BiTree::clear()
{
    elements.clear();
    processNode([this](BiTreeNode& node){
                if (&node != &head) //delete all nodes but the head
                    delete &node;
                },head,false); //can't be tail recursive because otherwise we'd delete parents before children
    resetNode(head);
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
            nodes[i] = nullptr;
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

void RawQuadTree::remove(Positional& obj, PositionalCompare func)
{
    if (!obj.collides(region)) //if obj isn't even in the region, don't bother
    {
        return ;
    }
    int size = vec.size();
    for (int i = 0; i < size; i ++) //see if obj is in this quadtree
    {
        if ((func && func(*vec[i],obj)) || vec[i] == &obj)
        {
            vec.erase(vec.begin() + i);
            return;
        }
    }
    if (nodes[0]) //if not in this quadtree, search children
    {
        for (int i = 0; i < 4; i ++)
        {
            nodes[i]->remove(obj,func);
        }
    }
    return;
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
        throw new std::invalid_argument("Can't find positional!");
    }
    return newTree;

}

void SpatialGrid::updateEntities(const glm::vec4& region)
{            //func = (Positional&, Positional&) => void;
        taskID ++;
        processAllNodes(region,[this](const glm::vec2& point, Node& node) mutable {
                        int i =0;
                        glm::vec4 oldPos = glm::vec4(0);
                        for ( auto it = node.end; i < node.size; i++)
                        {
                            RectComponent* r1 = static_cast<RectComponent*>(*it);
                            oldPos = r1->getRect();
                            glm::vec4 oldBounding = r1->getBoundingRect();
                            if (it != positionals.begin()) //if not the literal first element (in which case there are no other positionals to process);
                            {
                                int j = i + 1;
                                for (auto it2 = std::prev(it); j < node.size; j++, --it2) //process all elements after "it" so each pair is only processed once
                                {
                                    RectComponent* r2 = static_cast<RectComponent*>(*it2);
                                    if (vecIntersect(oldPos,r2->getRect(),r1->getTilt(),r2->getTilt()))
                                    {
                                        r1->getEntity().collide(r2->getEntity());
                                        r2->getEntity().collide(r1->getEntity());
                                    }
                                }
                            }
                            r1->getEntity().updateOnce(taskID);
                            auto nextIt = std::prev(it); //have to do this because update function might invalidate "it";
                            update(*r1,oldBounding);
                            it = nextIt;
                        }
                        });
}
