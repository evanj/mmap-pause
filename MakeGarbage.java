/*
javac MakeGarbage.java
java -XX:+PrintGCDetails -XX:+PrintGC -XX:+PrintGCDateStamps -XX:+PrintGCApplicationStoppedTime -Xmx1G -Xms1G MakeGarbage

Disabled stats:
java -XX:+PrintGCDetails -XX:+PrintGC -XX:+PrintGCDateStamps -XX:+PrintGCApplicationStoppedTime -Xmx1G -Xms1G -XX:+PerfDisableSharedMem MakeGarbage &> /mnt/ej/java-no-mmap-stats.txt


This flag no longer works as of recent versions of Java: -Djava.io.tmpdir=/dev/shm 

See:
http://bugs.java.com/view_bug.do?bug_id=6938627
http://bugs.java.com/view_bug.do?bug_id=7009828

Stats location is hard coded to /tmp on Linux:
http://hg.openjdk.java.net/jdk9/client/hotspot/file/c3b117fa5bde/src/os/linux/vm/os_linux.cpp#l1582
*/

public class MakeGarbage {
  public static void main(String[] arguments) {
    final int ARRAY_SIZE = 4 << 20;
    final int LOOPS = 2000000000;
    int sum = 0;
    while (true) {
      for (int i = 0; i < LOOPS; i++) {
        byte[] array = new byte[ARRAY_SIZE];
        sum += array[0] + array.length;
      }
      System.out.println("hello " + sum);
    }
  }
}
