public class VBad extends Thread{
    int tid;
    VBad(int i) { tid = i; }
    static int N = 100;
    static  volatile
	boolean req = false;
    static  volatile
	boolean ack = false;    
  public void run() {
	int temp;
	if (tid==0) {
	    for (int i = 0; i < N; i++) {
		req = true;
		while (ack==false) {
		};
		req = false;
		while (ack==true) {
		}
	    }
	}
	else {
	    for (int i = 0; i < N; i++) {
		while (req==false) {
		}
		ack = true;
		while (req==true) {
		}
		ack = false;
	    }
	}
    }
    public static void main(String[] args) {
	try {
	    N = Integer.parseInt(args[0]); // User-supplied N
	} 
	catch (Exception e) {
	} 
	Thread t0 = new VBad(0);
	Thread t1 = new VBad(1);
	t0.start();
	t1.start();
	try {
	    t0.join();
	    t1.join();
	}
	catch (InterruptedException e) {
	}
	System.out.println("Done.");
    }
}

