package com.biblioteca.db;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;

public final class Db {

    private static final String URL = "jdbc:postgresql://10.43.99.128:5432/biblioteca";
    private static final String USER = "postgres";
    private static final String PASSWORD = "postgres";

    private Db() {}

    public static Connection getConnection() throws SQLException {
        return DriverManager.getConnection(URL, USER, PASSWORD);
    }
}
