# -*- coding: UTF-8 -*-
from __future__ import unicode_literals 
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import os
import csv

mediaData1 = 0.0
mediaData2 = 0.0

line1 = []
line2 = []
lineaux = [0,1,2,3,4,5,6]
#--------------------------------DEFININDO A PRIMEIRA FUNÇÃO--------------------------------------------#

fileList = os.listdir('/home/vinicius/Queria ser o Desktop/Trabalho Transmissão/quadra 5m canal 6/')
fileList = ['/home/vinicius/Queria ser o Desktop/Trabalho Transmissão/quadra 5m canal 6/'+filename for filename in fileList]

ListRead = []
for f in fileList:
		cr = csv.reader(open(f,"rb"))
		for row in cr:
			ListRead.append(row[2].split())
data1 = [-5000]
for x in range(20,125,21):
	ListRead[x] = str(ListRead[x]).replace("'","")
	ListRead[x] = str(ListRead[x]).replace("[","")
	ListRead[x] = str(ListRead[x]).replace("]","")
	data1.append(float(ListRead[x])/5)
#--------------------------------DEFININDO A SEGUNDA FUNÇÃO---------------------------------------------#
#fileList = os.listdir('/home/vinicius/Queria ser o Desktop/Trabalho Transmissão/hall 1m canal 125 obs/')
#fileList = ['/home/vinicius/Queria ser o Desktop/Trabalho Transmissão/hall 1m canal 125 obs/'+filename for filename in fileList]

#ListRead = []
#for f in fileList:
#		cr = csv.reader(open(f,"rb"))
#		for row in cr:
#			ListRead.append(row[2].split())
#data2 = [-5000]
#for x in range(20,125,21):
#	ListRead[x] = str(ListRead[x]).replace("'","")
#	ListRead[x] = str(ListRead[x]).replace("[","")
#	ListRead[x] = str(ListRead[x]).replace("]","")
#	data2.append(float(ListRead[x])/5)
#--------------------------------DEFININDO O TITULO DO GŔAFICO------------------------------------------#

title = 'Simulação Quadra Poliesportiva da UTFPR-CP: 5 metros(Sem Obstáculos)'

#-------------------------------------------------------------------------------------------------------#
for aux in range(1,6):
	mediaData1 = mediaData1 + float(data1[aux])
	#mediaData2 = mediaData2 + float(data2[aux])

mediaData1 = (mediaData1/5)
#mediaData2 = (mediaData2/5)

for aux in xrange(7):
	line1.append(mediaData1)
	#line2.append(mediaData2)

x1 = np.array(range(len(data1)))
#x2 = np.array(range(len(data2)))

#plt.plot( x2, data2, 'k:', color='blue') # linha pontilha azul
plt.plot( x1, data1, 'k:', color='red') # linha pontilha vermelha
plt.plot( x1, data1, 'ro', markersize = 13)
#plt.plot( x2, data2, 's', markersize = 8)

plt.plot(lineaux,line1, color = 'red')
#plt.plot(lineaux,line2, color = 'blue')


plt.title(title)

plt.grid(True)
plt.xlabel("Número dos Pacotes")
plt.ylabel("Quantidade de bits perdidos")

red_patch2 = mpatches.Patch(color='red', label='Canal 6 - Média = '+ str(round(mediaData1,2)))
#blue_patch2 = mpatches.Patch(color='blue', label='Canal 125 - Média = '+ str(round(mediaData2,2)))

#plt.legend(handles=[red_patch2,blue_patch2],loc='upper left')
plt.legend(handles=[red_patch2],loc='upper left')

plt.axis([0.5,5.5,-2,18])
plt.savefig(title + '.png')
plt.show()