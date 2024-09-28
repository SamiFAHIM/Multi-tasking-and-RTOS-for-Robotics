#include <string.h>
#include "WorkQueue.hpp"
#include "esp_log.h"

#define NOTIFICATION_WORK_IN_QUEUE (0x01)

#define MODULO_32_SIZE(X) (X+(32-X%32))

static const char *WORKQ_LOG_TAG = "WORKQ";

WorkQueue::WorkQueue(uint16_t stackSize, uint8_t priority, uint8_t workQueueLength, uint8_t coreID) : RTask(NTASK_TYPE_NOTIF_WORK_QUEU, "workQueue", stackSize, priority, 0, workQueueLength, (sizeof(WorkItem) + 8) * workQueueLength){};
/*
WorkQueue::~WorkQueue(){
    vQueueDelete(workQueue);
};
*/
BaseType_t WorkQueue::sendWork(WorkItem &item)
{
    return sendDataTo(this, &item, sizeof(item), portMAX_DELAY, true, (Notification_t){.d0=NOTIFICATION_WORK_IN_QUEUE});
};

void WorkQueue::run(void *args)
{
    WorkItem item;
    Notification_t notif;
    void *ret_data;
    while (true)
    {
        size_t size=0;
        // wait for work notification
        notif = receiveNotification(portMAX_DELAY);
        // notification received
        if (notif.value == NOTIFICATION_WORK_IN_QUEUE)
        {
            ret_data = receiveData(&size, portMAX_DELAY);
            if(size == sizeof(WorkItem)){
                memcpy(&item, ret_data, size);
                returnData(ret_data);

                // launching work here
                ret_data = (item.work_function)(item.work_args, &size);

                // work done : returning data if there is data
                if ((size>0)&&(ret_data !=nullptr))
                {
                    sendDataTo(static_cast<RTask *>(item.returning_task), ret_data, size, portMAX_DELAY, true, TO_NOTIFICATION(item.notif_value));
                    free(ret_data); // dynamically allocated memory
                }
                else
                {
                    sendNotificationTo(item.returning_task, item.notif_value, portMAX_DELAY);
                }
            }else{
                ESP_LOGE(WORKQ_LOG_TAG,"%X : Invalide size of received WorkItem from %X:%X : %db instead of %db",getID(), notif.Identifier.type, notif.Identifier.ID,size,sizeof(WorkItem));
            }
        }
        else
        {
            ESP_LOGE(WORKQ_LOG_TAG,"%X : Invalide notif received from %X:%X",getID(),notif.Identifier.type,notif.Identifier.ID);
        }
    }
};