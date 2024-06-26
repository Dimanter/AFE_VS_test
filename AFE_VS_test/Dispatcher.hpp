#pragma once

#include <algorithm>
#include <queue>
#include <unordered_map>

#include "Service.hpp"
#include "Transport.hpp"

using namespace std::placeholders;

/**
 * \Brief Реализация уровня диспетчерезации
 * \detail Является механизмом консолидирующим взаимодействие транспортного и сервисного уровня
 */
class Dispatcher final
{
    static constexpr size_t timeout = 100;

public:
    Dispatcher() = default;

    Dispatcher(std::function<void(void)> call) : callbackDeffered{ call } {}

    /**
     * \brief Регистрация функции приёма пакета по транспортному каналу
     * \param transport - указатель на транспорт
     * \detail При регистрации передаётся функция обратного вызова в качестве параметра для реализации приёма пакета с транспортного уровня
     */
    void registerTransport(std::shared_ptr<Transport> transport)
    {
        const auto addr = transport->getAddress();
        transports[addr] = transport;
        transport->registerReceiver(std::bind(&Dispatcher::receive, this, addr, _1, _2, _3));
    }

    /**
     * \brief Удаление регистрации функции приёма пакета по транспортному каналу
     * \param addr - адресс транспортного канала
     */
    void unregisterTransport(addressing_t addr)
    {
        transports.erase(addr);
    }

    /**
    \brief Регистрация функции передачи пакета сервисной службе
    \param service - указатель на сервисную службу
    \detail При регистрации передаётся функция обратного вызова в качестве параметра для реализации передачи пакета с сервисного уровня
     */
    void registerService(std::shared_ptr<Service> service)
    {
        const auto addr = service->getAddress();
        services[addr] = service;
        service->registerTransmitter(std::bind(&Dispatcher::transmit, this, _1, addr, _2, _3));
    }

    /**
     * \brief Удаление регистрации функции приёма пакета по транспортному каналу
     * \param addr - адрес сервисной службы
     */
    void unregisterService(addressing_t addr)
    {
        services.erase(addr);
    }

    /**
    \brief Перевод зарегистрированных сервисов в состояние готовности
    \detail При переводе сервиса в состояние готовности выполняется уведомление потребителей и сброс таймаутов
     */
    void resetService()
    {
        for (auto& [addr, service] : services) {
            if (service->getStatus() != Service::State::Ready) {
                process.erase(addr);
                service->Ack(Service::State::Cancel);
                service->Ack(Service::State::Ready);
            }
        }
    }

    /**
     * \brief Функтор циклической обработки состояния
     * \detail выполнение отложенных операций ввода/вывода
     */
    void operator()()
    {
        while (!deffered.empty()) {
            auto [service, pipe, type] = deffered.front();
            if (service->transmit(pipe, type) == Status::Success) {
                deffered.pop();
            }
        }
    }

    /**
     * \brief Метод переодичекой обработки состояния
     * \detail Для формирования сигнала таймаута для операций ввода/вывода
     */
    void clock()
    {
        for (auto& [addrService, time] : process) {
            time--;
            if (time == 0) {
                auto it = services.find(addrService);
                if (it != services.end()) {
                    it->second->Ack(Service::State::Timeout);
                    it->second->Ack(Service::State::Ready);
                }
            }
        }
        std::erase_if(process, [](const decltype(process)::value_type& task) { return task.second == 0; });
    }

    ~Dispatcher()
    {
        for (auto& [addr, time] : process) {
            services[addr]->Ack(Service::State::Cancel);
            services[addr]->Ack(Service::State::Ready);
        }
        for (auto& [addr, transport] : transports) {
            transport->unregisterReceiver();
        }
        for (auto& [addr, service] : services) {
            service->unregisterTransmitter();
        }
    }

private:
    /**
    \brief Передача пакета на транспортный уровень для передачи по каналам связи
    \param addrService - адресс сервиса
    \param pipe - адресс канала передачи данных
    \param typeTransfer - тип передачи с подтверждением, потоковая
    \param dataService - полезные данные
    \retval статус отправки пакета через транспортный уровень (в случае неудачи, сервис запросивший передачу переводится в исходное состояние)
    \detail Функция вызывается с сервисного уровня
    */
    Status transmit(addressing_t pipe, addressing_t addrService, Service::Type typeTransfer, const Data_t& dataService)
    {
        auto it = transports.find(pipe);
        if (it != transports.end()) {
            auto& [addr, transport] = *it;
            services[addrService]->Ack(Service::State::Busy);
            switch (typeTransfer) {
            case Service::Type::TransmitConfirmed:
                if (transport->transmit(addrService, Transport::typePacket::TransmitConfirm, dataService) != 0) {
                    process.emplace(addrService, timeout);
                    services[addrService]->Ack(Service::State::TransmitComplete);
                    services[addrService]->Ack(Service::State::Await);
                    return Status::Success;
                }
                break;
            case Service::Type::ReceiveConfirmed:
                if (transport->transmit(addrService, Transport::typePacket::ReqReceiveConfirm, dataService) != 0) {
                    process.emplace(addrService, timeout);
                    services[addrService]->Ack(Service::State::Await);
                    return Status::Success;
                }
                break;
            case Service::Type::TransmitStreaming:
                if (transport->transmit(addrService, Transport::typePacket::TransmitStream, dataService) != 0) {
                    services[addrService]->Ack(Service::State::TransmitComplete);
                    services[addrService]->Ack(Service::State::Ready);
                    return Status::Success;
                }
                break;
            case Service::Type::ReceiveStreaming:
                if (transport->transmit(addrService, Transport::typePacket::ReqReceiveStream, dataService) != 0) {
                    services[addrService]->Ack(Service::State::Ready);
                    return Status::Success;
                }
                break;
            }
            services[addrService]->Ack(Service::State::Fail);
            services[addrService]->Ack(Service::State::Ready);
        }
        return Status::Fail;
    }

    /**
    \brief Приём пакета с транспортного уровня для передачи на сервисный уровень
    \param type - тип передачи с подтверждением, потоковая
    \param addrService - адресс сервиса
    \param dataService - полезные данные
    \detail Функция вызывается с транспортного уровня
    */
    Status receive(addressing_t pipe, addressing_t addrService, Transport::typePacket type, Data_t&& dataService)
    {
        auto it = services.find(addrService);
        if (it != services.end()) {
            auto& [addr, service] = *it;
            switch (type) {
            case Transport::typePacket::Ack:
                service->Ack(Service::State::AckComplete);
                service->Ack(Service::State::Ready);
                break;
            case Transport::typePacket::TransmitConfirm:
                service->receive(dataService);
                service->Ack(Service::State::ReceiveComplete);
                service->Ack(Service::State::Ready);
                break;
            case Transport::typePacket::TransmitStream:
                service->receive(dataService);
                service->Ack(Service::State::ReceiveComplete);
                service->Ack(Service::State::Ready);
                break;
            case Transport::typePacket::ReqReceiveConfirm:
                deffered.emplace(service, pipe, Service::Type::TransmitConfirmed);
                if (callbackDeffered) {
                    (*callbackDeffered)();
                }
                break;
            case Transport::typePacket::ReqReceiveStream:
                deffered.emplace(service, pipe, Service::Type::TransmitStreaming);
                if (callbackDeffered) {
                    (*callbackDeffered)();
                }
                break;
            }
            return Status::Success;
        }
        return Status::Fail;
    }

private:
    std::optional<std::function<void(void)>>                                      callbackDeffered; ///< Функция обратного вызова сигнализации наличия отложенных задач
    std::unordered_map<addressing_t, std::shared_ptr<Transport>>                  transports;       ///< Перечень зарегистрированных каналов связи
    std::unordered_map<addressing_t, std::shared_ptr<Service>>                    services;         ///< Перечень зарегистрированных сервисов
    std::unordered_map<addressing_t, size_t>                                      process;          ///< Перечень текущих операцих ввода/вывода с требованием подтверждения
    std::queue<std::tuple<std::shared_ptr<Service>, addressing_t, Service::Type>> deffered;         ///< Отложенные задачи передачи
};
