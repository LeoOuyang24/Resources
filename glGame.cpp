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


void SpatialGrid::updateEntities(const glm::vec4& region)
{            //func = (Positional&, Positional&) => void;
        taskID ++;

        processAllExistingNodes(region,[this](const glm::vec2& point, Node& node) mutable {
                        int i =0;
                        for ( auto it = node.end; i < node.size; i++)
                        {
                            PositionalComponent* r1 = (*it);
                            auto nextIt = std::prev(it); //have to do this because update function might invalidate "it";
                            if (Entity* entity = r1->getEntity())
                            if (entity->getTaskID() != taskID) //if we haven't processed this element yet
                            {
                                glm::vec4 oldBounding = r1->getBoundingRect();
                                if (it != positionals.begin()) //if not the literal first element (in which case there are no other positionals to process)
                                {
                                    int j = i + 1;
                                    for (auto it2 = nextIt; j < node.size; j++) //Find collisions with other entities, process all elements after "it" so each pair is only processed once
                                    {
                                        auto nextIt2 = std::prev(it2); //just in case collision invalidates it2
                                        PositionalComponent* r2 = (*it2);
                                        if (Entity* other = r2->getEntity())
                                        if (vecIntersect(oldBounding,r2->getBoundingRect(),r1->getTilt(),r2->getTilt()))
                                        {
                                            entity->collide(*other);
                                            other->collide(*entity);
                                        }
                                        it2 = nextIt2;
                                    }
                                }
                                entity->updateOnce(taskID); //slight redudancy here checking taskID twice
                                update(*r1,oldBounding); //update position
                            }
                            it = nextIt;
                        }
                        });
}
