import sys
import os
import argparse
import errno

parser = argparse.ArgumentParser(
    description='Throughput Calculation for a application.')
parser.add_argument('inFile', type=str)
parser.add_argument('outFileBytes', type=str)
parser.add_argument('outFilePackets', type=str)
parser.add_argument('--startTime', type=float, default=0)
parser.add_argument('--packetSize', type=float, default=1500)
parser.add_argument('--destPort', type=int)
parser.add_argument('--destIp', type=str)
parser.add_argument('flowLabel', type=str)
args = parser.parse_args()

file_name = args.inFile
outFileBytes = args.outFileBytes
outFilePackets = args.outFilePackets
startTime = args.startTime
packetSize = args.packetSize
destIp = args.destIp
flowLabel = args.flowLabel
destPort = args.destPort

if not os.path.exists(os.path.dirname(outFileBytes)):
    try:
        os.makedirs(os.path.dirname(outFileBytes))
    except OSError as exc:  # Guard against race condition
        if exc.errno != errno.EEXIST:
            raise

if not os.path.exists(os.path.dirname(outFilePackets)):
    try:
        os.makedirs(os.path.dirname(outFilePackets))
    except OSError as exc:  # Guard against race condition
        if exc.errno != errno.EEXIST:
            raise

os.system("tshark -r " + file_name + " -T fields -e frame.time_relative -e frame.len -e ip.dst -e tcp.dstport > " +
          file_name[0:file_name.rindex('.')] + ".csv")
fBytes = open(outFileBytes, 'w')
fPackets = open(outFilePackets, 'w')
old_time = startTime
new_time = 0

pUDP = startTime
valUDP = 0
valPacketUDP = 0
totalvalUDP = 0
totalvalPacketUDP = 0

pTCP = startTime
valTCP = 0
valPacketTCP = 0
totalvalTCP = 0
totalvalPacketTCP = 0

p = startTime
new_time = 0
val = 0
valPacket = 0
totalval = 0
totalvalPacket = 0

with open(file_name[0:file_name.rindex('.')] + '.csv') as fq:
    fp = fq.readlines()
    for line in fp:
        new_time = float(line.split('\t')[0])
        val += float(line.split('\t')[1])
        totalval += float(line.split('\t')[1])
        valPacket += 1
        totalvalPacket += 1
        if (new_time - old_time > 0.1):
            val = val/(new_time - old_time)
            valPacket = valPacket/(new_time - old_time)
            fBytes.write(str(p) + " " + str(val * 8.0/(1000.0 * 1000.0)) + "\n")
            fPackets.write(str(p) + " " + str(valPacket) + "\n")
            p += (new_time - old_time)
            old_time = new_time
            val = 0
            valPacket = 0

        if (str(line.split('\t')[2]).strip() == destIp) and float(line.split('\t')[0]) >= startTime and (destPort == None or line.split('\t')[3].strip () == '' or destPort == int (line.split('\t')[3].strip ())):
            valUDP += float(line.split('\t')[1])
            totalvalUDP += float(line.split('\t')[1])
            valPacketUDP += 1
            totalvalPacketUDP += 1
            if (new_time - old_time > 0.1):
                valUDP = valUDP/(new_time - old_time)
                valPacketUDP = valPacketUDP/(new_time - old_time)
                fBytes.write(str(pUDP) + " " + str(valUDP * 8.0/(1000.0 * 1000.0)) + "\n")
                fPackets.write(str(pUDP) + " " + str(valPacketUDP) + "\n")
                pUDP += (new_time - old_time)
                old_time = new_time
                valUDP = 0
                valPacketUDP = 0
        else:
            valTCP += float(line.split('\t')[1])
            totalvalTCP += float(line.split('\t')[1])
            valPacketTCP += 1
            totalvalPacketTCP += 1
            if (new_time - old_time > 0.1):
                valTCP = valTCP/(new_time - old_time)
                valPacketTCP = valPacketTCP/(new_time - old_time)
                fBytes.write(str(pTCP) + " " + str(valTCP * 8.0/(1000.0 * 1000.0)) + "\n")
                fPackets.write(str(pTCP) + " " + str(valPacketTCP) + "\n")
                pTCP += (new_time - old_time)
                old_time = new_time
                valTCP = 0
                valPacketTCP = 0
    print("UDP:")
    print(flowLabel + ' In Mbps: ' + str(totalvalUDP * 8.0/(1000.0 * 1000.0)/(new_time - startTime)) + ' In Packets: ' + str(totalvalPacketUDP/(new_time - startTime)))
    print("\n\nTCP:")
    print(flowLabel + ' In Mbps: ' + str(totalvalTCP * 8.0/(1000.0 * 1000.0)/(new_time - startTime)) + ' In Packets: ' + str(totalvalPacketTCP/(new_time - startTime)))
    print("\n\nOverall:")
    print(flowLabel + ' In Mbps: ' + str(totalval * 8.0/(1000.0 * 1000.0)/(new_time - startTime)) + ' In Packets: ' + str(totalvalPacket/(new_time - startTime)))

fBytes.close()
fPackets.close()
os.remove(file_name[0:file_name.rindex('.')] + '.csv')
