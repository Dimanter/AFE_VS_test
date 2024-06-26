#pragma once

#include <list>
#include <map>

#include "Service.hpp"

/**
 * \Brief Консолидирование групповых процессов взаимодействия
 * \detail Является механизмом работы с множеством процессов (групповых операций)
 */
class Processes
{
    using callable_t = std::function<std::pair<Service::serviceID_t, Service::State>(void)>;

    /**
     * \Brief Описание многоэтапного процесса взаимодействия
     * \detail Является механизмом консолидирования группы операций представляющих составные части одного процесса взаимодействия
     */
    class Process
    {
        enum State : addressing_t
        {
            Idle,
            Exec
        };
        using container_t = std::list<callable_t>;

    public:
        /// @brief Добавление операции
        /// @param call вызываемый объект, возвращаемое значение которого представляет идентификатор события по возникновению которого процесс перейдет на другой шаг
        void add(callable_t&& call)
        {
            steps.push_back(std::forward<callable_t>(call));
        }

        void clear()
        {
            steps.clear();
        }

        /// @brief Запуск процесса на выполнение
        void run()
        {
            it = steps.begin();
            if (it != steps.end()) 
            {
                running = true;
                mode = Exec;
                std::tie(awaitService, awaitState) = (*it)();
            }
            else
            {
                running = false;
            }
            
        }

        bool IsRunning()
        {
            return running;
        }

        /// @brief Обработка поступившего события
        /// @param service идентификатор сервиса
        /// @param state состояние сервиса
        /// @return состояние сервиса
        bool notify(const Service::serviceID_t& service, const Service::State& state)
        {
            if (mode == Exec && awaitService == service && awaitState == state) 
            {
                if (it != steps.end()) 
                {
                    ++it;
                    if (it != steps.end()) 
                    {
                        std::tie(awaitService, awaitState) = (*it)();
                    }
                    else {
                        mode = Idle;
                        return true;
                    }
                }
                else 
                {
                    mode = Idle;
                    return false;
                }
            }
            return false;
        }

    private:
        State                 mode{ Idle };
        Service::serviceID_t  awaitService;
        Service::State        awaitState;
        container_t::iterator it;
        container_t           steps;
        bool running = true;
    };

public:
    using processID_t = Service::serviceID_t;
    using eventCallback_t = std::function<void(const processID_t&)>;

    Processes(eventCallback_t call) : callback{ call } {}

    /// @brief Регистрация процесса
    /// @param process процесс
    void add(processID_t id, callable_t&& call)
    {
        processes[id].add(std::forward<callable_t>(call));
    }

    bool getStatus(processID_t id)
    {
        return processes[id].IsRunning();
    }

    void clear(processID_t id)
    {
        processes[id].clear();
    }

    

    void run(const processID_t& id)
    {       
        processes[id].run();
    }

    /// @brief Обработка поступившего события
    /// @param service идентификатор сервиса
    /// @param state состояние сервиса
    void notify(const Service::serviceID_t& service, const Service::State& state)
    {
        for (auto& [id, process] : processes) {
            if (process.notify(service, state)) {
                callback(id);
            }
        }
    }

private:
    const eventCallback_t          callback;
    std::map<processID_t, Process> processes;

};
