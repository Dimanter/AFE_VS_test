#ifndef SERVICE_HPP
#define SERVICE_HPP

#include <functional>
#include <optional>

#include "Typedef.hpp"

#include "FactoryWrapper.hpp"

class Service
{
public:
    enum class Type
    {
        TransmitConfirmed, ///< Запрос передачи данных с подтверждением
        TransmitStreaming, ///< Запрос передачи данных в потоковом режиме
        ReceiveConfirmed,  ///< Запрос получения данных с подтверждением
        ReceiveStreaming   ///< Запрос получения данных в потоковом режиме
    };

    enum class State
    {
        Ready,            ///< Готов к приёму или передачи данных
        Busy,             ///< Занят, попытка отправить пакет
        Await,            ///< Ожидание подтверждения
        AckComplete,      ///< Подтверждение получено
        ReceiveComplete,  ///< Данные получены
        TransmitComplete, ///< Данные отправлены
        Timeout,          ///< Время ожидания подтверждения истекло
        Fail,             ///< Не удалось передать пакет
        Cancel            ///< Запрос отменён
    };

    using serviceID_t = addressing_t;
    using eventCallback_t = std::function<void(const State&)>;

    Service(eventCallback_t call, addressing_t serv, uint16_t size = 0) : service{ serv }, event{ call }
    {
        buffer->reserve(size);
    }

    State getStatus() const { return state; }

    addressing_t getAddress() const { return service; }

    operator addressing_t() const { return getAddress(); }

    /**
    \brief Регистрация функции передачи пакета на диспетчерский уровень
    \param callback указатель на функцию обратного вызова
    \detail При регистрации сервиса в диспетчере метод вызывается для сохранения функция обратного вызова
     */
    void registerTransmitter(std::function<Status(addressing_t, Type, const Data_t&)> callback)
    {
        transmitPayload = callback;
    }

    void unregisterTransmitter()
    {
        transmitPayload = std::nullopt;
    }

    /*!
    \Brief Оповещение изменения состояния
    \param st - новое состояние
    \detail Оповещение происходит при отправке, получении, невозможности отправки по таймауту или при удалении диспетчера. Вызывается диспетчерским уровнем
    */
    void Ack(const State& st)
    {
        state = st;
        event(st);
    }

    /*!
    \Brief Приём данных на запись с диспетчерского уровня
    \param data - полезные данные
    \detail Вызывается с диспетчерского уровня при получении и валидации данных, в случае пересылки с подтверждением происходит автоматическое уведомление отправителя (на транспортном уровне)
    */
    void receive(const Data_t& data)
    {
        *buffer = data;
    }

    /*!
    \Brief Отправка данных
    \param pipe - идентификатор канала по которому необходимо отправить данные
    \param typeTransfer - тип передачи с подтверждением или потоковая
    \return Статус отправки данных success или fail в случае ошибки по причине занятости сервиса, нет зарегистрированного диспетчера
    \detail Вызывается пользователем системы при необходимости отправки данных
    */
    Status transmit(addressing_t pipe, Type typeTransfer)
    {
        Status stat{ Status::Fail };
        if (transmitPayload && state == State::Ready) {
            switch (typeTransfer) {
            case Type::TransmitConfirmed:
                stat = (*transmitPayload)(pipe, typeTransfer, buffer->data());
                break;
            case Type::ReceiveConfirmed:
                stat = (*transmitPayload)(pipe, typeTransfer, Data_t{});
                break;
            case Type::TransmitStreaming:
                stat = (*transmitPayload)(pipe, typeTransfer, buffer->data());
                break;
            case Type::ReceiveStreaming:
                stat = (*transmitPayload)(pipe, typeTransfer, Data_t{});
                break;
            }
            return Status::Success;
        }
        return stat;
    }

    virtual ~Service() = default;

protected:
    std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>();
    FactoryWrapper          factory{ buffer };

private:
    State                                                                    state{ State::Ready };
    const serviceID_t                                                        service;
    eventCallback_t                                                          event;                         ///< Функция обратного вызова для уведомленя об изменении состояния
    std::optional<std::function<Status(addressing_t, Type, const Data_t&)>> transmitPayload{ std::nullopt }; ///< Функция обратного вызова для отправки полезных данных на диспетчерский уровень
};

#endif // SERVICE_HPP
