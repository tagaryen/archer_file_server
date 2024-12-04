package com.archer.rpcjson;

import java.util.concurrent.locks.ReentrantLock;

import com.archer.net.Bytes;

public class ArcherMessage {
	

	private static final byte VERSION_BYTE = 1;
	
    ReentrantLock frameLock = new ReentrantLock(true);
    
	private byte[] data;
	private int pos;
	
	
	public ArcherMessage() {}
	
	public byte[] read(Bytes in) {
		frameLock.lock();
		try {
			int readCount;
			if(data == null) {
				int dataLen = in.readInt32();
				byte version = (byte) in.readInt8();
				if(VERSION_BYTE != version) {
					throw new ArcherException("Invalid message version " + version);
				}
				
				data = new byte[dataLen - 1];
				pos = 0;
				readCount = dataLen > in.available() ? in.available() : dataLen;
			} else {
				int remain = data.length - pos;
				readCount = remain > in.available() ? in.available() : remain;
			}
			in.read(data, pos, readCount);
			pos += readCount;
			if(pos >= data.length) {
				byte[] read =  data;
				data = null;
				pos = 0;
				return read;
			}
			return null;
		} finally {
			frameLock.unlock();
		}
	}
}
