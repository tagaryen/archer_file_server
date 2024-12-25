package com.archer.archerfile;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.SynchronousQueue;
import java.util.concurrent.ThreadFactory;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.SynchronousQueue;
import java.util.function.Supplier;

import com.archer.xjson.JavaTypeRef;
import com.archer.xjson.XJSON;
import com.archer.xjson.XJSONException;

import redis.clients.jedis.Jedis;

public class RedisCli {
	
	static AtomicInteger c = new AtomicInteger(0);
	
    public static void main( String[] args )
    {
    	
    	final Jedis cli = new Jedis("10.8.19.244", 6379);
    	
    	try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
        
    	final XJSON json = new XJSON();
    	LocalDateTime now = LocalDateTime.now();
    	System.out.println(now.toString());
    	final Base<Xyer> xy = new Base<>(new Xyer(1996, 11.17D, "xuyi", now), "xuyi shi da shuai ge"); 
//    	final Base<Xyer> xycpy = null;

		ExecutorService pool = Executors.newFixedThreadPool(1);
        List<CompletableFuture<Void>> cfList = new ArrayList<>();
    	int total = 100;
    	long t0 = System.currentTimeMillis();
		for(int i = 0; i < total; i++) {
//			try {
//		    	cli.set("xuyia", json.stringify(xy));
//		    	Base<Xyer> xycpy = json.parse(cli.get("xuyia"), new JavaTypeRef<Base<Xyer>>() {});
//			} catch(Exception e) {
//			}
			CompletableFuture<Void> cf = CompletableFuture.supplyAsync(new Supplier<Void> () {
				@Override
				public Void get() {
					String str = "no err";
					try {
				    	cli.set("xuyia", json.stringify(xy));
				    	str = cli.get("xuyia");
				    	Base<Xyer> xycpy = json.parse(str, new JavaTypeRef<Base<Xyer>>() {});
					} catch(Exception e) {
						if(e instanceof XJSONException) {
							System.out.println(str);
						}
						c.incrementAndGet();
					}
					return null;
				}
				
			}, pool);
			cfList.add(cf);
		}
        CompletableFuture<Void> all = CompletableFuture.allOf(cfList.toArray(new CompletableFuture[0]));
        all.join();
    	long t1 = System.currentTimeMillis();

//    	System.out.println("get "+xycpy.getData().getD().toString());
    	System.out.println("cost = " + (t1 - t0));
    	System.out.println("err count = " + c.get());
    }
    
}
