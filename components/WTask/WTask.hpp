#ifndef WTASK_HPP_
#define WTASK_HPP_
#include "sdkconfig.h"

#if (CONFIG_WORKQUEUE_SUPPORT)
#include "Task.hpp"
#include "NTask.hpp"
#include "RTask.hpp"
#include "WorkQueue.hpp"
#elif (CONFIG_RTASK_SUPPORT)
#include "Task.hpp"
#include "NTask.hpp"
#include "RTask.hpp"
#elif (CONFIG_NTASK_SUPPORT)
#include "Task.hpp"
#include "NTask.hpp"
#elif (CONFIG_TASK_SUPPORT)
#include "Task.hpp"
#endif

#endif //WTASK_HPP_