# Taller 1 - Sistema de Biblioteca Distribuido

Sistema cliente-servidor para gestión de una biblioteca usando gRPC. El servidor está implementado en Java y el cliente en Python, con PostgreSQL como base de datos.

## Arquitectura

- **Base de datos:** PostgreSQL (máquina de base de datos)
- **Servidor:** Java 17 + Maven + gRPC (máquina servidor)
- **Cliente:** Python 3 + gRPC (máquina cliente)

La comunicación entre cliente y servidor se realiza mediante gRPC con Protocol Buffers (`.proto`) como contrato de la API.

---

## Fase 1 - Configuración de la base de datos

1. Actualizar paquetes del sistema:
```bash
sudo apt update
```

2. Instalar PostgreSQL:
```bash
sudo apt install postgresql
```

3. Ingresar a PostgreSQL:
```bash
sudo -u postgres psql
```

4. Crear la base de datos:
```sql
CREATE DATABASE biblioteca;
```

5. Conectarse a la base de datos:
```sql
\c biblioteca
```

6. Ejecutar el script de creación de tablas (archivo `fase1scriptsCreacionDeTablas`). Este script crea:
   - Enums: `estado_libro`, `estado_prestamo`
   - Tablas: `usuario`, `libro`, `prestamo` (con sus foreign keys y constraints)
   - Índices para consultas y concurrencia

7. Verificar la creación:
```sql
\d    -- ver tablas
\dT   -- ver tipos/enums
\dt   -- ver tablas con detalle
```

8. Insertar datos de prueba (archivo `inserts usuario`).

---

## Fase 2 - Configuración del servidor (Java + gRPC)

### Instalación de dependencias

1. Actualizar paquetes:
```bash
sudo apt update
```

2. Instalar Java 17 y Maven:
```bash
sudo apt install openjdk-17-jdk -y
sudo apt install maven -y
```

### Creación del proyecto

3. Crear la carpeta del proyecto (ej: `Documents/a`) y dentro de ella generar el proyecto Maven:
```bash
mvn archetype:generate
```

4. Actualizar el `pom.xml` con las dependencias necesarias:
   - gRPC (netty-shaded, protobuf, stub)
   - Servlets (anotaciones Java)
   - Driver PostgreSQL
   - Plugin de protobuf (compilador y generación de código gRPC)
   - Plugin de Maven para ejecución

### Estructura del servidor

El código fuente se organiza en `src/main/java/com/biblioteca/`:

- `ServerMain.java` — Arranca el servidor en el puerto 50051 y lo deja escuchando
- `service/` — Recibe solicitudes gRPC, llama a la capa de negocio y devuelve respuestas
- `business/` — Lógica del sistema (roles admin y cliente)
- `repository/` — Acceso a datos, consultas SQL puras
- `db/` — Conexión a PostgreSQL
- `util/` — Manejo de excepciones

El archivo `.proto` se ubica en `src/main/proto/` y define el contrato de la API (funciones RPC y mensajes).

### Compilación y ejecución

5. Compilar el proyecto:
```bash
mvn clean compile
```

6. Iniciar el servidor:
```bash
mvn -q exec:java -Dexec.mainClass="com.biblioteca.ServerMain"
```

7. Verificar que el puerto esté escuchando:
```bash
ss -tulnp | grep 50051
```

### Configuración de acceso remoto a PostgreSQL

8. Asignar contraseña al usuario postgres:
```bash
sudo -u postgres psql
\password
```
Contraseña: `postgres`

9. Actualizar `Db.java` con la contraseña de la base de datos y recompilar:
```bash
mvn clean compile
```

10. Editar la configuración de PostgreSQL en `/etc/postgresql/14/main/`:
    - En `postgresql.conf`, cambiar: `listen_addresses = '*'`
    - En `pg_hba.conf`, agregar al final: `host    biblioteca    postgres    <IP_SERVIDOR>/32    md5`

11. Reiniciar PostgreSQL y verificar:
```bash
sudo systemctl restart postgresql
ss -tulnp | grep 5432
```

12. Desde el servidor, verificar conectividad a la base de datos:
```bash
nc -vz <IP_BASE_DATOS> 5432
```

13. Instalar cliente PostgreSQL en el servidor para pruebas:
```bash
sudo apt install postgresql-client -y
psql -h <IP_BASE_DATOS> -U postgres -d biblioteca
```

---

## Fase 3 - Configuración del cliente (Python)

1. Actualizar paquetes e instalar pip:
```bash
sudo apt update
sudo apt install python3-pip -y
```

2. Crear la carpeta del proyecto (ej: `Documents/cliente-python`).

3. Con el servidor activo, copiar el archivo `.proto` desde el servidor:
```bash
scp estudiante@<IP_SERVIDOR>:/home/estudiante/Documents/a/biblioteca-server/src/main/proto/biblioteca.proto .
```

4. Generar el código gRPC en Python:
```bash
python3 -m grpc_tools.protoc -I. --python_out=. --grpc_python_out=. biblioteca.proto
```

5. Crear el archivo `cliente.py` con la lógica del cliente.

6. Ejecutar el cliente:
```bash
python3 cliente.py
```

Repetir estos pasos en cada máquina cliente que se quiera configurar.
