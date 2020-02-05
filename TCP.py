import socket
import numpy as np

print("HI")
address = ('192.168.11.242', 1999)
s= socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect(address)

def send():
    msg = input("input:")
    s.send(bytes(msg, "utf-8"))
   
while(True):
    send()

    
