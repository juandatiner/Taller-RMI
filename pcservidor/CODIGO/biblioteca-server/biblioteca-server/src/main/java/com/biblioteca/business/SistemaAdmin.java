package com.biblioteca.business;

import com.biblioteca.grpc.*;
import com.biblioteca.repository.LibroRepository;
import com.biblioteca.util.AppException;

import java.sql.Connection;

public class SistemaAdmin {

    private static final String ADMIN_KEY = "admin";

    private final LibroRepository libroRepo;

    public SistemaAdmin(LibroRepository libroRepo) {
        this.libroRepo = libroRepo;
    }

    public void validarAdmin(String adminKey) {
        if (adminKey == null || !adminKey.equals(ADMIN_KEY)) {
            throw AppException.denied("No autorizado (adminKey inválida)");
        }
    }

    public Respuesta cargarBaseDeDatos(Connection conn) throws Exception {
        int inserted = libroRepo.seedLibrosSiVacio(conn);
        return Respuesta.newBuilder()
                .setOk(true)
                .setMensaje("Carga/seed OK. Libros insertados: " + inserted)
                .build();
    }

    public Respuesta agregarLibro(Connection conn, LibroRequest libro) throws Exception {
        if (libro.getIsbn().isBlank() || libro.getTitulo().isBlank() || libro.getAutor().isBlank()) {
            throw AppException.invalid("isbn/titulo/autor no pueden ser vacíos");
        }
        int rows = libroRepo.insertLibro(conn, libro);
        return Respuesta.newBuilder()
                .setOk(rows > 0)
                .setMensaje(rows > 0 ? "Libro agregado" : "No se agregó el libro")
                .build();
    }

    public Respuesta eliminarLibro(Connection conn, String isbn) throws Exception {
        if (isbn == null || isbn.isBlank()) throw AppException.invalid("ISBN vacío");
        int rows = libroRepo.deleteLibro(conn, isbn);
        return Respuesta.newBuilder()
                .setOk(rows > 0)
                .setMensaje(rows > 0 ? "Libro eliminado" : "No existe ese ISBN")
                .build();
    }

    public ListaLibros verBiblioteca(Connection conn) throws Exception {
        return libroRepo.findAll(conn);
    }

    public ListaLibros verLibrosRepetidos(Connection conn) throws Exception {
        return libroRepo.findRepetidosPorTituloAutor(conn);
    }
}
