package com.biblioteca.util;

import io.grpc.Status;

public final class GrpcErrors {
    private GrpcErrors() {}

    public static RuntimeException toStatus(Throwable t) {
        if (t instanceof AppException ae) {
            return switch (ae.getCode()) {
                case INVALID_ARGUMENT -> Status.INVALID_ARGUMENT.withDescription(ae.getMessage()).asRuntimeException();
                case NOT_FOUND -> Status.NOT_FOUND.withDescription(ae.getMessage()).asRuntimeException();
                case PERMISSION_DENIED -> Status.PERMISSION_DENIED.withDescription(ae.getMessage()).asRuntimeException();
                case FAILED_PRECONDITION -> Status.FAILED_PRECONDITION.withDescription(ae.getMessage()).asRuntimeException();
                case INTERNAL -> Status.INTERNAL.withDescription(ae.getMessage()).asRuntimeException();
            };
        }
        return Status.INTERNAL.withDescription("Error interno: " + t.getMessage()).withCause(t).asRuntimeException();
    }
}
