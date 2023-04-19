# install pyserial first
# pip install pyserial
import time
import serial
import time
from tkinter import *

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
    tempx = ""
    tempy = ""
    tempz = ""

    def write_read(x):
        arduino.write(str.encode(x))
        time.sleep(0.10)
        data = arduino.readline()
        data = data.decode() #removed b''
        print(data)
        return data
#===================== testing window =====================
    def show_values():
            write_read(comm.get())

    def slider():
        # if (tempx != str(x.get())) or (tempy != str(y.get())) or (tempz != str(z.get())):
        #     tempx = str(x.get())
        #     tempy = str(y.get())
        #     tempz = str(z.get())
        joints = ("<c1:"+str(x.get())+","+str(y.get())+", "+str(z.get())+">")
        write_read(joints)

    def forward():
        joints = ("<c2:f,500>")
        write_read(joints)
    def backward():
        joints = ("<c2:b,500>")
        write_read(joints)
    def sensor():
        joints = ("<c3>")
        write_read(joints)

    def auto():
        write_read("<c1:10,10,90>")
        time.sleep(1)
        write_read("<c1:10,0,90>")
        time.sleep(1)
        write_read("<c1:21,0,90>")
        time.sleep(1)
        write_read("<c1:22,10,90>")
        time.sleep(.5)
        while(TRUE):
            distance = write_read("<c3>")
            time.sleep(.5)
            if int(distance) > 7:
                write_read("<c1:21,0,90>")
                time.sleep(1)
                write_read("<c1:22,10,90>")
                time.sleep(.5)
            else:
                write_read("<c4>")
                break
        write_read("<c1:10,10,90>")
        time.sleep(1)


    master = Tk()
    x = Scale(master, from_=-10, length=200, tickinterval=10, to=30, orient=HORIZONTAL)
    y = Scale(master, from_=-10, length=200, tickinterval=10, to=30, orient=HORIZONTAL)
    z = Scale(master, from_=0, length=300, tickinterval=10, to=180, orient=HORIZONTAL)
    comm = Entry(master)

    x.set(10)
    y.set(10)
    z.set(90)

    

    # b = Scale(master, from_=0, length=600, tickinterval=10, to=180, orient=HORIZONTAL)
    # a1 = Scale(master, from_=0, length=600, tickinterval=10, to=180, orient=HORIZONTAL)
    # a2 = Scale(master, from_=0, length=600, tickinterval=10, to=180, orient=HORIZONTAL)
    # a3 = Scale(master, from_=0, length=600, tickinterval=10, to=180, orient=HORIZONTAL)

    # b.set(90)
    # a1.set(90)
    # a2.set(90)
    # a3.set(90)

    # b.pack()
    # a1.pack()
    # a2.pack()
    # a3.pack()
    Button(master, text='>> Forward >>', command=forward).pack()
    Button(master, text='<< Backward <<', command=backward).pack()
    x.pack()
    y.pack()
    z.pack()
    Button(master, text='- Slider -', command=slider).pack()
    Button(master, text='- Sensor Distance -', command=sensor).pack()
    comm.pack()
    Button(master, text='Command', command=show_values).pack()
    Button(master, text='Auto', command=auto).pack()

#==========================================================

    print("command: c1 = angle")
    print("         c2 = motor")
    print("         r1 = read claw")
    print("Format = <command:values_if_command_is_used>")
    print(" claw position = <c1:x,y,rotation>")
    print("robot position = <c2:direction(f/b),millisecond>")
    print(" claw distance = <c3>")
    print("")

 
    

    
    mainloop()

    # while arduino.isOpen():
    #     num = input("INPUT: ") # Taking input from user
    #     #num = ["<c1:90,90,90>","<c1:90,90,180>","<c1:120,0,180>","<c1:90,90,180>","<c1:90,90,90>","<c1:60,45,90>","<c1:90,90,90>","<c2:f:500>","<c1:60,45,90>","<c1:90,90,90>","<c1:90,90,0>","<c1:120,0,0>","<c1:90,90,0>","<c1:90,90,90>","<c2:b:500>"]
    #     # for r in range(3):
    #     #     for x in range(15):
    #     #         if num[x] != "":
    #     #             print(num[x])
    #     #             value = write_read(num[x])#value = write_read(num)
    #     #             print(value) # printing the value
    #     #             time.sleep(1.5)

    #     #         else:
    #     #             arduino.close()
    #     if num != "":
    #         print(num)
    #         value = write_read(num)#value = write_read(num)
    #         print(value) # printing the value
    #         time.sleep(1.5)

    #     else:
    #         arduino.close()
            

else:
    print("")
    print("<============ Arduino not detected! ============>")
    print("")
    print("list of ports:")
    for port, desc, hwid in sorted(ports):
        print("> {}: {}".format(port, desc))