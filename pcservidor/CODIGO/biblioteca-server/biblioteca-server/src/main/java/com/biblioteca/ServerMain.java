package com.biblioteca;

import com.biblioteca.service.BibliotecaServiceImpl;
import io.grpc.Server;
import io.grpc.ServerBuilder;

public class ServerMain {

    public static void main(String[] args) throws Exception {
        int port = 50051;

        Server server = ServerBuilder
                .forPort(port)
                .addService(new BibliotecaServiceImpl())
                .build()
                .start();

        System.out.println("Servidor gRPC iniciado en puerto " + port);

        Runtime.getRuntime().addShutdownHook(new Thread(() -> {
            System.out.println("Apagando servidor...");
            server.shutdown();
        }));

        server.awaitTermination();
    }
}
