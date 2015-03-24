# Pauses when writing mmap-ed files

See my [blog post for details](http://www.evanjones.ca/jvm-mmap-pause.html).

1. Start a new Ubuntu image.
2. Run the following commands:
  ```
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install linux-virtual gcc screen openjdk-8-jdk
sudo reboot
  ```
3. Copy these files to this machine.
4. `gcc --std=gnu99 -O3 -Wall -Wextra -o mmapwritepause mmapwritepause.c`
5. Start screen. In one terminal: `./mmapwritepause /tmp/mmapout`
6. In another terminal: `./diskload.sh`
7. Watch the output of `mmapwritepause`; it will print long pauses.


# JVM reproduction

1. `javac MakeGarbage.java`
2. `java -XX:+PrintGCDetails -XX:+PrintGC -XX:+PrintGCDateStamps -XX:+PrintGCApplicationStoppedTime -Xmx1G -Xms1G MakeGarbage`
3. In another terminal, run `./diskload.sh`

This will show large safepoint times, correlated with when mmapwritepause reports pauses. To verify that disabling the stats fixes the problem, add `-XX:+PerfDisableSharedMem` and rerun it.
