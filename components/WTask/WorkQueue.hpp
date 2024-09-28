#ifndef WORKQUEUE_HPP_
#define WORKQUEUE_HPP_

#include "RTask.hpp"
#include "freertos/semphr.h"


/**
 * @brief WorkItem represent a job to do that does not need task mechanisms, aka communication between tasks, notification or sending. \\ 
 * It is just a function that doesnt even know that task exist.
 * 
 */

/**
 * @brief Definition of a working function
 * @param [in] args void pointer on arguments needed by the function
 * @param [out] ret_size pointer used to know the size of the returned data
 * @return pointer on data, can be nullptr if no data has to be returned :
 * data should be put on dynamically allocated memory for them to be returned well. 
 * The working queue will free them after
 */
typedef void *(*WorkFunction)(void*,size_t*);

/**
 * @brief Structure of work : this structure is used to transmit work to the workqueue
 * @param work_args pointer on arguments used by the working_function
 * @param work_function if data is returned by the function, it should be dynamically allocated
 * and it will be temporally stored in the heap of the workqueue, so make sure, that the heap is large enough
 * for that, and also the RIngBuffer of the receiving task
 * @param returning_task Make sure that the returning task is of type RTask if data has to be returned :
 * data is returned if ret_size is set to non-zero value by the work function and returned pointer is not nullptr.
 */
struct WorkItem {
    void *work_args;
    WorkFunction work_function;
    NTask *returning_task;
    uint16_t notif_value;
};

class WorkQueue : public RTask
{
public:
    WorkQueue(uint16_t stackSize=5000, uint8_t priority=3, uint8_t workQueueLength=3, uint8_t coreID=0);
    ~WorkQueue();
    BaseType_t sendWork(WorkItem &item);
private:
    void run(void *args);
};

#endif /*WORKQUEUE_HPP_*/