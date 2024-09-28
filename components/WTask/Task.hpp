#ifndef COMPONENTS_CPP_UTILS_TASK_H_
#define COMPONENTS_CPP_UTILS_TASK_H_

#include <string>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


/**
 * @brief Encapsulate a runnable task.
 *
 * This class is designed to be subclassed with the method:
 *
 * @code{.cpp}
 * void run(void *data) { ... }
 * @endcode
 *
 * For example:
 *
 * @code{.cpp}
 * class CurlTestTask : public Task {
 *    void run(void *data) {
 *       // Do something
 *    }
 * };
 * @endcode
 *
 * implemented.
 */
class Task {
public:
	Task(std::string taskName = "Task", uint16_t stackSize = 10000, uint8_t priority = 5);
	virtual ~Task();
	void setStackSize(uint16_t stackSize);
	void setPriority(uint8_t priority);
	void setName(std::string name);
	void setCore(BaseType_t coreId);
	void suspend();
	void resume();
	void start(void* taskData = nullptr);
	void stop();
	static void delay(int ms);
	/**
	 * @brief Body of the task to execute.
	 *
	 * This function must be implemented in the subclass that represents the actual task to run.
	 * When a task is started by calling start(), this is the code that is executed in the
	 * newly created task.
	 *
	 * @param [in] data The data passed in to the newly started task.
	 */
	uint32_t getCore(void){return m_coreId;};
	std::string getName(void){return m_taskName;};
	bool is_task_running(){return m_running;};
protected:
	TaskHandle_t m_handle;
	std::string m_taskName;
	uint16_t    m_stackSize;
	uint8_t     m_priority;
	BaseType_t  m_coreId;
	bool 		m_running;
private:
	void*       m_taskData;
	static void runTask(void* data);
	virtual void run(void* data) = 0; // Make run pure virtual
};

#endif /* COMPONENTS_CPP_UTILS_TASK_H_ */