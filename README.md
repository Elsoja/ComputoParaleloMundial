VAMOS  PROBANDO
DB script
````
sudo podman run -d --replace --name=tournament_db --network development -e POSTGRES_PASSWORD=password -p 5432:5432 postgres:17.6-alpine3.22
sudo podman exec -i tournament_db psql -U postgres -d postgres < database/db_script.sql
````

activemq
````
HEAD
podman run -d --replace --name ZenonCitio --network developer -p 61616:61616 -p 8161:8161 -p 5672:5672  apache/activemq-classic:6.1.7
````

podman run -d --replace --name artemis --network development -p 61616:61616 -p 8161:8161 -p 5672:5672  apache/activemq-classic:6.1.7
````
bebad83aacc5af989b5fdd58453042e5a0f16f80
