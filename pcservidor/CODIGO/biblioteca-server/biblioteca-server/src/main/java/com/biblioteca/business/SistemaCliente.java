package com.biblioteca.business;

import com.biblioteca.grpc.*;
import com.biblioteca.repository.*;
import com.biblioteca.util.AppException;

import java.sql.Connection;
import java.sql.SQLException;

public class SistemaCliente {

    private final LibroRepository libroRepo;
    private final PrestamoRepository prestamoRepo;
    private final UsuarioRepository usuarioRepo;

    public SistemaCliente(LibroRepository libroRepo, PrestamoRepository prestamoRepo, UsuarioRepository usuarioRepo) {
        this.libroRepo = libroRepo;
        this.prestamoRepo = prestamoRepo;
        this.usuarioRepo = usuarioRepo;
    }

    public ListaLibros verLibros(Connection conn) throws Exception {
        return libroRepo.findAll(conn);
    }

    public Libro buscarLibro(Connection conn, String isbn) throws Exception {
        if (isbn == null || isbn.isBlank()) throw AppException.invalid("ISBN vacío");
        Libro l = libroRepo.findByIsbn(conn, isbn);
        if (l == null) throw AppException.notFound("No existe ese ISBN");
        return l;
    }

    public ListaPrestamos verLibrosPrestados(Connection conn, int idUsuario) throws Exception {
        if (idUsuario <= 0) throw AppException.invalid("idUsuario inválido");
        if (!usuarioRepo.existsById(conn, idUsuario)) throw AppException.notFound("Usuario no existe");
        return prestamoRepo.findPrestamosActivosPorUsuario(conn, idUsuario);
    }

    public Respuesta pedirPrestado(Connection conn, int idUsuario, String isbn) throws Exception {
        if (idUsuario <= 0) throw AppException.invalid("idUsuario inválido");
        if (isbn == null || isbn.isBlank()) throw AppException.invalid("ISBN vacío");
        if (!usuarioRepo.existsById(conn, idUsuario)) throw AppException.notFound("Usuario no existe");

        String estado = libroRepo.getEstadoForUpdate(conn, isbn);
        if (estado == null) throw AppException.notFound("ISBN no existe");
        if (!estado.equals("disponible")) throw AppException.precondition("Libro no disponible");

        try {
            prestamoRepo.insertPrestamoActivo(conn, idUsuario, isbn);
        } catch (SQLException e) {
            // Puede disparar por uq_prestamo_isbn_activo (préstamo activo duplicado)
            throw AppException.precondition("No se puede prestar: ya existe préstamo activo para ese ISBN");
        }

        libroRepo.updateEstado(conn, isbn, "prestado");

        return Respuesta.newBuilder().setOk(true).setMensaje("Préstamo realizado").build();
    }

    public Respuesta devolverLibro(Connection conn, int idPrestamo) throws Exception {
        if (idPrestamo <= 0) throw AppException.invalid("idPrestamo inválido");

        String isbn = prestamoRepo.getIsbnPrestamoActivoForUpdate(conn, idPrestamo);
        if (isbn == null) throw AppException.notFound("Préstamo activo no existe");

        int rows = prestamoRepo.marcarDevuelto(conn, idPrestamo);
        if (rows == 0) throw AppException.precondition("No se pudo devolver: préstamo no está activo");

        libroRepo.updateEstado(conn, isbn, "disponible");

        return Respuesta.newBuilder().setOk(true).setMensaje("Devolución realizada").build();
    }
}
