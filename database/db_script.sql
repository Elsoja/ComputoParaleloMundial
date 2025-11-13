-- ====================================
-- Script de inicialización de la BD
-- ====================================

-- 1. Crear el usuario 'tournament_svc' (si no existe)
DO $$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_catalog.pg_user WHERE usename = 'tournament_svc') THEN
        CREATE USER tournament_svc WITH PASSWORD 'password';
    END IF;
END
$$;

-- 2. Crear la base de datos (si no existe)
SELECT 'CREATE DATABASE tournament_db OWNER tournament_svc'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = 'tournament_db')\gexec

-- 3. Conectar a la base de datos tournament_db
\c tournament_db

-- 4. Crear extensión para UUIDs (si no existe)
CREATE EXTENSION IF NOT EXISTS "uuid-ossp";

-- ====================================
-- Tablas
-- ====================================

---
--- Tabla de Equipos (Teams)
---
CREATE TABLE IF NOT EXISTS TEAMS (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_teams_doc ON TEAMS USING GIN(document);

---
--- Tabla de Torneos (Tournaments)
---
CREATE TABLE IF NOT EXISTS TOURNAMENTS (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_tournaments_doc ON TOURNAMENTS USING GIN(document);

---
--- Tabla de Grupos (Groups)
---
CREATE TABLE IF NOT EXISTS GROUPS (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_groups_doc ON GROUPS USING GIN(document);

---
--- Tabla de Partidos (Matches)
---
CREATE TABLE IF NOT EXISTS MATCHES (
    id UUID PRIMARY KEY DEFAULT uuid_generate_v4(),
    document JSONB NOT NULL
);
CREATE INDEX IF NOT EXISTS idx_matches_doc ON MATCHES USING GIN(document);

-- ====================================
-- Trigger para 'updated_at'
-- ====================================
CREATE OR REPLACE FUNCTION trigger_set_timestamp()
RETURNS TRIGGER AS $$
BEGIN
    NEW.document = NEW.document || jsonb_build_object('updated_at', to_jsonb(CURRENT_TIMESTAMP));
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Aplicar el trigger
DROP TRIGGER IF EXISTS set_matches_timestamp ON MATCHES;
CREATE TRIGGER set_matches_timestamp
BEFORE UPDATE ON MATCHES
FOR EACH ROW
EXECUTE FUNCTION trigger_set_timestamp();

-- ====================================
-- Permisos
-- ====================================
GRANT ALL PRIVILEGES ON DATABASE tournament_db TO tournament_svc;
GRANT ALL PRIVILEGES ON ALL TABLES IN SCHEMA public TO tournament_svc;
GRANT ALL PRIVILEGES ON ALL SEQUENCES IN SCHEMA public TO tournament_svc;
GRANT EXECUTE ON ALL FUNCTIONS IN SCHEMA public TO tournament_svc;

-- Permisos por defecto para objetos futuros
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON TABLES TO tournament_svc;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT ALL ON SEQUENCES TO tournament_svc;
ALTER DEFAULT PRIVILEGES IN SCHEMA public GRANT EXECUTE ON FUNCTIONS TO tournament_svc;

-- Mensaje de confirmación
\echo 'Base de datos inicializada correctamente ✓'