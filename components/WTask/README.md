# WTask related classes

This component allows you to encapsulate FRERTOS task into objects.
There are 4 level of abstraction:
    - Task objects are basic task object.
    - NTask objects are more advanced object with intelligent notification handling (with a queue) and Ntask list registering in a list for all NTask object to be reachable by another NTask object.
    - RTask object is am advanced NTask object with thread safe buffer implementation to allow sending safely data between tasks.
    - WorkQueue is child class of RTask object, and it is done to execute some job that you can send to it and even possibly return values or structures.

## Task Object

This class in an abstract class : you have to overload the private "run" function
If inherited, it allows you to run an object with a FREERTOS component inside, like a thread.
You can choose the core, the priority and the stack size of the corresponding task.

The object can be suspended and also resumed. The task is named for debug purpose also.

## NTask Object

This class inherit Task class. It is also an abstract class.
It mostly adds functionalities for Notification between NTask tasks.
When a Ntask is constructed, it is registered inside a static list that can be accessed by all other NTask objects.
It is register with an ID, that is determined depending on the previous registered NTask, and the type of the NTask.


### Sending and receiving notification
To send a Notification to a NTask, you can use the function members already implemented. 
A Notification is made of the ID of the sender, and a notification information.
The received Notification is stored inside a queue and can be processed safely by the receiver.
The sending process can send the new notification to the front of the queue in case it needs 
to be treated in priority by the receiving task.

The functions to wait for a Notification are blocking functions and a timeout can be specified.

The notification holds the ID of the sender, so it is possible to reply or process different actions depending on the sender.

### Sending notifications from ISR
As ISR are not Ntask objects, they don't have the same mechanism to send notification, but the ID is statically defined by a macro.
Also it is not possible to reply to an ISR.

## RTask Object

This class inherit NTask class. It is also an abstract class.
In addition, it provides some mechanism to send and receive data through a RingBuffer.
The RingBuffer allows to send variable size data.

This class implements ways to synchronize the sending process of the data with Notifications, 
so that the receiving task only has to wait for a Notification to knows that there is data to retrieve.

The main thing about RTask is that it owns a RingBuffer so that other task can write to it (even NTask by using static functions of RTask class).
It is not possible to send data from ISR for now, because the RingBuffer has to be kept synchronized with the notification queue for preserve the RingBuffer integrity(order of packet of data is the same as the order of the corresponding notifications).

It is possible to use the RTask without the synchronization mechanism, but this is not recommended because it can lead to a RingBuffer overflow.
## WorkQueue

This class inherit RTask and is not an abstract class. 
It provides some mechanism to but used as a working task : other NTask objects can send it a work to do, and this task will do it.

### Work to do
A work to do is a structure with a pointer on a function to execute with some arguments. 
The structure hold also a NTask to notify when the job is done.
If the function return some data, then the WorkQueue can send it to the object to notify. In this special case, the object to notify has to be a RTask object to be able to receive the returned data.