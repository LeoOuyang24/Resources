#ifndef SEQUENCER_H_INCLUDED
#define SEQUENCER_H_INCLUDED

#include <memory>
#include <unordered_set>

#include "vanilla.h"
#include "SDLhelper.h"
#include "render.h"

//this file helps chain events together(ex: create a hitbox when an animation ends)
//realistically, it'll probably be used with animations primarily but can be used to do anything that occurs over time

struct Callable //creates a base class for all sequence units to inherit from. Really just exists so we can have a list of templated SequenceUnits
{
    //Callable can terminate through one of two mechanisms:
    //  1) enough repetitions were completed
    //  2) the Callable operator returned true, meaning it terminated itself
    //Only one of these conditions has to be true to terminate

    unsigned int time = 0; //how long to run one repetition
    unsigned int repetitions = 0; //number of times to run, 0 means to run until operator returns true
    bool inFrames = true; //true if "time" is expressed in frames, otherwise, milliseconds
    virtual bool operator()(int) = 0;
};

template<typename T>
struct SequenceUnit : public Callable
{
    T func; //func is a function that will be run for the duration of "time". Returns true if this unit should no longer be processed func: (int) => bool;
    SequenceUnit(int time_, int repetitions_, bool inFrames_, const T& func_) : func(func_)
    {
        time = time_;
        repetitions = repetitions_;
        inFrames = inFrames_;
    }
    bool operator()(int passed) //time spent on this callable, in either milliseconds or frames, is passed to the function
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
        if (callable.time == 0) //callables' "time" variable can't be less than 0
        {
            callable.time = 1;
        }
        return callable.inFrames ? timer.getFramesPassed()/callable.time
                                  : timer.getTimePassed()/callable.time;
    }
    virtual bool perUnitProcess() //what to run for the current sequenceUnit. Returns whether or not to continue running current
    {
        bool done = false;
        int repsElapsed = repetitionsElapsed(*current->get());
        while (repsElapsed > numOfReps && ((*current)->repetitions > numOfReps || (*current)->repetitions == 0) && !done) //Call the current Callable the number of times we should've called it since our last time calling. Terminate early if the function returns "true" (meaning it is done) or if we have done the max number of reps
        {
            numOfReps++;
            auto cur = ((current->get()));
            auto time = (*current)->inFrames ? timer.getFramesPassed() : timer.getTimePassed();
            auto result = (*cur)(time);
            done = (result) ||
                   numOfReps >= (*current)->repetitions; //run the function and update done. Also pass in how many frames/milliseconds since we began running this Callable
            //done is true if function returned true or if we have reached the number of repetitions needed
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
    template<typename Callable1_, typename Callable2_, typename... CallableList_>
    void setup(Callable1_& a, Callable2_& b, CallableList_&... c) //can be used to recursively pass in a list of Callables or CallablePtrs
    {
        setup(a);
        setup(b,c...);
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
