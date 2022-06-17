#include <stdio.h> //for sprintf function
#include <stdlib.h>
#include "memstat.h" // my file to get value from /proc/meminfo
#include <time.h> //for get current time
#include <unistd.h> //for time sleep child thread
#include <sqlite3.h> // lib for sqlite database
#include <sys/socket.h> // {
#include <sys/types.h>  //	for work in lan
//#include <netinet/in.h> // }
#include <arpa/inet.h>
//#include <semaphore.h>
#include <string.h>
#include <pthread.h> //for multithreading
int have_a_reques=0;
pthread_mutex_t lock;
pthread_mutex_t lock_o;

char* err;
sqlite3 *db;
sqlite3_stmt* stmt;

int exit_s=0;
char buf_i[200]={};
//char buf_o[10240];
//size_t 
int write_mem_value_in_database(sqlite3 *db){
	time_t nowtime=time(NULL);
	struct tm *time_s;     		//value for get mem and time
	struct me_memory memory;
	char query[200]; //query to sqlite database 200 may be fix
	char *err;
	getmeminfo(&memory);
	nowtime=time(NULL);  //update
	time_s=localtime(&nowtime);
	
	sprintf(query,"insert into memstat VALUES(%d,%d,%d,%d,%d,%d,%d,%d,%d,%d);\0",memory.mTotal/1024,memory.mFree/1024,memory.mAvailable/1024,memory.mCached/1024,time_s->tm_sec,time_s->tm_min,time_s->tm_hour,time_s->tm_mday,time_s->tm_mon+1,time_s->tm_year+1900);
	printf("%s",query);
	int rc=sqlite3_exec(db,query,NULL,NULL,&err);
	if(rc!=SQLITE_OK){printf("Error add value - %c\n",&err);return 1;}
	else printf("\n");
	return 0;}
void show_all_base_mem(sqlite3 *db,sqlite3_stmt* stmt){
	sqlite3_prepare_v2(db,"select m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year from memstat",-1,&stmt,0);	
	int m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year;
	while(sqlite3_step(stmt)!=SQLITE_DONE){
		m_total=sqlite3_column_int(stmt,0);
		m_free=sqlite3_column_int(stmt,1);
		m_cached=sqlite3_column_int(stmt,2);
		m_available=sqlite3_column_int(stmt,3);
		t_sec=sqlite3_column_int(stmt,4);
		t_min=sqlite3_column_int(stmt,5);
		t_hour=sqlite3_column_int(stmt,6);
		t_day=sqlite3_column_int(stmt,7);
		t_mon=sqlite3_column_int(stmt,8);
		t_year=sqlite3_column_int(stmt,9);
		printf("%ld %ld %ld %ld ",m_total, m_free, m_cached, m_available);
		printf("%d-%d:%d Day:%d Month:%d Year:%d\n\n\0",t_sec,t_min,t_hour,t_day,t_mon,t_year);}}

	
void send_base_mem(sqlite3 *db,sqlite3_stmt* stmt,int sock){
	char temp_buf[128]={};
	int m_total, m_free, m_cached, m_available, t_sec,t_min,t_hour,t_day,t_mon,t_year;
	while(sqlite3_step(stmt)!=SQLITE_DONE){
		m_total=sqlite3_column_int(stmt,0);
		m_free=sqlite3_column_int(stmt,1);
		m_cached=sqlite3_column_int(stmt,2);
		m_available=sqlite3_column_int(stmt,3);
		t_sec=sqlite3_column_int(stmt,4);
		t_min=sqlite3_column_int(stmt,5);
		t_hour=sqlite3_column_int(stmt,6);
		t_day=sqlite3_column_int(stmt,7);
		t_mon=sqlite3_column_int(stmt,8);
		t_year=sqlite3_column_int(stmt,9);
		sprintf(temp_buf,"MemTotal: %ld Mb\n"
						"MemFree: %ld Mb\n"
						"Cached: %ld Mb\n"
						"MemAvailable: %ld Mb\n"

		
		"%d:%d - %d\nDay:%d Month:%d Year:%d\n\n\0",m_total, m_free, m_cached, m_available,t_hour,t_min,t_sec,t_day,t_mon,t_year);
		int i=0;
		for(;temp_buf[i]!='\0';i+=1);
		//printf("<<%d>> ",i);
		send(sock,&temp_buf,i,0);
	}}
	

void *thread_w_in_database(void *arg){
	
	printf("Thread write in database active\n");
	while(1){
		sleep(60);
		write_mem_value_in_database((sqlite3 *)arg);}}
void *thread_r_socket(void *arg){
	int *have_request=arg;
	int sock,listener;
	struct sockaddr_in addr;
	int bytes_read=0;
	listener=socket(AF_INET,SOCK_STREAM,0);
	if(listener<0){printf("Socket not open!!\n");exit(1);}
	else printf("Socket was opened successfully\n");
	
	addr.sin_family=AF_INET;
	addr.sin_port=htons(4000);
	addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	if(bind(listener,(struct sockaddr *)&addr,sizeof(addr))<0){printf("Not connect to socket\n");exit(2);}
	else printf("Connect to socket successfull\n");
	listen(listener,1);
	while(1){
		sock=accept(listener,NULL,NULL);
		if(sock<0){printf("Client not accept\n");exit_s=1;}
		
		pthread_mutex_lock(&lock);
		bytes_read=recv(sock,&buf_i,sizeof(buf_i),0);
		if(bytes_read<=0){continue;}
		
		*have_request=1;
		pthread_mutex_unlock(&lock);
		
		
		printf("%s\n",buf_i);
		sqlite3_prepare_v2(db,buf_i,-1,&stmt,0);
		
		pthread_mutex_lock(&lock_o);
		send_base_mem(db,stmt,sock);
		pthread_mutex_unlock(&lock_o);
		close(sock);

		}}
int main(int argc, char **argv)
{

	int rc=sqlite3_open("stat_database.db",&db);
	if(rc){
		printf("Error not open base break");
		sqlite3_close(db); 
		return 1;}
	printf("Database open\n");
	
	rc=sqlite3_exec(db,"CREATE TABLE IF NOT EXISTS memstat(m_total INT, m_free INT, m_cached INT, m_available INT, t_sec INT,t_min INT,t_hour INT,t_day INT,t_mon INT,t_year INT);",NULL,NULL,&err);
	if(rc!=SQLITE_OK){printf("Error create table - %c",err);}
	
	
	pthread_t tid_w,tid_sr;
	pthread_create(&tid_w,NULL,thread_w_in_database,(void *)db); 
	pthread_create(&tid_sr,NULL,thread_r_socket,(void *)&have_a_reques);
	printf("MAIN THREAD ACTIVE\n");
	while(exit_s!=1){
		usleep(5000);
		}
	sqlite3_close(db);
	
	
	
	return 0;
}
