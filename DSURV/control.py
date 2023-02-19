# install pyserial first
# pip install pyserial

import serial
import time

import serial.tools.list_ports

ports = serial.tools.list_ports.comports()

portStatus = False
portCon = ""

#check if arduino is connected
for port, desc, hwid in sorted(ports):
    if desc == "USB-SERIAL CH340 "+"("+port+")":
        portStatus = True
        portCon = port

if portStatus == True:
    arduino = serial.Serial(portCon,9600,timeout=1)

    def write_read(x):
        arduino.write(str.encode(x))
        time.sleep(0.10)
        data = arduino.readline()
        data = data.decode() #removed b''
        return data

    print("command: c1 = angle")
    print("         c2 = motor")
    print("         r1 = read claw")
    print("Format = <command:values_if_command_is_used>")
    print("Example = <c1:x,y,rotation>")
    print("          <c2:direction(f/b),millisecond>")
    print("")

    while arduino.isOpen():
        num = input("INPUT: ") # Taking input from user
        #num = ["<c1:90,90,90>","<c1:90,90,180>","<c1:120,0,180>","<c1:90,90,180>","<c1:90,90,90>","<c1:60,45,90>","<c1:90,90,90>","<c2:f:500>","<c1:60,45,90>","<c1:90,90,90>","<c1:90,90,0>","<c1:120,0,0>","<c1:90,90,0>","<c1:90,90,90>","<c2:b:500>"]
        # for r in range(3):
        #     for x in range(15):
        #         if num[x] != "":
        #             print(num[x])
        #             value = write_read(num[x])#value = write_read(num)
        #             print(value) # printing the value
        #             time.sleep(1.5)

        #         else:
        #             arduino.close()
        if num != "":
            print(num)
            value = write_read(num)#value = write_read(num)
            print(value) # printing the value
            time.sleep(1.5)

        else:
            arduino.close()
            

else:
    print("")
    print("<============ Arduino not detected! ============>")
    print("")
    print("list of ports:")
    for port, desc, hwid in sorted(ports):
        print("> {}: {}".format(port, desc))