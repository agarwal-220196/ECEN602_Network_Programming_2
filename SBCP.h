#ifndef SBCP
#define SBCP
//Structure for SBCP header
struct SBCP_header{
    unsigned int vrsn: 9;
    unsigned int type: 7;
    int length;
};

//Structure for SBCP attributes
struct SBCP_attribute{
    int type;
    int length;
    char payload[562];
};

//Structure for SBCP message
struct SBCP_message{
    struct SBCP_header header;
    struct SBCP_attribute attribute[];
};
//Structure for saving client's information
struct SBCP_client_info{
    char username[16];
    int fd;
    int clientNum;
};

#endif