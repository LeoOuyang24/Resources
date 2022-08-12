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
    bool inFrames = true; //true if "time" is expressed in frames, otherwise, milliseconds
    virtual bool operator()(int) = 0;
};

template<typename T>
struct SequenceUnit : public Callable
{
    T func; //func is a function that will be run for the duration of "time". Returns true if this unit should no longer be processed func: (int) => bool;
    SequenceUnit(int time_, int repetitions_, bool inFrames_, T func_) : func(func_)
    {
        time = time_;
        repetitions = repetitions_;
        inFrames = inFrames_;
    }
    bool operator()(int passed) //time spent on this repetition, in either milliseconds or frames, is passed to the function
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
protected:
    typedef std::shared_ptr<Callable> CallablePtr;
    typedef std::list<CallablePtr> Sequence;

    DeltaTime timer;
    int numOfReps = 0; //number of times we've called the current callable
    Sequence::iterator current; //the unit we are currently on
    Sequence sequence;

    bool durationDone(int time, bool inFrames) //returns if a unit is done
    {
        return (inFrames && timer.framesPassed(time)) || (!inFrames && timer.timePassed(time));
    }
    int repetitionsElapsed(Callable& callable) //given a callable, returns how many times it should have been called in the time that has elapsed
    {
        //return an int because we want to round down
        float timePerRep = ((float)callable.time/callable.repetitions); //time that should be spent on each repetition
        return callable.inFrames ? timer.getFramesPassed()/timePerRep
                                  : timer.getTimePassed()/timePerRep;
    }
    virtual bool perUnitProcess() //what to run for the current sequenceUnit. Returns whether or not to continue running current
    {
        bool done = false;
        int repsElapsed = repetitionsElapsed(*current->get());
        while (repsElapsed > numOfReps && !done) //Call the current Callable the number of times we should've called it since our last time calling. Terminate early if the function returns "true" (meaning it is done)
        {
            done = done || (*(current->get()))((*current)->inFrames ? timer.getFramesPassed() : timer.getTimePassed()); //run the function and update done. Also pass in how many frames/milliseconds since we began running this Callable
            numOfReps++;
        }
        return done;
    }
    virtual void perUnitDone() //what to do if a sequenceUnit is done
    {
        std::advance(current,1);
        timer.reset();
        numOfReps = 0;
    }

public:
    Sequencer()
    {
        current = sequence.end();
    }
    void reset()
    {
        numOfReps = 0;
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
            if (!timer.isSet())
            {
                timer.set();
            }
            bool done = perUnitProcess();
            if (done || (numOfReps == ((*current)->repetitions))) //we've spent enough time on current, move on
            {
                perUnitDone();
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
