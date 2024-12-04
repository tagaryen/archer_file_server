package com.archer.rpcjson;

import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
import com.archer.net.Bytes;
import com.archer.net.ChannelContext;
import com.archer.net.handler.Handler;

public class ArcherHandler implements Handler {

	private static ConcurrentHashMap<ChannelContext, ArcherMessage> frameCache = new ConcurrentHashMap<>();
	
	private ArcherSequence seqs = new ArcherSequence(1024);
	
	public ArcherHandler() {}

	@Override
	public void onAccept(ChannelContext ctx) {}
	
	@Override
	public void onConnect(ChannelContext ctx) {
		toFrameMessage(ctx);
		ctx.toNextOnConnect();
	}

	@Override
	public void onRead(ChannelContext ctx, Bytes in) {
		ArcherMessage frame = toFrameMessage(ctx);
		while(in.available() > 0) {
			byte[] read = frame.read(in);
			if(read != null) {
				byte[] seq = Arrays.copyOfRange(read, 0, 32);
				ArcherMessageType type = ArcherMessageType.from(read[32]);
				byte[] data = Arrays.copyOfRange(read, 33, read.length);
				ArcherCallback callback = seqs.find(seq);
				if(callback != null) {
					callback.call(type, data);
				}
			}
		}
	}
	
	@Override
	public void onWrite(ChannelContext ctx, Bytes out) {}

	@Override
	public void onDisconnect(ChannelContext ctx) {
		frameCache.remove(ctx);
	}

	@Override
	public void onError(ChannelContext ctx, Throwable t) {
		t.printStackTrace();
	}
	
	@Override
	public void onSslCertificate(ChannelContext ctx, byte[] cert) {}
	
	public void addCallback(byte[] seq, ArcherCallback callback) {
		seqs.add(seq, callback);
	}
	

	
	private ArcherMessage toFrameMessage(ChannelContext ctx) {
		ArcherMessage msg = frameCache.getOrDefault(ctx, null);
		if(msg == null) {
			msg = new ArcherMessage();
			frameCache.put(ctx, msg);
		}
		return msg;
	}
	
	
}
