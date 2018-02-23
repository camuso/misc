#!/usr/bin/python
import subprocess
import os
import sys
import time

start=time.time()
cmds='for i in `seq 1 30`; do netperf -H 172.16.20.20 -P0 -l60 -- -m 10&  done'
o=subprocess.Popen(cmds,stdout=subprocess.PIPE,close_fds=True,shell=True)
lines = o.stdout.readlines()
stop= time.time()
suma=0.0
for l in lines:
	instance=float(l.split()[4])
	suma+=instance;
	print instance;

print '------------'
print 'time',stop-start;
print 'throughput',suma;
print 'instances',len(lines);

