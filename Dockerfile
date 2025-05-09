# Imagen base con Java y APT disponible
FROM openjdk:21-bookworm

# Instala compilador de C y libcjson
RUN apt update && apt install -y build-essential libcjson-dev

# Establece el directorio de trabajo
WORKDIR /app

# Copia todos los archivos al contenedor
COPY . .

# Compila la librería compartida en C
RUN gcc -I /usr/local/openjdk-21/include \
         -I /usr/local/openjdk-21/include/linux \
         -shared -fPIC -o libRoundRobin/libRoundRobin.so libRoundRobin/libRoundRobin_RoundRobin.c -lcjson

# Compila el código Java (puedes omitirlo si lo haces fuera del contenedor)
RUN javac aplicacion/Aplicacion.java

# Ejecuta la aplicación Java que usa la librería nativa
CMD ["java", "-cp", ".:./libRoundRobin", "-Djava.library.path=./libRoundRobin", "aplicacion.Aplicacion"]

