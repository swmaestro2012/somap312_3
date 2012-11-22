#include "agent.h"

/* BDB */
void open_db(DB_ENV *envp, DB *dbp, DB_TXN *txn, DBC *cs, const char *db_name, u_int cache_size)
{
	int ret, ret_c, nRet;
	u_int32_t db_flags, env_flags;
	
	ret = db_env_create(&envp, 0);
	if (ret != 0) {
		fprintf(stderr, "Error creating environment handle: %s\n",
		db_strerror(ret));
		exit(1);
	}
	
	env_flags = DB_CREATE | /* Create the environment if it does not already exist. */
	DB_INIT_TXN | /* Initialize transactions */
	DB_INIT_LOCK | /* Initialize locking. */
	DB_INIT_MPOOL; /* Initialize the in-memory cache. */
	
	ret = envp->open(envp, NULL, env_flags, 0);
	if (ret != 0) {
		fprintf(stderr, "Error opening environment: %s\n", db_strerror(ret));
		goto err;
	}
	
	
	/* Initialize the DB handle */
    ret = db_create(&dbp, envp, 0);
    if (ret != 0) {
        fprintf(stderr, "Error creating database handle: %s\n",
                    db_strerror(ret));
        goto err;
    }
	/*
	ret = dbp->set_cachesize(dbp, 
                             0,     /* 0 gigabytes */
	//						cache_size,     
     //                       1);     /* Create 1 cache. All memory will 
                                    /* be allocated contiguously. */
									/*
    if (ret != 0) {
		printf("ret = %d\n", ret);
        dbp->err(dbp, ret, "Database open failed");
        goto err;
    }
	*/
	
	db_flags = DB_CREATE | DB_AUTO_COMMIT;
	
	/* 
     * Open the database. Note that the file name is NULL. 
     * This forces the database to be stored in the cache only.
     * Also note that the database has a name, even though its
     * file name is NULL.
     */
    ret = dbp->open(dbp,        /* Pointer to the database */
                    NULL,       /* Txn pointer */
                    NULL,       /* File name is not specified
                                 * on purpose       */
                    db_name,    /* Logical db name. */
                    DB_BTREE,   /* Database type (using btree) */
                    db_flags,   /* Open flags */
                    0);         /* File mode. Using defaults */
    if (ret != 0) {
        dbp->err(dbp, ret, "Database open failed");
        goto err;
    }
	
	if ((nRet = envp->txn_begin(envp, NULL, &txn, 0)) != 0)
    {
        printf("fail to begin transaction, %s\n", db_strerror(nRet));
        goto err;
    }
	
	if ( (nRet = dbp->cursor(dbp, NULL, &cs, 0)) != 0 )
    {
        printf("fail to cursor, %s\n", db_strerror(nRet));
        goto err;
    }
	
err:
	/* Close the database */
    if (pmf_dbp != NULL) {
        ret_c = pmf_dbp->close(pmf_dbp, 0);
        if (ret_c != 0) {
            fprintf(stderr, "%s database close failed.\n",
                 db_strerror(ret_c));
            ret = ret_c;
        }
    } 
	
	if (mf_dbp != NULL) {
        ret_c = mf_dbp->close(mf_dbp, 0);
        if (ret_c != 0) {
            fprintf(stderr, "%s database close failed.\n",
                 db_strerror(ret_c));
            ret = ret_c;
        }
    } 
}


/* packet queue */
packetqueue*	init_packet_queue(int capacity)
{
	packetqueue *q = (packetqueue *)malloc(sizeof(packetqueue));
	
	q->capacity = capacity;
	q->front	= 0;
	q->rear		= 0;
	q->size = 0;
	q->data		= (packet *)malloc(sizeof(packet)*capacity);
	q->use		= false;

	return q;
}


void			delete_packet_queue(packetqueue* queue)
{
	while(queue->use == true){};
	queue->use = true;

	while( packet_queue_is_empty(queue) != true)
	{
		packet p;
		packet_dequeue(&p, queue);
		free(p.data);
	}

	free(queue->data);
	free(queue);
}


bool			packet_enqueue(packet data, packetqueue* queue)
{
	while(queue->use == true){};
	queue->use = true;


	if( packet_queue_is_full(queue) )	return false;

	queue->rear = (queue->rear + 1) % queue->capacity;
	queue->data[queue->rear] = data;
	queue->size++;

	queue->use = false;
	return true;
}


void			packet_dequeue(packet *p, packetqueue* queue)
{
	while(queue->use == true){};
	queue->use = true;


	if( packet_queue_is_empty(queue) )
	{
	}
	else
	{
		*p = (queue->data[queue->front]);
		queue->front = (queue->front + 1) % queue->capacity;
		queue->size--;
	
		queue->use = false;
	}
}

u_int			get_packet_queue_size(packetqueue* queue)
{
	if (queue->front <= queue->rear)
		return queue->rear - queue->front;
	else
		return (queue->capacity + 1) - queue->front + queue->rear;
}

bool			packet_queue_is_empty(packetqueue* queue)
{
	if (queue->front == queue->rear)
		return true;
	else
		return false;
}

bool			packet_queue_is_full(packetqueue* queue)
{
	if(((queue->rear+1) % queue->capacity) == queue->front)
		return true;
	else
		return false;
}



/* packet spliter */
void init_split_thread(int num_of_threads)
{
	pool_size = num_of_threads;
	pool = (manufacturingthread*)malloc(sizeof(manufacturingthread)*num_of_threads);
	
	int i;
	for(i = 0; i < num_of_threads; i++)
	{
		pool[i].running = false;
		if(pthread_create(pool[i].handle, NULL, manufacturing_thread, (void *)(&pool[i]) ))
		{
			i--;
		}
	}
	
	pthread_create(spliter_handle, NULL, packet_spliter, NULL);
}

void* packet_spliter(void *)
{
	int i;
	while(1)
	{
		for(i = 0; i < pool_size; i++)
		{
			if(packet_queue_is_empty == false && pool[i].running == false)
			{
				packet_dequeue(&(pool[i].p), pq);
				pool[i].running = true;
			}
		}
	}
}

 
/* pcap */
void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data)
{
	int i, data_size; 
	packet p;
	p.header = *header;
	data_size = 24 + sizeof(u_char *) * ((struct iphdr *)(pkt_data + sizeof(struct ethhdr)))->tot_len ;
	p.data = ( u_char *)malloc( data_size );
	
	for(int i = 0; i < data_size; i++)
	{
		p.data[i] = pkt_data[i];
	}
	packet_enqueue(p, pq);
}

void init_pcap_thread(char* device)
{
	if ((adhandle= pcap_open_live(device, // name of the device
                                 65536,             // portion of the packet to capture. 
                                                // 65536 grants that the whole packet will be captured on all the MACs.
                                 1,         // promiscuous mode (nonzero means promiscuous)
                                 0,              // read timeout
                                 errbuf         // error buffer
                                 )) == NULL)
    {
        fprintf(stderr,"\nUnable to open the adapter. %s is not supported by pcap\n", device);
        exit(1);
    }
	
	pq = init_packet_queue(1048576);
}

void start_pcap_thread()
{
	pcap_loop(adhandle, 0, packet_handler, NULL);
}

void stop_pcap_thread()
{
	pcap_close(adhandle);
}



/* manufacturing */
void* manufacturing_thread(void *arg)
{
	manufacturingthread* job = (manufacturingthread*) arg;
	

	while(1)
	{
		while(!job->running);
		
		{
			struct iphdr *iph;
			premanufacturedflow pmf;
			manufacturedflow	mf;
			struct tm *time;
			DBT db_key, db_data;
			key *k;
			
			iph = (struct iphdr *)(job->p.data + sizeof(struct ethhdr));
			
			
			
			pmf.SIP		= iph->saddr;
			pmf.DIP		= iph->daddr;
			pmf.PROT	= iph->protocol;
			pmf.CNT		= 1;
			pmf.BYTE	= iph->tot_len;
			
			mf.FLAG 	= false;
			mf.SQC 		= false;
			
			
			if(pmf.PROT == 6)//TCP
			{
				struct tcphdr *tcph = (struct tcphdr *)(job->p.data + sizeof(struct ethhdr) + (iph->ihl * 4));
				pmf.DPRT = tcph->dest;
				pmf.SPRT = tcph->source;
				pmf.SQC	 = tcph->seq;
				pmf.FLAG = tcph->urg;
				pmf.FLAG = (pmf.FLAG << 1) + tcph->ack;
				pmf.FLAG = (pmf.FLAG << 1) + tcph->psh;
				pmf.FLAG = (pmf.FLAG << 1) + tcph->rst;
				pmf.FLAG = (pmf.FLAG << 1) + tcph->syn;
				pmf.FLAG = (pmf.FLAG << 1) + tcph->fin;
			}
			else if(pmf.PROT == 17)//UDP
			{
				struct udphdr *udph = (struct udphdr *)(job->p.data + sizeof(struct ethhdr) + (iph->ihl * 4));
				pmf.SPRT = udph->source;
				pmf.DPRT = udph->dest;
			}
			else if(pmf.PROT == 1)//ICMP
			{
			}
			else //else
			{
			}
			
			time = localtime(&job->p.header.ts.tv_sec);
			
			pmf.ST = mktime(time);
			pmf.DT = mktime(time);
			
			pmf.BUND = get_bound((struct ether_header*)job->p.data);
			
			
			k = (key *)malloc(sizeof(key));
			
			k->SIP = pmf.SIP;
			k->DIP = pmf.DIP;
			k->SPRT = pmf.SPRT;
			k->DPRT = pmf.DPRT;
			k->PROT = pmf.PROT;
			
			memset(&db_key, 0, sizeof(DBT));
			memset(&db_data, 0, sizeof(DBT));

			db_key.data = (void *)k;
			db_key.size = sizeof(key);

			db_data.data = malloc(sizeof(premanufacturedflow));
			db_data.ulen = sizeof(premanufacturedflow);
			db_data.flags = DB_DBT_USERMEM;

			if ( pmf_dbp->get(pmf_dbp, pmf_txn, &db_key, &db_data, 0) == 0 )
			{
				premanufacturedflow* prev_pmf = (premanufacturedflow *)(db_data.data);
				pmf.ST = prev_pmf->ST;
				pmf.CNT++;
				pmf.BYTE = pmf.BYTE + prev_pmf->BYTE;
				
				if(pmf.PROT == 6) //TCP일 경우 처리
				{
					if(prev_pmf->FLAG == pmf.FLAG)
					{
						mf.FLAG = true;
					}
				
					if(prev_pmf->SQC == pmf.SQC)
					{
						mf.SQC = true;
					}
				}
			}
			
			free(db_data.data);
			db_data.data = (void *)(&pmf);
			db_data.size = sizeof(premanufacturedflow);
			db_data.flags = 0;
			pmf_dbp->put(pmf_dbp, NULL, &db_key, &db_data, 0);
			//위 값이 0보다 작다면 에러
			//후에 에러처리 바람
			
			
			
			mf.SIP = pmf.SIP;
			mf.DIP = pmf.DIP;
			mf.SPRT = pmf.SPRT;
			mf.DPRT = pmf.DPRT;
			mf.ST = pmf.ST;
			mf.DT = pmf.DT;
			mf.CNT = pmf.CNT;
			mf.BYTE = pmf.BYTE;
			mf.PROT = pmf.PROT;
			mf.BUND = pmf.BUND;			

			db_data.data = (void *)(&mf);
			db_data.size = sizeof(manufacturedflow);
			db_data.flags = 0;
			mf_dbp->put(mf_dbp, NULL, &db_key, &db_data, 0);
			
			
			free(k);
		}
		
		
		job->running = false;
	}
}

u_int8_t get_bound(struct ether_header *ethh)
{
	int i;
	for (i = 0; i < ETH_ALEN; i++)
	{
		if( ethh->ether_shost[i] != rt_mac_addr[i] )
		{
			return 1;
		}
	}
	return 0;
}

void init_ts_thread(int size, string addr, int port)
{
	client = new TCPIO_CLIENT(size, addr, port);
	if(pthread_create(ts_handle, NULL, ts_thread, NULL))
	{
		printf("fail init tcpio_thread");
	}
}

void* ts_thread(void *)
{
	int nRet;
	DBT db_key, db_data;
	
	while(1)
	{
		/*
			iterate each record
		*/
		
		while ((nRet = mf_cursor->get(mf_cursor, &db_key, &db_data, DB_NEXT)) == 0)
		{
			client->SendPacket((void*)&db_data );
		}   
	}
}


int main(int argc, char *argv[])
{
	//config 읽어오게 변경 필
	int i;
	
	db_home = NULL;
	
	for( i = 0; i < ETH_ALEN; i++)
	{
		rt_mac_addr[i] = 0xFF;
	}
	
	open_db(pmf_envp, pmf_dbp, pmf_txn, mf_cursor, pmf_db_name, 500 * 1024 * 1024);/* 500 megabytes */
	open_db(mf_envp, mf_dbp, mf_txn, mf_cursor, mf_db_name, 500 * 1024 * 1024);/* 500 megabytes */
	init_pcap_thread("br0"); 
	init_split_thread(4);
	start_pcap_thread();
	init_ts_thread(sizeof(manufacturedflow), "10.12.17.212", 12170);
	
	return 0;
}
