#!/usr/bin/python3
import sys
import os
import random
import signal
import socket
import struct
import threading
import time
from datetime import datetime
from datetime import timedelta

# Diccionarios para facilitar la busqueda de paquetes
packtypes = {
    "REG_REQ": int("0xa0", base=16), "REG_ACK": int("0xa1", base=16), "REG_NACK": int("0xa2", base=16),
    "REG_REJ": int("0xa3", base=16), "REG_INFO": int("0xa4", base=16), "INFO_ACK": int("0xa5", base=16),
    "INFO_NACK": int("0xa6", base=16), "INFO_REJ": int("0xa7", base=16), "ALIVE": int("0xb0", base=16),
    "ALIVE_NACK": int("0xb1", base=16),  "ALIVE_REJ": int("0xb2", base=16),
    "SEND_DATA": int("0xc0", base=16), "DATA_ACK": int("0xc1", base=16), "DATA_NACK": int("0xc2", base=16),
    "DATA_REJ": int("0xc3", base=16), "SET_DATA": int("0xc4", base=16), "GET_DATA": int("0xc5", base=16)
}

mutex = threading.Lock()
mutex2 = threading.Lock()
debug_mode = False
# Diccionarios para facilitar la busqueda de paquetes
client = {} 
server = {}
lista_clients = []  # Lista de todos los clientes para clase client
sockets = None
filename = "server.cfg"
datab_name = "bbdd_dev.dat"

class Sockets:
    def __init__(self):
        self.udp_socket = None
        self.new_udp_port = random.randint(0, 65535)
        self.udp_port = None

        self.tcp_socket = None
        self.tcp_port = None

class Client:
    def __init__(self):
        self.name = None
        self.state = "DISCONNECTED"
        self.random_num = random.randint(0, 9999999999)
        self.udp_port = None
        self.new_udp_port = random.randint(0, 65535)
        self.ip_address = None
        self.elements = None
        self.alive_recibido = False
        self.cont_alives = 0
        self.alive_registred = False
        self.conf_tcp_socket = None

W = 3
X = 3
R = 2
V = 4
Z = 2

def stat():
    global status
    mutex.acquire()
    print("**********DADES SERVER**********")
    print("   Id: " + server["Id"])
    print("   UDP-port: " + server["UDP-port"] + " DEFAULT")
    print("   TCP-port: " + server["TCP-port"] + " DEFAULT")

    print("\n ------ID------ -----IDENTIFICADOR----- --------IP-------- ---------ESTAT--------- ---------ELEMENTS------------------")
    
    for key in lista_clients:
        if key.state != "DISCONNECTED":
            print("  " + key.name +"   \t"+"\t"+ str(key.random_num) +"\t    "+ str(key.ip_address) +"\t\t  "+ key.state +"\t"+ str(key.elements))
        else:
            print("  " + key.name +"\t    ---------------" +"  "+"\t   ------------\t" +"  \t "+ key.state +"  "+"\t\t --------------" )
    print("")
    print("************************************")
    sys.stdout.flush()
    mutex.release()
    
def input_keyboard():
    global debug_mode
    while True:
        command = input("Introduce una comanda: \n")
        try:
            if command.split()[0] == "quit":
                quit(0, 0)
            elif command.split()[0] == "list":
                stat()
            elif command.split()[0] == "set":
                cset(command.split()[1], command.split()[2])
            elif command.split()[0] == "help":
            	help()
            elif command.split()[0] == "debug":
                if debug_mode:
                    debug_mode = False
                    print("Modo debug desactivado")
                else:
                    print("Modo debug activado")
                    debug_mode = True
            else:
                print("Comanda errònea, escriu: help => per ajuda")
        except IndexError:
            print("Comanda errònea, escriu: help => per ajuda")

def help():
    print(date()+"MSG.  => \n  quit -> Salir del servidor \n" + "list -> lista de los clientes"+" debug -> para mostrar informacion mas extensa del servidor \n ")
def quit(signum, handler):
    print("BYE bye Exiting")
    
    raise SystemExit
    
def parse_argv(argv):
    global filename, datab_name, sockets, lista_clients
    sockets = Sockets()
    if len(sys.argv) > 1:
        for argcounter, arg in enumerate(sys.argv):

            try:
                if arg == "-d":
                    debug_mode = True
                    print("Mode debug activado")
                elif arg.endswith(".cfg") and sys.argv[argcounter - 1] == "-c":
                    try:
                        f = open(arg, "r")
                        f.close()
                        filename = arg

                    except FileNotFoundError:
                        print("No se a podido encontrar el archivo de configuracion")
                        exit(-1)
                elif arg == "-u":
                    if sys.argv[argcounter + 1].endswith(".dat"):
                        try:
                            f = open(sys.argv[argcounter + 1], "r")
                            f.close()
                            datab_name = sys.argv[argcounter + 1]

                        except FileNotFoundError:
                            print("No se a podido encontrar el archivo de configuracion")
                            exit(-1)
            except IndexError:
                print("Uso: python servidor.py {-c <nombre_fitxero>} {-d} {-u <nombre_fitxero>}\n")
                exit(-1)

    with open(filename, "r") as f:
        server["Id"] = {}
        for line in f.readlines():
            if line.split("=")[0][:-1] != "":
                server[line.split("=")[0][:-1]] = line.split("=")[1][1:-1]
    f.close()
    sockets.udp_port = int(server["UDP-port"])
    sockets.tcp_port = int(server["TCP-port"])

    with open(datab_name, "r") as f:
        client["Id"] = {}
        for line in f.readlines():
            for id in line.split("\n"):
                if id != "":
                    clients = Client()
                    clients.name = id
                    lista_clients.append(clients)
                    client["Id"][id] = "DISCONNECTED"
                    
    f.close()

def date():
    return datetime.now().strftime("%d/%m/%Y %H:%M:%S => ")

def setup_udp_socket():
    global sockets
    sockets.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sockets.udp_socket.bind(("", sockets.udp_port))

def setup_udp2_socket():
    global sockets
    sockets.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sockets.udp_socket.bind(("", sockets.new_udp_port))
    
def service():
    if debug_mode:
        print(date()+"DEBUG -> UDP socket enabled")

    serve_udp_connection()


def recibe_pack_udp(bytes_to_receive):
    received_package_packed, (client_ip_address, client_udp_port) = sockets.udp_socket.recvfrom(bytes_to_receive)             
    tupla = struct.unpack('B11s11s61s', received_package_packed)
    if debug_mode:
        client_name = str(tupla[1].decode('ascii').rstrip('\x00'))
        random_num = int(tupla[2].decode('ascii').rstrip('\x00'))
        data = str(tupla[3].decode('ascii', errors='ignore').rstrip('\x00'))
        decomposed = {"Tipus": tupla[0], "Id": client_name, "Random": str(random_num), "Dades": str(data)}  
        print(str(decomposed))
        
    return tupla, client_ip_address, client_udp_port

def serve_udp_connection():
    while True:
        tupla, client_ip_address, client_udp_port = recibe_pack_udp(84)
        package_type = tupla[0]
        client_name = str(tupla[1].decode('ascii').rstrip('\x00'))
        clients = get_client(client_name)
        if clients==None:
               if debug_mode:
                    print(date()+"DEBUG -> Declined REG_REQUEST. REG_REQ's rand is not 0000000000")
               reg_rej = create_reg_rej("Nombre incorrecto no podemos gestionar los paquetes")
               send_pack_udp(reg_rej, client_udp_port, client_ip_address)
               print(date()+"MSG. -> REG_REQ nombre erroneo")
        elif package_type == packtypes["REG_REQ"] and clients.state!="SEND_ALIVE" or package_type == packtypes["REG_INFO"]:
            reg_control(tupla, client_ip_address, client_udp_port)
        elif package_type == packtypes["ALIVE"] and clients.state!="DISCONNECTED":
            alive_control(tupla, client_ip_address, client_udp_port)
        elif package_type != packtypes["ALIVE"] and clients.state=="SEND_ALIVE":
            cambio_estado(clients.name, "DISCONNECTED")
            reg_control(tupla, client_ip_address, client_udp_port)

def reg_info_split(clients, data):

    token = data.split(",")
    clients.tcp_socket = token[0]

    for client in lista_clients:
        if client.name == clients.name:
            client.ip_address= clients.ip_address
            client.random_num = clients.random_num
            client.elements = token[1]
            return

def reg_control(tupla, client_ip_address, client_udp_port):
    
    client_name = str(tupla[1].decode('ascii').rstrip('\x00'))
    random_num = int(tupla[2].decode('ascii').rstrip('\x00'))
    clients = get_client(client_name)
    

    if clients.state == "DISCONNECTED":                             
        # save client's ip address and udp port
        clients.ip_address = client_ip_address
        clients.udp_port = client_udp_port
        if tupla[0] == packtypes["REG_REQ"]:
            if random_num != 0:
                if debug_mode:
                    print(date()+"DEBUG -> Error REG_REQ identificacion no es 0")
                reg_rej = create_reg_rej("Paquete recivido incorrecto")
                send_pack_udp(reg_rej, client_udp_port, client_ip_address)
                print(date()+"MSG. -> REG_REQ incorrecto nombre erroneo o identificacion")
                return
            else:    
                setup_udp2_socket()
                register_ack = create_reg_ack(get_client_random_num(client_name))
                sockets.udp_socket.sendto(register_ack, (client_ip_address, client_udp_port))
                server["status"] = "WAIT_INFO"

            
        if tupla[0] == packtypes["REG_INFO"] and server["status"] == "WAIT_INFO":           
            reg_ack = create_info_ack(get_client_random_num(client_name))
            sockets.udp_socket.sendto(reg_ack, (client_ip_address, client_udp_port))
            setup_udp_socket()
            mutex2.acquire()
            cambio_estado(client_name, "REGISTERED")
            mutex2.release()
            data = str(tupla[3].decode('utf-8', errors='ignore').rstrip('\x00'))
            reg_info_split(clients, data)
            timeout = datetime.now() + timedelta(seconds=W)
            alive_thread = threading.Thread(target=alive_handling, args=(client_name, timeout))
            alive_thread.start()
    else:
        alive_ack = create_alive_ack(clients.random_num, clients.name)
        send_pack_udp(alive_ack, client_udp_port, client_ip_address)
    return None

def send_pack_udp(package_to_send, to_udp_port, to_ip_address):
    sockets.udp_socket.sendto(package_to_send, (to_ip_address, to_udp_port))
    tupla = struct.unpack('B11s11s61s', package_to_send)
    decomposed = {"SENT "+"Tipus": tupla[0], "Id": tupla[1].decode(), "Random": tupla[2].decode(),
                  "Dades": tupla[3].decode(errors="ignore")}
    #print(str(decomposed))
    if debug_mode:
        print("DEBUG -> Sent " + str(decomposed))

def get_client(client_name):

    for key in lista_clients:
        if key.name == client_name:
            return key
    return None

def get_client_random_num(client_name):
    for client in lista_clients:
        if client.name == client_name:
            return client.random_num
    return None

def cambio_estado(client_name, new_state):
    for client in lista_clients:
        if client.name == client_name:
            if client.state != new_state:
                client.state = new_state
                if new_state == "REGISTERED" and debug_mode:
                    print(date()+"MSG. => Client: " + client.name +
                                  " successfully signed up on server; " +
                                  " ip: " + client.ip_address +" rand_num: " +
                                  str(client.random_num))
                print(date()+"MSG.  => -> Client " + client_name + " changed its state to: "
                              + new_state)


def are_random_num_and_ip_address_valid(client_name, to_check_random_num, to_check_ip_address):
    for client in lista_clients:
        if client.name == client_name:
            return client.ip_address == to_check_ip_address and client.random_num == to_check_random_num
    return False


def alive_handling(client_name, timeout):
    clients = get_client(client_name)
        
    if clients.state == "REGISTERED":
        received_first_packet = False
        while datetime.now() < timeout:
            if clients.alive_registred == True and clients.receive_alive:
                received_first_packet = True
                cambio_estado(client_name, "SEND_ALIVE")
                break
        if not received_first_packet:
            print("Primer Paquet ALIVE ha arribat a temps")
            change_status(client_name, "DISCONNECTED")
            return

    while clients.state == "SEND_ALIVE":
        tiempo_alive = datetime.now() + timedelta(seconds=W)
        clients.receive_alive = False
        received_alive = False
        while datetime.now() < tiempo_alive:
            if clients.receive_alive:
                clients.cont_alives = 0
                received_alive = True
        if not received_alive:
            clients.cont_alives += 1
            print(date(),"No se ha recibido el paquete num:",clients.cont_alives,"/3 ALIVE seuidos del client: ", client_name)

        if clients.cont_alives == 3:
            print(date(),"No se han recibido 3 ALIVE seuidos del client: ", client_name)
            clients.cont_alives = 0
            cambio_estado(client_name, "DISCONNECTED")
            return

def alive_control(tupla, client_ip_address, client_udp_port):

    client_name = str(tupla[1].decode('ascii').rstrip('\x00'))
    random_num = int(tupla[2].decode('ascii').rstrip('\x00'))
    clients = get_client(client_name)

    if not are_random_num_and_ip_address_valid(client_name, random_num, client_ip_address):
        if debug_mode:
            print(date()+"DEBUG -> Error in received ALIVE_INF. Client:" + client_name + " ip:" +
                client_ip_address + ", rand_num:" + str(random_num) + 
                " (Registered as: " + clients.name +", ip:" + 
                clients.ip_address + ", rand_num:" + str(clients.random_num) + ")")

        alive_rej = create_alive_rej("wrong data received")
        send_pack_udp(alive_rej, client_udp_port, client_ip_address)
        mutex2.acquire()
        cambio_estado(clients.name, "DISCONNECTED")
        mutex2.release()
        return
    else:  # everything correct
        clients.alive_recibido = True
        clients.alive_registred = True
        clients.receive_alive = True
        cambio_estado(clients.name, "SEND_ALIVE")
        alive_ack = create_alive_ack(clients.random_num, clients.name)
        send_pack_udp(alive_ack, client_udp_port, client_ip_address)

#fase reg
def create_reg_rej(reason):
    reg_rej = struct.pack('B11s11s61s', packtypes["REG_REJ"], "".encode(), "0000000000".encode(), reason.encode())
    return reg_rej
    
def create_reg_nack(reason):
    register_nack = struct.pack('B11s11s61s', packtypes["REG_NACK"], "".encode(), "0000000000".encode(), reason.encode())
    return register_nack

def create_reg_ack(client_random_num):
    register_ack = struct.pack('B11s11s61s', packtypes["REG_ACK"], server["Id"].encode(), str(client_random_num).encode(), str(sockets.new_udp_port).encode())
    return register_ack
    
def create_info_ack(client_random_num):
    register_ack = struct.pack('B11s11s61s', packtypes["INFO_ACK"], server["Id"].encode(), str(client_random_num).encode(), 
    server["TCP-port"].encode())
    return register_ack
#fase alive
def create_alive_rej(reason):
    alive_rej = struct.pack('B11s11s61s', packtypes["ALIVE_REJ"], "".encode(), "0000000000".encode(), reason.encode())
    return alive_rej


def create_alive_nack(reason):
    alive_nack = struct.pack('B11s11s61s', packtypes["ALIVE_NACK"], "".encode(), "0000000000".encode(), reason.encode())
    return alive_nack


def create_alive_ack(client_random_num, name):
    alive = struct.pack('B11s11s61s', packtypes["ALIVE"],
                            server["Id"].encode(), str(client_random_num).encode(), name.encode())
    return alive


# input: python servidor.py {-d} {-c <software_config_file>} {-u <allowed_devices_file>}
if __name__ == '__main__':
    try:
        thread_input = threading.Thread(target=input_keyboard)
        thread_input.daemon = True
        thread_input.start()
        parse_argv(sys.argv)
        setup_udp_socket()
        alive_loop = threading.Thread(target=service)
        alive_loop.start()
    except(KeyboardInterrupt, SystemExit):
        mutex.acquire()
        for clients in lista_clients:
            if clients.conf_tcp_socket is not None:
                clients.conf_tcp_socket.close()
        mutex.release()
        sockets.udp_socket.close()
        exit(1)
