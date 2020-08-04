#include "worker.h"
#include "usermanager.h"

Worker* Worker::_instance = 0;

void Worker::Start()
{
    if(!_instance){ 
        std::cout << "WorkThread Starting" << std::endl << std::flush;
        _instance = new Worker();
        _instance->_worker = std::thread([](){_instance->WorkLoop();});
    }
}

void Worker::WorkLoop()
{
    std::chrono::time_point runStart = std::chrono::system_clock::now();
    while(true)
    {
        runStart = std::chrono::system_clock::now();
        if(_addWorkList.size() > 0){
            std::scoped_lock<std::mutex> lock(_changeLock);
            for (auto &&work : _addWorkList)
            {
                _workList.push_back(work);
            }
            _addWorkList.clear();
        }
        if(NeedIteration()){
            std::scoped_lock<std::mutex> lock(_workLock);
            //Call Work1 ..
            //UserManager::Get()->DoWork()
            //MatchManager::Get()->DoWork()
            //GameBrowser::Get()->DoWork()
            for (auto &&work : _workList)
            {
                if(std::chrono::system_clock::now() > _lastTimeWorkedOn[work->id] + work->interval)
                {
                    _lastTimeWorkedOn[work->id] = std::chrono::system_clock::now();
                    work->func();
                }
            }
            
        }
        std::this_thread::sleep_until(runStart + std::chrono::milliseconds(WORK_INTERVAL_MILI));
    }
}

bool Worker::NeedIteration()
{
    return UserManager::Get()->ActiveUsers() > 0;
}

bool Worker::AddWork(std::shared_ptr<Work> work)
{
    std::scoped_lock<std::mutex> lock(_instance->_changeLock);
    if(std::find(_instance->_workList.begin(), _instance->_workList.end(), work) == _instance->_workList.end())
    {
        _instance->_addWorkList.push_back(work);
        return true;
    }else{
        //Allready contains work with id
        return false;
    }
}
// void Worker::RemoveWork(std::function<void()> func)
// {

// }


// void Worker::w_AddRemoveFunctions()
// {
//     std::scoped_lock<std::mutex> lock(_changeLock);
//     //_workFunctions.
// }
