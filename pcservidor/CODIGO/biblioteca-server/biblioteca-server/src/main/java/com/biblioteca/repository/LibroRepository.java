package com.biblioteca.repository;

import com.biblioteca.grpc.*;

import java.sql.*;

public class LibroRepository {

    public int insertLibro(Connection conn, LibroRequest libro) throws SQLException {
        String sql = """
            INSERT INTO libro (isbn, titulo, autor, editorial, anio_publicacion, estado)
            VALUES (?, ?, ?, ?, ?, 'disponible')
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, libro.getIsbn());
            ps.setString(2, libro.getTitulo());
            ps.setString(3, libro.getAutor());
            ps.setString(4, libro.getEditorial().isBlank() ? null : libro.getEditorial());
            ps.setInt(5, libro.getAnioPublicacion());
            return ps.executeUpdate();
        }
    }

    public int deleteLibro(Connection conn, String isbn) throws SQLException {
        String sql = "DELETE FROM libro WHERE isbn = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, isbn);
            return ps.executeUpdate();
        }
    }

    public Libro findByIsbn(Connection conn, String isbn) throws SQLException {
        String sql = """
            SELECT isbn, titulo, autor, editorial, anio_publicacion, estado::text AS estado
            FROM libro WHERE isbn = ?
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, isbn);
            try (ResultSet rs = ps.executeQuery()) {
                if (!rs.next()) return null;

                return Libro.newBuilder()
                        .setIsbn(rs.getString("isbn"))
                        .setTitulo(rs.getString("titulo"))
                        .setAutor(rs.getString("autor"))
                        .setEditorial(rs.getString("editorial") == null ? "" : rs.getString("editorial"))
                        .setAnioPublicacion(rs.getInt("anio_publicacion"))
                        .setEstado(rs.getString("estado"))
                        .build();
            }
        }
    }

    public ListaLibros findAll(Connection conn) throws SQLException {
        String sql = """
            SELECT isbn, titulo, autor, editorial, anio_publicacion, estado::text AS estado
            FROM libro ORDER BY titulo
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql);
             ResultSet rs = ps.executeQuery()) {

            ListaLibros.Builder out = ListaLibros.newBuilder();
            while (rs.next()) {
                out.addLibros(Libro.newBuilder()
                        .setIsbn(rs.getString("isbn"))
                        .setTitulo(rs.getString("titulo"))
                        .setAutor(rs.getString("autor"))
                        .setEditorial(rs.getString("editorial") == null ? "" : rs.getString("editorial"))
                        .setAnioPublicacion(rs.getInt("anio_publicacion"))
                        .setEstado(rs.getString("estado"))
                        .build());
            }
            return out.build();
        }
    }

    public ListaLibros findRepetidosPorTituloAutor(Connection conn) throws SQLException {
        // "Repetidos" = mismo (titulo, autor) con count > 1
        String sql = """
            SELECT isbn, titulo, autor, editorial, anio_publicacion, estado::text AS estado
            FROM libro
            WHERE (titulo, autor) IN (
              SELECT titulo, autor
              FROM libro
              GROUP BY titulo, autor
              HAVING COUNT(*) > 1
            )
            ORDER BY titulo, autor
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql);
             ResultSet rs = ps.executeQuery()) {

            ListaLibros.Builder out = ListaLibros.newBuilder();
            while (rs.next()) {
                out.addLibros(Libro.newBuilder()
                        .setIsbn(rs.getString("isbn"))
                        .setTitulo(rs.getString("titulo"))
                        .setAutor(rs.getString("autor"))
                        .setEditorial(rs.getString("editorial") == null ? "" : rs.getString("editorial"))
                        .setAnioPublicacion(rs.getInt("anio_publicacion"))
                        .setEstado(rs.getString("estado"))
                        .build());
            }
            return out.build();
        }
    }

    public String getEstadoForUpdate(Connection conn, String isbn) throws SQLException {
        String sql = "SELECT estado::text AS estado FROM libro WHERE isbn = ? FOR UPDATE";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, isbn);
            try (ResultSet rs = ps.executeQuery()) {
                return rs.next() ? rs.getString("estado") : null;
            }
        }
    }

    public int updateEstado(Connection conn, String isbn, String estado) throws SQLException {
        String sql = "UPDATE libro SET estado = ?::estado_libro WHERE isbn = ?";
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, estado);
            ps.setString(2, isbn);
            return ps.executeUpdate();
        }
    }

    public int seedLibrosSiVacio(Connection conn) throws SQLException {
        String countSql = "SELECT COUNT(*) FROM libro";
        try (PreparedStatement cps = conn.prepareStatement(countSql);
             ResultSet rs = cps.executeQuery()) {
            rs.next();
            if (rs.getInt(1) > 0) return 0;
        }

        int inserted = 0;
        inserted += seedInsert(conn, "9780001", "Sistemas Distribuidos", "Tanenbaum", "Pearson", 2017);
        inserted += seedInsert(conn, "9780002", "Bases de Datos", "Silberschatz", "McGraw-Hill", 2019);
        inserted += seedInsert(conn, "9780003", "Redes", "Kurose", "Pearson", 2016);
        return inserted;
    }

    private int seedInsert(Connection conn, String isbn, String titulo, String autor, String editorial, int anio) throws SQLException {
        String sql = """
            INSERT INTO libro (isbn, titulo, autor, editorial, anio_publicacion, estado)
            VALUES (?, ?, ?, ?, ?, 'disponible')
            ON CONFLICT (isbn) DO NOTHING
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setString(1, isbn);
            ps.setString(2, titulo);
            ps.setString(3, autor);
            ps.setString(4, editorial);
            ps.setInt(5, anio);
            return ps.executeUpdate();
        }
    }
}
