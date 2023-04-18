//
// Created by Aarón Arenas Tomás on 12/03/2022.
//


/* auxiliar functions declaration */
bool is_received_udp(struct Package received_package);
char *get_packet_string_from_type(unsigned char type);
char *read_from_stdin(int max_chars_to_read);
int get_waiting_time_after_sent(int reg_reqs_sent);
struct Package recibe_pack_udp();
unsigned char get_packet_type_from_string();
void cambio_estado(char *new_state);
void cntrlc(int signal);
void *alive_control();
struct Package create_alive_inf();
void *input_keyboard();
void read_file_cfg(FILE *cfg_file);
void parse_argv(int argc, const char *argv[]);
void ayuda();
void print_data(char *to_print);
void save_reg_ack_data(struct Package package_received);
void send_pack_udp(struct Package package_to_send);
void service_loop();
void setup_UDP_socket();
void registered();
void stat();
struct Package create_reg_req();
struct Package create_reg_info();

char *get_packet_string_from_type(unsigned char type) {
    char *packet_string;

    /* prceso de registro */
    if (type == (unsigned char) 0xa0) {
        packet_string = "REG_REQ";
    } else if (type == (unsigned char) 0xa1) {
        packet_string = "REG_ACK";
    } else if (type == (unsigned char) 0xa2) {
        packet_string = "REG_NACK";
    } else if (type == (unsigned char) 0xa3) {
        packet_string = "REG_REJ";
    } else if (type == (unsigned char) 0xa4) {
        packet_string = "REG_INFO";
    } else if (type == (unsigned char) 0xa5) {
        packet_string = "INFO_ACK";
    } else if (type == (unsigned char) 0xa6) {
        packet_string = "INFO_NACK";
    } else if (type == (unsigned char) 0xa7) {
        packet_string = "INFO_REJ";
    } /* keep in touch packet types */
    else if (type == (unsigned char) 0xb0) {
        packet_string = "ALIVE";
    } else if (type == (unsigned char) 0xb1) {
        packet_string = "ALIVE_NACK";
    } else if (type == (unsigned char) 0xb3) {
        packet_string = "ALIVE_REJ";
    } /* send and set configuration packet types */
    else if (type == (unsigned char) 0xc0) {
        packet_string = "SEND_DATA";
    } else if (type == (unsigned char) 0xc1) {
        packet_string = "DATA_ACK";
    } else if (type == (unsigned char) 0xc2) {
        packet_string = "DATA_NACK";
    } else if (type == (unsigned char) 0xc3) {
        packet_string = "DATA_REJ";
    } else if (type == (unsigned char) 0xc4) {
        packet_string = "SET_DATA";
    } else if (type == (unsigned char) 0xc5) {
        packet_string = "GET_DATA";
    }

    return packet_string;
}

unsigned char get_packet_type_from_string(char *string) {
    unsigned char packet_type;

    /* prceso de registro */
    if (strcmp(string, "REG_REQ") == 0) {
        packet_type = (unsigned char) 0xa0;
    } else if (strcmp(string, "REG_ACK") == 0) {
        packet_type = (unsigned char) 0xa1;
    } else if (strcmp(string, "REG_NACK") == 0) {
        packet_type = (unsigned char) 0xa2;
    } else if (strcmp(string, "REG_REJ") == 0) {
        packet_type = (unsigned char) 0xa3;
    } else if (strcmp(string, "REG_INFO") == 0) {
        packet_type = (unsigned char) 0xa4;
    } else if (strcmp(string, "INFO_ACK") == 0) {
        packet_type = (unsigned char) 0xa5;
    } else if (strcmp(string, "INFO_NACK") == 0) {
        packet_type = (unsigned char) 0xa6;
    } else if (strcmp(string, "INFO_REJ") == 0) {
        packet_type = (unsigned char) 0xa7;
    } /* alive types */
    else if (strcmp(string, "ALIVE") == 0) {
        packet_type = (unsigned char) 0xb0;
    } else if (strcmp(string, "ALIVE_NACK") == 0) {
        packet_type = (unsigned char) 0xb1;
    } else if (strcmp(string, "ALIVE_REJ") == 0) {
        packet_type = (unsigned char) 0xb3;
    } /* send configuration packet types */
    else if (strcmp(string, "SEND_DATA") == 0) {
        packet_type = (unsigned char) 0xc0;
    } else if (strcmp(string, "DATA_ACK") == 0) {
        packet_type = (unsigned char) 0xc1;
    } else if (strcmp(string, "DATA_NACK") == 0) {
        packet_type = (unsigned char) 0xc2;
    } else if (strcmp(string, "DATA_REJ") == 0) {
        packet_type = (unsigned char) 0xc3;
    } else if (strcmp(string, "SET_DATA") == 0) {
        packet_type = (unsigned char) 0xc4;
    } else if (strcmp(string, "GET_DATA") == 0) {
        packet_type = (unsigned char) 0xc5;
    }
    return packet_type;
}
