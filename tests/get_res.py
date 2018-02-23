#!/usr/bin/python
# coding=utf8

import socket
import fcntl
import struct
import os
import sys
sys.path.append('/exports/perf/python')
from SimpleXMLRPCServer import SimpleXMLRPCServer
import xmlrpclib
import time
import signal
import copy
import socket
import subprocess
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.font_manager import fontManager, FontProperties
import lxml.etree as et
import shutil
import math
import random


res_list=[                                 
	['../../../results/netperf/xml_results/dl380g7_RHEL56.xml','RHEL55','black',None,[]],
	['../../../results/netperf/xml_results/dl380g7_RHEL56.xml','RHEL56','yellow',None,[]],
	['../../../results/netperf/xml_results/dl380g7_RHEL57-20110429.xml','RHEL57-20110429','red',None,[]],
	]

for ri in res_list:
	if not os.path.isfile(ri[0]):
		print ri[0], 'not found'
		exit()

out_dir='results'
out_file='index.html'

t_distr=[
[12.71,	4.303,3.182,2.776,2.571,2.447,2.365,2.306,2.262,2.228,2.201,2.179,2.160,2.145,2.131,2.120,2.110,2.101,2.093,2.086],
[6.314,2.2920,2.353,2.132,2.015,1.943,1.895,1.860,1.833,1.812,1.796,1.782,1.771,1.761,1.753,1.746,1.740,1.734,1.729,1.725]
]

names = {
			'tcp_simple_stream_v4': 
			[
				'TCP/IPv4 simple stream test','t4_',
				[
					['Troughput','Troughput versus message size','Message size [Bytes]','Troughput [Mb/s]','_tp'],
					['Send_local_utiliztion','Send local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]','_slu'],
					['Recv_remote_utiliztion','Recv remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]','_rru'],
					#['Service_demand_Send_local','Service demand send local vs message size','Message size [Bytes]','Service demand send local [us/KB]','_sdsl'],
					#['Service_demand_Recv_remote','Service demand recieve remote vs message size','Message size [Bytes]','Service demand recieve remote [us/KB]','_sdrr'],
				]
			
			], 
				
			'udp_simple_stream_v4': 
			[
				'UDP/IPv4 simple stream test','u4_'
			],
			
			'tcp_simple_crr_v4' :   
			[
				'TCP/IPv4 simple connect/request/response test','crr4_',
				[
					['Trans_rate_per_sec','Transaction rate versus message size','Message size [Bytes]','Trans rate [Tr/s]','_trps']
				]
			],
			
			'tcp_crr_stack_performance_v4': 
			[
				'TCP/IPv4 connect/request/response loopback test','pscrr4_',
				[
					['Trans_rate_per_sec','Transaction rate versus message size','Message size [Bytes]','Trans rate [Tr/s]','_trps']
				]
			],
			
			'tcp_stream_stack_performance_v4': 
			[
				'TCP/IPv4 simple stream loopback test','pst4_',
				[
					['Troughput','Troughput versus message size','Message size [Bytes]','Troughput [Mb/s]','_tp'],
					['Send_local_utiliztion','Send local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]','_slu'],
					['Recv_remote_utiliztion','Recv remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]','_rru'],
					#['Service_demand_Send_local','Service demand send local vs message size','Message size [Bytes]','Service demand send local [us/KB]','_sdsl'],
					#['Service_demand_Recv_remote','Service demand recieve remote vs message size','Message size [Bytes]','Service demand recieve remote [us/KB]','_sdrr'],
				]
			],
			
			'tcp_multi_stream_v4': 
			[
				'TCP/IPv4 multi stream test','t4_multi_',
				[
					['Total_troughput','Agregate troughput versus message size','Message size [Bytes]','Troughput [Mb/s]','_altp'],
					['Send_local_utiliztion','Send local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]','_slu'],
					['Recv_remote_utiliztion','Recv remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]','_rru'],					

				]
			],
			
			'tcp_multi_stream_v6': 
			[
				'TCP/IPv6 multi stream test','t6_multi_',
				[
					['Total_troughput','Agregate troughput versus message size','Message size [Bytes]','Troughput [Mb/s]','_altp'],
					['Send_local_utiliztion','Send local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]','_slu'],
					['Recv_remote_utiliztion','Recv remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]','_rru'],					

				]
			],
			
			'tcp_simple_stream_v6': 
			[
				'TCP/IPv6 simple stream test','t6_',
				[
					['Troughput','Troughput versus message size','Message size [Bytes]','Troughput [Mb/s]','_tp'],
					['Send_local_utiliztion','Send local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]','_slu'],
					['Recv_remote_utiliztion','Recv remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]','_rru'],
					#['Service_demand_Send_local','Service demand send local vs message size','Message size [Bytes]','Service demand send local [us/KB]','_sdsl'],
					#['Service_demand_Recv_remote','Service demand recieve remote vs message size','Message size [Bytes]','Service demand recieve remote [us/KB]','_sdrr'],
				]
			], 
			
			'udp_simple_stream_v6': 
			[
				'UDP/IPv6 simple stream test','u6_'
			],
			
			'tcp_simple_crr_v6' :   
			[
				'TCP/IPv6 simple connect/request/response test','crr6_',
				[
					['Trans_rate_per_sec','Transaction rate versus message size','Message size [Bytes]','Trans rate [Tr/s]','_trps']
				]
			],
			
			'tcp_crr_stack_performance_v6': 
			[
				'TCP/IPv6 connect/request/response loopback test','pscrr6_',
				[
					['Trans_rate_per_sec','Transaction rate versus message size','Message size [Bytes]','Trans rate [Tr/s]','_trps']
				]
			],
			
			'tcp_stream_stack_performance_v6': 
			[
			'TCP/IPv6 simple stream loopback test','pst6_',
				[
					['Troughput','Troughput versus message size','Message size [Bytes]','Troughput [Mb/s]','_tp'],
					['Send_local_utiliztion','Send local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]','_slu'],
					['Recv_remote_utiliztion','Recv remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]','_rru'],
					#['Service_demand_Send_local','Service demand send local vs message size','Message size [Bytes]','Service demand send local [us/KB]','_sdsl'],
					#['Service_demand_Recv_remote','Service demand recieve remote vs message size','Message size [Bytes]','Service demand recieve remote [us/KB]','_sdrr'],
				]
			],
	    }
	    
	    
def write_header(htmldoc,hostname):
	html_header='''
	<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
	<html>
	<head>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/> 
	<title>Network performance testing results</title>
	</head>
	<body>
	<link rel="stylesheet" type="text/css" href="stylesheet.css">
	<div class="main">
	<div class="inner">
	<img src="redhat-logo.jpg" alt="Red Hat"  />

	<h1>
	Network performance testing results
	</h1>
	'''
	htmldoc.write(html_header)
	htmldoc.write('<h3 >'+hostname+' </h3>\n')
	htmldoc.write('<hr>')
	return;

def write_footer(htmldoc):
	html_footer='''
	</div>
	</div>
	</body>
	</html>
	'''
	htmldoc.write(html_footer)
	return

def get_iface_attributes(iface):
	srcip = iface.get('src_ip')
	dstip = iface.get('dst_ip')
	srcmac = iface.get('src_mac')
	dstmac = iface.get('dst_mac')	
	srcif = iface.get('src_if')
	dstif = iface.get('dst_if')	
	srchw = iface.get('src_hw')
	dsthw = iface.get('dst_hw')	
	srcdrv = iface.get('src_driver')
	dstdrv = iface.get('dst_driver')	
	dsthname=iface.get('hostname')
	comment=iface.get('comment')
	
	return srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname,comment;
	
def make_iftable(htmldoc,srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname):
	htmldoc.write('<table class="rest">\n')	
	htmldoc.write('<tr>')	
	htmldoc.write('<td>'+hostname[:hostname.find('.')]+'</td><td>'+srcif+'</td><td>'+srchw+'</td><td>'+srcip+'</td><td>'+srcmac+'</td><td>'+srcdrv+'</td>\n')	
	htmldoc.write('</tr><tr>')	
	htmldoc.write('<td>'+dsthname[:hostname.find('.')]+'</td><td>'+dstif+'</td><td>'+dsthw+'</td><td>'+dstip+'</td><td>'+dstmac+'</td><td>'+dstdrv+'</td>\n')
	htmldoc.write('</tr>')	
	htmldoc.write('</table>\n')
	return	


def make_plot(htmldoc,graphlabel,xlabel,ylabel,data,name,semilogx=False,semilogy=False):
	#print data
	ok=True
	l=len(data[0][2])
	for data_item in data[1:]:
		for run in data_item[2]:
			if len(run)!=l:
				ok=False
	if ok:
		print 'writing figure '+name+'.png'
	else:
		print 'figure '+name+' has different vector sizes, skipping plot'
		sys.stdout.flush()
		print l,
		for data_item in data[1:]:
			for run in data_item[2]:
				print len(run),
				
		print ''
		return
	
	htmldoc.write('<a name="'+name+'"></a> \n')	
	htmldoc.write('<img src=\"'+name+'.png\" alt=\"'+graphlabel+'\" class="plot"  />\n')
	

	htmldoc.write('<br>\n')
	htmldoc.write('<table style=\'border-top:2px solid; border-bottom:2px solid; border-color:#CC0000;\'>\n')
	
	htmldoc.write('<tr style=\'border-bottom:1px solid; border-color:#CC0000;\'>')
	htmldoc.write('<td></td>\n')
	htmldoc.write('<td>'+data[0][0]+'</td>\n')
	for m in data[0][2]:
		htmldoc.write('<td>'+str(int(m))+'</td>\n')
	htmldoc.write('</tr>\n')
		
	
	for i in range(1,len(data)):
		
		count=0
		s=[]
		ssq=[]
		devs=[]
		avgs=[]
		ci_mins=[]
		ci_maxes=[]
		errs=[]	 
		
		
		for f in range(len(data[1][2][0])):
			s.append(0.0)
			ssq.append(0.0)
			avgs.append(0.0)
			devs.append(0.0)
			ci_mins.append(0.0)
			ci_maxes.append(0.0)		
			errs.append(0.0)		
		
		for v in range(len(data[i][2])):
			htmldoc.write('<tr>')
			count+=1;
			#print v,((len(data[i][2])+4)/2)
			if v==((len(data[i][2])+4)/2)-1:
				htmldoc.write('<td>'+data[i][0]+'</td>\n')
			else:	
				htmldoc.write('<td></td>\n')
			
			htmldoc.write('<td> run #'+str(count)+'</td>\n')
			for j in range(len(data[i][2][v])):
				r=0#random.random()
				htmldoc.write('<td>'+str(round(float(data[i][2][v][j])+r,2))+'</td>\n')
				s[j]+=float(data[i][2][v][j])+r
				ssq[j]+=((float(data[i][2][v][j]))+r)*((float(data[i][2][v][j]))+r)
			htmldoc.write('</tr>\n')
	
		htmldoc.write('<tr>\n')
		htmldoc.write('<td></td><td>mean val.</td>\n')
		for m in range(len(s)):
			avgs[m]=round((s[m]/count),2)
			htmldoc.write('<td>'+str(avgs[m])+'</td>\n')
		htmldoc.write('</tr>\n')
		
		htmldoc.write('<tr>\n')
		htmldoc.write('<td></td><td>standar dev.</td>\n')
		for m in range(len(s)):
			devs[m]=math.sqrt(math.fabs(round((ssq[m] / (count) - (s[m]/(count)) * (s[m]/(count))),10)*count/(count-1)))
			#devs[m]=math.sqrt((ssq[m]-(1/count)*(s[m])*(s[m]))/(count-1))
			htmldoc.write('<td>'+str(round(devs[m],2))+'</td>\n')
		htmldoc.write('</tr>\n')
		
		htmldoc.write('<tr>\n')
		htmldoc.write('<td></td><td>ci. max.</td>\n')
		for m in range(len(s)):
			errs[m]= t_distr[1][count-2]*devs[m]/math.sqrt(count)
			ci_maxes[m]=round(avgs[m]+errs[m],2)
			htmldoc.write('<td>'+str(ci_maxes[m])+'</td>\n')
		htmldoc.write('</tr>\n')
		
		htmldoc.write('<tr style=\'border-bottom:1px solid; border-color:#CC0000;\'>\n')
		htmldoc.write('<td></td><td>ci. min.</td>\n')
		for m in range(len(s)):
			ci_mins[m]=round(avgs[m]-errs[m],2)
			htmldoc.write('<td>'+str(ci_mins[m])+'</td>\n')
		htmldoc.write('</tr>\n')
	
		data[i].append(avgs)
		data[i].append(ci_mins)
		data[i].append(ci_maxes)
		data[i].append(errs)
	
	htmldoc.write('</table>\n')
	plt.clf()
	#for i in range(1,len(data)):
	#	for j in range(len(data[i][2])): 
	#		p=plt.plot(data[0][2],data[i][2][j],'o-',color=data[i][1],label=data[i][0])
	
	
	for i in range(1,len(data)):
		p=plt.plot(data[0][2],data[i][3],'ro-',color=data[i][1],label=data[i][0])
		plt.errorbar(data[0][2],data[i][3], data[i][6]  ,color=data[i][1], fmt='o',)
	
	plt.grid(True)
	plt.semilogx()
	plt.xlabel(xlabel)
	plt.ylabel(ylabel)
	plt.title(graphlabel)
	font= FontProperties(size='small');
	plt.legend(loc=0, prop=font);
	
	plt.savefig(out_dir+'/'+name)
	plt.clf()
	
	htmldoc.write('<br>\n')	
	return
	

def mine_data(data,res_list,scenario_tag,data_tag,srcip,desc=''):
	dataline=[]
	values=[]
	for res_item in res_list:
		for ifc in res_item[3].findall(scenario_tag)[0].findall('interfaces'):
			if ifc.get('src_ip')==srcip:
				setting=ifc.findall('setting')
				
				for run in range(len(setting[0].findall('test'))):
					for c in setting:
						datalist=c.findall('test')[run].findall('resoluts')[0].findall(data_tag)
						if len(datalist)<=0:
							continue			
						dataline.append(datalist[0].text)
					#print dataline
					values.append(copy.deepcopy(dataline))
					dataline=[]
				data.append([res_item[1]+' '+desc,res_item[2],copy.deepcopy(values)])
				values=[]
		
def mine_data_duplex(data,res_list,scenario_tag,data_tag,i,direction_tag,desc=''):
	dataline=[]
	values=[]
	for res_item in res_list:
		setting=res_item[3].findall(scenario_tag)[0].findall('interfaces')[i].findall('setting')
		for run in range(len(setting[0].findall('pair'))):
			for c in setting:
				datalist=c.findall('pair')[run].findall(direction_tag)[0].findall('test')[0].findall('resoluts')[0].findall(data_tag)
				if len(datalist)<=0:
					continue			
				dataline.append(datalist[0].text)
			#print dataline
			values.append(copy.deepcopy(dataline))
			dataline=[]
		data.append([res_item[1]+' '+desc,res_item[2],copy.deepcopy(values)])
		values=[]
		
		
def mine_message_size(data,res_list,scenario_tag,srcip):
	dataline=[]
	for ifc in res_item[3].findall(scenario_tag)[0].findall('interfaces'):
		if ifc.get('src_ip')==srcip:
			setting=ifc.findall('setting')
	
	
	#concurent=res_item[3].findall(scenario_tag)[0].findall('interfaces')[i].findall('setting')
	for c in setting:
		ms=c.get('message_size')			
		dataline.append(ms)
	data.append(['Message size [B]',res_item[2],copy.deepcopy(dataline)])
	dataline=[]
	


def make_conntent_table(htmldoc,res_list,names):
	htmldoc.write('<a name="menu"></a> \n')
	htmldoc.write('<ul class="outerlist">\n')
	for scenario in res_list[0][3].findall("./*"):
		print scenario.tag
		htmldoc.write('<li>'+names[scenario.tag][0]+'</li>\n')
		htmldoc.write('<ul class="innerlist">\n')
		for interface in scenario.findall("./*"):
			name=interface.get('src_ip')+'->'+interface.get('src_ip')+'\t'+interface.get('comment')
			target=names[scenario.tag][1]+interface.get('src_ip').replace('.','-').replace(':','-').replace('%','-') +'_to_'+ interface.get('dst_ip').replace('.','-').replace(':','-').replace('%','-')
			htmldoc.write('<li><a href="#'+target+'">'+name+'</a></li>')
			print '    ',interface.get('src_ip'),'->',interface.get('src_ip'),'('+interface.get('comment')+')'
		print ' '
		htmldoc.write('</ul>')
	htmldoc.write('</ul>\n')
	htmldoc.write('<br>\n')
	return

def get_result(names,scenario_tag):
	
	heading=names[scenario_tag][0]
	scenario_fname=names[scenario_tag][1]
	plots=names[scenario_tag][2]
	
	if len(res_list[0][3].findall(scenario_tag))<=0:
		return
	
	ifaces=res_list[0][3].findall(scenario_tag)[0].findall('interfaces')
	htmldoc.write('<a name="'+scenario_tag+'"></a> \n')
	data_b=[]
	mine_message_size(data_b,res_list,scenario_tag,ifaces[0].get('src_ip'))

	
	for iface in ifaces:
		srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname,comment = get_iface_attributes(iface)
		name= scenario_fname+srcip.replace('.','-').replace(':','-').replace('%','-') +'_to_'+ dstip.replace('.','-').replace(':','-').replace('%','-')

		htmldoc.write('<a name="'+name+'"></a> \n')		
		htmldoc.write('<h2>'+heading+'</h2>\n')
		
		data=[]
		if len(data_b[0][2])==0:
			continue;
		
		if len(comment)!=0:
			htmldoc.write('<h3>'+comment+'</h3>\n')
		
		make_iftable(htmldoc,srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname)	
		
		for plot in plots:
			data=copy.deepcopy(data_b)		
			mine_data(data,res_list,scenario_tag,plot[0],srcip)
			make_plot(htmldoc,plot[1],plot[2],plot[3],data,name + plot[4],semilogx=True,semilogy=False)
		

		htmldoc.write('<br>\n')
		htmldoc.write('<br>\n')
		htmldoc.write('<div style="text-align: right; "><a href="#menu">back to menu</a></div>')
		htmldoc.write('<hr>\n')
		htmldoc.write('<br>\n')
		
	return
	
	


	if len(res_list[0][3].findall('tcp_simple_stream_v6'))<=0:
		return
		
	ifaces=res_list[0][3].findall('tcp_simple_stream_v6')[0].findall('interfaces')
	htmldoc.write('<a name="tcp_simple_stream_v6"></a> \n')
	data_b=[]
	mine_message_size(data_b,res_list,'tcp_simple_stream_v6',ifaces[0].get('src_ip'))

	for iface in ifaces :
		
		srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname,comment = get_iface_attributes(iface)

		name= 't6_'+srcip.replace('.','-').replace(':','-').replace('%','-') +'_to_'+ dstip.replace('.','-').replace(':','-').replace('%','-')
		htmldoc.write('<a name="'+name+'"></a> \n')
		htmldoc.write('<h2>TCP/IPv6 simple stream test</h2>\n')

		
		data=[]
			
		if len(data_b[0][2])==0:
			continue;
		
		if len(comment)!=0:
			htmldoc.write('<h3>'+comment+'</h3>\n')	
		
		make_iftable(htmldoc,srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname)		
		name= 't6_'+srcip.replace('.','-').replace(':','-').replace('%','-') +'_to_'+ dstip.replace('.','-').replace(':','-').replace('%','-')
		
		
		data=copy.deepcopy(data_b)		
		mine_data(data,res_list,'tcp_simple_stream_v6','Troughput',srcip)	
		make_plot(htmldoc,'Troughput versus message size','Message size [Bytes]','Troughput [Mb/s]',data,name + '_tp',semilogx=True,semilogy=False)
		
		#############################
		

		data=copy.deepcopy(data_b)			
		mine_data(data,res_list,'tcp_simple_stream_v6','Send_local_utiliztion',srcip)
		make_plot(htmldoc,'Send local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]',data,name + '_slu',semilogx=True,semilogy=False)
		
		
		#############################
		
			
		data=copy.deepcopy(data_b)			
		mine_data(data,res_list,'tcp_simple_stream_v6','Recv_remote_utiliztion',srcip)
		make_plot(htmldoc,'Recv remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]',data,name + '_rru',semilogx=True,semilogy=False)
		
		#############################
		
			
		#data=copy.deepcopy(data_b)				
		#mine_data(data,res_list,'tcp_simple_stream_v6','Service_demand_Send_local',srcip)
		#make_plot(htmldoc,'Service demand send local vs message size','Message size [Bytes]','Service demand send local [us/KB]',data,name + '_sdslu',semilogx=True,semilogy=False)	
		
		#############################	
		
		#data=copy.deepcopy(data_b)			
		#mine_data(data,res_list,'tcp_simple_stream_v6','Service_demand_Recv_remote',srcip)	
		#make_plot(htmldoc,'Service demand recieve remote vs message size','Message size [Bytes]','Service demand recieve remote [us/KB]',data,name + '_sdrru',semilogx=True,semilogy=False)
						
		
		htmldoc.write('<br>\n')
		htmldoc.write('<br>\n')
		htmldoc.write('<br>\n')
		htmldoc.write('<div style="text-align: right; "><a href="#menu">back to menu</a></div>')
		htmldoc.write('<hr>\n')
		
	return

def udp_simple_stream_v4():
	if len(res_list[0][3].findall('udp_simple_stream_v4'))<=0:
		return

	ifaces=res_list[0][3].findall('udp_simple_stream_v4')[0].findall('interfaces')

	data_b=[]
	mine_message_size(data_b,res_list,'udp_simple_stream_v4',ifaces[0].get('src_ip'))
	
	for iface in ifaces :


		srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname,comment = get_iface_attributes(iface)
		name= 'u4_'+srcip.replace('.','-').replace(':','-').replace('%','-') +'_to_'+ dstip.replace('.','-').replace(':','-').replace('%','-')

		htmldoc.write('<a name="'+name+'"></a> \n')
		htmldoc.write('<h2>UDP/IPv4 simple stream test</h2>\n')
		
		data=[]

		if len(data_b[0][2])==0:
			continue;
		if len(comment)==0:
			htmldoc.write('<h3>Cleartext</h3>\n')
		else:
			htmldoc.write('<h3>'+comment+'</h3>\n')	
		make_iftable(htmldoc,srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname)		
		
		data=copy.deepcopy(data_b)		
		mine_data(data,res_list,'udp_simple_stream_v4','Local_Troughput',srcip)
		make_plot(htmldoc,'Local troughput versus message size','Message size [Bytes]','Troughput [Mb/s]',data,name + '_tp',semilogx=True,semilogy=False)
		data_n=copy.deepcopy(data)		


		#############################
		
		
		data=copy.deepcopy(data_b)		
		mine_data(data,res_list,'udp_simple_stream_v4','Remote_Troughput',srcip)
		make_plot(htmldoc,'Remote troughput versus message size','Message size [Bytes]','Troughput [Mb/s]',data,name + '_rtp',semilogx=True,semilogy=False)


		#############################
		data_l=copy.deepcopy(data_b)
		for l in range(1,len(data_n)):
			losses=[]
			for m in range(len(data_n[1][2])):
				loss=[]
				for j in xrange(len(data_n[1][2][m])):
					loss.append(float(data_n[l][2][m][j])-float(data[l][2][m][j]))
				if len(data_n[1][2][m])<0:
					continue
				losses.append(copy.deepcopy(loss))
			data_l.append([res_list[l-1][1]+'',res_list[l-1][2],copy.deepcopy(losses)])
			
		make_plot(htmldoc,'UDP loss versus message size','Message size [Bytes]','Troughput [Mb/s]',data_l,name + '_lt',semilogx=True,semilogy=False)
		
			
		#############################	
			
		data=copy.deepcopy(data_b)			
		mine_data(data,res_list,'udp_simple_stream_v4','Local_cpu_utilization',srcip)
		make_plot(htmldoc,'Local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]',data,name + '_slu',semilogx=True,semilogy=False)
		
		
		#############################
		
			
		#data=copy.deepcopy(data_b)			
		#mine_data(data,res_list,'udp_simple_stream_v4','Remote_cpu_utilization',srcip)
		#make_plot(htmldoc,'Remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]',data,name + '_rru',semilogx=True,semilogy=False)
		
		#############################
		
			
		#data=copy.deepcopy(data_b)				
		#mine_data(data,res_list,'udp_simple_stream_v4','Local_service_demand',srcip)
		#make_plot(htmldoc,'Service demand send local vs message size','Message size [Bytes]','Service demand send local [us/KB]',data,name + '_sdslu',semilogx=True,semilogy=False)	
		
		#############################	
		
		#data=copy.deepcopy(data_b)			
		#mine_data(data,res_list,'udp_simple_stream_v4','Remote_service_demand',srcip)	
		#make_plot(htmldoc,'Service demand recieve remote vs message size','Message size [Bytes]','Service demand recieve remote [us/KB]',data,name + '_sdrru',semilogx=True,semilogy=False)
						
		
		htmldoc.write('<br>\n')
		htmldoc.write('<br>\n')
		htmldoc.write('<div style="text-align: right; "><a href="#menu">back to menu</a></div>')
		htmldoc.write('<hr>\n')
		htmldoc.write('<br>\n')
	return
	
def udp_simple_stream_v6():
	if len(res_list[0][3].findall('udp_simple_stream_v6'))<=0:
		return

	ifaces=res_list[0][3].findall('udp_simple_stream_v6')[0].findall('interfaces')

	data_b=[]
	mine_message_size(data_b,res_list,'udp_simple_stream_v6',ifaces[0].get('src_ip'))
	

	for iface in ifaces:


		srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname,comment = get_iface_attributes(iface)
		name= 'u6_'+srcip.replace('.','-').replace(':','-').replace('%','-') +'_to_'+ dstip.replace('.','-').replace(':','-').replace('%','-')
		
		htmldoc.write('<a name="'+name+'"></a> \n')
		htmldoc.write('<h2>UDP/IPv6 simple stream test</h2>\n')

		data=[]


		if len(data_b[0][2])==0:
			continue;
		if len(comment)!=0:
			htmldoc.write('<h3>'+comment+'</h3>\n')	
		make_iftable(htmldoc,srcip,dstip,srcmac,dstmac,srcif,dstif,srchw,dsthw,srcdrv,dstdrv,dsthname)		
		
		data=copy.deepcopy(data_b)		
		mine_data(data,res_list,'udp_simple_stream_v6','Local_Troughput',srcip)
		make_plot(htmldoc,'Local troughput versus message size','Message size [Bytes]','Troughput [Mb/s]',data,name + '_tp',semilogx=True,semilogy=False)
		data_n=copy.deepcopy(data)		


		#############################
		
		
		data=copy.deepcopy(data_b)		
		mine_data(data,res_list,'udp_simple_stream_v6','Remote_Troughput',srcip)
		make_plot(htmldoc,'Remote troughput versus message size','Message size [Bytes]','Troughput [Mb/s]',data,name + '_rtp',semilogx=True,semilogy=False)


		#############################
		data_l=copy.deepcopy(data_b)
		for l in range(1,len(data_n)):
			losses=[]
			for m in range(len(data_n[1][2])):
				loss=[]
				for j in xrange(len(data_n[1][2][m])):
					loss.append(float(data_n[l][2][m][j])-float(data[l][2][m][j]))
				if len(data_n[1][2][m])<0:
					continue
				losses.append(copy.deepcopy(loss))
			data_l.append([res_list[l-1][1]+' Lost UDP traffic [Mbit/s]',res_list[l-1][2],copy.deepcopy(losses)])
			
		make_plot(htmldoc,'UDP loss versus message size','Message size [Bytes]','Troughput [Mb/s]',data_l,name + '_lt',semilogx=True,semilogy=False)
		
			
		#############################	
			
		data=copy.deepcopy(data_b)			
		mine_data(data,res_list,'udp_simple_stream_v6','Local_cpu_utilization',srcip)
		make_plot(htmldoc,'Local CPU utilization vs message size','Message size [Bytes]','Send local utilization [%]',data,name + '_slu',semilogx=True,semilogy=False)
		
		
		#############################
		
			
		#data=copy.deepcopy(data_b)			
		#mine_data(data,res_list,'udp_simple_stream_v6','Remote_cpu_utilization',srcip)
		#make_plot(htmldoc,'Remote CPU utilization vs message size','Message size [Bytes]','Recieve remote utilization [%]',data,name + '_rru',semilogx=True,semilogy=False)
		
		#############################
		
			
		#data=copy.deepcopy(data_b)				
		#mine_data(data,res_list,'udp_simple_stream_v6','Local_service_demand',srcip)
		#make_plot(htmldoc,'Service demand send local vs message size','Message size [Bytes]','Service demand send local [us/KB]',data,name + '_sdslu',semilogx=True,semilogy=False)	
		
		#############################	
		
		#data=copy.deepcopy(data_b)			
		#mine_data(data,res_list,'udp_simple_stream_v6','Remote_service_demand',srcip)	
		#make_plot(htmldoc,'Service demand recieve remote vs message size','Message size [Bytes]','Service demand recieve remote [us/KB]',data,name + '_sdrru',semilogx=True,semilogy=False)
						
		
		htmldoc.write('<br>\n')
		htmldoc.write('<br>\n')
		htmldoc.write('<div style="text-align: right; "><a href="#menu">back to menu</a></div>')
		htmldoc.write('<hr>\n')
		htmldoc.write('<br>\n')
	return






for res_item in res_list:
	print 'parsing',res_item[0]
	res_item[3] = et.parse(res_item[0]).getroot()	
print 'parsing done	'

try:
	shutil.rmtree('./'+out_dir)
except:
	pass
os.makedirs(out_dir)
shutil.copyfile('./redhat-logo.jpg',out_dir+'/redhat-logo.jpg')
shutil.copyfile('./stylesheet.css',out_dir+'/stylesheet.css')
htmldoc=open(out_dir+'/'+out_file,'w')
hostname=res_list[0][3].findall(".")[0].get('hostname')


write_header(htmldoc,hostname);

make_conntent_table(htmldoc,res_list,names)
htmldoc.write('<hr>\n')


for scenario in res_list[0][3].findall("./*"):
	print scenario.tag
	if scenario.tag == 'udp_simple_stream_v4':
		udp_simple_stream_v4()
	elif scenario.tag == 'udp_simple_stream_v6':
		udp_simple_stream_v6()
	else:
		get_result(names,scenario.tag)

write_footer(htmldoc)
htmldoc.close()
