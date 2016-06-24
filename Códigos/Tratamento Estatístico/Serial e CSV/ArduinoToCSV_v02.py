import serial
import csv
from datetime import datetime
##----------------------------------------------------------------##
stop = 1
error = 0
##----------------------------------------------------------------##
ser = serial.Serial('/dev/ttyACM0',115200,timeout = 5)
ser.flush()

csvName = "Transmission - " + str(datetime.now()) + ".csv" ## Nomeia Csv
c = csv.writer(open(csvName, "wb")) ## Cria Cvs
List = ['Numero do Pacote','Conteudo','Erro Instantaneo'] ## Define colunas
c.writerow(List) ## Escreve no Arquivo

def compare_string(str1,str2,size):
	error = 0
	for x in xrange (size):
		error += (int(str1[x]) + int(str2[x])) % 2	
	return error

data = ser.readline()

print data

while(stop): 
    
    data = ser.readline() ## Leitura do Serial do arduino

    while(data == ''):
    	data = ser.readline()

    if data.split()[0] != 'Fim':
        print ('String = ' + data.split()[5])
        ## List = [x,data.split()[-1]]
        ## x += 1
        ## c.writerow(List) ## Grava no Csv
        LastList = []
        ListAux = data.split()[5]
        for z in xrange(len(ListAux)):
            LastList.append(ListAux[z])
    else:
        stop = 0
        print len(LastList)
        for y in xrange(len(LastList)):
            if LastList[y] == None:
                LastList = 'X' 
            if LastList[y] == 'X':
                error = error + 1
            List = [y+1,LastList[y],round(float(error*100)/float(y+1),2)]
            c.writerow(List) ## Grava no Csv
            List = []

print(csvName + " criado com sucesso !")

