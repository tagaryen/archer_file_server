package com.archer.archerfile;

public class Base<T> {

	T data;
	
	String desc;
	
	public Base() {
		super();
	}

	public Base(T data, String desc) {
		super();
		this.data = data;
		this.desc = desc;
	}

	public T getData() {
		return data;
	}

	public String getDesc() {
		return desc;
	}

	public void setData(T data) {
		this.data = data;
	}

	public void setDesc(String desc) {
		this.desc = desc;
	}
	
	
}
