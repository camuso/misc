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

xml_file=xml.dom.minidom.Document()

########################################################################
#							 TESTS SECTION 
########################################################################
runing_tests=[]
def time_start():
	return time.time()	
	
def time_stop(t):
	return time.time() - t
	
def time_write(xmlnode,r):
	test=xml_file.createElement('duration')
	test.setAttribute('time',str(r))
	xmlnode.appendChild(test)


def cpustats_start():
	state=[]
	f = open('/proc/stat', 'r')
	while True:
		l=(f.readline()).split()
		if (l[0]).find('cpu')<0:
			break
		state.append(l)
	return state
	
def cpustats_stop(state):
	now=cpustats_start()
	for c in range(len(now)):
		for i in range(1,len(now[c])):
			now[c][i]=int(now[c][i])-int(state[c][i])
			
	return now
	
	
def cpustats_write(xmlnode,res):
	return
	test=xml_file.createElement('cpu_statistics')
	
	for i in range(len(res)):
		if i==0:
			r=xml_file.createElement('overall_statistics')
		else:
			r=xml_file.createElement('cpu')
			r.setAttribute('cpu_id',res[i][0])
		
		r2=xml_file.createElement('user')
		r2.setAttribute('time',str(res[i][1]))
		r.appendChild(r2)
		r2=xml_file.createElement('niced')
		r2.setAttribute('time',str(res[i][2]))
		r.appendChild(r2)
		r2=xml_file.createElement('system')
		r.appendChild(r2)
		r2.setAttribute('time',str(res[i][3]))
		r.appendChild(r2)
		r2=xml_file.createElement('idle')
		r2.setAttribute('time',str(res[i][4]))
		r.appendChild(r2)
		r2=xml_file.createElement('iowait')
		r2.setAttribute('time',str(res[i][5]))
		r.appendChild(r2)
		r2=xml_file.createElement('irq')
		r2.setAttribute('time',str(res[i][5]))
		r.appendChild(r2)
		r2=xml_file.createElement('softirq')
		r2.setAttribute('time',str(res[i][5]))
		r.appendChild(r2)
		
		test.appendChild(r)
	xmlnode.appendChild(test)

	return
	


def start_netserver(portnum):
	cmd1='killall -9 netserver >/dev/null 2>/dev/null'
	cmd2='netserver -6 -p '+str(portnum)+' >/dev/null 2>/dev/null; echo $?'
	o=os.popen(cmd1)
	time.sleep(1)
	o=os.popen(cmd2)
	ret=int(o.readline())
	if ret !=0:
		raise Exception('Netserver failure')
	return(ret);
	
def run_test(test):
	sys.stdout.flush()
	global runing_tests
	cmds=test[1] 
	o=subprocess.Popen(cmds,stdout=subprocess.PIPE,close_fds=True,shell=True)
	runing_tests.append(o)
	test[0]=runing_tests.index(o)
	handle=test
	#print 'handle',handle
	#print '...done'
	return handle

def wait_for_test(handle):
	#print 'waiting for end...',
	ret=runing_tests[handle[0]].wait()
	handle.append(ret);
	#print 'ended'
	return handle

def _stream_prepare(local,remote,test=' TCP_STREAM ',len=None,cpu_loc=None,cpu_rem=None,msg_send=None,msg_recv=None,cork=False,nodelay=False,request_size=None,response_size=None,instances=1,comment=None):
	
	settings=[]
	
	settings.append(['test',test])
	settings.append(['local_ip',local])
	settings.append(['remote_ip',remote])

	
	cpu_str=''
	if cpu_loc!=None or cpu_rem!=None:
		if cpu_loc!=None:
			cpu_loc_str=str(cpu_loc)
			settings.append(['local_cpu_bind',str(cpu_loc)])
		else:
			cpu_loc_str=''
		if cpu_rem!=None:
			cpu_rem_str=str(cpu_rem)
			settings.append(['remote_cpu_bind',str(cpu_rem)])
		else:
			cpu_rem_str=''
		cpu_str=' -T '+cpu_loc_str +','+cpu_rem_str
		
	len_str=''	
	if len!=None:
		len_str+=' -l '+ str(len)
		settings.append(['test_length',str(len)])
	
	msg_send_str=''	
	if msg_send!=None:
		msg_send_str+=' -m '+ str(msg_send)
		settings.append(['msg_send_size',str(msg_send)])
	msg_recv_str=''	
	if msg_recv!=None:
		msg_recv_str+=' -M '+ str(msg_recv)
		settings.append(['msg_recv_size',str(msg_recv)])
		
	cork_str=''	
	if cork:
		cork_str=' -C '
		settings.append(['cork_enabled','True'])
		
	nodelay_str=''	
	if nodelay:
		nodelay_str=' -D '
		settings.append(['nodelay_enabled','True'])
		
		
	request_response_str=''
	if request_size!=None or response_size!=None:
		if request_size!=None:
			request_size_str=str(request_size)
			settings.append(['request_message_size',str(request_size)])
		else:
			request_size_str=''
		if response_size!=None:
			response_size_str=str(response_size)
			settings.append(['response_message_size',str(response_size)])
		else:
			response_size_str=''
		request_response_str=' -r '+request_size_str +','+response_size_str
	
	if test=='TCP_MULTISTREAM':
		cmd_test='TCP_STREAM'
	else:
		cmd_test=test
				
	cmd='netperf '+'-t '+cmd_test+' -L '+ local +' -H '+remote +cpu_str +' -cC -P0  '+len_str+ ' -- ' +msg_send_str + msg_recv_str + cork_str + nodelay_str + request_response_str
	settings.append(['netperf_commandline','netperf '+cmd])
	
	#print 'netperf ' + cmd
	if test=='TCP_MULTISTREAM':
		cmd='for i in `seq 1 '+str(instances)+'`; do '+cmd+'& done; wait;'
		settings.append(['concurent_instances',str(instances)])

	if comment:
		settings.append(['comment',str(comment)])		
		
	ret=[]
	ret=['',cmd,settings]
	print '\t   '+cmd
	return ret
	



	
def get_test_res(test):
	line=runing_tests[test[0]].communicate()[0]
	l=line.split()
	res=[]
	if test[3]!=0:
		res.append(['test_failed',str(test[3])])
		res.append(['netperf_stdout',line.replace('\n',' ')])
		return [test[2],res];
	elif test[2][0][1]=='TCP_STREAM' :
		#res.append(['Recv_socket_size',l[0]])
		#res.append(['Send_socket_size',l[1]])
		#res.append(['Send_message_size',l[2]])
		#res.append(['Time_elapsed',l[3]])
		res.append(['Troughput',l[4]])
		res.append(['Send_local_utiliztion',l[5]])
		res.append(['Recv_remote_utiliztion',l[6]])
		res.append(['Service_demand_Send_local',l[7]])
		res.append(['Service_demand_Recv_remote',l[8]])
	elif test[2][0][1]=='UDP_STREAM' :
		#res.append(['Local_socket_size',l[0]])
		#res.append(['Remote_socket_size',l[8]])
		#res.append(['Message_size',l[1]])
		#res.append(['Elapsed_time',l[2]])
		#res.append(['Local_okay_messages',l[3]])
		#res.append(['Remote_okay_messages',l[10]])
		#res.append(['Local_error_messages',l[4]])
		res.append(['Local_Troughput',l[5]])
		res.append(['Remote_Troughput',l[11]])
		res.append(['Local_cpu_utilization',l[6]])
		res.append(['Remote_cpu_utilization',l[12]])
		res.append(['Local_service_demand',l[7]])
		res.append(['Remote_service_demand',l[13]])
	elif (test[2][0][1]=='TCP_CRR'  or test[2][0][1]=='TCP_CC'  or test[2][0][1]=='TCP_CRR'  or test[2][0][1]=='UDP_RR') :
		#res.append(['Local_socket_send_bytes',l[0]])
		#res.append(['Remote_size_recv_bytes',l[1]])
		#res.append(['Request_size',l[2]])
		#res.append(['Response_size',l[3]])
		#res.append(['Time_elapsed',l[4]])
		res.append(['Trans_rate_per_sec',l[5]])
		res.append(['Local_cpu_utilization',l[6]])
		res.append(['Remote_cpu_utilization',l[7]])
		res.append(['Local_service_demand',l[8]])
		res.append(['Remote_service_demand',l[9]])
	elif test[2][0][1]=='TCP_MULTISTREAM' :
		sum_troughput=0;
		sum_slu=0;
		sum_rru=0;
		sum_sdsl=0;
		sum_sdrr=0;
		#print 'parsing results..',
		ll=line.split('\n');
		ll=ll[:len(ll)-1]
		measures = len(ll)
		if int(test[2][7][1]) != int(measures):
			res.append(['test_failed','some of concurent instances did not finished'])
			res.append(['netperf_stdout',' '])
			print 'some of concurent instances did not finish'
			print [test[2],res]
			return [test[2],res]
		for lll in ll:
			lls=lll.split()
			
			sum_troughput+=float(lls[4])
			sum_slu+=float(lls[5])
			sum_rru+=float(lls[6])
			sum_sdsl=float(lls[7])
			sum_sdrr=float(lls[8])
		res.append(['Total_troughput',sum_troughput])
		res.append(['Send_local_utiliztion',sum_slu/measures])
		res.append(['Recv_remote_utiliztion',sum_rru/measures])
		res.append(['Service_demand_Send_local',sum_sdsl/measures])
		res.append(['Service_demand_Recv_remote',sum_sdrr/measures])
		#print 'done'
		
	runing_tests[test[0]].stdout.close()
	runing_tests[test[0]].wait()
	#runing_tests[test[0]].stdout.close()
	#runing_tests[test[0]].stderr.close()
			
	return [test[2],res];
	
def write_test_res(res,xmlnode):
	test=xml_file.createElement('test')
	#test_settings=xml_file.createElement('settings')
	test_resoluts=xml_file.createElement('resoluts')
	#for i in res[0]:
	#	st=xml_file.createElement(i[0])
	#	data=xml_file.createTextNode(str(i[1]))
		#print res[0],res[1]
	#	st.appendChild(data)
	#	test_settings.appendChild(st)
	for i in res[1]:
		print '\t    #',i[0],i[1]
		r=xml_file.createElement(i[0])
		data=xml_file.createTextNode(str(i[1]))
		r.appendChild(data)
		test_resoluts.appendChild(r)
	
	#test.appendChild(test_settings)
	test.appendChild(test_resoluts)
	xmlnode.appendChild(test)
	return
		
	
def tcp_stream_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,msg_send=None,msg_recv=None,cork=False,nodelay=False,comment=None):
	return _stream_prepare(local,remote,'TCP_STREAM',len,cpu_loc,cpu_rem,msg_send,msg_recv,cork,nodelay,None,None,comment)
	
def tcp_maerts_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,msg_send=None,msg_recv=None,cork=False,nodelay=False,comment=None):
	return _stream_prepare(local,remote,'TCP_MAERTS',len,cpu_loc,cpu_rem,msg_send,msg_recv,cork,nodelay,None,None,comment)
	
def udp_stream_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,msg_send=None,msg_recv=None,comment=None):
	return _stream_prepare(local,remote,'UDP_STREAM',len,cpu_loc,cpu_rem,msg_send,msg_recv,None,None,None,None,comment=comment)
	
def tcp_rr_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,request_size=1,response_size=1,cork=False,nodelay=False,comment=None):
	return _stream_prepare(local,remote,'TCP_RR',len,cpu_loc,cpu_rem,None,None,cork,nodelay,request_size,response_size,comment=comment)
	
def tcp_crr_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,request_size=1,response_size=1,cork=False,nodelay=False,comment=None):
	return _stream_prepare(local,remote,'TCP_CRR',len,cpu_loc,cpu_rem,None,None,cork,nodelay,request_size,response_size,comment=comment)
	
def tcp_cc_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,request_size=1,response_size=1,cork=False,nodelay=False,comment=None):
	return _stream_prepare(local,remote,'TCP_CC',len,cpu_loc,cpu_rem,None,None,cork,nodelay,request_size,response_size,comment=comment)
	
def udp_rr_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,request_size=1,response_size=1,comment=None):
	return _stream_prepare(local,remote,'UDP_RR',len,cpu_loc,cpu_rem,None,None,None,None,request_size,response_size,comment=comment)

def tcp_multistream_prepare(local,remote,len=None,cpu_loc=None,cpu_rem=None,msg_send=None,msg_recv=None,cork=False,nodelay=False,comment=None,instances=1):
	return  _stream_prepare(local,remote,'TCP_MULTISTREAM',len,cpu_loc,cpu_rem,msg_send,msg_recv,cork,nodelay,None,None,instances,comment)	
	

	

	
def run_test_concurent(tests):
	handles=[]

	for i in xrange(len(tests)):
		handles.append(run_test(tests[i]))

	return handles;
	
def wait_for_test_concurent(tests):
	handles=[]

	for i in xrange(len(tests)):
		handles.append(wait_for_test(tests[i]))

	return handles;


