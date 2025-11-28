# ComputoParaleloMundial

Proyecto de ejemplo para el curso de Cómputo Paralelo.

## Prerrequisitos

Asegúrate de tener instaladas las siguientes herramientas:

- **CMake** (v3.28 o superior)
- **Ninja** o **Make**
- **Compilador C++** con soporte para C++23 (GCC 13+ o Clang 16+)
- **Podman** (o Docker) para la infraestructura

## Construcción del Proyecto

Para compilar el proyecto, ejecuta los siguientes comandos desde la raíz del repositorio:

```bash
# Configurar el proyecto (si no se ha hecho antes)
cmake -S . -B out/build/clang -G Ninja

# Compilar
cmake --build out/build/clang
```

## Configuración de Infraestructura

El proyecto requiere una base de datos PostgreSQL y un broker ActiveMQ. Puedes levantarlos usando Podman:

### 1. Crear red de desarrollo
```bash
podman network create development
```

### 2. Base de Datos (PostgreSQL)
```bash
# Iniciar contenedor
podman run -d --replace --name=tournament_db --network development \
  -e POSTGRES_PASSWORD=password -p 5432:5432 \
  docker.io/library/postgres:17.6-alpine3.22

# Esperar unos segundos y luego inicializar el esquema
podman exec -i tournament_db psql -U postgres -d postgres < database/db_script.sql
```

### 3. ActiveMQ
```bash
# Iniciar contenedor
podman run -d --replace --name artemis --network development \
  -p 61616:61616 -p 8161:8161 -p 5672:5672 \
  docker.io/apache/activemq-classic:6.1.7
```

## Ejecución

Una vez compilado y con la infraestructura lista, puedes ejecutar el consumidor:

```bash
cd out/build/clang/tournament_consumer
./tournament_consumer
```

El programa debería mostrar:
```
✅ All listeners started
📡 Listening to:
   - 'tournament.created' (External via ActiveMQ)
   - 'ScoreRegistered' (Internal via EventBus)
```
