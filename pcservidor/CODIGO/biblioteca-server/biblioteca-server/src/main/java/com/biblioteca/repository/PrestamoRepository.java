package com.biblioteca.repository;

import com.biblioteca.grpc.*;

import java.sql.*;

public class PrestamoRepository {

    public void insertPrestamoActivo(Connection conn, int idUsuario, String isbn) throws SQLException {
        String sql = """
            INSERT INTO prestamo (id_usuario, isbn, estado)
            VALUES (?, ?, 'activo')
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, idUsuario);
            ps.setString(2, isbn);
            ps.executeUpdate();
        }
    }

    public ListaPrestamos findPrestamosActivosPorUsuario(Connection conn, int idUsuario) throws SQLException {
        String sql = """
            SELECT id_prestamo, id_usuario, isbn,
                   fecha_prestamo, fecha_devolucion,
                   estado::text AS estado
            FROM prestamo
            WHERE id_usuario = ? AND estado = 'activo'
            ORDER BY fecha_prestamo DESC
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, idUsuario);
            try (ResultSet rs = ps.executeQuery()) {
                ListaPrestamos.Builder out = ListaPrestamos.newBuilder();
                while (rs.next()) {
                    out.addPrestamos(Prestamo.newBuilder()
                            .setIdPrestamo(rs.getInt("id_prestamo"))
                            .setIdUsuario(rs.getInt("id_usuario"))
                            .setIsbn(rs.getString("isbn"))
                            .setFechaPrestamo(rs.getDate("fecha_prestamo").toString())
                            .setFechaDevolucion(rs.getDate("fecha_devolucion") == null ? "" : rs.getDate("fecha_devolucion").toString())
                            .setEstado(rs.getString("estado"))
                            .build());
                }
                return out.build();
            }
        }
    }

    public String getIsbnPrestamoActivoForUpdate(Connection conn, int idPrestamo) throws SQLException {
        String sql = """
            SELECT isbn
            FROM prestamo
            WHERE id_prestamo = ? AND estado = 'activo'
            FOR UPDATE
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, idPrestamo);
            try (ResultSet rs = ps.executeQuery()) {
                return rs.next() ? rs.getString("isbn") : null;
            }
        }
    }

    public int marcarDevuelto(Connection conn, int idPrestamo) throws SQLException {
        String sql = """
            UPDATE prestamo
            SET estado = 'devuelto', fecha_devolucion = CURRENT_DATE
            WHERE id_prestamo = ? AND estado = 'activo'
        """;
        try (PreparedStatement ps = conn.prepareStatement(sql)) {
            ps.setInt(1, idPrestamo);
            return ps.executeUpdate();
        }
    }
}
