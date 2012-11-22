#include <malloc.h>
#include <stdio.h>
#include <pcap.h>
#include <db.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <netinet/if_ether.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>

#include "tcpio.h"

//typedef enum {false, true} bool;

#pragma pack(1) 
typedef struct 
{
	struct pcap_pkthdr header;
	u_char		*data;
} packet;


/* BDB */
const char *pmf_db_name = "pmf_db";
const char *mf_db_name = "mf_db";
DB_ENV *pmf_envp;
DB_ENV *mf_envp;
DB *pmf_dbp;
DB *mf_dbp;
DB_TXN *pmf_txn;
DB_TXN *mf_txn;
//char *db_home = "/usr/local/BerkeleyDB.5.3/";
char *db_home = "./";
void open_db(DB_ENV *envp, DB *dbp, DB_TXN *txn, DBC *cs, const char *db_name, u_int cache_size);




/* PacketQueue */
#pragma pack(1)
typedef struct 
{
	u_int	capacity;
	u_int	size;
	u_int	front;
	u_int	rear;
	packet*	data;
	bool	use;
}packetqueue;

packetqueue *pq;

#pragma pack(1)
typedef struct
{
	unsigned int SIP;
	unsigned int DIP;
	unsigned short SPRT;
	unsigned short DPRT;
	unsigned char PROT;
}key;

packetqueue*	init_packet_queue(int capacity);
void			delete_packet_queue(packetqueue* queue);
bool			packet_enqueue(packet data, packetqueue* queue);
void			packet_dequeue(packet *packet, packetqueue* queue);
u_int			get_packet_queue_size(packetqueue* queue);
bool			packet_queue_is_empty(packetqueue* queue);
bool			packet_queue_is_full(packetqueue* queue);



/* packet spliter */
typedef struct {
	pthread_t handle;
	bool running;
	packet p;
} manufacturingthread;

pthread_t spliter_handle;
manufacturingthread* pool;
int	pool_size;

void init_split_thread(int num_of_thread);
void* packet_spliter(void *);



/* pcap */
pcap_t *adhandle;
char errbuf[PCAP_ERRBUF_SIZE];

void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
void init_pcap_thread(char* device);
void start_pcap_thread();
void stop_pcap_thread();



/* manufacturing */ 
#pragma pack(1)
typedef struct{
	unsigned int SIP;
	unsigned int DIP;
	unsigned short SPRT;
	unsigned short DPRT;
	unsigned int ST;
	unsigned int DT;
	unsigned int CNT;
	unsigned int BYTE;
	unsigned char PROT;
	unsigned char BUND; 
	unsigned char FLAG;
	unsigned char RSRV;
	unsigned int SQC;
} premanufacturedflow;

#pragma pack(1)
typedef struct{
	unsigned int SIP;
	unsigned int DIP;
	unsigned short SPRT;
	unsigned short DPRT;
	unsigned int ST;
	unsigned int DT;
	unsigned int CNT;
	unsigned int BYTE;
	unsigned char PROT;
	unsigned char BUND; 
	unsigned char FLAG;
	unsigned char SQC;
} manufacturedflow;

u_int8_t rt_mac_addr[ETH_ALEN]; // 1: out bound, 0: in bound
void* manufacturing_thread(void *arg);
u_int8_t get_bound(struct ether_header *ethh);


TCPIO_CLIENT *client;

/* transmission */
DBC *mf_cursor;
pthread_t ts_handle;
void init_ts_thread(int size, string addr, int port);
void* ts_thread(void *);
