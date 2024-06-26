#ifndef TRANSPORT_HPP
#define TRANSPORT_HPP

#include <array>
#include <functional>
#include <initializer_list>
#include <memory>
#include <optional>
#include <unordered_set>

#include "Typedef.hpp"

/*!
\Brief Реализация транспортного уровня
\detail Формирование пакета и отправка через канал связи, так-же приём потока данных из канала связи декодирование и проверка целостности
*/
class Transport final
{
    constexpr static std::initializer_list<uint8_t> sign{ 0xAA, 0x55 };

    enum class State
    {
        Sign,
        Count,
        Pipe,
        Service,
        Type,
        Data,
        Check
    };

    void getCRC(const uint8_t data, uint16_t& crc)
    {
        crc ^= data;
        for (size_t i = 0; i < 8; i++) {
            if (crc & 0x01) {
                crc = (crc >> 1) ^ 0xA001;
            }
            else {
                crc = crc >> 1;
            }
        }
    }

public:
    enum class typePacket : addressing_t
    {
        Ack = 0U, ///< Подтверждение
        TransmitConfirm = 2U, ///< Передача данных с подтверждением
        TransmitStream = 3U, ///< Потоковая передача данных
        ReqReceiveConfirm = 4U, ///< Запрос передачи данных с подтверждением
        ReqReceiveStream = 5U  ///< Запрос передачи данных в потоковом режиме
    };

    /**
    \Brief Прототип функции обратного вызова.
    \detail Используется для передачи исходящего пакета по физическим каналам связи
    \retval статус передачи, 0 в случае удачной отправки
    */
    using callback_t = std::function<uint8_t(std::unique_ptr<addressing_t[]>, size_t)>;

    Transport(const addressing_t addr, callback_t callback) : addrPipe{ addr }, transmitData{ callback } {}

    inline addressing_t getAddress() const { return addrPipe; }

    inline operator addressing_t() const { return getAddress(); }

    void registerReceiver(std::function<void(addressing_t, typePacket, Data_t&&)> callback)
    {
        receiveData = callback;
    }

    void unregisterReceiver()
    {
        receiveData = std::nullopt;
    }

    /*!
    \Brief Приём данных из интерфейса передачи
    \param value - байт данных принятый через интерфейс передачи
    \return Статус обработки потока данных success или fail в случае ошибки декодирования структуры пакета
    \detail Приём потока байтов, декодирование структуры пакета, выделение полезных данных, передача на уровень диспетчеризации и уведомление отправителя об успешном получении пакета.
    Вызывается пользователем системы при получении данных по физическому каналу связи
    */
    Status receive(uint8_t value)
    {
        switch (state) {
        case State::Sign:
            if (value == *(sign.begin() + stage)) {
                stage++;
                if (stage == sign.size()) {
                    state = State::Count;
                    count = 0;
                    stage = 0;
                }
            }
            else {
                stage = 0;
            }
            break;
        case State::Count:
            getCRC(value, receiveCRC);
            count |= value << 8 * stage;
            stage++;
            if (stage == sizeof(count)) {
                state = State::Pipe;
                stage = 0;
                data.clear();
                data.reserve(count);
            }
            break;
        case State::Pipe:
            getCRC(value, receiveCRC);
            if (value == addrPipe) {
                state = State::Service;
            }
            else {
                state = State::Sign;
                return Status::Fail;
            }
            break;
        case State::Service:
            getCRC(value, receiveCRC);
            receiveAddr = value;
            state = State::Type;
            break;
        case State::Type:
            getCRC(value, receiveCRC);
            receiveType = static_cast<typePacket>(value);
            if (count != 0) {
                state = State::Data;
            }
            else {
                state = State::Check;
            }
            stage = 0;
            break;
        case State::Data:
            getCRC(value, receiveCRC);
            data.push_back(value);
            stage++;
            if (stage == count) {
                state = State::Check;
                stage = 0;
            }
            break;
        case State::Check:
            if ((receiveCRC & 0xFF << 8 * stage) == (value << 8 * stage)) {
                stage++;
                if (stage == sizeof(receiveCRC)) {
                    if (receiveData) {
                        if (receiveType == typePacket::ReqReceiveConfirm || receiveType == typePacket::TransmitConfirm) {
                            transmit(receiveAddr, typePacket::Ack, Data_t{});
                        }
                        (*receiveData)(receiveAddr, receiveType, std::move(data));
                    }
                    state = State::Sign;
                    receiveCRC = 0xFFFF;
                    stage = 0;
                }
            }
            else {
                state = State::Sign;
                receiveCRC = 0xFFFF;
                stage = 0;
                return Status::Fail;
            }
            break;
        }
        return Status::Success;
    }

    /*!
    \Brief Приём данных из интерфейса передачи данных
    \param pData - указатель на блок данных
    \param size - размер блока
    \return Статус обработки потока данных success или fail в случае ошибки декодирования структуры пакета
    \detail Приём блока данных и передача в функцию потоковой обработки
    Вызывается пользователем системы при получении данных по физическому каналу связи
    */
    Status receive(uint8_t* pData, size_t size)
    {
        Status st{ Status::Success };
        for (size_t idx = 0; idx < size; ++idx) {
            Status tmp = receive(*(pData + idx));
            if (tmp == Status::Fail) {
                st = tmp;
            }
        }
        return st;
    }

    /*!
    \Brief Приём данных из интерфейса передачи данных
    \param pData - интеллектуальный указатель на данные
    \param size - размер данных
    \return Статус обработки потока данных success или fail в случае ошибки декодирования структуры пакета
    \detail Приём блока данных и передача в функцию потоковой обработки
    Вызывается пользователем системы при получении данных по физическому каналу связи
    */
    Status receive(std::unique_ptr<uint8_t[]> pData, size_t size)
    {
        return receive(pData.get(), size);
    }

    /*!
    \Brief Передача данных в канал связи
    \param addrService - адрес сервиса
    \param type - тип передачи (пакет с подтверждением, подтверждение, пакет без подтверждения)
    \param dataService - контейнер (вектор) содержащий блок данных
    \return количество переданных байт
    \detail Формирование структуры пакета и передача в канал связи
    */
    uint8_t transmit(addressing_t addrService, typePacket type, const Data_t& dataService)
    {
        const size_t sizePacket = sign.size() + dataService.size() + 7U;
        auto         pData = std::make_unique<addressing_t[]>(sizePacket);

        size_t idx{ 0 };

        uint16_t transmitCRC{ 0xFFFF };

        for (const addressing_t& item : sign) {
            pData[idx++] = item;
        }
        auto count = dataService.size();
        getCRC(static_cast<uint8_t>(count & 0xFF), transmitCRC);
        getCRC(static_cast<uint8_t>(count >> 8), transmitCRC);
        pData[idx++] = static_cast<uint8_t>(count & 0xFF);
        pData[idx++] = static_cast<uint8_t>(count >> 8);
        getCRC(addrPipe, transmitCRC);
        pData[idx++] = addrPipe;
        getCRC(addrService, transmitCRC);
        pData[idx++] = addrService;
        getCRC(static_cast<uint8_t>(type), transmitCRC);
        pData[idx++] = static_cast<uint8_t>(type);
        for (const addressing_t& item : dataService) {
            getCRC(item, transmitCRC);
            pData[idx++] = item;
        }
        pData[idx++] = static_cast<uint8_t>(transmitCRC & 0xFF);
        pData[idx++] = static_cast<uint8_t>(transmitCRC >> 8);

        return transmitData(std::move(pData), idx);
    }

    ~Transport() = default;

private:
    Data_t       data; ///< Контейнер промежуточного хранения принятых данных
    size_t       stage{ 0 };
    uint16_t     count{ 0 };
    typePacket   receiveType;
    addressing_t receiveAddr;
    State        state{ State::Sign };
    uint16_t     receiveCRC{ 0xFFFF };

    const addressing_t                                                      addrPipe;                  ///< Номер канала приема/передачи
    const callback_t                                                        transmitData;              ///< Функция обратного вызова для отправки данных через интерфейс передачи
    std::optional<std::function<void(addressing_t, typePacket, Data_t&&)>> receiveData{ std::nullopt }; ///< Функция обратного вызова для обработки принятого пакета
};

#endif // TRANSPORT_HPP
