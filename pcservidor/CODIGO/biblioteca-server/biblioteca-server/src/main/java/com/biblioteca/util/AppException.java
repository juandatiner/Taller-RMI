package com.biblioteca.util;

public class AppException extends RuntimeException {

    public enum Code {
        INVALID_ARGUMENT,
        NOT_FOUND,
        PERMISSION_DENIED,
        FAILED_PRECONDITION,
        INTERNAL
    }

    private final Code code;

    public AppException(Code code, String message) {
        super(message);
        this.code = code;
    }

    public Code getCode() {
        return code;
    }

    // Helpers
    public static AppException invalid(String msg) { return new AppException(Code.INVALID_ARGUMENT, msg); }
    public static AppException notFound(String msg) { return new AppException(Code.NOT_FOUND, msg); }
    public static AppException denied(String msg) { return new AppException(Code.PERMISSION_DENIED, msg); }
    public static AppException precondition(String msg) { return new AppException(Code.FAILED_PRECONDITION, msg); }
    public static AppException internal(String msg) { return new AppException(Code.INTERNAL, msg); }
}
