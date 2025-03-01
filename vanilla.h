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
#include <regex>



template<typename T>
T lerp(T a1, T a2, float t, bool clamp = false)
{
    return a1 + (a2 - a1)*(clamp ? std::max(std::min(t,1.0f),0.0f) : t); //if clamp is true, clamp the float to between 1 and 0
}


int convertTo1(double number); // a method that converts a number to 1 or -1 depending on its sign. If entry is 0, return 0;

bool floatEquals(float a, float b, int precision); //given floats "a" and "b" returns true if they are equivalent up to "precision" decimal points

size_t hashCombine(size_t h1, size_t h2); //combines two hashes https://stackoverflow.com/questions/5889238/why-is-xor-the-default-way-to-combine-hashes
float round(float decimal, int n); //rounds decimal to the nth digit
bool angleRange(float rad1, float rad2, float range); //returns true if angle rad1 is within range of rad2 (inclusive)
double randomDecimal(int places); //generates a random decimal between 0 and 1, not including 1. places is the number of decimal places

std::pair<std::string,bool> readFile(std::string file); //reads "file" and returns the contents. The bool represents whether or not the file was found and successfully opened

int charCount(std::string s, char c); //returns how many times c shows up in s;

std::string* divideString(std::string input); //divides a string into parts and puts them all into an array

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

template <typename T>
class GlobalMount //if you have a class you want a single global copy of and that class has a constructor with 0 arguments, this class can make it
                //easier to mount the class to global scope without having to explicitly declaring the class
{
    static std::shared_ptr<T> ptr;
public:
    static std::shared_ptr<T> getSharedPtr()
    {
        if (!ptr.get())
        {
            ptr.reset(new T);
        }
        return ptr;
    }
    static T* getPtr()
    {
        return getSharedPtr().get();
    }
};

template <typename T>
std::shared_ptr<T> GlobalMount<T>::ptr;

template<typename Iter>
struct is_reverse_iterator : std::false_type { }; //used to determine if an iterator is reverse or not.
                                                  //stolen from https://stackoverflow.com/questions/22360697/determine-if-a-c-iterator-is-reverse

template<typename Iter>
struct is_reverse_iterator<std::reverse_iterator<Iter>>
: std::integral_constant<bool, !is_reverse_iterator<Iter>::value>
{ };

template<typename...>
struct AllSameTypes; //can be used to make sure that a parameter pack is all the same type at compile time


template<typename T1>
struct AllSameTypes<T1>
{
  constexpr static bool value = true;
};

template<typename sameAs, typename t1, typename... ts>
struct AllSameTypes<sameAs, t1, ts...>
{
  constexpr static bool value = std::is_same<sameAs,t1>::value && AllSameTypes<sameAs,ts...>::value;
};

template<typename...>
struct AllDerivedFrom; //used to make sure that a parameter pack is all derived from the same type at compile time

template<typename T1>
struct AllDerivedFrom<T1>
{
    constexpr static bool value = true;
};

template<typename Base, typename Derived, typename ... Ds>
struct AllDerivedFrom<Base, Derived, Ds...>
{
    constexpr static bool value = std::is_base_of<Base, Derived>::value && AllDerivedFrom<Base, Ds...>::value;
};




template<typename... Args>  struct __attribute__((packed, aligned(1))) TightTuple;

template<typename T, typename... Args>
struct  TightTuple<T,Args...>
{
    T t;
    TightTuple<Args...> next;
    TightTuple(T t_, Args... args) : t(t_), next(args...)
    {

    }
};

template<typename T>
struct TightTuple<T>
{
    T t;
    TightTuple(T t_) : t(t_)
    {

    }
};

template<typename... Args>
void fillBytesVec(std::vector<char>& bytesVec, int totalBytes, Args... args) //fill a byte array with "args"
{
    /*Recursively fill "bytesVec" with the bytes of our arguments. "totalBytes" is the total number of bytes of data we are providing, useful if we want to
    pad with 0s if we under supplied arguments; -1 to do no padding*/

    TightTuple<Args...> tup(args...);
    char* bytes = reinterpret_cast<char*>(&tup);
    /*for (int i = 0; i < sizeof(tup); i += sizeof(float))
    {
        float x;
        memcpy(&x, bytes +  i, sizeof(float));
        std::cout << x << " ";
    }
    std::cout << "\n";*/
    bytesVec.insert(bytesVec.end(),bytes,bytes+sizeof(tup));
    bytesVec.resize(bytesVec.size() + totalBytes - sizeof(tup),'\0');
}

template<typename Lambda>
void regexSearch(std::string reg, std::string str, Lambda lambda)
{
    //given a string and a regex, parses the string using the regex, and runs "lambda" on each match
    //lambda: (const std::smatch& -> void)
    std::regex rgx(reg);
    /*for (std::sregex_iterator it = std::sregex_iterator(str.begin(), str.end(), rgx);
    it != std::sregex_iterator(); it++)
    {
        lambda(it);
    }*/
    std::smatch match;
    while (std::regex_search (str,match,rgx)) {
        lambda(match);
        str = match.suffix().str();
   }
}

template<typename Lambda>
std::string regexReplace(const std::regex rgx, std::string str, Lambda lambda)
{
    //std::cout << str << "\n";
    std::smatch match;
    if (std::regex_search(str,match,rgx))
    {
        return match.prefix().str() + lambda(str,match) + regexReplace(rgx,match.suffix().str(),lambda);
    }
    return str;
}

/**
  *   \brief Pass a function that is then run on each element in a container. If the function returns true, then it terminates early
  *
  *   \param start: iterator to start at
  *   \param end: iterator to end at, noninclusive
  *   \param func: A "Callable", which is anything with a () operator. T = (IteratorType) -> void or bool
  *
  *   \return nothing lol its a void function
  **/
template<typename IteratorType, typename Func>
void doForEachElement(IteratorType start, IteratorType end, Func func) //run a function on each entity
{
    for (auto it = start; it != end; ++it)
    {
        if constexpr (!std::is_same<decltype(func(std::declval<IteratorType>())),bool>::value) //if the function doesn't return a bool, keep going until we've processed every planet
        {
            func(it);
        }
        else //if the function returns bool and is true, we stop running
        {
            auto val = func(it);
            if (val)
            {
                return;
            }
        }
    }
}


#endif // VANILLA_H_INCLUDED
