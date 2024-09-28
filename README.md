# Multi-tasking-and-RTOS-for-Robotics

## FreeRTOS-Based Robot Control System
This project is a real-time robot control system developed using FreeRTOS on an ESP32 platform. The system uses a hierarchical task structure to manage different functionalities, including motor control, obstacle avoidance, and task scheduling. The project encapsulates FreeRTOS tasks into object-oriented abstractions, making it easier to manage, develop, and extend the system.

## Project Structure
### Main Directory
 - app_main.c: The entry point for the FreeRTOS-based application. It includes an infinite loop where the robot's main logic is executed.
components/: Contains additional modules used in the project.
 - WTask/: Contains task-related classes and abstractions, providing encapsulation and notification handling between tasks.
CMakeLists.txt: Configuration file for building the project using CMake.
README.md: This documentation file.
component.mk: Makefile for managing component dependencies.
 - miscellaneous/: Additional helper functions and modules.
 - ultrasound/: Manages ultrasound sensor integration for obstacle detection.
### Task Subdirectories
The task-related classes are structured into different directories based on their levels of abstraction and functionality:

Task/: Basic task object encapsulation using FreeRTOS.
NTask/: Advanced task handling with inter-task notifications.
RTask/: Extends NTask to include thread-safe buffers for communication between tasks.
WorkQueue/: A specific task for handling jobs asynchronously, with optional return values.
### Features
 - FreeRTOS-Based: Uses FreeRTOS for real-time task management, including multi-tasking, task synchronization, and inter-task communication.
 - Object-Oriented Task Abstraction: Provides a hierarchy of task objects (Task, NTask, RTask, WorkQueue), making it easier to extend and manage complex systems.
 - Robot Control: The project is designed for controlling a robot, managing tasks such as motor control and sensor input processing.
 - Obstacle Avoidance: Uses ultrasonic sensors for detecting obstacles, pausing the robot's movement when necessary.
 - Team Configurations: The robot can be configured to behave differently depending on its team (e.g., blue team vs. other teams).
 - Real-Time Notifications: Tasks communicate and synchronize using FreeRTOS notifications, allowing for efficient coordination between components.
 - Thread-Safe Buffering: RTask objects provide safe mechanisms for exchanging data between tasks using a ring buffer.
### Class Descriptions
Task Object
The Task class encapsulates a FreeRTOS task into an object. It is an abstract class, where you must implement the private run function. This allows you to run tasks as objects with custom logic, making it possible to assign tasks to different CPU cores, set priorities, and control the stack size.

NTask Object
NTask extends Task and adds functionality for task notifications. Each NTask registers itself in a static list upon creation, enabling other tasks to easily send notifications. Notifications are stored in a queue and are processed in the order they are received. This mechanism simplifies inter-task communication.

RTask Object
RTask further extends NTask by introducing a thread-safe ring buffer for passing data between tasks. Notifications are synchronized with the ring buffer, ensuring that tasks can send and receive variable-sized data safely. This makes RTask suitable for managing larger, more complex data exchanges.

WorkQueue
WorkQueue is a specific implementation of RTask designed for executing asynchronous jobs. Other tasks can submit jobs to the WorkQueue, which then processes the jobs and optionally returns the results. Jobs are defined as structures containing a function pointer and its arguments, as well as a notification mechanism to alert when the job is done.
