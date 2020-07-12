#include <list>
#include <map>
#include <functional>
#include <thread>
#include <mutex>
#include <chrono>

struct Work
{
    std::string id;
    std::chrono::seconds interval;
    std::function<void()> func;

    Work(const std::string& id, std::chrono::seconds interval, std::function<void()> func)
    {
        this->id = id; this->interval = interval; this->func = func;
    }

    bool operator==(const Work& work)
    {
        return id == work.id;
    }
    
};

class Worker
{
    private:
    static Worker* _instance;
    #define WORK_INTERVAL_MILI 1000

    //MAYBEDO add condition_variable to wake of thread if more work is needed
    
    std::mutex _workLock;
    //std::map<std::string, std::function<void()>> _workFunctions;
    std::thread _worker;

    std::list<std::shared_ptr<Work>> _workList;
    std::map<std::string, std::chrono::time_point<std::chrono::system_clock>> _lastTimeWorkedOn;

    std::mutex _changeLock;
    std::list<std::shared_ptr<Work>> _addWorkList;
    //std::map<std::string, std::function<void()>> _removeFunctions;

    Worker() {}
    void WorkLoop();
    bool NeedIteration();
    void w_AddRemoveFunctions();
    public:
    static void Start();
    static bool AddWork(std::shared_ptr<Work>);
    //static void RemoveWork(std::function<void()> func);


};