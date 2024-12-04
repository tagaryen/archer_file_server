package com.archer.rpcjson;

import java.lang.reflect.Type;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;

import com.archer.xjson.JavaTypeRef;
import com.archer.xjson.XJSON;

public abstract class ArcherCallback {
	
	private static XJSON json = new XJSON();
	
	private static final long TIMEOUT = 3000;
	
	private Object lock = new Object();
	
	protected String content = null;
	
	protected String key = null;
	
	protected boolean ok = false;
	
	protected abstract void onResponse();
	
	protected void call(ArcherMessageType type, byte[] data) {
		ok = (ArcherMessageType.ResOk == type);
		try {

			int b0 = data[0], b1 = data[1];
			b0 = b0 < 0 ? b0 + 256 : b0;
			b1 = b1 < 0 ? b1 + 256 : b1;
			int keyLen = (b0 << 8) | b1;

			int off = 2;
			this.key = new String(Arrays.copyOfRange(data, off, off + keyLen), StandardCharsets.UTF_8);
			off += keyLen;
			
			if(off < data.length) {
				this.content = new String(Arrays.copyOfRange(data, off, data.length), StandardCharsets.UTF_8);
			}
			
			if(!ok && ArcherMessageType.ResFail != type) {
				this.content = "Invalid response type " + type.name();
			}
		} catch(Exception e) {
			ok = false;
			this.content = "Parse response content failed, " + e.getLocalizedMessage();
		}
		onResponse();
		unlock();
	}
	
	protected void lock() {
		long s = System.currentTimeMillis();
		synchronized(lock) {
			try {
				lock.wait(TIMEOUT);
			} catch (InterruptedException e) {
				throw new ArcherException(e);
			}
		}
		long e = System.currentTimeMillis();
		if(e - s >= TIMEOUT) {
			throw new ArcherException("Wait for response timeout.");
		}
	}
	
	protected void unlock() {
		synchronized(lock) {
			lock.notifyAll();
		}
	}
	
	public XJSON getJsonEncoder() {
		return json;
	}
	
	public static class DefaultSaveCallback extends ArcherCallback {

		public void waitForSuccess() {
			lock();
			if(!ok) {
				throw new ArcherException(content);
			}
		}

		@Override
		protected void onResponse() {}
	}
	
	public static class DefaultGetCallback<T> extends ArcherCallback {
		
		JavaTypeRef<T> ref;
		
		public DefaultGetCallback(JavaTypeRef<T> ref) {
			this.ref = ref;
		}

		public T waitForResponse() {
			lock();
			if(!ok) {
				throw new ArcherException(content);
			}
			return json.parse(content, ref);
		}

		public String waitForStringResponse() {
			lock();
			if(!ok) {
				throw new ArcherException(content);
			}
			return content;
		}
		
		@Override
		protected void onResponse() {}
		
	}
	
	public static abstract class JsonSaveCallback extends ArcherCallback {

		@Override
		protected void onResponse() {
			if(!ok) {
				throw new ArcherException(content);
			}
			
			finished(key);
		}

		public abstract void finished(String key);
		
		
	}
	
	public static abstract class JsonGetCallback<T> extends ArcherCallback {
		
		JavaTypeRef<T> ref;
		
		public JsonGetCallback(JavaTypeRef<T> ref) {
			this.ref = ref;
		}
		
		@SuppressWarnings("unchecked")
		@Override
		protected void onResponse() {
			if(!ok) {
				throw new ArcherException(content);
			}
			Type javaType = ref.getJavaType();
			if(javaType.getClass().equals(Class.class) && String.class.equals((Class<?>) javaType)) {
				finished(key, (T)content);
				return ;
			}
			T obj = json.parse(content, ref);
			finished(key, obj);
		}
		

		public abstract void finished(String key, T obj);
	}
}
