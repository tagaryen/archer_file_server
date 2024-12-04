package com.archer.rpcjson;

import com.archer.rpcjson.ArcherCallback.DefaultGetCallback;
import com.archer.rpcjson.ArcherCallback.DefaultSaveCallback;
import com.archer.rpcjson.ArcherCallback.JsonGetCallback;
import com.archer.rpcjson.ArcherCallback.JsonSaveCallback;
import com.archer.xjson.JavaTypeRef;

public class ArcherJSONClient {
	
	private ArcherConnector connector;
	
	public ArcherJSONClient(ArcherConnector connector) {
		this.connector = connector;
		this.connector.doConnect();
	}
	
	public void save(String key, Object data) {
		DefaultSaveCallback callback = new DefaultSaveCallback();
		connector.sendSaveJson(key, data, callback);
		callback.waitForSuccess();
	}
	public void saveAsync(String key, Object data, JsonSaveCallback callback) {
		connector.sendSaveJson(key, data, callback);
	}
	public <T> T get(String key, JavaTypeRef<T> ref) {
		DefaultGetCallback<T> callback = new DefaultGetCallback<>(ref);
		connector.sendGetJson(key, callback);
		return callback.waitForResponse();
	}	
	public void getAsync(String key, JsonGetCallback<?> callback) {
		connector.sendGetJson(key, callback);
	}
	
	public String getString(String key) {
		DefaultGetCallback<?> callback = new DefaultGetCallback<>(null);
		connector.sendGetJson(key, callback);
		return callback.waitForStringResponse();
	}	
	public void getStringAsync(String key, JsonGetCallback<?> callback) {
		connector.sendGetJson(key, callback);
	}
}
