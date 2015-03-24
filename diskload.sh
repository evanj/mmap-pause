#!/bin/bash
#
# Generates disk load to trigger long mmap write pauses

set -e
#set -x

PARALLEL_DD=2
MAX_PARALLEL_DD=8


# Generate random bytes in case some weird disk system uses compression
echo "generating random source data ..."
date
dd if=/dev/urandom of=rnd-1g bs=1M count=1024
echo "replicating random source data with $MAX_PARALLEL_DD processes ..."
date
for i in `seq $MAX_PARALLEL_DD`; do
  dd if=rnd-1g of=rnd-1g-$i bs=1M &
done
wait

echo "generating parallel load with $PARALLEL_DD dd processes"
for i in `seq 1000`; do
  date
  for j in `seq $PARALLEL_DD`; do
    dd if=rnd-1g-$j of=rnd-1g-$j-2 bs=1M &
  done
  wait
done
