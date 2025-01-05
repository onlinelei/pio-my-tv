#ifndef TASK_BASE_H
#define TASK_BASE_H

#include <functional>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class TaskBase
{
protected:
    uint32_t delay_ms;
    std::function<void()> task_func;

    static void taskWrapper(void *param)
    {
        auto *task = static_cast<TaskBase *>(param);
        for (;;)
        {
            vTaskDelay(task->delay_ms / portTICK_PERIOD_MS);
            if (task->task_func)
            {
                task->task_func();
            }
        }
    }

public:
    TaskBase(uint32_t delay_ms, std::function<void()> func)
        : delay_ms(delay_ms), task_func(func) {}

    virtual ~TaskBase() = default;

    void createTask(const char *name, uint32_t stack_size, UBaseType_t priority, BaseType_t core_id)
    {
        TaskHandle_t task_handle;
        xTaskCreatePinnedToCore(
            taskWrapper,
            name,
            stack_size,
            this,
            priority,
            &task_handle,
            core_id);
    }
};

#endif // TASK_BASE_H