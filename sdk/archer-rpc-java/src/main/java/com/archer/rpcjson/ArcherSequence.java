package com.archer.rpcjson;


public class ArcherSequence {

    private static final int P = 32;
    private static final int C = 23;

    N[] map;
    int size;

    public ArcherSequence(int size) {
        this.size = size / P + 1;
        map = new N[this.size];
    }
    
    long calP (byte[] item) {
    	long p = 0;
        for(int i = 0; i < C; i++) {
            long l = (long) item[i];
            if(l < 0) {
                l += 256;
            }
            p |= (l << ((C - i) << 3));
        }
        if(p < 0) {
            p = -p;
        }
        return p;
    }
    //32bytes input
    public synchronized void add(byte[] item, ArcherCallback obj) {
        long p = calP(item);
        int r = (int) (p % size);
        if(map[r] == null) {
            map[r] = new N(item);
            map[r].obj = obj;
        } else {
        	N cur = map[r];
        	while(cur.next != null) {
        		cur = cur.next;
        	}
        	cur.next = new N(item);
        	cur.next.obj = obj;
        }
    }

    public synchronized ArcherCallback find(byte[] bs) {
    	long p = calP(bs);
        int m = map.length, r = (int) (p % m);
    	N prev = null;
    	N cur = map[r];
        boolean ok = true;
        while(cur != null) {
            byte[] d = cur.data;
            if(d.length == bs.length) {
            	ok = true;
                for(int i = 0; i < bs.length; i++) {
                    if(d[i] != bs[i]) {
                        ok = false;
                        break;
                    }
                }
                if(ok) {
                	if(cur == map[r]) {
                		map[r] = cur.next;
                	} else {
                		prev.next = cur.next;
                	}
                    return cur.obj;
                }
            }
            prev = cur;
            cur = cur.next;
        }
        return null;
    }
	
    
	static class N {
		byte[] data;
		N next;
		
		ArcherCallback obj;
		
		public N(byte[] data) {
			this.data = data;
		}
	}
}

