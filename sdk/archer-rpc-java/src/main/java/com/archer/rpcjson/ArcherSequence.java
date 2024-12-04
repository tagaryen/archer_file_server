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

    //32bytes input
    public synchronized void add(byte[] item, ArcherCallback obj) {
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
        int r = (int) (p % size);
        if(map[r] == null) {
            map[r] = new N(item);
            map[r].obj = obj;
        } else {
            N cur = new N(item);
            map[r].obj = obj;
            map[r].next = cur;
            cur.last = map[r];
            map[r] = cur;
        }
    }

    public ArcherCallback find(byte[] bs) {
        long p = 0;
        for(int i = 0; i < C; i++) {
            long l = (long) bs[i];
            if(l < 0) {
                l += 256;
            }
            p |= (l << ((C - i) << 3));
        }
        if(p < 0) {
            p = -p;
        }
        int m = map.length;
        int r = (int) (p % m);
        N cur = map[r];
        while(cur != null) {
            byte[] d = cur.data;
            if(d.length == bs.length) {
                boolean ok = true;
                for(int i = 0; i < bs.length; i++) {
                    if(d[i] != bs[i]) {
                        ok = false;
                        break;
                    }
                }
                if(ok) {
                	N prev = cur.last;
                	N next = cur.next;
                	if(cur == map[r]) {
                		map[r] = next;
                	}
                	if(prev != null) {
                		
                		prev.next = next;
                	}
                	if(next != null) {
                		next.last = prev;
                	}
                    return cur.obj;
                }
            }
            cur = cur.last;
        }
        return null;
    }
	
    
	static class N {
		byte[] data;
		N last;
		N next;
		
		ArcherCallback obj;
		
		public N(byte[] data) {
			this.data = data;
		}
	}
}

