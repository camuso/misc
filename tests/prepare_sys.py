#!/usr/bin/python
from interfaces import *
import time
import os
import sys

hostname=get_hostname()
authorized_pubkeys=['ssh-rsa AAAAB3NzaC1yc2EAAAABIwAAAQEAk/4n4owA+7sRXqG/bhz7Q0lrHccDFAQ7JT6oF+ky/E8UGdBoQ5eGhk/inOsKrpeUAIdNqxG5PGSD6rYOiFO7tXQPw4MdyCHRiGczYaz1LXTWzkkRYc0z3WbiSmD1WZvYElVcnnraPL6i6qTuFU5HRPofM+AKoTt6aJYmFWBY2BBys6yRvOCMm3qua7cQ0YZG3aK3jIf1kGslrF2vjOzb58bRm4+9R6l91rpNjb1JxQX2wUHBQZdkbTdFmEWmSl/mIayXH9WBhdAMLpKjgkYWKauQ6yryDa56W/tJJAVWig6/7c0ieIY4C4ZhaXmBJq2dGsY6OHYEuJbVkX5WYzAuhQ== aokuliar\n']

sysctl_commands= '''
#sysctl net.ipv4.conf.all.forwarding=1  2>/dev/null;
sysctl net.ipv4.tcp_tw_recycle=1 2>/dev/null;
sysctl net.ipv4.tcp_tw_reuse=1 2>/dev/null;	

sysctl net.netfilter.nf_conntrack_tcp_timeout_fin_wait=5  2>/dev/null;  
sysctl net.netfilter.nf_conntrack_tcp_timeout_close_wait=5  2>/dev/null;  
sysctl net.netfilter.nf_conntrack_tcp_timeout_time_wait=5 2>/dev/null;  
sysctl net.nf_conntrack_max=262144  2>/dev/null;  
 

sysctl net.ipv4.ip_conntrack_max=262144  2>/dev/null;
sysctl net.ipv4.netfilter.ip_conntrack_tcp_timeout_time_wait=5  2>/dev/null;  
sysctl net.ipv4.netfilter.ip_conntrack_tcp_timeout_close_wait=5  2>/dev/null; 
sysctl net.ipv4.netfilter.ip_conntrack_tcp_timeout_fin_wait=5  2>/dev/null;  

sysctl net.ipv4.conf.all.send_redirects=0  2>/dev/null
sysctl net.ipv4.conf.default.send_redirects=0  2>/dev/null
echo "1800000" > /proc/sys/net/ipv4/tcp_max_tw_buckets
	
'''


service_commands= '''
service ipsec restart
service iptables stop
service ip6tables stop
service anacron stop
service sendmail stop
service avahi-daemon stop
service ntpd stop
service cpuspeed stop
'''



ifaces_hp_dl380g7_01=[
	['d4:85:64:c7:b2:ee','bnx2_1',['172.16.20.10/24','172.16.20.11/24'],['fd20::11/64','fd20::10/64']],
	['d4:85:64:c7:b2:f0','bnx2_2',['172.16.21.10/24'],['fd21::10/64']],
	['d4:85:64:c7:b2:f2','bnx2_3',['172.16.22.10/24'],['fd22::10/64']],
	
	['00:1b:21:63:89:58','igb0',[],[]],
	['00:1b:21:63:89:59','igb1',[],[]],
	['00:1b:21:63:89:5a','igb2',['172.16.23.10/24'],['fd23::10/64']],
	['00:1b:21:63:89:5b','igb3',['172.16.24.10/24'],['fd24::10/64']],
	
	['00:0e:1e:02:05:a4','netxen0',[],[]],
	['00:0e:1e:02:05:a5','netxen1',[],[]],
	['00:0e:1e:02:05:a6','netxen2',['172.16.25.10/24'],['fd25::10/64']],
	['00:0e:1e:02:05:a7','netxen3',['172.16.26.10/24'],['fd26::10/64']],
	
	['00:1b:21:69:9a:a0','ixgbe',['172.16.29.10/24'],['fd29::10/64']],
]

ifaces_hp_dl385g7_01=[
	['1c:c1:de:1e:4d:8a','bnx2_1',['172.16.20.20/24','172.16.20.21/24'],['fd20::21/64','fd20::20/64']],
	['1c:c1:de:1e:4d:8c','bnx2_2',['172.16.21.20/24'],['fd21::20/64']],
	['1c:c1:de:1e:4d:8e','bnx2_3',['172.16.22.20/24'],['fd22::20/64']],
	
	['00:1b:21:64:06:dc','igb0',[],[]],
	['00:1b:21:64:06:dd','igb1',[],[]],
	['00:1b:21:64:06:de','igb2',['172.16.23.20/24'],['fd23::20/64']],
	['00:1b:21:64:06:df','igb3',['172.16.24.20/24'],['fd24::20/64']],
	
	['00:0e:1e:02:05:80','netxen0',[],[]],
	['00:0e:1e:02:05:81','netxen1',[],[]],
	['00:0e:1e:02:05:82','netxen2',['172.16.25.20/24'],['fd25::20/64']],
	['00:0e:1e:02:05:83','netxen3',['172.16.26.20/24'],['fd26::20/64']],
	
	['00:1b:21:90:c3:86','ixgbe',['172.16.29.20/24'],['fd29::20/64']],
]

ifaces_ibm_x3650m3_02=[
	
	['e4:1f:13:45:00:5a','bnx2_1',[],[]],
	
	
	['00:1b:21:64:06:d8','igb0',[],[]],
    ['00:1b:21:64:06:d9','igb1',[],[]],
    ['00:1b:21:64:06:da','igb2',[],[]],
    ['00:1b:21:64:06:db','igb3',[],[]],

	['00:0e:1e:02:06:80','netxen0',[],[]],
	['00:0e:1e:02:06:81','netxen1',[],[]],
	['00:0e:1e:02:06:82','netxen2',[],[]],
	['00:0e:1e:02:06:83','netxen3',[],[]],

]

tunnels_hp_dl380g7_01_hp_dl385g7_01=[
	[	'tr',	'redhat',	'ipv4',	'172.16.20.11','172.16.20.21',	None,		None,			None,		None,],
	[	'tr6',	'redhat',	'ipv6',	'fd20::11'    ,'fd20::21'    ,	None,		None,			None,		None,]
	#	name,	key,		family,	leftip,			rightip,		leftnexthop,rightnexthop,	leftsubnet,rightsubnet
]

bonds_hp_dl380g7_01=[
	#['bond_bnx2',['bnx2_2','bnx2_3'],['172.16.16.14/23'],['fddd::24/64'],'802.3ad'],
	['bond_igb',['igb0','igb1'],['172.16.27.10/24'],['fd27::10/64'],'0'],
	['bond_netxen',['netxen0','netxen1'],['172.16.28.10/24'],['fd28::10/64'],'0'],
]

bonds_hp_dl385g7_01=[
	#['bond_bnx2',['bnx2_2','bnx2_3'],['172.16.16.24/23'],['fddd::24/64'],'802.3ad'],
	['bond_igb',['igb0','igb1'],['172.16.27.20/24'],['fd27::20/64'],'0'],
	['bond_netxen',['netxen0','netxen1'],['172.16.28.20/24'],['fd28::20/64'],'0'],
]



def add_pubkeys(keys):

	try:
		os.makedirs('/root/.ssh')
	except:
		pass	

	authorized_keys=open('/root/.ssh/authorized_keys',"w")
	
	for key in keys:
		authorized_keys.write(key)
	
	authorized_keys.close()
	subprocess.Popen('chcon -R -t ssh_home_t /root/.ssh >/dev/null 2>&1',close_fds=True,shell=True)
	subprocess.Popen('semanage fcontext -a -t ssh_home_t /root/.ssh >/dev/null 2>&1 ',close_fds=True,shell=True)
	return


def install_netperf():
	if open('/etc/redhat-release').readline().find('5.') >=0:
		subprocess.Popen('rpm -Uvh http://download.devel.redhat.com/brewroot/packages/netperf/2.4.5/1.el5/x86_64/netperf-2.4.5-1.el5.x86_64.rpm',close_fds=True,shell=True).wait()
	elif open('/etc/redhat-release').readline().find('6.') >=0:
		subprocess.Popen('rpm -Uvh http://download.devel.redhat.com/brewroot/packages/netperf/2.4.5/1.el6/x86_64/netperf-2.4.5-1.el6.x86_64.rpm',close_fds=True,shell=True).wait()
	return

def install_openswan():
	subprocess.Popen('yum -y install openswan',close_fds=True,shell=True).wait()


if (hostname=='hp-dl385g7-01.lab.eng.brq.redhat.com'):
	ifaces=ifaces_hp_dl385g7_01
	tunnels=tunnels_hp_dl380g7_01_hp_dl385g7_01
	bonds=bonds_hp_dl385g7_01
elif (hostname=='hp-dl380g7-01.lab.eng.brq.redhat.com'):
	ifaces=ifaces_hp_dl380g7_01
	tunnels=tunnels_hp_dl380g7_01_hp_dl385g7_01
	bonds=bonds_hp_dl380g7_01
elif (hostname=='ibm-x3650m3-02.lab.eng.brq.redhat.com'):
	ifaces=ifaces_ibm_x3650m3_02
	tunnels=[]
	bonds=[]
else:
	ifaces=[]
	tunnels=[]
	bonds=[]
	print "No configuration found for host",hostname
	print "exitting"
	exit(1)



for iface in ifaces:
	setup_interface(iface)

ipsec_init()
for tunnel in tunnels:
	make_tunnel(tunnel)

make_bonds(bonds)

install_openswan()
install_netperf()
add_pubkeys(authorized_pubkeys)
subprocess.Popen(sysctl_commands,close_fds=True,shell=True).wait()
subprocess.Popen(service_commands,close_fds=True,shell=True).wait()

