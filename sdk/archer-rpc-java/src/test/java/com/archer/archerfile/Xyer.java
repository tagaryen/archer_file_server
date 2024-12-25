package com.archer.archerfile;

import java.time.LocalDateTime;

public class Xyer {
	int a;
	
	double b;
	
	String c;
	
	LocalDateTime d;
	
	public Xyer() {
		super();
	}

	public Xyer(int a, double b, String c, LocalDateTime d) {
		super();
		this.a = a;
		this.b = b;
		this.c = c;
		this.d = d;
	}

	public int getA() {
		return a;
	}

	public double getB() {
		return b;
	}

	public String getC() {
		return c;
	}

	public LocalDateTime getD() {
		return d;
	}

	public void setA(int a) {
		this.a = a;
	}

	public void setB(double b) {
		this.b = b;
	}

	public void setC(String c) {
		this.c = c;
	}

	public void setD(LocalDateTime d) {
		this.d = d;
	}

}
