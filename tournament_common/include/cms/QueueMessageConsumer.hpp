#ifndef QUEUE_MESSAGE_CONSUMER_HPP
#define QUEUE_MESSAGE_CONSUMER_HPP

#include "cms/ConnectionManager.hpp"
#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include <thread> // Para hilos
#include <chrono> // Para sleep

namespace cms { // Asegurarse de que esté dentro del namespace

class QueueMessageConsumer {
private:
    std::shared_ptr<ConnectionManager> connectionManager;
    std::function<void(const std::string&)> messageCallback;

public:
    explicit QueueMessageConsumer(std::shared_ptr<ConnectionManager> manager)
        : connectionManager(std::move(manager)) {}

    virtual ~QueueMessageConsumer() = default;

    // ✅ CAMBIO: Añadir el método SetMessageCallback
    void SetMessageCallback(std::function<void(const std::string&)> callback) {
        messageCallback = std::move(callback);
    }

    // ✅ CAMBIO: Añadir el método Start
    virtual void Start(const std::string& queueName) {
        std::cout << "Iniciando listener en la cola: " << queueName << std::endl;
        
        // --- INICIO DE LÓGICA SIMULADA ---
        // (Deberás reemplazar esto con tu lógica real de ActiveMQ)
        std::thread([this, queueName]() {
            while(true) { // Simular escucha continua
                std::this_thread::sleep_for(std::chrono::seconds(10));
                if (messageCallback) {
                    std::cout << "Mensaje (simulado) recibido en " << queueName << "!" << std::endl;
                    // Simular un evento de torneo creado
                    messageCallback("{\"event_type\":\"tournament.created\", \"id\":\"simulated-id-123\"}");
                }
            }
        }).detach();
        // --- FIN DE LÓGICA SIMULADA ---
    }
};

} // namespace cms

#endif // QUEUE_MESSAGE_CONSUMER_HPP