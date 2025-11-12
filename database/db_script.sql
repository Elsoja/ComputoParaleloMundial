-- podman run -d --replace --name=tournament_db --network development -e POSTGRES_PASSWORD=password -p 5432:5432 postgres:17.6-alpine3.22
-- podman exec -i tournament_db psql -U postgres -d postgres < db_script.sql

CREATE USER tournament_svc WITH PASSWORD 'password';
CREATE USER tournament_admin WITH PASSWORD 'password';

CREATE DATABASE tournament_db;

\connect tournament_db

grant all privileges on database tournament_db to tournament_admin;
grant all privileges on database tournament_db to tournament_svc;
grant usage on schema public to tournament_admin;
grant usage on schema public to tournament_svc;

GRANT SELECT ON ALL TABLES IN SCHEMA public TO tournament_admin;
GRANT DELETE ON ALL TABLES IN SCHEMA public TO tournament_admin;
GRANT UPDATE ON ALL TABLES IN SCHEMA public TO tournament_admin;
GRANT INSERT ON ALL TABLES IN SCHEMA public TO tournament_admin;
GRANT CREATE ON SCHEMA public TO tournament_admin;

\connect tournament_db tournament_admin

CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

CREATE TABLE TEAMS (
    id UUID DEFAULT uuid_generate_v4() PRIMARY KEY,
    document JSONB NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
CREATE UNIQUE INDEX team_unique_name_idx ON teams ((document->>'name'));

CREATE TABLE TOURNAMENTS (
    id UUID DEFAULT uuid_generate_v4() PRIMARY KEY,
    document JSONB NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
CREATE UNIQUE INDEX tournament_unique_name_idx ON TOURNAMENTS ((document->>'name'));

CREATE TABLE GROUPS (
                        id UUID DEFAULT uuid_generate_v4() PRIMARY KEY,
                        TOURNAMENT_ID UUID not null references TOURNAMENTS(ID),
                        document JSONB NOT NULL,
                        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
CREATE UNIQUE INDEX tournament_group_unique_name_idx ON GROUPS (tournament_id,(document->>'name'));

CREATE TABLE MATCHES (
    id SERIAL PRIMARY KEY,
    tournament_id INTEGER NOT NULL REFERENCES TOURNAMENTS(id) ON DELETE CASCADE,
    team1_id INTEGER REFERENCES TEAMS(id) ON DELETE SET NULL,
    team2_id INTEGER REFERENCES TEAMS(id) ON DELETE SET NULL,
    winner_id INTEGER REFERENCES TEAMS(id) ON DELETE SET NULL,
    team1_score INTEGER,
    team2_score INTEGER,
    phase VARCHAR(50) NOT NULL, -- GROUP_STAGE, QUARTERFINALS, SEMIFINALS, FINALS, etc.
    status VARCHAR(20) NOT NULL DEFAULT 'PENDING', -- PENDING, IN_PROGRESS, COMPLETED, CANCELLED
    match_number INTEGER NOT NULL,
    next_match_id INTEGER REFERENCES MATCHES(id) ON DELETE SET NULL,
    group_id INTEGER REFERENCES GROUPS(id) ON DELETE SET NULL,
    scheduled_date TIMESTAMP,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    CONSTRAINT valid_scores CHECK (
        (team1_score IS NULL AND team2_score IS NULL) OR 
        (team1_score IS NOT NULL AND team2_score IS NOT NULL)
    ),
    CONSTRAINT valid_winner CHECK (
        winner_id IS NULL OR 
        winner_id = team1_id OR 
        winner_id = team2_id
    )
);

-- Índices para mejorar performance
CREATE INDEX IF NOT EXISTS idx_matches_tournament ON MATCHES(tournament_id);
CREATE INDEX IF NOT EXISTS idx_matches_phase ON MATCHES(phase);
CREATE INDEX IF NOT EXISTS idx_matches_status ON MATCHES(status);
CREATE INDEX IF NOT EXISTS idx_matches_group ON MATCHES(group_id);
CREATE INDEX IF NOT EXISTS idx_matches_team1 ON MATCHES(team1_id);
CREATE INDEX IF NOT EXISTS idx_matches_team2 ON MATCHES(team2_id);
CREATE INDEX IF NOT EXISTS idx_matches_next ON MATCHES(next_match_id);

-- Trigger para actualizar updated_at
CREATE OR REPLACE FUNCTION update_matches_updated_at()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

CREATE TRIGGER IF NOT EXISTS trigger_update_matches_updated_at
    BEFORE UPDATE ON MATCHES
    FOR EACH ROW
    EXECUTE FUNCTION update_matches_updated_at();

-- Vista para ver partidos con información de equipos
CREATE OR REPLACE VIEW v_matches_detail AS
SELECT 
    m.id,
    m.tournament_id,
    t.name as tournament_name,
    t1.name as team1_name,
    t2.name as team2_name,
    tw.name as winner_name,
    m.team1_score,
    m.team2_score,
    m.phase,
    m.status,
    m.match_number,
    g.name as group_name,
    m.scheduled_date,
    m.created_at,
    m.updated_at
FROM MATCHES m
JOIN TOURNAMENTS t ON m.tournament_id = t.id
LEFT JOIN TEAMS t1 ON m.team1_id = t1.id
LEFT JOIN TEAMS t2 ON m.team2_id = t2.id
LEFT JOIN TEAMS tw ON m.winner_id = tw.id
LEFT JOIN GROUPS g ON m.group_id = g.id;

-- Función para obtener estadísticas de un equipo en el torneo
CREATE OR REPLACE FUNCTION get_team_tournament_stats(
    team_id_param INTEGER,
    tournament_id_param INTEGER
)

ALTER TABLE MATCHES 
ADD CONSTRAINT valid_score_range CHECK (
    (team1_score IS NULL OR (team1_score >= 0 AND team1_score <= 10)) AND
    (team2_score IS NULL OR (team2_score >= 0 AND team2_score <= 10))
);

RETURNS TABLE (
    matches_played BIGINT,
    wins BIGINT,
    losses BIGINT,
    draws BIGINT,
    goals_for BIGINT,
    goals_against BIGINT,
    goal_difference BIGINT,
    points BIGINT
) AS $$
BEGIN
    RETURN QUERY
    SELECT 
        COUNT(*) as matches_played,
        COUNT(*) FILTER (WHERE m.winner_id = team_id_param) as wins,
        COUNT(*) FILTER (WHERE m.winner_id != team_id_param AND m.winner_id IS NOT NULL) as losses,
        COUNT(*) FILTER (WHERE m.winner_id IS NULL AND m.status = 'COMPLETED') as draws,
        (COALESCE(SUM(CASE WHEN m.team1_id = team_id_param THEN m.team1_score ELSE 0 END), 0) +
         COALESCE(SUM(CASE WHEN m.team2_id = team_id_param THEN m.team2_score ELSE 0 END), 0)) as goals_for,
        (COALESCE(SUM(CASE WHEN m.team1_id = team_id_param THEN m.team2_score ELSE 0 END), 0) +
         COALESCE(SUM(CASE WHEN m.team2_id = team_id_param THEN m.team1_score ELSE 0 END), 0)) as goals_against,
        ((COALESCE(SUM(CASE WHEN m.team1_id = team_id_param THEN m.team1_score ELSE 0 END), 0) +
          COALESCE(SUM(CASE WHEN m.team2_id = team_id_param THEN m.team2_score ELSE 0 END), 0)) -
         (COALESCE(SUM(CASE WHEN m.team1_id = team_id_param THEN m.team2_score ELSE 0 END), 0) +
          COALESCE(SUM(CASE WHEN m.team2_id = team_id_param THEN m.team1_score ELSE 0 END), 0))) as goal_difference,
        (COUNT(*) FILTER (WHERE m.winner_id = team_id_param) * 3 +
         COUNT(*) FILTER (WHERE m.winner_id IS NULL AND m.status = 'COMPLETED')) as points
    FROM MATCHES m
    WHERE m.tournament_id = tournament_id_param
        AND m.status = 'COMPLETED'
        AND (m.team1_id = team_id_param OR m.team2_id = team_id_param);
END;
$$ LANGUAGE plpgsql;

GRANT SELECT ON ALL TABLES IN SCHEMA public TO tournament_svc;
GRANT DELETE ON ALL TABLES IN SCHEMA public TO tournament_svc;
GRANT UPDATE ON ALL TABLES IN SCHEMA public TO tournament_svc;
GRANT INSERT ON ALL TABLES IN SCHEMA public TO tournament_svc;
