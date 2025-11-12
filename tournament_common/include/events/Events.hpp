#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace events {

// Evento base
class Event {
public:
    virtual ~Event() = default;
    virtual std::string GetType() const = 0;
};

// Evento: Se registró un puntaje
class ScoreRegisteredEvent : public Event {
private:
    std::string matchId;
    std::string tournamentId;
    int team1Score;
    int team2Score;
    std::string winnerId;
    std::string phase;

public:
    ScoreRegisteredEvent(const std::string& matchId, const std::string& tournamentId, int team1Score, 
                        int team2Score, const std::string& winnerId, const std::string& phase)
        : matchId(matchId), tournamentId(tournamentId), 
          team1Score(team1Score), team2Score(team2Score),
          winnerId(winnerId), phase(phase) {}

    std::string GetType() const override { return "ScoreRegistered"; }

    const std::string& MatchId() const { return matchId; }
    const std::string& TournamentId() const { return tournamentId; }
    int Team1Score() const { return team1Score; }
    int Team2Score() const { return team2Score; }
    const std::string& WinnerId() const { return winnerId; }
    const std::string& Phase() const { return phase; }
};

// Evento: Se registró un equipo a un grupo
class TeamRegisteredToGroupEvent : public Event {
private:
    std::string tournamentId;
    std::string groupId;
    std::string teamId;

public:
    TeamRegisteredToGroupEvent(const std::string& tournamentId, const std::string& groupId, const std::string& teamId)
        : tournamentId(tournamentId), groupId(groupId), teamId(teamId) {}

    std::string GetType() const override { return "TeamRegisteredToGroup"; }

    const std::string& TournamentId() const { return tournamentId; }
    const std::string& GroupId() const { return groupId; }
    const std::string& TeamId() const { return teamId; }
};

// Handler de eventos
using EventHandler = std::function<void(const Event&)>;

// Event Bus - Sistema de publicación/suscripción
class EventBus {
private:
    std::map<std::string, std::vector<EventHandler>> handlers;
    std::mutex mutex_;
    static std::shared_ptr<EventBus> instance;
    static std::mutex instanceMutex;

    EventBus() = default;

public:
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    static std::shared_ptr<EventBus> Instance() {
        std::lock_guard<std::mutex> lock(instanceMutex);
        if (!instance) {
            instance = std::shared_ptr<EventBus>(new EventBus());
        }
        return instance;
    }

    void Subscribe(const std::string& eventType, EventHandler handler) {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers[eventType].push_back(handler);
    }

    void Publish(const Event& event) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto eventType = event.GetType();
        if (handlers.find(eventType) != handlers.end()) {
            for (const auto& handler : handlers[eventType]) {
                handler(event);
            }
        }
    }

    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        handlers.clear();
    }


}; 

// Definición de estáticos fuera de la clase
inline std::shared_ptr<EventBus> EventBus::instance = nullptr;
inline std::mutex EventBus::instanceMutex;

} // namespace events

#endif // EVENTS_HPP