########################################################################
#							Interfaces
########################################################################       
import os
import socket
import copy
from tests import *


def ifacelist():
# read the file /proc/net/dev
		f = open('/proc/net/dev','r')

		# put the content to list
		ifacelist = f.read().split('\n') 

		# close the file
		f.close()

		# remove 2 lines header
		ifacelist.pop(0)
		ifacelist.pop(0)
		out=[];

		# loop to check each line
		for line in ifacelist:

			ifacedata = line.replace(' ','').split(':')

			# check the data have 2 elements
			if len(ifacedata) == 2:

					# print the interface
					out.append(ifacedata[0]);
		
		return out;

def get_ip6(ifname,f):
	if f==6:
		cmd='ip -f inet6 a s ' +ifname+' | grep inet6'
		begin=10;
	elif f==4:
		cmd='ip -f inet a s ' +ifname+' | grep inet'
		begin=9;		
	#print cmd;
	out=[]
	o=os.popen(cmd)
	while True:
		line=o.readline()
		if not line:
			break
		end=line.find('/')
		#print line[10:end];
		v6a=line[begin:end]
		if v6a.find('fe80')>=0:
					v6a+='%'+ifname
		out.append(v6a)
	return out;
	

def get_hw(ifname):
    cmd='ip  a s ' +ifname+' | grep ether'
    o=os.popen(cmd)
    return o.readline()[15:32]

def get_hostname():
	cmd='hostname'
	o=os.popen(cmd)
	line=o.readline()
	end=line.find('\n')
	return line[:end]   
	
def get_uname():
	cmd='uname -porv'
	o=os.popen(cmd)
	line=o.readline()
	end=line.find('\n')
	return line[:end] 
	
def get_release():
	cmd='cat /etc/*release 2>/dev/null'
	o=os.popen(cmd)
	line=o.readline()
	end=line.find('\n')
	return line[:end]
	
def get_build():
	f=open('/etc/motd',"r")
	s=f.read()
	i1=s.find("DISTRO=")
	if i1<0:
		return "rhel"
	i1+=len('DISTRO=')
	i2=s.find("\n",i1)
	f.close()
	return s[i1:i2]
   

def get_desc(ifname):
	cmd='ls -l /sys/class/net/'+ifname+'/device 2>/dev/null'
	o=os.popen(cmd)
	line=o.readline();
	begin=len(line)-8
	#print line[begin:]
	
	cmd='lspci | grep ' + line[begin:] + ' 2>/dev/null ' 
	o=os.popen(cmd)
	line=o.readline();
	begin=line.find(':',3)+2
	return line[begin:len(line)-1]

def get_module(ifname):
	cmd='ls -l /sys/class/net/'+ifname+'/device/driver 2>/dev/null'
	o=os.popen(cmd)
	line=o.readline();
	if not line:
			cmd='ls -l /sys/class/net/'+ifname+'/driver 2>/dev/null'
			o=os.popen(cmd)
			line=o.readline();	
	begin=line.rfind('/')+1
	return line[begin:len(line)-1]
	
	
	
def test_connection():
	return 'OK'	

def print_ifaceinfo(iface):
	print 'Device:\t'+iface[0]
	print 'MAC address:\t'+ iface[1]
	print 'IPV4 addresses:\t'+str(iface[2])
	print 'IPV6 addresses:\t'+str(iface[3])
	print 'Hardware:\t'+iface[4]
	print 'Kernel module:\t'+iface[5]
		
def get_ifaceinfo(iface):
	ifi=[]
	ifi.append(iface)
	ifi.append(get_hw(iface));
	ifi.append(get_ip6(iface,4));
	ifi.append(get_ip6(iface,6));
	ifi.append(get_desc(iface));
	ifi.append(get_module(iface));
	return ifi;

def get_rhts_ip(hostname):
	cmd='nslookup '+hostname+' | grep Add | tail -n 1'
	o=os.popen(cmd)
	line=o.readline();
	end=line.find('\n')
	return line[9:end]

def get_turn(ip):
	ip_bytes=ip.split('.')
	return int(ip_bytes[0] + ip_bytes[1] + ip_bytes[2]+ ip_bytes[3]) 		
		
def get_all_info():
	ifl=ifacelist();
	all=[]
	all.append(get_hostname())                      # INDEX 0
	
	all.append([])        							# INDEX 1
	for iface in ifl:
		all[1].append(get_ifaceinfo(iface))
	
	all.append(get_release())						#index[2]
	all.append(socket.gethostbyname(get_hostname()))#index[3]
	all.append(get_uname())							#index[4]
	all.append(get_rhts_ip(all[0]))					#index[5]
	all.append(get_turn(all[5]))					#index[6]
	all.append(get_build())							#index[7]
	
	return all

def get_rhts_ips_v4(host_sorted_dbase):
	rhts_ips=[]
	for host in host_sorted_dbase:
		rhts_ips.append(host[5])
		
	return rhts_ips
	
def get_rhts_ips_v6(host_sorted_dbase):
	
	rhts_ip6s=[]
	for host in host_sorted_dbase:
		v4=(host[5])
		for iface in host[1]:
			if v4 in iface[2]:
				for v6a in iface[3]:
					rhts_ip6s.append(v6a)
					
	return rhts_ip6s
	
def get_my_ips_v4(sync,host_sorted_dbase,include_rhts=True):
	
	if include_rhts:
		rhts_ips=[]
	else:
		rhts_ips=get_rhts_ips_v4(host_sorted_dbase)
		
	myid=sync[4]
	ifacelist=host_sorted_dbase[myid][1]
	my_ips=[]
	for iface in ifacelist:
		for ip in iface[2]:
			if ip!='127.0.0.1' and ip not in rhts_ips:
				my_ips.append(ip)
				
	return my_ips

def get_my_ips_v6(sync,host_sorted_dbase,include_rhts=True):
	
	if include_rhts:
		rhts_ips=[]
	else:
		rhts_ips=get_rhts_ips_v6(host_sorted_dbase)
		
	myid=sync[4]
	ifacelist=host_sorted_dbase[myid][1]
	my_ips=[]
	for iface in ifacelist:
		for ip in iface[3]:
			if ip!='::1' and ip not in rhts_ips:
				my_ips.append(ip)
				
	return my_ips

		
	
def get_other_ips_v4(sync,host_sorted_dbase,include_rhts=True):
	
	if include_rhts:
		rhts_ips=[]
	else:
		rhts_ips=get_rhts_ips_v4(host_sorted_dbase)
	
	myid=sync[4]
	ifacelist=host_sorted_dbase[myid][1]
	other_ips=[]
	others_sorted_dbase=copy.deepcopy(host_sorted_dbase)
	others_sorted_dbase.remove(others_sorted_dbase[myid])
	for host in others_sorted_dbase:
		for iface in host[1]:
			if iface[2]:
				for ip in iface[2]:
					if ip!='127.0.0.1' and ip not in rhts_ips:
						other_ips.append([host[0],ip])
						
	return other_ips
	
def get_other_ips_v6(sync,host_sorted_dbase,include_rhts=True):
	
	if include_rhts:
		rhts_ips=[]
	else:
		rhts_ips=get_rhts_ips_v6(host_sorted_dbase)
		

	
	myid=sync[4]
	ifacelist=host_sorted_dbase[myid][1]
	other_ips=[]
	others_sorted_dbase=copy.deepcopy(host_sorted_dbase)
	others_sorted_dbase.remove(others_sorted_dbase[myid])
	for host in others_sorted_dbase:
		for iface in host[1]:
			if iface[3]:
				for ip in iface[3]:
					if ip!='::1' and ip not in rhts_ips:
						other_ips.append([host[0],ip])
						
	return other_ips
	
	
def get_ip_pairs_v4(sync,host_sorted_dbase,myifaceid,myipindex,theirhostname,theirifaceid,theiripindex,comment=None):
	
	myid=sync[4]
	myhostname=host_sorted_dbase[myid][0]
	
	for host in host_sorted_dbase:
		if host[0]!=myhostname:
			continue
		for iface in host[1]:
			if iface[0]!=myifaceid:
				continue
			myip=iface[2][myipindex]
			break
	
	if theirhostname=='localhost':
		theirhostname=myhostname
		
	for host in host_sorted_dbase:
		if host[0]!=theirhostname:
			continue
		for iface in host[1]:
			if iface[0]!=theirifaceid:
				continue
			theirip=iface[2][theiripindex]
			break				
			
	return myhostname,myip,theirhostname,theirip,comment
	
	
def get_ip_pairs_v6(sync,host_sorted_dbase,myifaceid,myipindex,theirhostname,theirifaceid,theiripindex,comment=None):
	
	myid=sync[4]
	myhostname=host_sorted_dbase[myid][0]
	
	for host in host_sorted_dbase:
		if host[0]!=myhostname:
			continue
		for iface in host[1]:
			if iface[0]!=myifaceid:
				continue
			myip=iface[3][myipindex]
			break
	
	if theirhostname=='localhost':
		theirhostname=myhostname
		
	for host in host_sorted_dbase:
		if host[0]!=theirhostname:
			continue
		for iface in host[1]:
			if iface[0]!=theirifaceid:
				continue
			theirip=iface[3][theiripindex]
			break				
			
	return myhostname,myip,theirhostname,theirip,comment
	

def get_ifaceinfo_by_ip(ip,host_sorted_dbase):
	for host in host_sorted_dbase:
		for iface in host[1]:
			if ip in iface[2] or ip in  iface[3]:
				return iface+[host[0]]
	return None;
		
def print_all_info(all):
	print '================================================================================'
	print "Hostname:\t"+all[0]
	print "Release:\t"+all[2]
	print "Kernel Release\t"+all[4]
	print "RHTS IP\t\t"+all[5]
	print "Turn\t\t"+str(all[6])
	for iface in all[1]:
		print '--------------------------------------------------------------------------------'
		print_ifaceinfo(iface)
	
	print '================================================================================'			
	return all
	
	
def make_bonds(bonds_list):
	print "creating bonding interfaces"
	subprocess.Popen('rmmod bonding',close_fds=True,shell=True).wait()
	subprocess.Popen('modprobe bonding max_bonds='+str(len(bonds_list)),close_fds=True,shell=True).wait()
	for i in range(len(bonds_list)):
		rename_iface('bond'+str(i),bonds_list[i][0])
		for slave in bonds_list[i][1]:
			subprocess.Popen('ifenslave '+bonds_list[i][0]+' '+slave,close_fds=True,shell=True).wait()
			print 'enslaving '+slave+' into ' +bonds_list[i][0]
		for ip4 in bonds_list[i][2]:
			subprocess.Popen('ip a a '+ip4+' dev '+bonds_list[i][0],close_fds=True,shell=True).wait()
			print 'assigning '+bonds_list[i][0] +' address '+ip4
		for ip6 in bonds_list[i][3]:
			subprocess.Popen('ip a a '+ip6+' dev '+bonds_list[i][0],close_fds=True,shell=True).wait()
			print 'assigning '+bonds_list[i][0] +' address '+ip6
	return


def get_iface_by_mac(mac):
	for iface in get_all_info()[1]:
		if iface[1]==mac:
			return iface[0]
	return None
	
def set_ips(interface):
	subprocess.Popen('ip a f '+interface[1],close_fds=True,shell=True).wait()
	for ip4 in interface[2]:
		subprocess.Popen('ip a a '+ip4+' dev '+interface[1],close_fds=True,shell=True).wait()
		print 'assigning '+interface[1] +' address '+ip4

	for ip6 in interface[3]:
		subprocess.Popen('ip a a '+ip6+' dev '+interface[1],close_fds=True,shell=True).wait()
		print 'assigning '+interface[1] +' address '+ip6

	
def rename_iface(old_name,new_name):
	print 'renaming ',old_name,'to',new_name 
	subprocess.Popen('ip link set '+old_name+' down',close_fds=True,shell=True).wait()
	#print 'down..',
	subprocess.Popen('ip link set '+old_name+' name '+new_name,close_fds=True,shell=True).wait()
	#print 'rename.. ',
	subprocess.Popen('ip link set '+new_name+' up',close_fds=True,shell=True).wait()
	#print 'up.. '
	return
	
	
def setup_interface(iface_settings):
	print iface_settings
	old_name=get_iface_by_mac(iface_settings[0])
	if old_name == None:
		return
	rename_iface(old_name,iface_settings[1])
	set_ips(iface_settings);
	return



def ipsec_init():
	ipsec_conf=open('/etc/ipsec.conf','w')
	ipsec_secrets=open('/etc/ipsec.secrets','w')

	config_string= \
	'version	2.0	# conforms to second version of ipsec.conf specification\n'+ \
	'config setup\n'+ \
	'	protostack=netkey\n'+ \
	'	nat_traversal=yes\n'+ \
	'	virtual_private=\n'+  \
	'	oe=off\n'+ \
	'	nhelpers=0\n'
	
	secrets_string="include /etc/ipsec.d/*.secrets\n"

	ipsec_conf.write(config_string);
	ipsec_conf.close();
	ipsec_secrets.write(config_string);
	ipsec_secrets.close();
	return




def make_tunnel(tun):
	
	name,key,family,leftip,rightip,leftnexthop,rightnexthop,leftsubnet,rightsubnet= tun[0],tun[1],tun[2],tun[3],tun[4],tun[5],tun[6],tun[7],tun[8]
	
	if leftnexthop==None:
		leftnexthop=rightip
	if rightnexthop==None:
		rightnexthop=leftip
		
	if leftsubnet==None and rightsubnet==None:
		ipsec_type='transport'
	elif leftsubnet!=None and rightsubnet!=None:
		ipsec_type='tunnel'
	else:
		raise Exception('bad ipsec parameters')
		
	ipsec_conf=open('/etc/ipsec.conf','a')
	ipsec_secrets=open('/etc/ipsec.secrets','a')
	
	ipsec_conf.write(
	'conn '+name+'\n'+
	'	type='+ipsec_type+'\n'+
	'	connaddrfamily='+family+'\n'+
	'	authby=secret'+'\n'+
	'	left='+leftip+'\n'+
	'	leftnexthop='+leftnexthop+'\n'+
	'	right='+rightip+'\n'+
	'	rightnexthop='+rightnexthop+'\n'+
	'	esp=3des-sha1'+'\n'+
	'	keyexchange=ike'+'\n'+
	'	ike=3des-sha1'+'\n'+
	'	pfs=no'+'\n'+
	'	auto=start'+'\n'
	)
	
	
	
	
	ipsec_secrets.write(leftip+" "+rightip+' : PSK "'+key+'"\n')	
	ipsec_conf.close()
	ipsec_secrets.close()
	return
	
