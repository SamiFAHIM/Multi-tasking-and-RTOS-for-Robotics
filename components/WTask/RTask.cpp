#include "RTask.hpp"
#include "esp_log.h"

static const char *RTASK_LOG_TAG = "RTASK";

/**
 * @brief Send data to RTask destination : base function
 * @warning The use of this function can lead to a deadlock in case of inverting the sending of data and the sending of the notification : that's why the notification is always sended after the data for the receiving task to take the data after they are already there
 * @param destination RTask to send data to
 * @param data pointer on data
 * @param size size of the data
 * @param ticktowait tick to wait for both mutex taking and ringbuffer sending
 * @param usenotif use notification or not
 * @param notif_value value to notify if used
 * @return BaseType_t pdTRUE on success
 */
BaseType_t RTask::sendDataTo(RTask *destination, void *data, uint32_t size, TickType_t ticktowait, bool usenotif, Notification_t notification)
{
    BaseType_t t;
    if ((destination==nullptr) || (data==nullptr))
        return pdFALSE;
    if (xSemaphoreTake(destination->mutex_receiving_buff,ticktowait))
    {
        t = xRingbufferSend(destination->receiving_buff, data, size, ticktowait);
         
        if (usenotif){
            // infinite delay for the notification, otherwise, it could occure that the data are send to the ring buffer 
            // but no notification is sended because the notification is full for too long time, 
            // this could lead to RingBuffer overflow and buffer integrity corruption
            sendNotificationTo(destination, notification, portMAX_DELAY, queueSEND_TO_BACK); 
        }
        xSemaphoreGive(destination->mutex_receiving_buff);
        return t;
    }
    ESP_LOGE(RTASK_LOG_TAG, "Unable to take RingBuffer Semaphore of RTask %s", destination->getName().c_str());
    return pdFALSE;
};
/**
 * @brief Construct a new RTask::RTask object
 * 
 * @param ntype type of the task
 * @param taskName 
 * @param stackSize stack size in byte
 * @param priority priority of the task
 * @param notification_queue_size size of the notification queue
 * @param ringbuffer_size size of the ring buffer in byte
 */
RTask::RTask(char ntype, std::string taskName, uint16_t stackSize, uint8_t priority, uint8_t coreId, uint8_t notification_queue_size, uint32_t ringbuffer_size) : NTask(ntype, taskName, stackSize, priority, coreId, notification_queue_size)
{
    configASSERT(ringbuffer_size>0);
    receiving_buff = xRingbufferCreate(ringbuffer_size, RINGBUF_TYPE_NOSPLIT);
    mutex_receiving_buff = xSemaphoreCreateMutex();
};

/**
 * @brief Destroy the RTask::RTask object
 * 
 */
RTask::~RTask()
{
    vSemaphoreDelete(mutex_receiving_buff);
    vRingbufferDelete(receiving_buff);
};