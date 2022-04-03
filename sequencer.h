#ifndef SEQUENCER_H_INCLUDED
#define SEQUENCER_H_INCLUDED

#include "SDLhelper.h"

//this file helps chain events together(ex: create a hitbox when an animation ends)
//realistically, it'll probably be used with animations primarily but can be used to do anything that occurs over time

struct Callable //creates a base class for all sequence units to inherit from. Really just exists so we can have a list of templated SequenceUnits
{
    int time= 0; //how long to run func
    int repetitions = 0; //number of times to run
    bool inFrames = true; //true if time is expressed in frames, otherwise, milliseconds
    virtual void operator()(int)
    {

    }
protected:
    Callable() //protected constructor because you aren't technically supposed to actually make any of these
    {

    }
};

template<typename T>
struct SequenceUnit : public Callable
{
    T func; //func is a function that will be run for the duration of "time". func: (int) => void;
    SequenceUnit(int time_, int repetitions_, int inFrames_, T func_) : func(func_)
    {
        time = time_;
        repetitions = repetitions_;
        inFrames = inFrames_;
    }
    void operator()(int passed) //time passed, in either milliseconds or frames, is passed to the function
    {
        //func(passed);
    }
};

class Sequencer
{
    typedef std::vector<std::unique_ptr<Callable>> Sequence;

    DeltaTime timer;
    Sequence sequence;
    Sequence::iterator current; //the unit we are currently on
    bool durationDone(int time, bool inFrames) //returns if a unit is done
    {
        return (inFrames && timer.framesPassed(time)) || (inFrames && timer.timePassed(time));
    }
public:
    Sequencer()
    {
        current = sequence.end();
    }
    void setup(Callable& cs)
    {
        sequence.push_back(std::unique_ptr<Callable>(&cs));
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
                if (durationDone((*current)->time,(*current)->inFrames))
                {
                    std::advance(current,1);
                }
                else if (durationDone((*current)->time/(*current)->repetitions,(*current)->inFrames))
                {
                    (*(current->get()))((*current)->inFrames ? timer.getFramesPassed() : timer.getTimePassed());
                    timer.set();
                }
            }
        }
    }
};

#endif // SEQUENCER_H_INCLUDED
