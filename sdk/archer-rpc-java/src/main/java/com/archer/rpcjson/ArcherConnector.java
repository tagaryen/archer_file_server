package com.archer.rpcjson;

import java.nio.charset.StandardCharsets;
import java.util.Random;

import com.archer.net.Channel;
import com.archer.net.HandlerList;

public class ArcherConnector {
	
	private String host;
	
	private int port;
	
	private Channel channel;
	
	private ArcherHandler handler;
	
	private Random r = new Random();
	
	
	public ArcherConnector(String host, int port) {
		this.host = host;
		this.port = port;
		
		this.channel = new Channel();
		this.handler = new ArcherHandler();
	}
	
	public void doConnect() {

		HandlerList handlers = new HandlerList();
		handlers.add(this.handler);
		this.channel.handlerList(handlers);
		
		channel.connect(host, port);
	}
	
	protected void sendSaveJson(String key, Object obj, ArcherCallback callback) {
		String value = callback.getJsonEncoder().stringify(obj);
		byte[] keyBs = key.getBytes(StandardCharsets.UTF_8);
		byte[] valueBs = value.getBytes(StandardCharsets.UTF_8);
		byte[] input = new byte[2 + keyBs.length + valueBs.length];
		input[0] = (byte) ((keyBs.length >> 8) & 0xff);
		input[1] = (byte) (keyBs.length & 0xff);
		System.arraycopy(keyBs, 0, input, 2, keyBs.length);
		System.arraycopy(valueBs, 0, input, 2 + keyBs.length, valueBs.length);
		
		doSend(input, ArcherMessageType.SaveJson, callback);
	}
	
	protected void sendGetJson(String key, ArcherCallback callback) {
		byte[] keyBs = key.getBytes(StandardCharsets.UTF_8);
		doSend(keyBs, ArcherMessageType.GetJson, callback);
	}
	
	private void doSend(byte[] input, ArcherMessageType type, ArcherCallback callback) {
		int sendLen = 32 + 2 + input.length;
		byte[] send = new byte[4 + sendLen];
		int off = 0;
		send[off++] = (byte)((sendLen >> 24) & 0xff);
		send[off++] = (byte)((sendLen >> 16) & 0xff);
		send[off++] = (byte)((sendLen >> 8) & 0xff);
		send[off++] = (byte)(sendLen & 0xff);
		
		send[off++] = 1;
		byte[] seq = new byte[32];
		r.nextBytes(seq);
		System.arraycopy(seq, 0, send, off, seq.length);
		off += 32;
		if(ArcherMessageType.GetJson == type) {
			send[off++] = 0;
		} else if(ArcherMessageType.SaveJson == type) {
			send[off++] = 1;
		} else {
			throw new ArcherException("Invalid message type when send content " + type.name());
		}

		System.arraycopy(input, 0, send, off, input.length);
		
		handler.addCallback(seq, callback);
		channel.write(send);
	}
}
