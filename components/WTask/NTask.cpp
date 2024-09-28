#include "NTask.hpp"
#include "esp_log.h"

#define NTASK_ID_STARTING (0x01)
static const char *NTASK_LOG_TAG = "NTASK";

std::vector<NTask *> NTask::ntask_list;

/**
 * @brief Check either if Identifier's ID is already taken or not
 * @param Identifier
 * @return true if Identifier's ID is already taken
 */
bool NTask::isIDTaken(Identifier_t identifier)
{
    uint32_t i = 0;
    while (i < NTask::ntask_list.size())
    {
        if (identifier.w_id == NTask::ntask_list[i]->identifier.w_id)
        {
            return true;
        }
        i++;
    }
    return false;
};

/**
 * @brief Get ID not already taken : this is usefull for dynamic creation of task
 * @param type : type of the object to search for a free ID
 * @return ID
 */
char NTask::getIDnotTaken(Identifier_t identifier)
{

    for (identifier.ID = NTASK_ID_STARTING; identifier.ID < 255; identifier.ID++)
    {
        if (!isIDTaken(identifier))
        {
            return identifier.ID;
        }
    }
    ESP_LOGE(NTASK_LOG_TAG, "Can't assign other peripheral ID because all ID are taken");
    return 255;
};

/**
 * @brief Base function to send notification to a task (used by other functions)
 *
 * @param dest NTask to notify
 * @param notif notification to send
 * @param ticktowait tick to wait for completion in case the queue is full
 * @param notif_position position on the queue to put the notification (front or back)
 * @return BaseType_t pDTRUE on success
 */
BaseType_t NTask::sendNotificationTo(NTask *dest, Notification_t notif, TickType_t ticktowait, BaseType_t notif_position)
{
    return (dest == nullptr) ? pdFALSE : xQueueGenericSend(dest->notification_queue, &notif, ticktowait, notif_position);
};

/**
 * @brief Used to send notification to a task from ISR context, should only be used in ISR context
 *
 * @param dest task destination
 * @param notif_value value to send
 * @param pxHigherPriorityTaskWoken to know if context switching will occur or not
 * @return BaseType_t pdTRUE on success
 */
BaseType_t NTask::sendNotificationFromIsrTo(NTask *dest, uint16_t notif_value, BaseType_t *pxHigherPriorityTaskWoken)
{
    Notification_t notif = NOTIFICATION_FROM_ISR(notif_value);
    return xQueueSendFromISR(dest->notification_queue, &notif, pxHigherPriorityTaskWoken);
};

/**
 * @brief Send notification to the front of the notification queue of a task: should only be used from ISR context
 *
 * @param [in] dest
 * @param [in] notif_value
 * @param [out] pxHigherPriorityTaskWoken
 * @return BaseType_t pdTRUE on success
 */
BaseType_t NTask::sendNotificationToFrontFromIsrTo(NTask *dest, uint16_t notif_value, BaseType_t *pxHigherPriorityTaskWoken)
{
    Notification_t notif = NOTIFICATION_FROM_ISR(notif_value);
    return xQueueSendToFrontFromISR(dest->notification_queue, &notif, pxHigherPriorityTaskWoken);
};

/**
 * @brief Wait for a notification
 *
 * @param ticktowait time to wait
 * @return Notification_t value
 */
Notification_t NTask::receiveNotification(TickType_t ticktowait)
{
    Notification_t notif;
    if (xQueueReceive(notification_queue, &notif, ticktowait))
    {
        return notif;
    }
    return (Notification_t){.d0 = 0};
};

/**
 * @brief Construct a new NTask::NTask object
 *
 * @param ntype type of the ntask
 * @param taskName
 * @param stackSize in byte
 * @param priority (lowest is 0), prefer always using different priorities to avoid priority switching
 * @param notification_queue_size size of the notification queue
 */
NTask::NTask(char ntype, std::string taskName, uint16_t stackSize, uint8_t priority, uint8_t coreId, uint8_t notification_queue_size) : Task(taskName, stackSize, priority)
{
    setCore(coreId);
    identifier.type = ntype;
    identifier.ID = getIDnotTaken(identifier);
    ntask_list.push_back(this);
    notification_queue = xQueueCreate(notification_queue_size, sizeof(Notification_t));
};
/**
 * @brief Destroy the NTask::NTask object
 *
 */
NTask::~NTask()
{
    uint32_t i = 0;
    while (i < ntask_list.size() && this != ntask_list.at(i))
    {
        i++;
    }
    if (i < ntask_list.size()) // if we can't find the item, it means that it has already been deleted from the list
        ntask_list.erase(ntask_list.begin() + i);
    vQueueDelete(notification_queue);
};

/**
 * @brief return a pointer on a ntask object based on its ID
 * @param Type of the task to search for
 * @param id of the ntask object
 * @return NTask*
 */
NTask *NTask::getNTaskByIdentifier(Identifier_t identifier)
{
    uint32_t i = 0;
    while (i < ntask_list.size() && (ntask_list[i]->identifier.w_id != identifier.w_id))
    {
        i++;
    }
    if (i == ntask_list.size())
    {
        ESP_LOGE(NTASK_LOG_TAG, "Can't find Ntask corresponding to Type:ID %X:%X", identifier.type, identifier.ID);
        return nullptr;
    }
    return ntask_list[i];
};

/**
 * @brief return a vector filled with all ntask object with the correct type
 *
 * @param type to search for
 * @return std::vector<NTask *>
 */
std::vector<NTask *> NTask::getNTaskByType(char type)
{
    std::vector<NTask *> list;
    for (uint32_t i = 0; i < ntask_list.size(); i++)
    {
        if (ntask_list[i]->identifier.type == type)
        {
            list.push_back(ntask_list.at(i));
        }
    }
    return list;
};

void NTask::printAllNtask()
{
    if (ntask_list.size() == 0)
    {
        printf("There is no ntask currently registered \n");
    }
    else
    {
        printf("NTask registered list \n");
        printf(" Type |  ID | NTask name | Core | State\n");
        printf("------|-----|------------|------|------\n");
        for (uint32_t i = 0; i < ntask_list.size(); i++)
        {
            NTask *temp = ntask_list[i];
            printf(" %4d | %3d | %10s | %4d | %5d\n", temp->identifier.type, temp->identifier.ID, temp->m_taskName.substr(0, 10).c_str(), temp->m_coreId, temp->m_running);
        }
    }
};