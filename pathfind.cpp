#include <algorithm>
#include <unordered_set>

#include "vanilla.h"

#include "pathfind.h"

void renderPath(Path& path, RenderCamera* cam)
{
    if (path.size() >= 1)
    {
        auto end = path.end();
        glm::vec2* prev = &(path.begin()->point), *next;

        if (cam)
        {
            PolyRender::requestNGon(10,cam->toScreen(*prev),2,{1,0,0,1},0,true,1);
        }
        else
        {
            PolyRender::requestNGon(10,*prev,2,{1,0,0,1},0,true,1);
        }
        if( path.size() > 1)
        {
            for (auto it = path.begin() + 1; it != end; ++it)
           {
                next = &(*it).point;
               // GameWindow::requestNGon(10,*prev,2,{1,0,0,1},0,true,0,false);
                if (cam)
                {
                     PolyRender::requestNGon(10,cam->toScreen(*next),2,{1,0,0,1},0,true,1);
                    PolyRender::requestLine(glm::vec4(cam->toScreen(*prev),cam->toScreen(*next)),{1,1,1,1},1);
                }
                else
                {
                     PolyRender::requestNGon(10,*next,2,{1,0,0,1},0,true,1);
                    PolyRender::requestLine(glm::vec4(*prev,*next),{1,1,1,1},1);
                }
                prev = next;
                //std::cout << p1.x << " " << p1.y << " " << p2.x << " " << p2.y << std::endl;
           }
         //  glm::vec2 startP = path.front().point;
          // glm::vec2 endP = path.back().point;
           // PolyRender::requestLine({startP.x, startP.y, endP.x, endP.y},{.5,0,.5,1},GameWindow::interfaceZ);
        }
    }
}

bool compareRect(const glm::vec4* o1, const glm::vec4* o2)
{
    return o1->x < o2->x;
}

std::size_t HashPoint::operator()(const glm::vec2& p1) const
{
    int multiplier = pow(10,floor(log(abs(p1.y))));
    return multiplier*abs(p1.x) + abs(p1.y);
}

NavMesh::NavMeshNode::NavMeshNode(const glm::vec4& rect) : RectPositional(rect)
{

}

void NavMesh::NavMeshNode::addNode(NavMeshNode& n)
{
    const glm::vec4* otherArea = &(n.getRect());
    glm::vec4 intersect = vecIntersectRegion(*otherArea,rect);
    glm::vec4 line = {intersect.x,intersect.y, intersect.x + intersect.z, intersect.y + intersect.a};
    nextTo[&n] = line;
    if (n.getNextTo().count(this) == 0 || n.getNextTo()[this] != line) //if the other node hasn't added us or doesn't have the same intersection line, add it!
    {
        n.addNode(*this);
    }

}

bool NavMesh::NavMeshNode::isNextTo(NavMeshNode& n)
{
    return nextTo.count(&n) != 0;
}

void NavMesh::NavMeshNode::removeNode(NavMeshNode& n)
{
    auto found = nextTo.find(&n);
    if ( (found) != nextTo.end())
    {
        nextTo.erase(found);
    }
}

void NavMesh::NavMeshNode::setDimen(const glm::vec2& dimens)
{
    rect.z = dimens.x;
    rect.a = dimens.y;
}
void NavMesh::NavMeshNode::setRect(const glm::vec4& rect)
{
   this->rect = rect;
}

NavMesh::Neighbors& NavMesh::NavMeshNode::getNextTo()
{
    return nextTo;
}

void NavMesh::NavMeshNode::render() const
{
    if (RenderCamera::currentCamera)
    {
      // std::cout << "RECT: ";
      // printRect(RenderCamera::currentCamera->getRect());
        glm::vec4 overlap1 = vecIntersectRegion(RenderCamera::currentCamera->getRect(),rect);
        glm::vec2 center = {overlap1.x + overlap1.z/2, overlap1.y + overlap1.a/2};
        PolyRender::requestRect(RenderCamera::currentCamera->toScreen(overlap1),{1,0,0,.3},true,0,1);
        for (auto it = nextTo.begin(); it != nextTo.end(); ++it)
        {
            //printRect(it->first->getRect());
          //  printRect(it->second);
            glm::vec4 overlap2 = vecIntersectRegion(RenderCamera::currentCamera->getRect(),it->first->getRect());
            if (overlap2.z != 0 && overlap2.a != 0)
            {
                glm::vec2 otherCenter = {overlap2.x + overlap2.z/2, overlap2.y + overlap2.a/2};
               PolyRender::requestLine(glm::vec4(RenderCamera::currentCamera->toScreen({center.x,center.y})
                                                 ,glm::vec2(otherCenter.x,otherCenter.y)),
                                                 {1,0,1,1},.5);
                PolyRender::requestRect(RenderCamera::currentCamera->toScreen({it->second.x,it->second.y - 10*(center.y < otherCenter.y),it->second.z - it->second.x,10}),{1,1,1,.5},true,0,1.1);
                PolyRender::requestRect(RenderCamera::currentCamera->toScreen(overlap2),{0,1,0,.4},true,0,1);
              //  printRect(overlap2);
            }
           //renders a rectangle in our node bordering the neighboring node
        }
       // PolyRender::requestRect(RenderCamera::currentCamera->toScreen(rect),{1,0,0,.3},true,0,1);

       // printRect(rect);
    }
}

NavMesh::NavMeshNode::~NavMeshNode()
{
    for (auto j = nextTo.begin(); j != nextTo.end(); ++j)
    {
        if (j->first)
        {
            j->first->removeNode(*this);
        }
    }
}

NavMesh::NavMeshNode* NavMesh::getNode(const glm::vec2& point)
{
    Positional p(point);
    auto vec = nodeTree.getNearest(p);
    int size = vec.size();
    for (int i = 0; i < size; ++i)
    {
        if (vec[i]->distance(point) == 0)
        {
            return static_cast<NavMeshNode*>(vec[i]);
        }
    }
    return nullptr;
}

NavMesh::NavMeshNode* NavMesh::getNearestNode(const glm::vec2& point)
{
    Positional p(point);
    auto vec = nodeTree.getNearest(p);
    int size = vec.size();
    if (size == 0)
    {
        return nullptr;
    }
    double minDistance = vec[0]->distance(point);
    Positional* ptr = vec[0];
    for (int i = 1; i < size; ++i)
    {
        double distance = vec[i]->distance(point);
        if (distance == 0)
        {
            return static_cast<NavMeshNode*>(vec[i]);
        }
        else if (distance < minDistance)
        {
            minDistance = distance;
            ptr = vec[i];
        }
    }
    return static_cast<NavMeshNode*>(ptr);
}

void NavMesh::NavMesh::addNode(const glm::vec4& rect)
{
    NavMeshNode* node = new NavMeshNode(rect);
    addNode(*node);
}

void NavMesh::addNode(NavMeshNode& node)
{
    nodeTree.add(node);
}

void NavMesh::removeNode(NavMeshNode& node)
{
    nodeTree.remove(node);
}

void NavMesh::splitNode(NavMeshNode& node, const glm::vec4& wall)
{
    const glm::vec4* nodeRect = &(node.getRect());
    glm::vec4 region = vecIntersectRegion(*nodeRect,wall);
    glm::vec4 borders[4] = { //the four new nodes of the current node is being split into
        {nodeRect->x, region.y, region.x - nodeRect->x, region.a}, //left
        {nodeRect->x, nodeRect->y,nodeRect->z, region.y - nodeRect->y }, //top
        {region.x + region.z, region.y,nodeRect->x + nodeRect->z - region.x - region.z, region.a}, //right
        {nodeRect->x, region.y + region.a, nodeRect->z, nodeRect->y + nodeRect->a - region.y - region.a} //bottom
        };
    NavMeshNode* left = nullptr, *right = nullptr, *top = nullptr, *bottom = nullptr;
    for (int i = 0; i < 4; ++i)
    {
        if (borders[i].z > 0 && borders[i].a > 0)
        {
            NavMeshNode* newNode = new NavMeshNode(borders[i]);
            addNode(*newNode); //add each node if they have non-0 dimensions and add them to neighbors
            auto neigh = &(node.getNextTo());
            auto end = neigh->end();
            for (auto j = neigh->begin(); j != end; ++ j)
            {
                glm::vec4 intersect = vecIntersectRegion(j->first->getRect(), borders[i]);
                if (j->first != newNode && (vecIntersect(j->first->getRect(), borders[i]) && (intersect.z != 0 || intersect.a != 0))) //the second statement ensures that rectangles that share a common side will still be neighbors but neighbors touching at the corner won't
                {
                    j->first->addNode(*newNode);
                }
            }
            switch (i)
            {
            case 0:
                left = newNode;
                break;
            case 1:
                top = newNode;
                break;
            case 2:
                right = newNode;
                break;
            case 3:
                bottom = newNode;
                break;
            }

        }
    }
    if (left)
    {
        if (top)
        {
            left->addNode(*top);
        }
        if (bottom)
        {
            left->addNode(*bottom);
        }
    }
    if (right)
    {
        if (top)
        {
            right->addNode(*top);
        }
        if (bottom)
        {
            right->addNode(*bottom);
        }
    }
}

glm::vec2 NavMesh::displacePoint(const glm::vec2& point, const glm::vec4& line, const glm::vec4& nodeRect, double width)
{
    glm::vec2 nextPoint = point;
    if (nextPoint.x == line.x + width) //on right side of obstacle
    {
        if ((nextPoint.y == nodeRect.y && nextPoint.x == nodeRect.x + width) || (nextPoint.y == nodeRect.y + nodeRect.a && nextPoint.x > nodeRect.x + width) ) //top right of obstacle
        {
            nextPoint.y -= width;
        }
        else //bot right
        {
            nextPoint.y += width;
        }
    }
    else if (nextPoint.x == line.z - width) //on left side of obstacle
    {
         if ((nextPoint.y == nodeRect.y && nextPoint.x == nodeRect.x + nodeRect.z - width) || (nextPoint.y == nodeRect.y + nodeRect.a && nodeRect.x + nodeRect.z -width > nextPoint.x)) //top left
         {
             nextPoint.y -= width;
         }
         else //bot left
         {
             nextPoint.y += width;
         }
    }
    return {round(nextPoint.x),round(nextPoint.y)};
}

void NavMesh::addWall(const glm::vec4& rect)
{
      if (nodeTree.size() == 0)
    {
        addNode(bounds);
    }
        std::vector<Positional*> vec = nodeTree.getNearest(rect);
        for (int i = vec.size() - 1; i >= 0 ; i--) //find the first node that collides with rect. Once we've found it, we use the helper to finish the job. This is slightly more efficient since we only have to process nodes that are guaranteed to collide with the rect
        {
            NavMeshNode* ptr = static_cast<NavMeshNode*>(vec[i]);
            if (vecInside(ptr->getRect(),rect))
                {
                    splitNode(*ptr,rect);
                    if (vecContains(ptr->getRect(),rect)) //if the wall is completely contained in a node, we are done
                    {
                        i = 0; //we use i = 0 rather than break because we want to removeNode. We can't remove node earlier as then vecContains may be undefined
                    }

                    removeNode(*ptr);
                    //std::cout << "Done removing" << std::endl;
                }
        }
}

NavMesh::NavMesh(const glm::vec4& bounds_): bounds(bounds_), negativeTree(bounds), nodeTree(bounds)
{
    addNode(bounds);
}

/*void NavMesh::init(ObjectStorage& storage)
{
    typedef std::pair<glm::vec3,std::vector<NavMeshNode*>> scanLine;
    std::vector<const glm::vec4*> vec;
    for (auto it = storage.begin(); it != storage.end(); ++it)
    {
        negativeTree.add(*(new RectPositional(it->first->getRect().getRect())));
        vec.push_back(&(it->first->getRect().getRect()));
    }
    glm::vec4* edge = new glm::vec4({bounds.x + bounds.z, bounds.y, 1, bounds.a});
    vec.push_back(edge);
    std::sort(vec.begin(), vec.end(),compareRect);
    int size = vec.size();
    std::vector<scanLine> frontLine;
    frontLine.push_back({{bounds.x, bounds.y, bounds.a},{}});
   // std::cout << frontLine[0].second << std::endl;
    for (int i = 0; i < size; ++i)
    {
        const glm::vec4* rect = vec[i];
        scanLine top = {{0,0,0},{}}, bottom = {{0,0,0},{}}; //the lines that will result from the top and bottom
        unsigned int it;
        //std::cout << frontLine[0].first.y << " " << frontLine[frontLine.size()-1].first.y << std::endl;
        for ( it = 0; it < frontLine.size(); )
        {
            glm::vec3* line = &(frontLine[it].first);
            if (vecIntersect({line->x,line->y,rect->x - line->x,line->z},*rect )) //if the next object intersects with a front line
                {
                    NavMeshNode* node = nullptr;
                    if (rect->x - line->x > 0) //no reason to create a node if it has a width of 0
                    {
                        node = new NavMeshNode({line->x,line->y, rect->x - line->x, line->z}); //create the new node that is from the line up to the object
                        addNode(*node);
                        int size1 = frontLine[it].second.size();
                        for (int j = 0; j < size1; ++j) //add all of the neighbors we've encountered so far
                        {
                            node->addNode(*(frontLine[it].second[j]));
                        }
                        if (it < frontLine.size() - 1 && frontLine[it + 1].first.x <= rect->x) //same but now for the line above. We don't have to check if this line is actually the previous line because its impossible for the bottom line to already be erased
                        {
                            frontLine[it + 1].second.push_back(node);
                        }
                        if (it > 0 && frontLine[it - 1].first.x <= rect->x && frontLine[it - 1].first.y + frontLine[it - 1].first.z == line->y) //if the line directly below this one is left of the rect, then its resulting node must be a neighbor
                        {
                            frontLine[it - 1].second.push_back(node);
                        }
                    }


                    glm::vec3 pushRect = {rect->x , line->y,rect->y - line->y}; //top line
                    if (pushRect.z > 0)
                    {
                        top.first = pushRect;
                        if (node)
                        {
                            top.second.push_back(node);
                        }
                    }
                    pushRect = {rect->x, rect->y + rect->a, line->y + line->z - (rect->y + rect->a) }; //bottom line
                    frontLine.erase(frontLine.begin() + it); //we erase here because if we erase any earlier, we wouldn't be able to access line's variables. If we erase later, we may not erase due to the break statement.
                    if (pushRect.z > 0)//if this line creates a bottom line, then we are certainly done; the next line would be even lower and will certainly not intersect with our object
                    {
                        bottom.first = pushRect;
                        if(node)
                        {
                            bottom.second.push_back(node);
                        }
                        break;
                    }
                }
            else
                {
                    ++it;
                }
        }
        if (top.first.z > 0) //add the new top node
        {
            frontLine.insert(frontLine.begin() + it, top);
            it++;
        }
        frontLine.insert(frontLine.begin() + it,{{rect->x + rect->z ,rect->y, rect->a },{}}); //middle node
        it++;
        if (bottom.first.z > 0) //bottom node
        {
            frontLine.insert(frontLine.begin() + it, bottom);
        }
       /* for (int a = 0; a < frontLine.size(); a++)
        {
            std::cout << frontLine[a].first.y  << " " << frontLine[a].first.z << std::endl;
        }
        std::cout << "\n";
    }
    //removeNode(*(getNode({0,0})));

}

void NavMesh::init2(ObjectStorage& storage)
{
    addNode(bounds);
    auto end = storage.end();
    for (auto i = storage.begin(); i != end; ++i)
    {
        smartAddNode(i->first->getRect().getRect());
    }
}*/


void NavMesh::smartAddNode(const glm::vec4& rect)
{
        addWall(rect);
       negativeTree.add(*(new RectPositional(rect)));
       // std::cout << nodeTree.size() << std::endl;
}

std::shared_ptr<RectPositional> NavMesh::smartAddWall(RectPositional& rect)
{
    addWall(rect.getRect());
    return std::static_pointer_cast<RectPositional>(negativeTree.add(rect));
}

void NavMesh::reset()
{
    negativeTree.clear();
    nodeTree.clear();
}

glm::vec4 NavMesh::validMove(const glm::vec4& start, const glm::vec2& displacement)
{
    auto near = negativeTree.getNearest(start);
    auto end = near.end();
    glm::vec4 finalRect = {start.x + displacement.x, start.y + displacement.y, start.z, start.a};
    //this section makes sure we never move out of the navmesh
    if (displacement.y != 0)
    {
        finalRect.y = moveRect(start,{bounds.x,bounds.y + bounds.a*(convertTo1(displacement.y)),bounds.z,bounds.a},{0,displacement.y}).y;
    }
    if (displacement.x != 0)
    {
         finalRect.x = moveRect(start,{bounds.x + bounds.z*(convertTo1(displacement.x)),bounds.y,bounds.z,bounds.a},{displacement.x,0}).x;
    }
    glm::vec4 closestWall;
    if ((closestWall = getWallRect({start.x+displacement.x,start.y,start.z, start.a})) != glm::vec4(0))
    {
        if (start.x < closestWall.x)
        {
            finalRect.x = closestWall.x - start.z - 1;
        }
        else
        {
            finalRect.x = closestWall.x + closestWall.z + 1;
        }
    }
    if ((closestWall = getWallRect({start.x,displacement.y + start.y,start.z, start.a})) != glm::vec4(0))
    {
        if (start.y < closestWall.y)
        {
            finalRect.y = closestWall.y - start.a - 1;
        }
        else
        {
            finalRect.y = closestWall.y + closestWall.a+ 1;
        }
    }

    return finalRect;

}

glm::vec4 NavMesh::getWallRect(const glm::vec4& rect)
{
    auto near = negativeTree.getNearest(rect);
    int size = near.size();
    for (int i = 0; i < size; ++i)
    {
        if (near[i]->collides(rect))
        {

            return static_cast<RectPositional*>(near[i])->getRect();
        }
    }
    return glm::vec4(0);
}

bool NavMesh::notInWall(const glm::vec4& rect)
{
    return getWallRect(rect) == glm::vec4(0);
}

glm::vec4 NavMesh::getNearestNodeRect(const glm::vec2& point)
{
    NavMesh::NavMeshNode* node = getNearestNode(point);
    if (node)
    {
        return node->getRect();
    }
    else
    {
        return glm::vec4(0);
    }
}

glm::vec2 NavMesh::closestPoint(const glm::vec2& point)
{
    NavMesh::NavMeshNode* node = getNearestNode(point);
    if (node)
    {
        return closestPointOnVec(node->getRect(),point);
    }
    else
    {
        std::cerr << "NavMesh::closestPoin couldn't find a nearest node!\n";
        throw std::logic_error("NavMesh::closestPoin couldn't find a nearest node!");
    }
}

Path NavMesh::getPath(const glm::vec2& startPoint, const glm::vec2& endPoint, int width)
{
    NavMeshNode* startNode = getNearestNode(startPoint);  //ndoe we start off with. Repurposed later to be the node we are currently working on
    NavMeshNode* endNode = getNearestNode(endPoint);
    if (startNode && endNode)
    {
        glm::vec2 start = closestPointOnVec(startNode->getRect(),startPoint); //sometimes, our start/end isn't on a node. In that case, move to the closest point possible
        glm::vec2 end = closestPointOnVec(endNode->getRect(),endPoint);
        //GameWindow::requestRect(endNode->getRect(),{.5,.5,1,1},true,0,0,false);
        if (startNode == endNode)
        {
            return {{start,glm::vec2(0),glm::vec2(0)},{end,glm::vec2(0),glm::vec2(0)}}; //if both the start and end is in the same node then just move lol
        }
        std::unordered_set<NavMeshNode*> visited; //set of visited nodes
        std::unordered_map<glm::vec2,NavMeshNode*,HashPoint> pointAndNodes; //map of points to their nodes. If a point is on the border of two nodes, it's the node that the previous point is not a part of
        std::unordered_map<glm::vec2,std::pair<double,glm::vec2>,HashPoint> paths; //shortest distance from start to paths as well as the closest point that leads to it. Used for backtracking. A node can lie on many nodes so we also keep track the nodes we've visited
        typedef std::pair<glm::vec2,NavMeshNode*> HeapPair ; //an important type for pathfinding
        MinHeap<HeapPair> heap; //finds the next node to process. We have to also store what node the point is associated with since the points all lie on the border of two nodes.
        heap.add({start,startNode},0);
        pointAndNodes[end] = endNode;
        pointAndNodes[start] = startNode;
        //double bestDist = -1; //the distance of the best path found so far. -1 until one path is found
       // glm::vec2 bestPoint = {0,0}; //the last point of the best path found so far. origin until one path is found
        glm::vec2 curPoint; //current point to analyze
        NavMeshNode* curNode = startNode; //current node to analyze
        bool startEdge = false; //there's a fun edge case where if the start point is on the edge of two nodes, the algorithm will skirt around the neighboring node. This helps fix that (see documentation).
        while (heap.size() != 0 && heap.peak().first != end) //we end either when the heap is empty (no path) or when the top of the heap is the end (there is a path)
        {
            curPoint = heap.peak().first;
            //curPoint.x = floor(curPoint.x);
            curNode = heap.peak().second;
            const glm::vec4* curRect = &(curNode->getRect());
            //printRect(*curRect);
           // PolyRender::requestRect(RenderCamera::currentCamera->toScreen(curNode->getRect()),{0,0,1,1},true,0,.1);
            //PolyRender::requestNGon(10,RenderCamera::currentCamera->toScreen(curPoint),13,{0,1,0,1},0,true,2);
            float angle = atan2(curNode->getCenter().y - curPoint.y, curNode->getCenter().x - curPoint.x);
            //PolyRender::requestLine(glm::vec4(curPoint,curPoint + glm::vec2(40*cos(angle),40*sin(angle))),{0,0,0,1},1,RenderCamera::currentCamera);
            heap.pop();
            if (curNode == endNode ) //if we are in the endNode, we can go directly to the end. This may not be the shortest path, so we keep searching
            {
                double score = pointDistance(curPoint,end) + paths[curPoint].first;
                if (paths.count(end) == 0)
                {
                    heap.add({end,endNode},score);
                    paths[end].first = score;
                    paths[end].second = curPoint;
                }
                else if (score < std::get<0>(paths[end]))
                {
                    paths[end].first = score;
                    paths[end].second = curPoint;
                }
               // PolyRender::requestNGon(10,RenderCamera::currentCamera->toScreen(curPoint),10,{.5,.5,1,1},0,true,5);
                //std::cout << score <<"\n";
                continue;
            }
            visited.insert(curNode); //if our node is not the endnode add it to the visited nodes set
           // Font::tnr.requestWrite({convert(num),GameWindow::getCamera().toScreen({curPoint.x,curPoint.y,10,10}),0,{1,1,1,1},2});
            Neighbors* nextTo = &(curNode->getNextTo());
            auto endIt = nextTo->end(); //get the end iterator
            for (auto it = nextTo->begin(); it != endIt; ++it)
            {
                if (visited.count(it->first) > 0)
                {
                    continue;
                }
              /*  if (((curPoint.x >= it->second.x && curPoint.x <= it->second.z) && (curPoint.y == it->second.y)) //since all lines are horizontal or vertical, this tests to see if the our current point is on the intersection of our neighbor.
                     || it->second.z - it->second.x < width) //also don't process if the line is too narrow
                {
                    if (curPoint == start && !startEdge) //The case where curPoint == start deserves special attention only once. startEdge ensures we only do it once
                    {
                        double distance = pointDistance(curPoint,end) + paths[curPoint].first ;
                        heap.add({curPoint,it->first},distance);  //The closest point between our current point and a line that we are already on is obviously just curPoint.
                        startEdge = true;
                    }
                    PolyRender::requestNGon(10,RenderCamera::currentCamera->toScreen(curPoint),10,{1,0,0,1},0,true,2);
                    PolyRender::requestLine(it->second,{1,0,0,1},1,RenderCamera::currentCamera);
                    continue;
                }*/

                glm::vec2 a = {it->second.x + width, it->second.y},
                b = {it->second.z - width, it->second.a}; //the endpoints of the intersection line segment.
                glm::vec2 midpoint;  //this is not actually the midpoint, but rather the point on the intersection line we think will be closest to the goal
                midpoint = lineLineIntersectExtend(curPoint,end,a,b);//this ensures that if there is a direct path to the end, we work towards it.
               // PolyRender::requestLine(glm::vec4(GameWindow::getCamera().toScreen(midpoint),GameWindow::getCamera().toScreen(curPoint)),{1,.5,.5,1},10);
                //PolyRender::requestRect(RenderCamera::currentCamera->toScreen((it)->first->getRect()),{0,1,0,1},true,0,1);

                if (!lineInLine(midpoint,end,a,b)) //if the midpoint isn't on the intersection, choose one of the endpoints
                {
                    midpoint = pointDistance(end,a) + pointDistance(curPoint,a) < pointDistance(end,b) + pointDistance(curPoint,b) ? a : b;
                }
                else
                {
                    midpoint = pointDistance(end,midpoint) + pointDistance(curPoint,midpoint) < pointDistance(end,a) + pointDistance(curPoint,a) ? midpoint : a;
                    midpoint = pointDistance(end,midpoint) + pointDistance(curPoint,midpoint) < pointDistance(end,b) + pointDistance(curPoint,b) ? midpoint : b;
                }
                const glm::vec4* nodeRect = &(it->first->getRect());
                midpoint = displacePoint(midpoint,it->second,*nodeRect,width);

               // PolyRender::requestLine({RenderCamera::currentCamera->toScreen(a),RenderCamera::currentCamera->toScreen(b)},{1,0,1,1},1);
                //PolyRender::requestNGon(10,RenderCamera::currentCamera->toScreen(midpoint),10,{.5,1,0,1},0,true,2);
                double newDistance =std::get<0>(paths[curPoint]) +  pointDistance(curPoint,midpoint); //distance from start to midpoint
                bool newPoint = paths.count(midpoint) == 0;
                //if we found the new shortest distance from start to this point, update.
                if (newPoint ||paths[midpoint].first > newDistance) //add a never before visited point/node to the heap or update if we found a new short distance
                {

                    paths[midpoint].first = newDistance;
                    paths[midpoint].second = curPoint;
                    pointAndNodes[midpoint] = pointInVec(endNode->getRect(),midpoint) ? endNode : it->first;
                    if (newPoint)
                    {
                        //the final score that also uses the heuristic
                        heap.add({midpoint,pointAndNodes[midpoint]},newDistance + pointDistance(midpoint,end));
                     //   heap.print();
                    }

                }
            }
        }
        Path finalPath;
        curPoint = end;
        if (heap.size() != 0) //the end node is always guaranteed to be still in the heap if a path was found.
        {
           // std::cout << "Repeat\n";
           glm::vec2 shortCut = curPoint; //shortcut is the last point that we know curPoint can move to without hitting a wall that has yet to be added to finalPath
            glm::vec2 leftBound = {bounds.x,0}, rightBound = {bounds.x + bounds.z, 0};
            bool boundsSet= false;
           finalPath.push_back({curPoint,glm::vec2(0),glm::vec2(0)}); //curPoint is the last point that was added to our final Path
           float prevAngle = 0; //the angle of our triangle should never increase
            while (shortCut != start)
            {
                //std::cout << paths.count(curPoint) << " " << curPoint.x << " " << curPoint.y << std::endl;
                glm::vec2 nextPoint = paths[shortCut].second; //nextPoint is the nextPoint to process.
              //  PolyRender::requestCircle({1,1,1,1},RenderCamera::currentCamera->toScreen(nextPoint),10,2);
                    glm::vec4 line = (nextPoint != start ?
                                            pointAndNodes[nextPoint]->getNextTo()[pointAndNodes[paths[nextPoint].second]] : //intersection between the two nodes
                                            glm::vec4(startNode->getRect().x ,start.y, startNode->getRect().x + startNode->getRect().z, start.y));// - glm::vec4(width,0,width,0);
                     line.y = nextPoint.y;
                     line.a = nextPoint.y;
                        if (!boundsSet)
                        {
                            leftBound = {line.x,line.y};
                            rightBound ={line.z, line.a};
                            boundsSet = true;
                        }
                       if (leftBound.y == curPoint.y && leftBound.y != line.y)
                            {
                                finalPath.push_front({shortCut,{line.x,line.y},{line.z,line.a}});
                                curPoint = shortCut;
                                leftBound = {bounds.x,0};
                                rightBound = {bounds.x + bounds.z, 0};
                                boundsSet = false;
                            }
                            else
                            {
                                if (leftBound.y != curPoint.y)
                                {
                                    leftBound = {std::max(line.x, lineLineIntersectExtend(leftBound,curPoint,{line.x,line.y},{line.z,line.a}).x),line.y};
                                    rightBound = {std::min(line.z,lineLineIntersectExtend(rightBound,curPoint,{line.x,line.y},{line.z,line.a}).x),line.y};
                                }
                             //   std::cout << leftBound.x <<" " << leftBound.y << " " << rightBound.x << " " << rightBound.y << " " << curPoint.x << " " << curPoint.y << " " << nextPoint.x << " " << nextPoint.y<< "\n";
                               /* PolyRender::requestNGon(10,RenderCamera::currentCamera->toScreen(rightBound),10,{0,0,1,1},0,10,2);
                                PolyRender::requestNGon(10,RenderCamera::currentCamera->toScreen(leftBound),10,{1,0,0,1},0,10,2);
                                PolyRender::requestLine(line,{0,1,0,1},3,RenderCamera::currentCamera);*/
                                boundsSet = true;
                                //rightBound.x < leftBound.x || !pointInTriangle(leftBound,rightBound,curPoint,nextPoint)
                                if (true) //if we can't move to nextPoint, then shortCut is the furthest we can move.
                                {
                                    finalPath.push_front({shortCut,{line.x,line.y},{line.z,line.a}});
                                    curPoint = shortCut;
                                    leftBound = {bounds.x,0};
                                    rightBound = {bounds.x + bounds.z, 0};
                                    boundsSet = false;
                                    shortCut = nextPoint;

                                }
                                else
                                {
                                    shortCut = nextPoint;
                                }
                            }
                //PolyRender::renderPolygons();

            }

            finalPath.push_front({start,glm::vec2(0),glm::vec2(0)});
        }
        else
        {
            return {};
        }
        return finalPath;
    }
    else
    {
      //  throw std::logic_error("Can't get path with no starting or ending node!");
      return {};
    }
}

bool NavMesh::straightLine(const glm::vec4& line)
{
    auto vec = negativeTree.getNearest({line.x,line.y,line.z - line.x, line.a - line.y});
    int size = vec.size();
    glm::vec2 a = {line.x, line.y}, b = {line.z, line.a};
    for (int i = 0; i < size; ++i)
    {
        glm::vec4 wallRect = (static_cast<RectPositional*>(vec[i]))->getRect();
        bool intersect = lineInVec(a,b,wallRect,0);
        //std::cout << "Points: ";
       // printRect(line);
       // printRect(wallRect);
        if (intersect && !pointInVec(wallRect,a.x,a.y,0) && !pointInVec(wallRect,b.x,b.y,0))//sometimes, buildings may target or be targeted so we want to make sure we don't mind if the line starts or ends in a wall.
        {
            return false;
        }
    }
    //PolyRender::requestLine(glm::vec4(GameWindow::getCamera().toScreen(a),GameWindow::getCamera().toScreen(b)),{1,0,0,1},10);
    return true;
}

glm::vec4 NavMesh::getRandomArea(const glm::vec2& origin, double minDist, double maxDist)
{
    if (nodeTree.size() == 0)
    {
        throw std::logic_error ("Can't get random area of blank navigation mesh!");
    }
    NavMeshNode* node = nullptr;
    while (!node)
    {
        double radius = fmod(rand(),std::max(1.0,(maxDist - minDist))) + minDist;
        double theta = rand()%360*M_PI/180;
        glm::vec2 point = {origin.x + radius*cos(theta), origin.y + radius*sin(theta)};

        point.x = std::max(std::min(bounds.x + bounds.z, point.x), bounds.x);
        point.y = std::max(std::min(bounds.y + bounds.a, point.y), bounds.y); //clamp the points so that the point can't be out of bounds
       node  = getNearestNode(point);
    }
    return node->getRect();
}

void NavMesh::removeWall(RectPositional& positional)
{
    glm::vec4 rect = positional.getRect();

   negativeTree.remove(positional,[](const Positional& p1, const Positional& p2)
                                {
                                   glm::vec4 r1 =  static_cast<const RectPositional*>(&p1)->getRect();
                                   glm::vec4 r2 = static_cast<const RectPositional*>(&p2)->getRect();
                                    return  r1 == r2;
                                });
    auto vec = nodeTree.getNearest({rect.x - 1, rect.y - 1, rect.z + 2, rect.a + 2}); //find nearest nodes. We use a slightly larger rect in case the rect is teh exact size of a quadtree node
    std::sort(vec.begin(),vec.end(), [](Positional* p1, Positional* p2){
              return p1->getPos().y < p2->getPos().y;
              }); //sort positionals by lowest y coord to highest
    std::vector<RectPositional*> top, bottom; //represents nodes at the top and bottom of the wall
    int size = vec.size();
    bool skippedFirst = false; //skip the first node that is in line with the wall
    NavMeshNode* baseNode = nullptr, //baseNode is the unfinished node,
                *topNode = nullptr, //topNode is the topMostNode,
                *botNode = nullptr; // botNode is the botMostNode
    bool sideWall = rect.x == bounds.x || rect.x + rect.z == bounds.x + bounds.z; //true if the wall borders the left or right edge of the mesh
    for (int i = 0; i < size; ++i)
    {
        NavMeshNode* node = static_cast<NavMeshNode*>(vec[i]);
        glm::vec4 blankRect = node->getRect();
        if (vecIntersect(blankRect,rect))
        {
            bool inWall = blankRect.x >= rect.x && blankRect.x + blankRect.z <= rect.x + rect.z; //if the rect is entirely enclosed in the wall
            if (blankRect.y + blankRect.a == rect.y && inWall)
            {
                top.push_back(node);
            }
            else if (blankRect.y == rect.y + rect.a && inWall)
            {
                bottom.push_back(node);
            }
            else if (blankRect.y >= rect.y && blankRect.y < rect.y + rect.a)
            {
                if (sideWall)
                {
                    //life is really easy if sideWall is true because we don't have to worry about a node on the other side of the wall.
                    //Simply adjust the dimensions of our current node
                    node->setRect({
                                  std::min(rect.x,blankRect.x),
                                  blankRect.y,
                                  blankRect.z + rect.z,
                                  blankRect.a
                                  });
                }
                else //if not sideWall, then the next rectPositional should be on the right side of the wall due to how we organized our list
                {
                    if (!skippedFirst || !baseNode)
                    {
                        skippedFirst = true;
                        baseNode = node;
                    }
                    else
                    {
                        glm::vec4 baseRect = baseNode->getRect();
                       NavMeshNode* finalNode = baseRect.a < blankRect.a ? baseNode : node; //finalNode is the one that we are done with
                       glm::vec4 finalRect = {
                                          std::min(baseRect.x,blankRect.x),
                                          baseRect.y, //it shouldn't matter which y we use because it should be the same
                                          baseRect.z + blankRect.z + rect.z,
                                          finalNode->getRect().a
                                            };
                       finalNode->setRect(finalRect);
                        if (finalRect.y == rect.y)
                        {
                            topNode = finalNode;
                        }
                        else if (finalRect.y == rect.y + rect.a)
                        {
                            botNode = finalNode;
                        }
                        auto nextTo = baseNode->getNextTo();
                        auto nextSize = nextTo.end();
                        for (auto j = nextTo.begin(); j != nextSize; ++j) //add all neighbors to the finalNode
                        {
                            if (finalNode->collides((j->first->getRect()))) //if the resized node collides with the neighbor and doesn't already have the node as a neighbor
                            {
                                finalNode->addNode(*(j->first));
                            }
                        }
                        if (blankRect.a == baseRect.a) //we start over
                        {
                            nodeTree.remove(*baseNode);
                            baseNode = nullptr;
                        }
                        else //otherwise, set baseNode to the remaining, smaller node
                        {
                            baseNode = finalNode == baseNode ? node : baseNode; //set baseNode to the unfinished node
                            baseNode->setRect({
                              baseNode->getRect().x,
                              finalRect.y + finalRect.a,
                              baseNode->getRect().z,
                              baseRect.y + baseNode->getRect().a - finalNode->getRect().a
                              });
                        }
                    }
                }
            }
        }
        else
        {
            continue;
        }
    }
    if (baseNode) //this should only occur if sideWall is also true
    {
        glm::vec4 baseRect = baseNode->getRect();
        baseNode->setRect({
                          std::min(rect.x,baseRect.x),
                          baseRect.y,
                          baseRect.z + rect.z,
                          baseRect.a
                          });
        if (baseRect.y == rect.y)
        {
            topNode = baseNode;
        }
        botNode = baseNode;
    }
    int topSize = top.size();
    int botSize = bottom.size();
    for (int i = 0; i < topSize; ++i)
    {
        if (i < topSize)
        {
            topNode->addNode(*static_cast<NavMeshNode*>(top[i]));
        }
    }
    for (int i = 0; i < botSize; ++i)
    {
        if (i < botSize)
        {
            botNode->addNode(*static_cast<NavMeshNode*>(bottom[i]));
        }
    }
}

glm::vec4 NavMesh::getBounds()
{
    return bounds;
}

int NavMesh::getNumWalls()
{
    return negativeTree.count();
}

int NavMesh::getNumNodes()
{
    return nodeTree.count();
}

void NavMesh::render()
{
    if (RenderCamera::currentCamera)
    {
     nodeTree.map([](const Positional& pos){
                 PolyRender::requestRect(RenderCamera::currentCamera->toScreen(static_cast<const RectPositional*>(&pos)->getRect()),
                                         {0,0,0,1},
                                         false,
                                         0,1);
                const NavMesh::NavMeshNode* node = static_cast<const NavMeshNode*>(&pos);
                node->render();
                 });
    }
    else
    {
        std::cerr << "NavMesh::render: RenderCamera::currentCamera not set!\n";
    }

}

void NavMesh::renderNode(const glm::vec2& point)
{
    if (NavMesh::NavMeshNode* node = getNode(point))
    {
        node->render();
    }
}

NavMesh::~NavMesh()
{
    nodeTree.clear();
    negativeTree.clear();
}

void EntityTerrainManager::init(const glm::vec4& rect)
{
    EntityPosManager::init(rect);
    mesh.reset(new NavMesh(rect));
}


void EntityTerrainManager::addTerrain(Terrain& wall)
{
    terrain.push_back(std::static_pointer_cast<Terrain>(mesh->smartAddWall(wall)));
}

void EntityTerrainManager::addEntity(Entity& entity, float x, float y)
{
    glm::vec2 pos = {x,y}; //adjusted position to move entity to in case there are walls
    if (!mesh->notInWall({x,y,1,1})) //if {x,y} collides with a wall
    {
        if (RectComponent* rectComp = entity.getComponent<RectComponent>())
        {
            glm::vec4 rect = rectComp->getRect();
            glm::vec4 nodeRect = mesh->getNearestNodeRect({x,y}) + glm::vec4({rect.z/2,rect.a/2,-rect.z,-rect.a}); //adjust nodeRect so any point in it can hold our center
            pos = closestPointOnVec(nodeRect,{x,y});
        }
    }
    EntityPosManager::addEntity(entity,pos.x,pos.y );
}

std::shared_ptr<NavMesh>& EntityTerrainManager::getMeshPtr()
{
    return mesh;
}


void EntityTerrainManager::update()
{
    auto end = terrain.end();
    if (RenderCamera::currentCamera)
    {
        for (auto it =terrain.begin(); it != end; ++it)
        {
            PolyRender::requestRect(RenderCamera::currentCamera->toScreen((*it)->getRect()),{1,0,0,1},true,0,1);
        }
    }
    EntityPosManager::update();
}

PathFindComponent::PathFindComponent(bool setAngle, std::shared_ptr<NavMesh>& mesh_,float speed, const glm::vec4& rect, Entity& entity) : MoveComponent(speed, rect, entity),
                                                                                                                            ComponentContainer<PathFindComponent>(entity),
                                                                                                                            mesh(mesh_),
                                                                                                                            adjustTilt(setAngle)
{

}

void PathFindComponent::setTarget(const glm::vec2& point)
{
    if (!mesh.expired() && (path.size() == 0 || path.back().point != point))
    {
        NavMesh* meshPtr = mesh.lock().get();
        path = meshPtr->getPath(getCenter(),point,std::max(rect.z,rect.a));
    }
}

bool PathFindComponent::atTarget()
{
    if (path.size())
    {
      return atPoint(path.back().point);
    }
    else
    {
        return MoveComponent::atTarget();
    }
}

void PathFindComponent::update()
{
    if (path.size() > 0)
    {
        if (MoveComponent::atTarget())
        {
            MoveComponent::setTarget(path.front().point);
           // std::cout <<target.x << " " <<target.y <<" " << getCenter().x << " " <<getCenter().y << "\n";
            path.pop_front();
           // std::cout << path.size () << "\n";
        }
    }
    MoveComponent::update();
    if (adjustTilt)
    {
        setTiltTowardsTarget();
    }
}

