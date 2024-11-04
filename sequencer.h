#ifndef SEQUENCER_H_INCLUDED
#define SEQUENCER_H_INCLUDED

#include <memory>
#include <unordered_set>
#include <functional>

#include "vanilla.h"
#include "SDLhelper.h"
#include "render.h"

//this file helps chain events together(ex: create a hitbox when an animation ends)
//realistically, it'll probably be used with animations primarily but can be used to do anything that occurs over time
typedef std::function<bool(int)> SequenceUnit; //a single unit of a sequence. The runtime is the only variable passed in
typedef std::shared_ptr<SequenceUnit> UnitPtr;
class Sequencer
{
    //runs a sequence of Sequence Units
protected:
    typedef std::list<UnitPtr> Sequence;

    unsigned int startTime = 0;

    Sequence::iterator current; //the unit we are currently on
    Sequence sequence;

    virtual bool perUnitProcess() //what to run for the current sequenceUnit. Returns whether or not to continue running current
    {
        return (**current)(SDL_GetTicks() - startTime);
    }
    virtual void perUnitDone() //what to do if a sequenceUnit is done
    {
        std::advance(current,1);
        startTime = SDL_GetTicks();
    }

public:
    template<typename Callable1_, typename... CallableList_>
    Sequencer(Callable1_ a, CallableList_... b) : Sequencer()
    {
        addUnits(a,b...);
    }
    Sequencer()
    {
        current = sequence.end();
    }
    void reset()
    {
        startTime = SDL_GetTicks();
        current = sequence.begin();
    }

    void addUnit(SequenceUnit&& cs)
    {
        sequence.emplace_back(new SequenceUnit(std::move(cs)));
    }
    void addUnit(UnitPtr& cs) //if you don't want the Sequencer to own the unit, pass in a shared pointer
    {
        sequence.emplace_back(cs);
    }

    template<typename Callable1_, typename... CallableList_>
    void addUnits(Callable1_ a, CallableList_... c) //can be used to recursively pass in a list of either SequenceUnit or UnitPtrs
    {
        addUnit(std::forward<Callable1_>(a));
        if constexpr (sizeof...(c) > 0)
        {
            addUnits(std::forward<CallableList_...>(c...));
        }
    }
    bool done()
    {
        return current == sequence.end();
    }
    void run()
    {
        if (sequence.size() > 0)
        {
            if (current == sequence.end())
            {
                reset();
            }

            bool done = perUnitProcess();
            if (done) //we've spent enough time on current, move on
            {
                perUnitDone();
            }
        }
    }
    void clear()
    {
        sequence.clear();
    }
};

class SequenceManager //contains a list of sequences to run
{
    static std::unordered_set<std::shared_ptr<Sequencer>> sequences;
public:
    //make a request, which is deleted permanently after finishing
    static void request(Sequencer& sequence)
    {
        std::shared_ptr<Sequencer> ptr = std::shared_ptr<Sequencer>(&sequence);
        request(ptr);
    }
    //make a request, which is not deleted after finishing if you have another shared_ptr
    static void request(std::shared_ptr<Sequencer>& sequence)
    {
        if (sequences.find(sequence) == sequences.end()) //if sequence has not been added yet...
        {
            sequence->reset(); //...reset it
            sequences.insert(sequence); // ...and add it to our sequences to run
        }
    }
    static void run()
    {
        auto end = sequences.end();
        for (auto it = sequences.begin(); it != end;)
        {
            if ((*it).get())
            {
                (*it)->run();
                if ((*it)->done())
                {
                    it = sequences.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            else
            {
                it = sequences.erase(it);
            }
        }
    }

};



#endif // SEQUENCER_H_INCLUDED
