#ifndef NTASK_HPP_
#define NTASK_HPP_

#include "Task.hpp"
#include "freertos/queue.h"
#include <vector>

#define NTASK_QUEUE_LENGTH (8)

/**
 * @brief Define all type of NTask
 * 
 */
#define NTASK_TYPE_NOTIF_ISR_CONTX 0xff
#define NTASK_TYPE_NOTIF_WORK_QUEU 0xfe

/**
 * @brief Macro to transform notif_value into Notification structure ( should only be used in NTask related context)
 */
#define TO_NOTIFICATION(X) ((Notification_t){{.Identifier =this->getIdentifier(),.value = (X)}})
/**
 * @todo AD ID mechanisme for ISR also : or maybe it won't be possible because an ISR can't know its ID becuse it's not concepted as an object
 */
#define NOTIFICATION_FROM_ISR(X) ((Notification_t){{.Identifier = {{.type=NTASK_TYPE_NOTIF_ISR_CONTX,.ID = 0}}, .value = (X)}})

/**
 * @brief Structure of identifer
 * 
 */
#pragma pack(2)
union Identifier_t
{
    struct
    {
        uint8_t type; //<! type of the emitter
        uint8_t ID;   //<! ID of the emitter
    };
    uint16_t w_id;
};
#pragma pack()
/**
 * @brief Structure of a notification
 * 
 */
#pragma pack(4)
union Notification_t
{
    struct
    {
        Identifier_t Identifier;
        uint16_t value; //<! value of the notification
    };
    uint32_t d0;
};
#pragma pack()

/**
 * @brief Class that encapsulate a runnable task with ID and notification mechanism
 * 
 * This class is designed to be subclassed with the method:
 */
class NTask : public Task
{
private:
    Identifier_t identifier;
    QueueHandle_t notification_queue;
    static std::vector<NTask *> ntask_list;
    static bool isIDTaken(Identifier_t identifier);
    static char getIDnotTaken(Identifier_t identifier);
    
    virtual void run(void *data) = 0;

protected:
    
    /**
     * @brief send Notification to a NTask object identified by its identifier number, to the back of the queue
     * 
     * @param identifier identifier umber of the destination
     * @param notif_value value to notify
     * @param ticktowait tick to wait for item to be sent
     * @return BaseType_t 
     */
    BaseType_t sendNotificationTo(Identifier_t identifier, uint16_t notif_value, TickType_t ticktowait){
        return sendNotificationTo(getNTaskByIdentifier(identifier),TO_NOTIFICATION(notif_value),ticktowait,queueSEND_TO_BACK);
    };
        /**
     * @brief send Notification to a NTask object identified by its identifier number, to the front of the queue
     * 
     * @param identifier identifier umber of the destination
     * @param notif_value value to notify
     * @param ticktowait tick to wait for item to be sent
     * @return BaseType_t 
     */
    BaseType_t sendNotificationToFrontTo(Identifier_t identifier, uint16_t notif_value, TickType_t ticktowait){
        return sendNotificationTo(getNTaskByIdentifier(identifier),TO_NOTIFICATION(notif_value),ticktowait,queueSEND_TO_FRONT);
    };
    /**
     * @brief send Notification to a NTask object, to the back of the queue
     * 
     * @param destination, pointer on NTask destination object
     * @param notif_value value to notify
     * @param ticktowait tick to wait for item to be sent
     * @return BaseType_t 
     */
    BaseType_t sendNotificationTo(NTask * destination, uint16_t notif_value, TickType_t ticktowait){
        return sendNotificationTo(destination,TO_NOTIFICATION(notif_value),ticktowait,queueSEND_TO_BACK);
    };
    /**
     * @brief send Notification to a NTask object, to the front of the queue
     * 
     * @param destination, pointer on NTask destination object
     * @param notif_value value to notify
     * @param ticktowait tick to wait for item to be sent
     * @return BaseType_t 
     */
    BaseType_t sendNotificationToFrontTo(NTask * destination, uint16_t notif_value, TickType_t ticktowait){
        return sendNotificationTo(destination,TO_NOTIFICATION(notif_value),ticktowait,queueSEND_TO_FRONT);
    };
    Notification_t receiveNotification(TickType_t ticktowait);

public:
static BaseType_t sendNotificationTo(NTask *dest, Notification_t notif, TickType_t ticktowait, BaseType_t notif_position);
    // add ID mechanism to the constructor
    NTask(char ntype = 0, std::string taskName = "Task", uint16_t stackSize = 10000, uint8_t priority = 2, uint8_t coreId = 0, uint8_t notification_queue_size = NTASK_QUEUE_LENGTH);
    ~NTask();
    // get NTask by ID or type
    static NTask *getNTaskByIdentifier(Identifier_t Identifier);
    static std::vector<NTask *> getNTaskByType(char type);
    // getter
    
    char getID(){
        return identifier.ID;
    };
    char getType(){
        return  identifier.type;
    };
    Identifier_t getIdentifier(){
        return identifier;
    };
    //QueueHandle_t getNotificationQueueHandle();
    // display
    static void printAllNtask();
    // send notification to a task from ISR context ( think it more carrefully, maybe add it to a subclass)
    // static for Isr () :  this have to be think more carefully because every task doesnt need Isr notification mechanism

    static BaseType_t sendNotificationFromIsrTo(NTask *dest, uint16_t notif_value, BaseType_t *pxHigherPriorityTaskWoken);
    static BaseType_t sendNotificationToFrontFromIsrTo(NTask *dest, uint16_t notif_value, BaseType_t *pxHigherPriorityTaskWoken);
};

#endif