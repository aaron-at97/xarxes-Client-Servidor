//
// Created by Aaron on 12/03/2022.
//

#include "common.h"
#include "funtion.h"

int registred_Error = 0;
/* ./cl {-d} {-c <cfg_file>}*/
int main(int argc, const char *argv[]) {

    strcpy(server_data.rand_num, "0000000000");
    signal(SIGINT, cntrlc);
    parse_argv(argc, argv);

    setup_UDP_socket();
    stat(); 
    service_loop();
    return 0;
}

/* Salidas de consola contrlc defecto + quit comando recurrente */

void cntrlc(int signal) {
    if (signal == SIGINT) {
        write(2, "\n Sortint per CNTRL+C \n", 20);

        close(sockets.tcp_socket);
        close(sockets.udp_socket);

        free(client_state);
        free(server_data.address);

        exit(0);
    }
}

void quit() {
        close(sockets.tcp_socket);
        close(sockets.udp_socket);

        free(client_state);
        free(server_data.address);

        exit(0);
}


void parse_argv(int argc, const char *argv[]) {
    FILE *cfg_file = NULL;
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0 && argc > (i + 1)) {
            if (access(argv[i + 1], F_OK) != -1) {
                cfg_file = fopen(argv[i + 1], "r");
            } else {
                char guardar_message[200];
                sprintf(guardar_message, "Error ! => No se puede abrir el archivo llamado: '%s'. se abrirá client.cfg (archivo de configuración predeterminado)\n",
                        argv[i + 1]);
                print_data(guardar_message);
            }
        } else if (strcmp(argv[i], "-d") == 0) {
            debug_mode = true;
            print_data("MSG. Debug Activado (-d)\n");
        } else if (strcmp(argv[i], "-f") == 0 && argc > (i + 1)) {
            network_dev_config_file_name = malloc(sizeof(argv[i + 1]));
            strcpy(network_dev_config_file_name, argv[i + 1]);
        }
    }
    if (debug_mode) { print_data("DEBUG=> Leemos la comanda\n"); }

    if (cfg_file == NULL) {
        if (access("client.cfg", F_OK) != -1) {
            cfg_file = fopen("client.cfg", "r");
        } else {
            print_data("Error ! => No disponemos de los archivos de configuracion por defecto tienes que importalos al directorio");
            exit(1);
        }
    }
    read_file_cfg(cfg_file);
    if (debug_mode) { print_data("DEBUG=> Leemos archivos de configuracion cliente\n"); }
}
/* Leectura fichero*/
void read_file_cfg(FILE *cfg_file) {
    char line[100];
    char delim[] = " = \n";
    char delimitador[] = ";";
    char *token;
    
    /* read line by line */
    while (fgets(line, 100, cfg_file)) {
        token = strtok(line, delim);

        if (strcmp(token, "Id") == 0) {
            token = strtok(NULL, delim);
            strcpy(client_data.id, token);
        } else if (strcmp(token, "Elements") == 0) {
            token = strtok(NULL, delim);
            strcpy(client_data.elements, token);
        } else if (strcmp(token, "Local-TCP") == 0) {
            sockets.tcp_port = atoi(strtok(NULL, delim));
            client_data.tcp_port = sockets.tcp_port;
        } else if (strcmp(token, "Server") == 0) {
            token = strtok(NULL, delim);
            server_data.address = malloc(strlen(token) + 1);
            strcpy(server_data.address, token);
        } else if (strcmp(token, "Server-UDP") == 0) {
            sockets.udp_port = atoi(strtok(NULL, delim));
        }
    }
    
    char buffer[80];
    sprintf(buffer, "DEFAULT;%s", client_data.elements);
    char *token2 = strtok(buffer, delimitador);
    if(token2 != NULL){
	int i = 0;
	while(token2 != NULL){
	     // Sólo en la primera pasamos la cadena; en las siguientes pasamos NULL
	     token2 = strtok(NULL, delimitador);
	     if (token2 != NULL) {
	     	strcpy(client_data.element[i], token2);
	     	strcpy(client_data.value[i], "NONE");
	     	i++;
	     }
	}
    }	

}


void setup_UDP_socket() {
    struct hostent *ent;
    struct sockaddr_in addr_cli;

     /* CrEa UN SOCKeT INET¡Dgram '. udp */
    sockets.udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockets.udp_socket < 0) {
        print_data("Error ! => No creado UDP socket\n");
        exit(1);
    }


    memset(&addr_cli, 0, sizeof(struct sockaddr_in));
    addr_cli.sin_family = AF_INET;
    addr_cli.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_cli.sin_port = htons(0);

    /* Fem el binding */
    if (bind(sockets.udp_socket, (struct sockaddr *) &addr_cli, sizeof(struct sockaddr_in)) < 0) {
        print_data("Error ! => No bind UDP socket\n");
        exit(1);
    }

    /* get server identity */
    ent = gethostbyname(server_data.address);
    if (!ent) {
        print_data("Error ! => No se puede encontrar el servidor al intentar configurar el socket UDP\n");
        exit(1);
    }
    
        /* Ompla l'estructrura d'adreça amb l'adreça del servidor on enviem les dades */
    memset(&sockets.udp_addr_server, 0, sizeof(struct sockaddr_in));
    sockets.udp_addr_server.sin_family = AF_INET;
    sockets.udp_addr_server.sin_addr.s_addr = (((struct in_addr *)ent->h_addr)->s_addr);
    sockets.udp_addr_server.sin_port = htons(sockets.udp_port);
     printf(" server rand numero setup %d \n", sockets.udp_addr_server.sin_port);
}

/* Creacion de los hilos para la comunicacion periodica con el servidor */
void service_loop() {
    cambio_estado("NOT_REGISTERED");
    pthread_create(&tid, NULL, input_keyboard, NULL);
    registered();
    alive_control();
}

void cambio_estado(char *new_state) {
    client_state = malloc(sizeof(new_state));
    strcpy(client_state, new_state);
    char guardar_message[50];
    sprintf(guardar_message, "MSG. Dispositiu passa a estat: %s\n", client_state);
    print_data(guardar_message);
}

/* Funcion para mostar la data a los mensajes importante con la cimunicacion de servidor*/
void print_data(char *to_print) {
    time_t now;
    struct tm *now_tm;
    char formated_time[100];

    now = time(NULL);
    now_tm = localtime(&now);
    strftime(formated_time, 100, "%x %X", now_tm);
    printf("%s - %s", formated_time, to_print);
    fflush(stdout);
}

/* Proceso de registro */
void registered() {
    
    while (registred_Error < O) {
        if (debug_mode) {
            char guardar_message[75];
            sprintf(guardar_message, "DEBUG=> Proceso de registro. Paso: %d / %d\n",
                    registred_Error + 1, O);
            print_data(guardar_message);
        }
        char guardar_message[75];
        sprintf(guardar_message, "DEBUG=> Proceso de registro. Paso: %d / %d\n", registred_Error + 1, O);
        
        for (int reg_reqs_sent = 0; reg_reqs_sent < N; reg_reqs_sent++) {
            struct Package reg_req;
            reg_req = create_reg_req();
            send_pack_udp(reg_req);
            cambio_estado("WAIT_ACK_REG");
            struct Package received_package;
            received_package = recibe_pack_udp(get_waiting_time_after_sent(reg_reqs_sent));
            
            if (received_package.type == get_packet_type_from_string("REG_REJ")) {
                cambio_estado("NOT_REGISTERED");
                registred_Error++;
		service_loop();
            } else if (received_package.type == get_packet_type_from_string("REG_NACK")) {
                registred_Error++;
                cambio_estado("NOT_REGISTERED");
                break;
            } else if (received_package.type == get_packet_type_from_string("REG_ACK")) {
		            
		    save_reg_ack_data(received_package);
		    struct Package reg_info;
		    reg_info = create_reg_info();
		    send_pack_udp(reg_info);
		    cambio_estado("WAIT_ACK_INFO");
            
		    received_package = recibe_pack_udp(get_waiting_time_after_sent(reg_reqs_sent));

		    	if (received_package.type != get_packet_type_from_string("INFO_ACK")){
		    	    cambio_estado("NOT_REGISTERED");
		    	}
		    	else{
		    	    cambio_estado("REGISTERED");
		    	}

 
		   if (debug_mode) {
		       char guardar_message[150];
		       sprintf(guardar_message,"Succesfully signed up on server: %s (name: %s, rand_num: %s, tcp port: %d)\n",
		       server_data.address, server_data.name, server_data.rand_num, sockets.tcp_port);
		       print_data(guardar_message);
		   }
		   return;
            }
            else if (debug_mode) {
                print_data("DEBUG=> No tenemos respuesta REG_REQ \n  Prueba en otro momento\n");
            }
            sleep(sockets.udp_timeout.tv_sec);
            usleep(sockets.udp_timeout.tv_usec);
        }
        sleep(U);
        registred_Error++;
    }
    print_data("ERROR ! Error conexion con el server vuelve a probar \n");
    exit(1);
}
/* Paquetes UDP fase de registro*/
struct Package create_reg_req() {
    struct Package reg_req;
    if (debug_mode) {
       char guardar_message[75];
       sprintf(guardar_message, "DEBUG=> Creado paquete REG_REQ \n");
       print_data(guardar_message);
    }
    /* fill Package */
    reg_req.type = get_packet_type_from_string("REG_REQ");
    strcpy(reg_req.id, client_data.id);
    strcpy(reg_req.com, server_data.rand_num);
    strcpy(reg_req.data, "");
    return reg_req;
}

struct Package create_reg_info() {
    struct Package reg_info;
    if (debug_mode) {
       char guardar_message[75];
       sprintf(guardar_message, "DEBUG=> Creado paquete REG_INFO \n");
       print_data(guardar_message);
    }
    
    char buffer[90];
    char guardar_message[75];
    sprintf(buffer, "%d,%s", client_data.tcp_port ,client_data.elements);
    sprintf(guardar_message, "MSG. Abierto puerto TCP %d para la comunicación con el servidor \n",client_data.tcp_port);
    print_data(guardar_message);
    reg_info.type = get_packet_type_from_string("REG_INFO");
    strcpy(reg_info.id, client_data.id);
    strcpy(reg_info.com, server_data.rand_num);
    strcpy(reg_info.data, buffer);
    return reg_info;
}


/* Envio de paquetes socket servidor*/
void send_pack_udp(struct Package package_to_send) {
    int a = sendto(sockets.udp_socket, &package_to_send, sizeof(package_to_send), 0,
                   (struct sockaddr *) &sockets.udp_addr_server, sizeof(sockets.udp_addr_server));
    char guardar_message[170];
    
    if (a < 0) {
        sprintf(guardar_message, "Error ! => No se pudo enviar el paquete a través del socket UDP \n");
        print_data(guardar_message);
    } else if (debug_mode) {
        sprintf(guardar_message,
                "DEBUG=> Sent %s;\n \tBytes:%lu,\n \tname:%s,\n\trand num:%s,\n\tdata:%s\n",
                get_packet_string_from_type(package_to_send.type), sizeof(package_to_send),
                package_to_send.id, package_to_send.com,
                package_to_send.data);
        print_data(guardar_message);
    }
}

/* Realizamos los tiempos de espera para nuestros request de peticiones de paquetes*/
int get_waiting_time_after_sent(int reg_reqs_sent) {
    if (reg_reqs_sent >= P - 1) {
        int times = 2 + (reg_reqs_sent + 1 - P);
        if (times > Q) {
            times = Q;
        }
        return times * T;
    }
    return T;
}
/* Funcion para recibir los paquetes del servidor comunicacion periodica*/
struct Package recibe_pack_udp(int max_timeout) {
    fd_set rfds;
    char *buf = malloc(sizeof(struct Package));
    struct Package *received_package = malloc(sizeof(struct Package));
    FD_ZERO(&rfds);
    FD_SET(sockets.udp_socket, &rfds);
    sockets.udp_timeout.tv_sec = max_timeout;
    sockets.udp_timeout.tv_usec = 0;
    
    if (select(sockets.udp_socket + 1, &rfds, NULL, NULL, &sockets.udp_timeout) > 0) {
        int a;
        a = recvfrom(sockets.udp_socket, buf, sizeof(struct Package), 0, (struct sockaddr *) 0, (socklen_t *) 0);
        if (a < 0) {
            print_data("Error ! => Could not receive from UDP socket\n");
        } else {
            received_package = (struct Package *) buf;
            if (debug_mode) {
                char guardar_message[220];
                sprintf(guardar_message,
                        "DEBUG=> Received %s;\n \tBytes:%lu,\n \tname:%s,\n \trand num:%s,\n \tdata:%s\n\n",
                        get_packet_string_from_type((unsigned char) (*received_package).type),
                        sizeof(*received_package), (*received_package).id, (*received_package).com,
                        (*received_package).data);
                print_data(guardar_message);
            }
        }
    }
    return *received_package;
}
/* He creado esta funcion para cuando recibamos el nuevo puerto poder guardarlo al nuevo socket*/
void save_reg_ack_data(struct Package received_package) {
    strcpy(server_data.rand_num, received_package.com);
    strcpy(server_data.name, received_package.id);
    sockets.udp_addr_server.sin_port = htons(atoi(received_package.data));
}

void *input_keyboard() {
    while (1) {
        int max_chars_to_read = 50;
        char *command = read_from_stdin(max_chars_to_read);

        if (strcmp(command, "quit") == 0) {
            quit();
            cntrlc(SIGINT);
        } else if (strcmp(command, "stat") == 0) {
            stat();
        }else if (strcmp(command,"set") == 0){
	    printf("En desarrollo  %d",command[0]);
	    //operation_result = set(command[1],command[2],command[3]);
	    
	    /*if(operation_result >= 0){
		 print_debug("Operació exitosa");
	    }else{
		print_debug("Operació fallida");
	    }*/
	}else if (strcmp(command,"send") == 0){
	    printf("En desarrollo  %d",command[0]);
	    //operation_result = set(command[1],command[2],command[3]);
	    
	    /*if(operation_result >= 0){
		 print_debug("Operació exitosa");
	    }else{
		print_debug("Operació fallida");
	    }*/
	}
        else if (strcmp(command, "\0") != 0) { /* in case '\n' entered */
            char guardar_message[150];
            sprintf(guardar_message, "Error ! => %s Comando no aceptado\n", command);
            print_data(guardar_message);
            ayuda();
        }	
    }
}

char *read_from_stdin(int max_chars_to_read) {
    char buffer[max_chars_to_read];
    if (fgets(buffer, max_chars_to_read, stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
    char *buffer_pointer = malloc(max_chars_to_read);
    strcpy(buffer_pointer, buffer);
    return buffer_pointer;
}


void ayuda() {
    print_data("MSG. Comandos aceptados: \n");
    printf("\t\t    quit -> cerrar progrma \n");
    printf("\t\t    stat -> Datos dispositivo\n");
    printf("\t\t    set -> modificar valor\n");
    printf("\t\t    send -> sends conf file to server via TCP\n");
}

void stat() {

    printf("********************* DADES DISPOSITIU ***********************\n");
    printf("\n");
    printf("Identificador: %s\n", client_data.id);
    printf("Estat: %s \n", client_state);
    printf("\n");
    printf("    Param                valor\n");
    printf(" -------------      -------------\n");
    int f;
    for(f=0;f<6;f++)
    {
        printf("   %s \t\t %s \n",client_data.element[f],client_data.value[f]);
    }
    printf("***************************************************************\n");
}
/* Funcion Comunicacion ALIVE*/
void *alive_control() {
    int contador_ack = 0;
    while (1) {
    
        struct Package alive_inf = create_alive_inf();
        send_pack_udp(alive_inf);
        if (debug_mode) {
             char guardar_message[190];
             sprintf(guardar_message, "DEBUG => comunicacio peridica creant alive \n");
                print_data(guardar_message);
            }
        struct Package received_package = recibe_pack_udp(R);
        sleep(sockets.udp_timeout.tv_sec);
        usleep(sockets.udp_timeout.tv_usec);
        if (received_package.type == get_packet_type_from_string("ALIVE") &&
            is_received_udp(received_package)) {
            if (strcmp(client_state, "SEND_ALIVE") != 0) { cambio_estado("SEND_ALIVE"); }
            contador_ack = 0;

        } else if (received_package.type == get_packet_type_from_string("ALIVE") &&
                   !is_received_udp(received_package)) {
            contador_ack++;
            if (debug_mode) {
                char guardar_message[190];
                sprintf(guardar_message,
                        "DEBUG=> Paquete ALIVE_ACK incorrecto recibido. (credenciales correctas: nombre: %s, número aleatorio: %s)\n\n",
                        server_data.name, server_data.rand_num);
                print_data(guardar_message);
            }

        } else if (received_package.type == get_packet_type_from_string("ALIVE_REJ") &&
                   strcmp(client_state, "SEND_ALIVE") == 0) {
            print_data("MSG. Se obtuvo el paquete ALIVE_REJ cuando el estado era ALIVE\n");
            pthread_cancel(tid);
            registred_Error++;
            service_loop();
            break;

        } else {
            contador_ack++;
            if (debug_mode) {
                char guardar_message[150];
                sprintf(guardar_message, "DEBUG=> No se ha recibido el paquete ACK num: %d / %d\n\n",
                        contador_ack, S);
                print_data(guardar_message);
            }
        }

        if (contador_ack == S) {
            print_data("Error ! => Maximo de peticiones de paquetes ack\n");
            pthread_cancel(tid);
            registred_Error++;
            service_loop();
            break;
        }
    }
    return NULL;
}
/* Paquetes ALIVE*/
struct Package create_alive_inf() {
    struct Package alive_inf;
    if (debug_mode) {
       char guardar_message[75];
       sprintf(guardar_message, "DEBUG=> Creado paquete ALIVE \n");
       print_data(guardar_message);
    }
    alive_inf.type = get_packet_type_from_string("ALIVE");
    strcpy(alive_inf.id, client_data.id);
    strcpy(alive_inf.com, server_data.rand_num);
    strcpy(alive_inf.data, "");
    sockets.udp_addr_server.sin_port = htons(sockets.udp_port);
    return alive_inf;
}

/* Comprobacion del nuevo puerto recivido si es correcto*/
bool is_received_udp(struct Package received_package) {
    return (strcmp(server_data.name, received_package.id) == 0 &&
            strcmp(server_data.rand_num, received_package.com) == 0);
}
