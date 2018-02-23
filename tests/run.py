#!/usr/bin/python
# coding=utf8

import socket
import fcntl
import struct
import os
import sys
from SimpleXMLRPCServer import SimpleXMLRPCServer
import SocketServer
import xmlrpclib
import time
import signal
import copy
import socket
import threading
import subprocess
import xml.dom.minidom
import xml.dom.ext


from interfaces import*
from tests import *





########################################################################
#							XML RPC Server
########################################################################


class server_thread(threading.Thread):
	running = True
	def __init__ (self,port):
		threading.Thread.__init__(self)
		self.server = SimpleXMLRPCServer(('', port),logRequests=False)
		self.server.register_introspection_functions()

	def run(self):
		while (self.running):
			self.server.handle_request()

	def stop(self):
		self.running = False
		self.server.server_close()
		os._exit(0)
	
	def register_function(self,f):
		self.server.register_function(f)





########################################################################
#							SYNCHRONIZATION 
########################################################################

def synchronization_init(servers,clients):
	sync=[]
	signal.signal(signal.SIGINT, exitf)
	sync.append(threading.Condition())
	hosts=servers + clients
	sync.append(hosts)
	others=copy.deepcopy(hosts)
	others.remove(get_hostname())
	sync.append(others)
	connections={}
	sync.append(connections)
	return sync

def print_order(sync,host_sorted_dbase):
	
	on_order=sync[4]
	my_order=sync[5]
	
	print ""
	print "Test schedule:"
	i=0
	my_hostname=get_hostname()
	for h in host_sorted_dbase:
		turn_string=''
		local_string=''
		if on_order>i:
			turn_string='done ☑'
		if on_order==i:
			turn_string='run  ↣'
		if on_order<i:
			turn_string='wait ☐'
		if i==my_order:
			local_string='[this system]'
		
		print turn_string+'\t'+str(i)+')'+h[0]+'\t'+local_string
		if h[0]==host_sorted_dbase[sync[5]][0]:
			print_scenario_list()
		
		i+=1 
	if on_order> len(host_sorted_dbase)-1:
		print "\tAll tests complete"
	sys.stdout.flush()


def get_my_order(host_sorted_dbase):
	my_hostname=get_hostname()
	i=0
	for actual in host_sorted_dbase:
		if actual[0]==my_hostname:
			return i
		i+=1
	return -1;	

def set_order(o):
	global sync
	sync[0].acquire()
	if o> sync[4]:
		sync[4]=o;
		#print 'setting order to',o
	sync[0].notify()
	sync[0].release()
	return sync[4]
	
def wait_for_others(sync,verbose=True):
	for o in sync[2]:
		if verbose:
			address='http://'+o+':'+str(rpcport)
		print 'waiting for ' + address
		sync[3][o]=xmlrpclib.ServerProxy(address)
		resp=None
		while not resp:
			try:
				resp=sync[3][o].test_connection()
				#print resp
				break
			except:
				if verbose:
					sys.stdout.write('.')
					sys.stdout.flush()
				time.sleep(1)
	time.sleep(1)
		

def get_hosts_info(sync,verbose=True):

	host_dbase=[]
	host_dbase.append(get_all_info())

	#print sync[3]
	for o in sync[2]:
		host_dbase.append(sync[3][o].get_all_info())

	if verbose:
		for h in host_dbase:
			print_all_info(h) 
		

	host_sorted_dbase=copy.deepcopy(host_dbase)
	host_sorted_dbase.sort(lambda x, y: cmp(x[6],y[6]))
	on_order=0
	my_order=get_my_order(host_sorted_dbase)
	sync.append(on_order)
	sync.append(my_order)
	return host_sorted_dbase
	
def wait_for_myturn(sync):
	sync[0].acquire()
	#print 'on_order=',sync[4]
	#print 'my_order=',sync[5]
	while not sync[4]==sync[5]:
		sync[0].wait()
		#print 'awaking'
		#print sync
		print_order(sync,host_sorted_dbase);
	sync[0].release()


def wait_for_end(sync,host_sorted_dbase):
	sync[4]+=1
	for i in range(len(host_sorted_dbase)-1):
		index=(sync[4]+i) % len(host_sorted_dbase) 
		#print "notyfying "+host_sorted_dbase[index ][0],' ',sync[4]
		sync[3][host_sorted_dbase[index][0]].set_order(sync[4])
		

	print_order(sync,host_sorted_dbase);
	while not sync[4]>len(host_sorted_dbase)-1:
		#print 'acquireing'
		sync[0].acquire()
		#print sync
		sync[0].wait()
		sync[0].release()
		print_order(sync,host_sorted_dbase);
	

		
########################################################################
#							 XML STUFF
########################################################################
	
def write_basic_info(host_sorted_dbase,sync,xmlnode):
	myid=sync[5]
	host=xmlnode.createElement('host')
	host.setAttribute("hostname", host_sorted_dbase[myid][0])
	host.setAttribute("release", host_sorted_dbase[myid][2])
	host.setAttribute("build", host_sorted_dbase[myid][7])
	host.setAttribute("kernel", host_sorted_dbase[myid][4])
	host.setAttribute("rhts_ip", host_sorted_dbase[myid][5])
	xmlnode.appendChild(host)
	return host

def xml_writeout(xmlfile): 
	global output_file
	if not globals().has_key('output_file'):
		output_file=get_build()+'.xml'
	f=open(output_file, "w")
	xml.dom.ext.PrettyPrint(xmlfile,f)
	f.close()
	return
	

def exitf(a=None,b=None):
	print "\n"
	s.stop()
	sys.exit()	
	

########################################################################
#						SCENARIOS SECTION
########################################################################



def print_scenario_list():
	#print scenario_list
	global scenario_list;
	for scenario in scenario_list:
		if scenario[2]=='not_started':
			print '\t  wait ☐',
		elif scenario[2]=='run':
			print '\t  run  ↣',
		elif scenario[2]=='done':
			print '\t  done ☑',
		print scenario[1]
	return

def run_scenarios(sync,host_sorted_dbase,xmlnode):
	global runing_tests	
	global scenario_list
	for scenario in scenario_list:
		scenario[2]='run'
		print_order(sync,host_sorted_dbase)
		scenario[0](sync,host_sorted_dbase,xmlnode)
		scenario[2]='done'
		runing_tests=[]
		xml_writeout(xml_file)
		
		

		
########################################################################
#								MAIN
########################################################################
from scenarios import *

sync=synchronization_init(servers,clients)


print "Starting netserver\t\t\t\t\t\t\t",
sys.stdout.flush()
try:
	ret=start_netserver(netserverport)
	print '[OK]'
except:
	print '[Fail]'
	print "Failed to start netserver, exitting"
	sys.exit()
	

print "Starting XmlRpcServer\t\t\t\t\t\t\t",
try:
	s=server_thread(rpcport)
	s.register_function(test_connection)
	s.register_function(get_all_info)
	s.register_function(set_order)
	s.register_function(run_test)
	s.register_function(run_test_concurent)
	s.register_function(wait_for_test_concurent)
	s.register_function(wait_for_test)
	s.register_function(get_test_res)
	s.start()
	print '[OK]'
except:
	print '[Fail]'
	print "Failed to start XmlRpcServer, exitting"
	sys.exit()


wait_for_others(sync)
host_sorted_dbase=get_hosts_info(sync,verbose=True)
print_order(sync,host_sorted_dbase);
node=write_basic_info(host_sorted_dbase,sync,xml_file)

wait_for_myturn(sync)
run_scenarios(sync,host_sorted_dbase,node)
wait_for_end(sync,host_sorted_dbase)
subprocess.Popen('source /usr/bin/rhts-environment.sh && report_result $TEST PASS ',shell=True)
xml_writeout(xml_file)
print "Bye, Bye..."
exitf()



