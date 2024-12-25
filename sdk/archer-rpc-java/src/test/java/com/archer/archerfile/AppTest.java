package com.archer.archerfile;

import java.time.LocalDateTime;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.function.Supplier;

import com.archer.rpcjson.ArcherConnector;
import com.archer.rpcjson.ArcherHandler;
import com.archer.rpcjson.ArcherJSONClient;
import com.archer.tools.http.client.JSONRequest;
import com.archer.xjson.JavaTypeRef;

public class AppTest 
{

	static AtomicInteger c = new AtomicInteger(0);
	
    public static void main( String[] args )
    {
    	ArcherConnector conn = new ArcherConnector("10.8.19.244", 9611);
    	final ArcherJSONClient acli = new ArcherJSONClient(conn);
    	
    	
    	try {
			Thread.sleep(100);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
    	
    	
    	LocalDateTime now = LocalDateTime.now();
    	System.out.println(now.toString());
    	final Base<Xyer> xy = new Base<>(new Xyer(1996, 11.17D, "xuyi", now), "xuyi shi da shuai ge"); 

    	
//    	long t0 = System.currentTimeMillis();
//    	acli.save("xuyia", xy);
//    	Base<Xyer> xycpy = acli.get("xuyia", new JavaTypeRef<Base<Xyer>>() {});
//    	long t1 = System.currentTimeMillis();
//    	System.out.println("get "+xycpy.getData().getD().toString());

		ExecutorService pool = Executors.newFixedThreadPool(2);
        List<CompletableFuture<Void>> cfList = new ArrayList<>();
    	int total = 100;
    	long t0 = System.currentTimeMillis();
		for(int i = 0; i < total; i++) {

//			try {
//		    	acli.save("xuyia", xy);
//		    	Base<Xyer> xycpy = acli.get("xuyia", new JavaTypeRef<Base<Xyer>>() {});
//			} catch(Exception e) {
//				e.printStackTrace();
//			}
			CompletableFuture<Void> cf = CompletableFuture.supplyAsync(new Supplier<Void> () {
				@Override
				public Void get() {
					try {
				    	acli.save("xuyia", xy);
				    	Base<Xyer> xycpy = acli.get("xuyia", new JavaTypeRef<Base<Xyer>>() {});
					} catch(Exception e) {
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
//    	System.out.println("re = " + ArcherHandler.re.get() + ", fr = " + ArcherHandler.fr.get());
    	System.out.println("cost = " + (t1 - t0));
    	System.out.println("err count = " + c.get());
    	
    	
    	
    }
}
