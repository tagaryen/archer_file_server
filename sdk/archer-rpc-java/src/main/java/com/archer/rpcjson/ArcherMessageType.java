package com.archer.rpcjson;

enum ArcherMessageType {
	GetJson,
	SaveJson,
	ResOk,
	ResFail;
	
	public static ArcherMessageType from(int t) {
		if(t == 0) {
			return GetJson;
		}
		if(t == 1) {
			return SaveJson;
		}
		if(t == 2) {
			return ResOk;
		}
		if(t == 3 ) {
			return ResFail;
		}
		throw new ArcherException("Invalid message type [" + t + "]");
	}
}
