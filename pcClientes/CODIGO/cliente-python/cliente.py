import grpc
import biblioteca_pb2 as pb
import biblioteca_pb2_grpc as pb_grpc

SERVER = "10.43.100.90:50051"


# ---------------- UTILIDADES ----------------

def mostrar_libros(lista):
    if not lista.libros:
        print("No hay libros.")
        return

    print("\n--- LIBROS ---")
    for l in lista.libros:
        print(f"ISBN: {l.isbn}")
        print(f"Título: {l.titulo}")
        print(f"Autor: {l.autor}")
        print(f"Editorial: {l.editorial}")
        print(f"Año: {l.anioPublicacion}")
        print(f"Estado: {l.estado}")
        print("---------------------------")


def mostrar_prestamos(lista):
    if not lista.prestamos:
        print("No hay préstamos.")
        return

    print("\n--- PRÉSTAMOS ---")
    for p in lista.prestamos:
        print(f"ID: {p.idPrestamo}")
        print(f"Usuario: {p.idUsuario}")
        print(f"ISBN: {p.isbn}")
        print(f"Fecha préstamo: {p.fechaPrestamo}")
        print(f"Fecha devolución: {p.fechaDevolucion}")
        print(f"Estado: {p.estado}")
        print("---------------------------")


# ---------------- CLIENTE ----------------

def menu_cliente(stub):
    while True:
        print("\n--- MENU CLIENTE ---")
        print("1) Ver libros")
        print("2) Buscar libro")
        print("3) Pedir préstamo")
        print("4) Devolver libro")
        print("5) Ver mis préstamos")
        print("0) Volver")

        op = input("Opción: ")

        try:
            if op == "1":
                resp = stub.VerLibros(pb.Vacio())
                mostrar_libros(resp)

            elif op == "2":
                isbn = input("ISBN: ")
                resp = stub.BuscarLibro(pb.LibroIdRequest(isbn=isbn))
                print(resp)

            elif op == "3":
                idu = int(input("ID usuario: "))
                isbn = input("ISBN: ")
                resp = stub.PrestarLibro(pb.PrestamoRequest(
                    idUsuario=idu,
                    isbn=isbn
                ))
                print(resp.mensaje)

            elif op == "4":
                idp = int(input("ID préstamo: "))
                resp = stub.DevolverLibro(pb.DevolucionRequest(
                    idPrestamo=idp
                ))
                print(resp.mensaje)

            elif op == "5":
                idu = int(input("ID usuario: "))
                resp = stub.VerLibrosPrestados(pb.UsuarioRequest(
                    idUsuario=idu
                ))
                mostrar_prestamos(resp)

            elif op == "0":
                break

        except grpc.RpcError as e:
            print("Error:", e.details())


# ---------------- ADMIN ----------------

def menu_admin(stub):
    key = input("Clave admin: ")

    while True:
        print("\n--- MENU ADMIN ---")
        print("1) Cargar base de datos")
        print("2) Agregar libro")
        print("3) Eliminar libro")
        print("4) Ver biblioteca")
        print("5) Ver libros repetidos")
        print("0) Volver")

        op = input("Opción: ")

        try:
            if op == "1":
                resp = stub.CargarBaseDeDatos(
                    pb.AdminVacioRequest(adminKey=key)
                )
                print(resp.mensaje)

            elif op == "2":
                isbn = input("ISBN: ")
                titulo = input("Título: ")
                autor = input("Autor: ")
                editorial = input("Editorial: ")
                anio = int(input("Año: "))

                libro = pb.LibroRequest(
                    isbn=isbn,
                    titulo=titulo,
                    autor=autor,
                    editorial=editorial,
                    anioPublicacion=anio
                )

                resp = stub.AgregarLibro(
                    pb.LibroAdminRequest(
                        adminKey=key,
                        libro=libro
                    )
                )
                print(resp.mensaje)

            elif op == "3":
                isbn = input("ISBN: ")
                resp = stub.EliminarLibro(
                    pb.LibroIdAdminRequest(
                        adminKey=key,
                        isbn=isbn
                    )
                )
                print(resp.mensaje)

            elif op == "4":
                resp = stub.VerBiblioteca(
                    pb.AdminVacioRequest(adminKey=key)
                )
                mostrar_libros(resp)

            elif op == "5":
                resp = stub.VerLibrosRepetidos(
                    pb.AdminVacioRequest(adminKey=key)
                )
                mostrar_libros(resp)

            elif op == "0":
                break

        except grpc.RpcError as e:
            print("Error:", e.details())


# ---------------- MAIN ----------------

def main():
    channel = grpc.insecure_channel(SERVER)
    stub = pb_grpc.BibliotecaServiceStub(channel)

    while True:
        print("\n====== SISTEMA BIBLIOTECA ======")
        print("1) Cliente")
        print("2) Admin")
        print("0) Salir")

        op = input("Opción: ")

        if op == "1":
            menu_cliente(stub)
        elif op == "2":
            menu_admin(stub)
        elif op == "0":
            break


if __name__ == "__main__":
    main()
