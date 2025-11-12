-- La conexión a \c tournament_db con el usuario tournament_svc
-- se hace automáticamente con el comando podman exec.

-- Crear extensión para UUIDs (si no existe)
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

---
--- Tabla de Equipos (Teams)
---
CREATE TABLE TEAMS (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX idx_teams_doc ON TEAMS USING GIN(document);

---
--- Tabla de Torneos (Tournaments)
---
CREATE TABLE TOURNAMENTS (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX idx_tournaments_doc ON TOURNAMENTS USING GIN(document);

---
--- Tabla de Grupos (Groups)
---
CREATE TABLE GROUPS (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX idx_groups_doc ON GROUPS USING GIN(document);

---
--- Tabla de Partidos (Matches)
---
CREATE TABLE MATCHES (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX idx_matches_doc ON MATCHES USING GIN(document);

-- --- Trigger para 'updated_at' ---
CREATE OR REPLACE FUNCTION trigger_set_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.document = NEW.document || jsonb_build_object('updated_at', to_jsonb(CURRENT_TIMESTAMP));
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Aplicar el trigger (sintaxis corregida sin 'IF NOT EXISTS')
CREATE TRIGGER set_matches_timestamp
BEFORE UPDATE ON MATCHES
FOR EACH ROW
EXECUTE FUNCTION trigger_set_timestamp();

-- --- Permisos ---
-- El usuario 'tournament_svc' ya es el dueño de la BD,
-- pero por si acaso, le damos permisos explícitos.
GRANT SELECT, INSERT, UPDATE, DELETE ON ALL TABLES IN SCHEMA public TO tournament_svc;
GRANT USAGE, SELECT ON ALL SEQUENCES IN SCHEMA public TO tournament_svc;
GRANT EXECUTE ON ALL FUNCTIONS IN SCHEMA public TO tournament_svc;