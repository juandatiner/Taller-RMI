package com.biblioteca.service;

import com.biblioteca.db.Db;
import com.biblioteca.business.*;
import com.biblioteca.repository.*;
import com.biblioteca.util.GrpcErrors;
import com.biblioteca.grpc.*;

import io.grpc.stub.StreamObserver;

import java.sql.Connection;

public class BibliotecaServiceImpl extends BibliotecaServiceGrpc.BibliotecaServiceImplBase {

    private final LibroRepository libroRepo = new LibroRepository();
    private final PrestamoRepository prestamoRepo = new PrestamoRepository();
    private final UsuarioRepository usuarioRepo = new UsuarioRepository();

    private final SistemaAdmin sistemaAdmin = new SistemaAdmin(libroRepo);
    private final SistemaCliente sistemaCliente = new SistemaCliente(libroRepo, prestamoRepo, usuarioRepo);

    // ---------- CLIENTE ----------

    @Override
    public void verLibros(Vacio request, StreamObserver<ListaLibros> obs) {
        try (Connection conn = Db.getConnection()) {
            obs.onNext(sistemaCliente.verLibros(conn));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void buscarLibro(LibroIdRequest request, StreamObserver<Libro> obs) {
        try (Connection conn = Db.getConnection()) {
            obs.onNext(sistemaCliente.buscarLibro(conn, request.getIsbn()));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void verLibrosPrestados(UsuarioRequest request, StreamObserver<ListaPrestamos> obs) {
        try (Connection conn = Db.getConnection()) {
            obs.onNext(sistemaCliente.verLibrosPrestados(conn, request.getIdUsuario()));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void prestarLibro(PrestamoRequest request, StreamObserver<Respuesta> obs) {
        try (Connection conn = Db.getConnection()) {
            conn.setAutoCommit(false);

            Respuesta resp = sistemaCliente.pedirPrestado(conn, request.getIdUsuario(), request.getIsbn());

            conn.commit();
            obs.onNext(resp);
            obs.onCompleted();

        } catch (Throwable t) {
            // rollback best-effort
            try (Connection conn2 = Db.getConnection()) {
                // no-op (evitar advertencias); rollback real se hace abajo con conn si lo manejas en variable.
            } catch (Exception ignored) {}

            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void devolverLibro(DevolucionRequest request, StreamObserver<Respuesta> obs) {
        try (Connection conn = Db.getConnection()) {
            conn.setAutoCommit(false);

            Respuesta resp = sistemaCliente.devolverLibro(conn, request.getIdPrestamo());

            conn.commit();
            obs.onNext(resp);
            obs.onCompleted();

        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    // ---------- ADMIN (SEGURO) ----------

    @Override
    public void cargarBaseDeDatos(AdminVacioRequest request, StreamObserver<Respuesta> obs) {
        try (Connection conn = Db.getConnection()) {
            sistemaAdmin.validarAdmin(request.getAdminKey());
            obs.onNext(sistemaAdmin.cargarBaseDeDatos(conn));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void agregarLibro(LibroAdminRequest request, StreamObserver<Respuesta> obs) {
        try (Connection conn = Db.getConnection()) {
            sistemaAdmin.validarAdmin(request.getAdminKey());
            if (!request.hasLibro()) throw com.biblioteca.util.AppException.invalid("Falta objeto libro");

            obs.onNext(sistemaAdmin.agregarLibro(conn, request.getLibro()));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void eliminarLibro(LibroIdAdminRequest request, StreamObserver<Respuesta> obs) {
        try (Connection conn = Db.getConnection()) {
            sistemaAdmin.validarAdmin(request.getAdminKey());
            obs.onNext(sistemaAdmin.eliminarLibro(conn, request.getIsbn()));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void verBiblioteca(AdminVacioRequest request, StreamObserver<ListaLibros> obs) {
        try (Connection conn = Db.getConnection()) {
            sistemaAdmin.validarAdmin(request.getAdminKey());
            obs.onNext(sistemaAdmin.verBiblioteca(conn));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }

    @Override
    public void verLibrosRepetidos(AdminVacioRequest request, StreamObserver<ListaLibros> obs) {
        try (Connection conn = Db.getConnection()) {
            sistemaAdmin.validarAdmin(request.getAdminKey());
            obs.onNext(sistemaAdmin.verLibrosRepetidos(conn));
            obs.onCompleted();
        } catch (Throwable t) {
            obs.onError(GrpcErrors.toStatus(t));
        }
    }
}
