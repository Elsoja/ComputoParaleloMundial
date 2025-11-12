# Match Messages - ActiveMQ

## Queue: `tournament.matches.register-score`

### Message: RegisterScore

**Formato:**
```json
{
  "message_type": "RegisterScore",
  "match_id": 1,
  "team1_score": 3,
  "team2_score": 1,
  "timestamp": "2024-01-15T14:30:00Z",
  "user_id": "admin123"
}
```

**Validaciones:**
- `team1_score` y `team2_score` deben estar entre 0 y 10
- Si el partido es de playoffs, no puede haber empate

**Respuesta (Topic: `tournament.matches.score-registered`):**
```json
{
  "message_type": "ScoreRegistered",
  "match_id": 1,
  "winner_id": 10,
  "phase": "GROUP_STAGE",
  "playoffs_generated": false,
  "timestamp": "2024-01-15T14:30:01Z"
}
```

---

## Queue: `tournament.matches.get-by-tournament`

### Message: GetMatchesByTournament

**Formato:**
```json
{
  "message_type": "GetMatchesByTournament",
  "tournament_id": 1,
  "request_id": "req-12345"
}
```

**Respuesta (Topic: `tournament.matches.list`):**
```json
{
  "message_type": "MatchesList",
  "request_id": "req-12345",
  "matches": [
    {
      "id": 1,
      "tournament_id": 1,
      "team1_id": 10,
      "team2_id": 11,
      "team1_score": 3,
      "team2_score": 1,
      "winner_id": 10,
      "phase": "GROUP_STAGE",
      "status": "COMPLETED"
    }
  ]
}
```