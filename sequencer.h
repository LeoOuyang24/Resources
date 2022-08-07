#ifndef SEQUENCER_H_INCLUDED
#define SEQUENCER_H_INCLUDED

#include <memory>
#include <unordered_set>

#include "vanilla.h"
#include "SDLhelper.h"

//this file helps chain events together(ex: create a hitbox when an animation ends)
//realistically, it'll probably be used with animations primarily but can be used to do anything that occurs over time

struct Callable //creates a base class for all sequence units to inherit from. Really just exists so we can have a list of templated SequenceUnits
{
    int time= 0; //how long to run func
    int repetitions = 0; //number of times to run
    bool inFrames = true; //true if time is expressed in frames, otherwise, milliseconds
    virtual bool operator()(int) = 0;
};

template<typename T>
struct SequenceUnit : public Callable
{
    T func; //func is a function that will be run for the duration of "time". Returns true if this unit should no longer be processed func: (int) => bool;
    SequenceUnit(int time_, int repetitions_, int inFrames_, T func_) : func(func_)
    {
        time = time_;
        repetitions = repetitions_;
        inFrames = inFrames_;
    }
    bool operator()(int passed) //time spent on this unit, in either milliseconds or frames, is passed to the function
    {
        if constexpr(std::is_void<decltype(func(passed))>::value) //if you are a big stinky fool and you forget to return something, this unit will run until time is up
        {
            func(passed);
            return false;
        }
        else
        {
            return func(passed);
        }
    }
};

class Sequencer
{
    //runs a sequence of Sequence Units
    //Sequencer moves on after a unit's time frame is up, so there is no guarantee that each unit will actually run the correct number of times due to overhead each frame.
protected:
    typedef std::shared_ptr<Callable> CallablePtr;
    typedef std::list<CallablePtr> Sequence;

    DeltaTime timer;
    long timePassed = 0; //time spent on a unit so far. Frames or milliseconds depends on unit
    Sequence sequence;
    Sequence::iterator current; //the unit we are currently on

    bool durationDone(int time, bool inFrames) //returns if a unit is done
    {
        return (inFrames && timer.framesPassed(time)) || (!inFrames && timer.timePassed(time));
    }
    virtual bool perUnitProcess() //what to run for the current sequenceUnit. Returns whether or not to continue running current
    {
        bool done = false;
        if (durationDone((*current)->time/(*current)->repetitions,(*current)->inFrames)) //if enough time has passed to run func
        {
            auto timeSpent = (*current)->inFrames ? timer.getFramesPassed() : timer.getTimePassed(); //time since last frame, in either milliseconds or frames
            timePassed += timeSpent;
            done = (*(current->get()))(timePassed); //run the function
            timer.set();
        }
        return done;
    }
    virtual void perUnitDone() //what to do if a sequenceUnit is done
    {
        std::advance(current,1);
        timer.reset();
        timePassed = 0;
    }

public:
    Sequencer()
    {
        current = sequence.end();
    }
    void reset()
    {
        timePassed = 0;
        timer.reset();
        current = sequence.begin();
    }
    void setup(Callable& cs)
    {
        sequence.emplace_back(&cs);
    }
    void setup(CallablePtr& cs) //if you don't want the Sequencer to own the unit, pass in a shared pointer
    {
        sequence.emplace_back(cs);
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
                current = sequence.begin();
            }
            if (timer.isSet()) //one sequence is already running
            {
                bool done = perUnitProcess();

                if (done || (timePassed >= (*current)->time)) //we've spent enough time on current, move on
                {
                    perUnitDone();
                }
            }
            else
            {
                timer.set();
            }
        }
    }
};

class SequenceManager //contains a list of sequences to run
{
    static std::unordered_set<Sequencer*> sequences;
public:
    static void request(Sequencer& sequence)
    {
        sequence.reset();
        sequences.insert(&sequence);
    }
    static void run()
    {
        auto end = sequences.end();
        for (auto it = sequences.begin(); it != end; ++it)
        {
            (*it)->run();
            if ((*it)->done())
            {
                sequences.erase(it);
            }
        }
    }

};

#endif // SEQUENCER_H_INCLUDED
