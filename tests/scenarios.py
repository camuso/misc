import os 
import xml.dom.minidom
from interfaces import *
from tests import *

###CONFIGURATION SECTION################################

rpcport=8888
netserverport=12865
servers=['hp-dl380g7-01.lab.eng.brq.redhat.com']
clients=['hp-dl385g7-01.lab.eng.brq.redhat.com']
#output_file='test_res.xml'



###SCENARIO SETTINGS#####################################
	
#message_sizes=1,2,5,10,32,64,100,200,1000,1500,7000,9000
message_sizes=10,16,30,64,100,128,200,512,1024,1400,6144

test_length=20
crr_test_length=10
test_runs=5
concurent_test_runs=5
concurent_instances=24
concurent_test_length=60
	
#########################################################



	
	
scenario_list=[];
xml_file=xml.dom.minidom.Document()

def activate_scenario(scenario,description):
	global scenario_list;
	scenario_list.append([scenario,description,'not_started'])
	return


def tcp_stream_stack_performance_v4(sync,host_sorted_dbase,xmlnode):
	
	s=xml_file.createElement("tcp_stream_stack_performance_v4")
	
	iplist=[]
	iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',0,'localhost','bnx2_1',0,'Broadcom NetXtreme II'))
	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
		s2=xml_file.createElement("interfaces")
		ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
		ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
		s2.setAttribute("src_if", ifaceinfo_s[0])
		s2.setAttribute("dst_if", ifaceinfo_d[0])
		s2.setAttribute("src_ip", pip[1])
		s2.setAttribute("dst_ip", pip[3])
		s2.setAttribute("src_mac",ifaceinfo_s[1] )
		s2.setAttribute("dst_mac",ifaceinfo_d[1] )
		s2.setAttribute("src_hw",ifaceinfo_s[4] )
		s2.setAttribute("dst_hw",ifaceinfo_d[4] )
		s2.setAttribute("src_driver",ifaceinfo_s[5] )
		s2.setAttribute("dst_driver",ifaceinfo_d[5] )
		s2.setAttribute("hostname",ifaceinfo_d[6] )
		s2.setAttribute("comment",pip[4] )
				
		for m in message_sizes:
			s3=xml_file.createElement("setting")
			s3.setAttribute("message_size", str(m))
			for r in range(test_runs):
				t1 = tcp_stream_prepare(pip[1],pip[3],len=test_length,msg_send=m,msg_recv=m,cpu_loc=0,cpu_rem=1)
				#print t1
				t1=run_test(t1)	
				t1=wait_for_test(t1)						
				r1 = get_test_res(t1)
				write_test_res(r1,s3)
				#print r
			s2.appendChild(s3)
		s.appendChild(s2)
	xmlnode.appendChild(s)
	return

def tcp_crr_stack_performance_v4(sync,host_sorted_dbase,xmlnode):

	
	s=xml_file.createElement("tcp_crr_stack_performance_v4")
	
	iplist=[]
	iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',0,'localhost','bnx2_1',0,'Broadcom NetXtreme II'))

	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
		s2=xml_file.createElement("interfaces")
		ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
		ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
		s2.setAttribute("src_if", ifaceinfo_s[0])
		s2.setAttribute("dst_if", ifaceinfo_d[0])
		s2.setAttribute("src_ip", pip[1])
		s2.setAttribute("dst_ip", pip[3])
		s2.setAttribute("src_mac",ifaceinfo_s[1] )
		s2.setAttribute("dst_mac",ifaceinfo_d[1] )
		s2.setAttribute("src_hw",ifaceinfo_s[4] )
		s2.setAttribute("dst_hw",ifaceinfo_d[4] )
		s2.setAttribute("src_driver",ifaceinfo_s[5] )
		s2.setAttribute("dst_driver",ifaceinfo_d[5] )
		s2.setAttribute("hostname",ifaceinfo_d[6] )
		s2.setAttribute("comment",pip[4] )

			
			
		for m in message_sizes:
			s3=xml_file.createElement("setting")
			s3.setAttribute("message_size", str(m))
			for r in range(test_runs):
				#time.sleep(5)
				t1 = tcp_crr_prepare(pip[1],pip[3],len=crr_test_length,request_size=m,response_size=m,cpu_loc=0,cpu_rem=1)
				#print t1
				t1=run_test(t1)	
				t1=wait_for_test(t1)										
				r1 = get_test_res(t1)
				write_test_res(r1,s3)
				#print r
			s2.appendChild(s3)
		s.appendChild(s2)
	#time.sleep(5)
	xmlnode.appendChild(s)
	return
	
def tcp_crr_stack_performance_v6(sync,host_sorted_dbase,xmlnode):
	
	s=xml_file.createElement("tcp_crr_stack_performance_v6")
	
	iplist=[]
	iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',0,'localhost','bnx2_1',0,'Broadcom NetXtreme II'))
		
	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
		s2=xml_file.createElement("interfaces")
		ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
		ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
		s2.setAttribute("src_if", ifaceinfo_s[0])
		s2.setAttribute("dst_if", ifaceinfo_d[0])
		s2.setAttribute("src_ip", pip[1])
		s2.setAttribute("dst_ip", pip[3])
		s2.setAttribute("src_mac",ifaceinfo_s[1] )
		s2.setAttribute("dst_mac",ifaceinfo_d[1] )
		s2.setAttribute("src_hw",ifaceinfo_s[4] )
		s2.setAttribute("dst_hw",ifaceinfo_d[4] )
		s2.setAttribute("src_driver",ifaceinfo_s[5] )
		s2.setAttribute("dst_driver",ifaceinfo_d[5] )
		s2.setAttribute("hostname",ifaceinfo_d[6] )
		s2.setAttribute("comment",pip[4] )
			
		for m in message_sizes:
			s3=xml_file.createElement("setting")
			s3.setAttribute("message_size", str(m))
			for r in range(test_runs):
				#time.sleep(5)
				t1 = tcp_crr_prepare(pip[1],pip[3],len=crr_test_length,request_size=m,response_size=m,cpu_loc=0,cpu_rem=1)
				#print t1
				t1=run_test(t1)	
				t1=wait_for_test(t1)					
				r1 = get_test_res(t1)
				write_test_res(r1,s3)
			s2.appendChild(s3)
		s.appendChild(s2)
	#time.sleep(5)
	xmlnode.appendChild(s)
	return
	

def tcp_stream_stack_performance_v6(sync,host_sorted_dbase,xmlnode):
	

	s=xml_file.createElement("tcp_stream_stack_performance_v6")
	
	iplist=[]
	iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',0,'localhost','bnx2_1',0,'Broadcom NetXtreme II'))

	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
		s2=xml_file.createElement("interfaces")
		ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
		ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
		s2.setAttribute("src_if", ifaceinfo_s[0])
		s2.setAttribute("dst_if", ifaceinfo_d[0])
		s2.setAttribute("src_ip", pip[1])
		s2.setAttribute("dst_ip", pip[3])
		s2.setAttribute("src_mac",ifaceinfo_s[1] )
		s2.setAttribute("dst_mac",ifaceinfo_d[1] )
		s2.setAttribute("src_hw",ifaceinfo_s[4] )
		s2.setAttribute("dst_hw",ifaceinfo_d[4] )
		s2.setAttribute("src_driver",ifaceinfo_s[5] )
		s2.setAttribute("dst_driver",ifaceinfo_d[5] )
		s2.setAttribute("hostname",ifaceinfo_d[6] )
		s2.setAttribute("comment",pip[4] )
				
		for m in message_sizes:
			s3=xml_file.createElement("setting")
			s3.setAttribute("message_size", str(m))
			for r in range(test_runs):

				t1 = tcp_stream_prepare(pip[1],pip[3],len=test_length,msg_send=m,msg_recv=m,cpu_loc=0,cpu_rem=1)
				#print t1
				t1=run_test(t1)	
				t1=wait_for_test(t1)								
				r1 = get_test_res(t1)
				write_test_res(r1,s3)
				#print r
			s2.appendChild(s3)
		s.appendChild(s2)
	xmlnode.appendChild(s)
	return


def tcp_simple_stream_v4(sync,host_sorted_dbase,xmlnode):
		
	s=xml_file.createElement("tcp_simple_stream_v4")
	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]
	
	iplist=[]
	
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))
			
	for pip in iplist:			
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(test_runs):
					t1 = tcp_stream_prepare(pip[1],pip[3],len=test_length,msg_send=m,msg_recv=m)
					t1=run_test(t1)	
					t1=wait_for_test(t1)
					#print t1							
					r1 = get_test_res(t1)
					write_test_res(r1,s3)
				s2.appendChild(s3)
			s.appendChild(s2)
	xmlnode.appendChild(s)
	return
	
	
def tcp_simple_crr_v4(sync,host_sorted_dbase,xmlnode):
	

	s=xml_file.createElement("tcp_simple_crr_v4")
	
	iplist=[]
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))
			
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(test_runs):
					#time.sleep(5)
					t1 = tcp_crr_prepare(pip[1],pip[3],len=crr_test_length,request_size=m,response_size=m)
					#print t1
					t1=run_test(t1)	
					t1=wait_for_test(t1)									
					r1 = get_test_res(t1)
					write_test_res(r1,s3)
				s2.appendChild(s3)
			s.appendChild(s2)
	#time.sleep(5)
	xmlnode.appendChild(s)
	return
	
def tcp_simple_crr_v6(sync,host_sorted_dbase,xmlnode):
	

	s=xml_file.createElement("tcp_simple_crr_v6")
	
	iplist=[]
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))
			
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(test_runs):
					time.sleep(5)
					t1 = tcp_crr_prepare(pip[1],pip[3],len=crr_test_length,request_size=m,response_size=m)
					#print t1
					t1=run_test(t1)	
					t1=wait_for_test(t1)				
					r1 = get_test_res(t1)
					write_test_res(r1,s3)
					#print r1
				s2.appendChild(s3)
			s.appendChild(s2)
	time.sleep(5)
	xmlnode.appendChild(s)
	return

	

def udp_simple_stream_v4(sync,host_sorted_dbase,xmlnode):
	
	
	
	s=xml_file.createElement("udp_simple_stream_v4")

	iplist=[]
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))
			
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(test_runs):
					t1 = udp_stream_prepare(pip[1],pip[3],len=test_length,msg_send=m,msg_recv=m)
					#print t1
					t1=run_test(t1)	
					t1=wait_for_test(t1)									
					r1 = get_test_res(t1)
					write_test_res(r1,s3)
					#print r
				s2.appendChild(s3)
			s.appendChild(s2)
	xmlnode.appendChild(s)
	return

def tcp_simple_stream_v6(sync,host_sorted_dbase,xmlnode):
	
	
	s=xml_file.createElement("tcp_simple_stream_v6")
	
	iplist=[]
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))
			
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(test_runs):
					t1 = tcp_stream_prepare(pip[1],pip[3],len=test_length,msg_send=m,msg_recv=m)
					#print t1
					t1=run_test(t1)	
					t1=wait_for_test(t1)				
					r1 = get_test_res(t1)
					write_test_res(r1,s3)
				s2.appendChild(s3)
			s.appendChild(s2)
	xmlnode.appendChild(s)
	return
	
	
	
def udp_simple_stream_v6(sync,host_sorted_dbase,xmlnode):
	
	s=xml_file.createElement("udp_simple_stream_v6")

	iplist=[]
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))
	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]

	for pip in iplist:
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(test_runs):
					t1 = udp_stream_prepare(pip[1],pip[3],len=test_length,msg_send=m,msg_recv=m)
					#print t1
					t1=run_test(t1)	
					t1=wait_for_test(t1)					
					r1 = get_test_res(t1)
					write_test_res(r1,s3)
				s2.appendChild(s3)
			s.appendChild(s2)
	xmlnode.appendChild(s)
	return



def tcp_multi_stream_v4(sync,host_sorted_dbase,xmlnode):
		
	s=xml_file.createElement("tcp_multi_stream_v4")
	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]
	
	iplist=[]
	
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v4(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))

			
			
	for pip in iplist:			
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			s2.setAttribute("concurent_instances",str(concurent_instances) )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(concurent_test_runs):
					t1 = tcp_multistream_prepare(pip[1],pip[3],len=concurent_test_length,msg_send=m,msg_recv=m,instances=concurent_instances)	
					
					
					start=time.time()
					#print 'starting local'
					#sys.stdout.flush()
					t1 = run_test(t1)
					#print 'starting done',time.time()-start
					#sys.stdout.flush()
					
					#print 'waiting for local'
					#sys.stdout.flush()
					t1 = wait_for_test(t1)	
					#print 'tests done', time.time()-start
					sys.stdout.flush()
					
					
					
					sys.stdout.flush()
					r1 = get_test_res(t1)
					write_test_res(r1,s3)			
					
					
				s2.appendChild(s3)
			s.appendChild(s2)
	xmlnode.appendChild(s)
	return
	
def tcp_multi_stream_v6(sync,host_sorted_dbase,xmlnode):
		
	s=xml_file.createElement("tcp_multi_stream_v6")
	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]
	
	iplist=[]
	
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))

			
			
	for pip in iplist:			
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			s2.setAttribute("concurent_instances",str(concurent_instances) )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(concurent_test_runs):
					t1 = tcp_multistream_prepare(pip[1],pip[3],len=concurent_test_length,msg_send=m,msg_recv=m,instances=concurent_instances)	
					
					
					start=time.time()
					#print 'starting local'
					#sys.stdout.flush()
					t1 = run_test(t1)
					#print 'starting done',time.time()-start
					#sys.stdout.flush()
					
					#print 'waiting for local'
					#sys.stdout.flush()
					t1 = wait_for_test(t1)	
					#print 'tests done', time.time()-start
					sys.stdout.flush()
					
					
					
					sys.stdout.flush()
					r1 = get_test_res(t1)
					write_test_res(r1,s3)			
					
					
				s2.appendChild(s3)
			s.appendChild(s2)
	xmlnode.appendChild(s)
	return
	

def tcp_multi_duplex_stream_v6(sync,host_sorted_dbase,xmlnode):
		
	s=xml_file.createElement("tcp_multi_stream_v6")
	
	connections=sync[3]
	concurent_tests_local=[]
	concurent_tests_remote=[]
	
	iplist=[]
	
	for h in host_sorted_dbase:
		if h[0]!=get_hostname():
			#iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))
			#iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bnx2_1',1,h[0],'bnx2_1',1,'IPSEC transport with 3des and sha1 across Broadcom'))
			#iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'igb2',0,h[0],'igb2',0,'Intel Corporation 82580 Gigabit ethernet'))
			#iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'netxen2',0,h[0],'netxen2',0,'NetXen NX3031 Gigabit ethernet'))
			#iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_igb',0,h[0],'bond_igb',0,'Bonding of two Intel 82580 gigabit NICs'))
			#iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'bond_netxen',0,h[0],'bond_netxen',0,'Bonding of two NetXen NX3031 gigabit NICs'))
			#iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'ixgbe',0,h[0],'ixgbe',0,'Intel 82599EB 10-Gigabit NIC'))

			iplist.append(get_ip_pairs_v6(sync,host_sorted_dbase,'eth10',0,h[0],'bnx2_1',0,'Broadcom NetXtreme II'))

			
	for pip in iplist:			
			s2=xml_file.createElement("interfaces")
			ifaceinfo_s=get_ifaceinfo_by_ip(pip[1],host_sorted_dbase)
			ifaceinfo_d=get_ifaceinfo_by_ip(pip[3],host_sorted_dbase)
			s2.setAttribute("src_if", ifaceinfo_s[0])
			s2.setAttribute("dst_if", ifaceinfo_d[0])
			s2.setAttribute("src_ip", pip[1])
			s2.setAttribute("dst_ip", pip[3])
			s2.setAttribute("src_mac",ifaceinfo_s[1] )
			s2.setAttribute("dst_mac",ifaceinfo_d[1] )
			s2.setAttribute("src_hw",ifaceinfo_s[4] )
			s2.setAttribute("dst_hw",ifaceinfo_d[4] )
			s2.setAttribute("src_driver",ifaceinfo_s[5] )
			s2.setAttribute("dst_driver",ifaceinfo_d[5] )
			s2.setAttribute("hostname",ifaceinfo_d[6] )
			s2.setAttribute("comment",pip[4] )
			s2.setAttribute("concurent_instances",str(concurent_instances) )
			
			for m in message_sizes:
				s3=xml_file.createElement("setting")
				s3.setAttribute("message_size", str(m))
				for r in range(test_runs):
					t1 = tcp_multistream_prepare(pip[1],pip[3],len=concurent_test_length,msg_send=m,msg_recv=m,instances=concurent_instances)	
					t2 = tcp_multistream_prepare(pip[3],pip[1],len=concurent_test_length,msg_send=m,msg_recv=m,instances=concurent_instances)	
					
					
					print 'runing remote'
					sys.stdout.flush()
					t2 = connections[pip[2]].run_test(t2)								
					print 'done'
					sys.stdout.flush()
					
					print 'runing local'
					sys.stdout.flush()
					t1 = run_test(t1)
					print 'done'
					sys.stdout.flush()
					
					print 'waiting for local'
					sys.stdout.flush()
					t1 = wait_for_test(t1)	
					print 'done'
					sys.stdout.flush()
					
					print 'waiting for remote'				
					sys.stdout.flush()
					t2 = connections[pip[2]].wait_for_test(t2)
					print 'done'
					sys.stdout.flush()
					
					print 'getting local res'
					sys.stdout.flush()
					r1 = get_test_res(t1)
					print 'done'
					sys.stdout.flush()
					
					print 'getting remote res'
					sys.stdout.flush()
					r2 = connections[pip[2]].get_test_res(t2)	
					print 'done'
					sys.stdout.flush()
					
					s4=xml_file.createElement("pair")
					s5=xml_file.createElement("local")
					write_test_res(r1,s5)			
					s5_2=xml_file.createElement("remote")
					write_test_res(r2,s5_2)
					s4.appendChild(s5)
					s4.appendChild(s5_2)
					s3.appendChild(s4)
					
					
				s2.appendChild(s3)
			s.appendChild(s2)
	xmlnode.appendChild(s)
	return


if get_hostname() == 'hp-dl380g7-01.lab.eng.brq.redhat.com':
#if True:
	
	#activate_scenario(tcp_crr_stack_performance_v4,"TCP/IPv4 TCP protocol stack connect/request/response performance")
	#activate_scenario(tcp_stream_stack_performance_v4,"TCP/IPv4 TCP protocol stack troughput performance")
	
	#activate_scenario(tcp_crr_stack_performance_v6,"TCP/IPv6 protocol stack connect/request/response performance")
	#activate_scenario(tcp_stream_stack_performance_v6,"TCP/IPv6 protocol stack troughput performance")
	
	activate_scenario(tcp_simple_stream_v4,"TCP/IPv4 Simple stream test")
	activate_scenario(udp_simple_stream_v4,"UDP/IPv4 Simple stream test")
	activate_scenario(tcp_simple_crr_v4,"TCP/IPv4 Simple connect/request/response test")	
	
	activate_scenario(tcp_simple_stream_v6,"TCP/IPv6 Simple stream test")
	activate_scenario(udp_simple_stream_v6,"UDP/IPv6 Simple stream test")
	activate_scenario(tcp_simple_crr_v6,"TCP/IPv6 Simple connect/request/response test")
	activate_scenario(tcp_multi_stream_v4,"TCP/IPv4 Multiinstance stream test")
	activate_scenario(tcp_multi_stream_v6,"TCP/IPv6 Multiinstance stream test")
	
	


