#ifndef RTASK_HPP_
#define RTASK_HPP_

#include "NTask.hpp"
#include "freertos/ringbuf.h"
#include "freertos/semphr.h"

class RTask : public NTask
{
private:
    RingbufHandle_t receiving_buff;
    SemaphoreHandle_t mutex_receiving_buff;
    virtual void run(void *data) = 0;

protected:
    /**
    * @brief Return a pointer on data received with timeout, if the data are sended by the normal way aka sendDataTo, then the timeout should be useless
    * 
    * @param size pointer on size of the received data
    * @param ticktowait tick to wait for receiving data
    * @return void* pointer on data, or nullptr if timeout and no data available
    */
    void *receiveData(size_t *size, TickType_t tickToWait){
        return xRingbufferReceive(receiving_buff, size, tickToWait);
    };

    /**
    * @brief Return a pointer on data without timeout
    * 
    * @param size of the received data
    * @return void* pointer on received data, or nullptr if not data available
    */
    void *receiveData(size_t *size){
        return xRingbufferReceive(receiving_buff, size, 0);
    };

    /**
    * @brief This function is used to return data after receiving it
    * 
    * @param data pointer on data in the ring buffer
    */
    void returnData(void *data){
        vRingbufferReturnItem(receiving_buff, data);
    };
    
    static BaseType_t sendDataTo(RTask *destination, void *data, uint32_t size, TickType_t ticktowait, bool usenotif, Notification_t notification);

    /**
     * @brief Send data to RTask and also send a notification
     * 
     * @param dest destination
     * @param data 
     * @param size 
     * @param ticktowait 
     * @param notif_value 
     * @return BaseType_t 
     */
    BaseType_t sendDataTo(RTask *dest, void *data, uint32_t size, TickType_t ticktowait, uint16_t notif_value){
        return sendDataTo(dest,data,size,ticktowait,true,TO_NOTIFICATION(notif_value));
    };

    /**
     * @brief Send data to RTask defined by its identifier and also a notification
     * 
     * @param dest_identifier : identifier of the destination
     * @param data 
     * @param size 
     * @param ticktowait 
     * @param notif_value 
     * @return BaseType_t 
     */
    BaseType_t sendDataTo(Identifier_t dest_identifier, void *data, uint32_t size, TickType_t ticktowait, uint16_t notif_value){
        return sendDataTo(static_cast<RTask *>(getNTaskByIdentifier(dest_identifier)),data,size,ticktowait,true,TO_NOTIFICATION(notif_value));
    };

    /**
     * @brief Send data to RTask : do use this function as it may lead to dealock because of the absence of notification mechanism
     * @param dest 
     * @param data 
     * @param size 
     * @param ticktowait 
     * @return BaseType_t 
     */
    BaseType_t sendDataTo(RTask *dest, void *data, uint32_t size, TickType_t ticktowait){
        return sendDataTo(dest,data,size,ticktowait,false,(Notification_t){.d0=0});
    };
    
    /**
     * @brief Send data to RTask defined by its identifier : do use this function as it may lead to dealock because of the absence of notification mechanism
     * 
     * @param dest_identifier 
     * @param data 
     * @param size 
     * @param ticktowait 
     * @return BaseType_t 
     */
    BaseType_t sendDataTo(Identifier_t dest_identifier, void *data, uint32_t size, TickType_t ticktowait){
        return sendDataTo(static_cast<RTask *>(getNTaskByIdentifier(dest_identifier)),data,size,ticktowait,false,(Notification_t){.d0=0});
    };
public:
    RTask(char ntype = 0, std::string taskName = "Task", uint16_t stackSize = 10000, uint8_t priority = 2, uint8_t coreId = 0, uint8_t notification_queue_size = NTASK_QUEUE_LENGTH, uint32_t ringbuffer_size = 128);
    ~RTask();
    //get_data and set_data are synchrounous functions that can't be implemented here
};

#endif /*RTASK_HPP_*/