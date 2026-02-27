Sistema Distribuido de Gestión de Préstamos
Taller – Introducción a Sistemas Distribuidos

-Descripción-

Este repositorio contiene la implementación de un sistema distribuido cliente-servidor desarrollado como taller académico para la asignatura Introducción a Sistemas Distribuidos.

El sistema permite gestionar préstamos de libros mediante comunicación remota usando gRPC. Los componentes del sistema se ejecutan en máquinas virtuales independientes, garantizando separación física real entre:

*Cliente
*Servidor
*Base de datos

La comunicación se realiza mediante gRPC sobre HTTP/2, utilizando Protocol Buffers como contrato de intercambio de datos.


-Arquitectura-

El sistema está compuesto por tres nodos principales:

--------------------------------------
Cliente (Python)
        │
        v
Servidor gRPC (Java 17)
        │
        v
PostgreSQL (Base de Datos Remota)
--------------------------------------

Características arquitectónicas:

*Separación física de nodos
*Comunicación remota estructurada
*Interoperabilidad entre lenguajes (Java <-> Python)
*Persistencia en base de datos remota
*Arquitectura por capas en el servidor


-Tecnologías Utilizadas-

*Java 17
*Maven
*gRPC
*Protocol Buffers
*Python 3
*PostgreSQL
*Linux (Máquinas Virtuales)

-Estructura del Proyecto-
	-Servidor (Java)-

		*Implementación del servicio gRPC
		*Lógica de negocio
		*Acceso a datos (DAO)
		*Conexión a PostgreSQL

	-Cliente (Python)-

		*Stub generado desde archivo .proto
		*Invocación de servicios remotos
		*Manejo de respuestas

	-Base de Datos (PostgreSQL)-

		*Tablas principales:
			*usuario
			*libro
			*prestamo

		Incluye:

			*Claves primarias
			*Claves foráneas
			*Restricciones CHECK
			*Restricciones UNIQUE
			*Tipos ENUM

-Funcionalidades-

*Registrar libros
*Consultar libros disponibles
*Buscar libros por criterios
*Registrar préstamos
*Consultar préstamos activos
*Devolver libros
*Eliminar libros

-Validaciones Implementadas-

*ISBN único
*Un libro no puede tener más de un préstamo activo
*No se permite préstamo para usuario inexistente
*Restricción sobre año de publicación
*Manejo estructurado de errores
*Validación de integridad referencial

-Puertos Utilizados-
---------------------------------
Servicio	|	Puerto
gRPC Server	|	50051
PostgreSQL	|	5432
---------------------------------

-Ejecución-

	-Iniciar PostgreSQL-
	Asegurar que el servidor de base de datos esté activo y configurado para conexiones remotas.

	-Ejecutar Servidor gRPC-
	mvn clean install
	mvn exec:java

	-Ejecutar Cliente Python-
	python cliente.py

-Objetivo Académico-

Este taller tiene como propósito aplicar los fundamentos de los sistemas distribuidos mediante:

*Implementación real de comunicación RPC
*Separación física de componentes
*Configuración de red entre máquinas virtuales
*Integración de tecnologías heterogéneas
*Persistencia remota de datos

-------------------------------------------------------------
Autores:

*Juan FElipe Gutierrez				|
*Juan Santamaría Orjuela					|
*Juan David Daza Caro						|
*Juan David Rincón Muñoz					|
-------------------------------------------------------------
*Ingeniería de Sistemas						|
*Asignatura: Introducción a Sistemas Distribuidos		|
-------------------------------------------------------------
--Comandos
-- Conexion base de datos
psql -U postgres -d biblioteca
-- Ver tablas
\dt

-- Ver estructura
\d usuario
\d libro
\d prestamo

-- Consultar información
SELECT * FROM usuario;
SELECT * FROM libro;
SELECT * FROM prestamo;

-- Ver libros disponibles
SELECT * FROM libro WHERE estado = 'disponible';

-- Ver préstamos activos
SELECT * FROM prestamo WHERE estado = 'activo';

-- Ver conexiones activas del sistema
SELECT client_addr, state
FROM pg_stat_activity
WHERE datname = 'biblioteca';

-- Salir
\q
-- verificar que ets econectado
ss -tulnp | grep 5432
-- compilar el proyecto 
mvn -q clean compile
-- ejecutar el servidor
mvn -q exec:java -Dexec.mainClass="com.biblioteca.ServerMain"
-- verificar que el servidor este escuchando las peticiones
ss -tulmp | grep 50051
--ver conexiones activas de la base d edtaos
ss -tn | grep 5432

--cliente
--correr el codigo
python3 cliente.py
