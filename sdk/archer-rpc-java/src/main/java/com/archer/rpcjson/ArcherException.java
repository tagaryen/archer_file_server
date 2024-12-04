package com.archer.rpcjson;

public class ArcherException extends RuntimeException {

    static final long serialVersionUID = -332129948L;
    
    public ArcherException(Throwable e) {
    	super(e.getMessage());
    }
    
    public ArcherException(String msg) {
    	super(msg);
    }
    
    public ArcherException(String msg, Throwable e) {
    	super(msg, e);
    }
}
