#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <assert.h>
#include <errno.h>

#include <netdb.h>
#include <unistd.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>

typedef enum {
	true  = (1 == 1),
	false = (1 != 1)
} bool;

typedef enum {
	failure = true,
	success = false
} error;

typedef unsigned char n8;
typedef signed char i8;

typedef unsigned short n16;
typedef signed short i16;

typedef unsigned int n32;
typedef signed int i32;

typedef unsigned long n64;
typedef signed long i64;

typedef union {
	struct in_addr in;
	struct { n8 A, B, C, D; } byte;
	n8 bytes[4];
	n32 dword;
} ipv4_addr;


static error ipv4_lookup(const char* hostname, ipv4_addr* addr)
{
	struct hostent* hostlookup = gethostbyname(hostname);
	if(hostlookup == NULL)
	{
		printf("ERROR:%s:%d: (ipv4_lookup) Could resolve %s: %s\n",
				__FILE__, __LINE__, hostname, strerror(h_errno));
		return failure;
	}

	if(hostlookup->h_addrtype != AF_INET)
	{
		printf("ERROR:%s:%d: (ipv4_lookup) Hostname %s resolved in a non IPv4 address\n",
				__FILE__, __LINE__, hostname);
		return failure;
	}

	*addr = *((ipv4_addr*)hostlookup->h_addr_list[0]);

#if DEBUG_ENABLE
	printf("Resolved %s to %u.%u.%u.%u\n", hostname,
			addr->byte.A, addr->byte.B, addr->byte.C, addr->byte.D);
#endif

	return success;
}


static error fetch(const char* hostname, const char* uri)
{
	struct sockaddr_in addr = {0};
	ipv4_addr ipaddr = {0};

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("ERROR:%s:%d: Could not open socket: %s\n",
				__FILE__, __LINE__, strerror(errno));
		return failure;
	}

	if(ipv4_lookup(hostname, &ipaddr))
	{
		printf("ERROR:%s:%d: (fetch) Could not lookup hostname\n",
				__FILE__, __LINE__);
		return failure;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.s_addr = ipaddr.dword;

	if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		printf("ERROR:%s:%d: (fetch) Could not connect to %u.%u.%u.%u: %s\n",
				__FILE__, __LINE__, ipaddr.byte.A, ipaddr.byte.B,
				ipaddr.byte.C, ipaddr.byte.D, strerror(errno));
		return failure;
	}

	{
		const char* msg =
			"GET /youtube/v3/playlists HTTP/1.1\r\n"
			"\r\n";

		n64 msg_len = (n64)strlen(msg);
		n64 totsent_b = 0;

#if DEBUG_ENABLE
		printf("------------------------------ Writing ------------------------------\n");
#endif
		while(totsent_b < msg_len)
		{
			ssize_t sent_b = write(sockfd, msg, msg_len);
			if(sent_b == -1)
			{
#if DEBUG_ENABLE
				printf("---------------------------- END Writing ----------------------------\n");
#endif
				printf("ERROR:%s:%d: (fetch) Could not write to socket: %s\n",
						__FILE__, __LINE__, strerror(errno));
				return failure;
			}

#if DEBUG_ENABLE
			printf("%.*s", (int)sent_b, msg);
#endif
			totsent_b += (n64)sent_b;
		}

		if(shutdown(sockfd, SHUT_WR) == -1)
		{
			printf("ERROR:%s:%d: (fetch) Could not shutdown WR connection: %s\n",
					__FILE__, __LINE__, strerror(errno));
			return failure;
		}

#if DEBUG_ENABLE
		printf("---------------------------- END Writing ----------------------------\n");
#endif
	}

	{
#define BUFF_LEN 1024
		char buffer[BUFF_LEN] = {0};
		ssize_t read_b = 0;

		FILE* dump = fopen("read_dump.http", "w");
		if(dump == NULL)
		{
			printf("ERROR:%s:%d: (fetch) Could not open dump file: %s\n",
					__FILE__, __LINE__, strerror(errno));
			return failure;
		}

#if DEBUG_ENABLE
		printf("------------------------------ Reading ------------------------------\n");
#endif
		do {
			read_b = recv(sockfd, buffer, BUFF_LEN, 0);
			/* read_b = read(sockfd, buffer, BUFF_LEN); */
			if(read_b == -1)
			{
#if DEBUG_ENABLE
				printf("\n---------------------------- END Reading ----------------------------\n");
#endif
				printf("ERROR:%s:%d: (fetch) Could not read from socket: %s\n",
						__FILE__, __LINE__, strerror(errno));
				return failure;
			}

#if DEBUG_ENABLE
			if(read_b > 0)
			{
				printf("%.*s", (int)read_b, buffer);
				fprintf(dump, "%.*s", (int)read_b, buffer);
			}
#endif

			printf("\033[31m[%d]\033[0m\n", (int)read_b);

		} while(read_b > 0);

#if DEBUG_ENABLE
		printf("\n---------------------------- END Reading ----------------------------\n");
#endif

		assert(fclose(dump) == 0);
	}

	assert(close(sockfd) == 0);

	(void)uri;
	return failure;
}

static error fetchs(const char* hostname, const char* uri)
{
	SSL* ssl = NULL;
	SSL_CTX* ssl_ctx = NULL;

	struct sockaddr_in addr = {0};
	ipv4_addr ipaddr = {0};

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1)
	{
		printf("ERROR:%s:%d: Could not open socket: %s\n",
				__FILE__, __LINE__, strerror(errno));
		return failure;
	}

	if(ipv4_lookup(hostname, &ipaddr))
	{
		printf("ERROR:%s:%d: (fetchs) Could not lookup hostname\n",
				__FILE__, __LINE__);
		return failure;
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(443);
	addr.sin_addr.s_addr = ipaddr.dword;

	if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
	{
		printf("ERROR:%s:%d: (fetchs) Could not connect to %u.%u.%u.%u: %s\n",
				__FILE__, __LINE__, ipaddr.byte.A, ipaddr.byte.B,
				ipaddr.byte.C, ipaddr.byte.D, strerror(errno));
		return failure;
	}

	ssl_ctx = SSL_CTX_new(TLS_method());
	if(ssl_ctx == NULL)
	{
		/* TODO: Check the error stack to find out the reason for failure */
		printf("ERROR:%s:%d: (ssl) Could not create new context\n", __FILE__, __LINE__);
		return failure;
	}

	ssl = SSL_new(ssl_ctx);
	if(ssl == NULL)
	{
		/* TODO: Check the error stack to find out the reason */
		printf("ERROR:%s:%d: (ssl) Could not create ssl?\n", __FILE__, __LINE__);
		return failure;
	}

	if(SSL_set_fd(ssl, sockfd) == 0)
	{
		/* TODO: Check the error stack to find out the reason */
		printf("ERROR:%s:%d: (ssl) Could not bind ssl to sockfd?\n", __FILE__, __LINE__);
		return failure;
	}

	if(SSL_connect(ssl) != 1)
	{
		/* TODO: Call SSL_get_error(3) with the return value ret to find out the reason */
		printf("ERROR:%s:%d: (ssl) SSL handshake was not succesful\n", __FILE__, __LINE__);
		return failure;
	}

	{
#define MSG_LEN 2048
		char msg[MSG_LEN] = "GET ";
		const char postfix[] =
			" HTTP/1.1\r\n"
			"\r\n"
			"Accept: application/json\r\n"
			"User-Agent: urmom\r\n";

		n64 i;

		n64 totsent_b = 0;
		n64 msg_len;

		assert(4+strlen(hostname)+strlen(postfix) < MSG_LEN);
		for(i = 0; i < strlen(hostname); ++i)
		{
			msg[4+i] = hostname[i];
		}
		assert(4+strlen(hostname)+strlen(uri)+strlen(postfix) < MSG_LEN);
		for(i = 0; i < strlen(uri); ++i)
		{
			msg[4+strlen(hostname)+i] = uri[i];
		}
		for(i = 0; i < strlen(postfix); ++i)
		{
			msg[4+strlen(hostname)+strlen(uri)+i] = postfix[i];
		}

		msg_len = (n64)strlen(msg);

#if DEBUG_ENABLE
		printf("------------------------------ Writing ------------------------------\n");
#endif
		while(totsent_b < msg_len)
		{
			size_t sent_b = 0;
			if(SSL_write_ex(ssl, msg, msg_len, &sent_b) == 0)
			{
#if DEBUG_ENABLE
				printf("---------------------------- END Writing ----------------------------\n");
#endif

				/* TODO: In the event of a failure, call SSL_get_error(3)
				 * to find out the reason which indicates whether the call is retryable or not */
				printf("ERROR:%s:%d: (ssl) Could not write to ssl socket\n", __FILE__, __LINE__);
				return failure;
			}

#if DEBUG_ENABLE
			printf("%.*s", (int)sent_b, msg);
#endif
			totsent_b += sent_b;
		}

		/* if(shutdown(sockfd, SHUT_WR) == -1) */
		/* { */
		/* 	printf("ERROR:%s:%d: (fetch) Could not shutdown WR connection: %s\n", */
		/* 			__FILE__, __LINE__, strerror(errno)); */
		/* 	return failure; */
		/* } */

#if DEBUG_ENABLE
		printf("---------------------------- END Writing ----------------------------\n");
#endif
	}

	{
#define BUFF_LEN 1024
		char buffer[BUFF_LEN] = {0};
		size_t read_b = 0;

		FILE* dump = fopen("read_dump.http", "w");
		if(dump == NULL)
		{
			printf("ERROR:%s:%d: (fetch) Could not open dump file: %s\n",
					__FILE__, __LINE__, strerror(errno));
			return failure;
		}

#if DEBUG_ENABLE
		printf("------------------------------ Reading ------------------------------\n");
#endif
		do {
			if(SSL_read_ex(ssl, buffer, BUFF_LEN, &read_b) == 0)
			{
				/* TODO: In the event of a failure, call SSL_get_error(3)
				 * to find out the reason which indicates whether the call is retryable or not */
#if DEBUG_ENABLE
				printf("\n---------------------------- END Reading ----------------------------\n");
#endif
				printf("ERROR:%s:%d: (ssl) Could not read from ssl socket\n", __FILE__, __LINE__);
				return failure;
			}

#if DEBUG_ENABLE
			if(read_b > 0)
			{
				printf("%.*s", (int)read_b, buffer);
				fprintf(dump, "%.*s", (int)read_b, buffer);
			}
#endif

			assert(fflush(dump) == 0);
			printf("\033[31m[%lu]\033[0m\n", read_b);

		} while(read_b > 0);

#if DEBUG_ENABLE
		printf("\n---------------------------- END Reading ----------------------------\n");
#endif

		assert(fclose(dump) == 0);
	}

	{
		int res = SSL_shutdown(ssl);
		if(res == -1)
		{
			/* TODO: Call SSL_get_error(3) with the return value ret to find out the reason */
			printf("ERROR:%s:%d: (ssl) Could not shutdown ssl socket\n", __FILE__, __LINE__);
			return failure;
		}

		if(res == 0)
		{
			res = SSL_shutdown(ssl);
			if(res == -1)
			{
				/* TODO: Call SSL_get_error(3) with the return value ret to find out the reason */
				printf("ERROR:%s:%d: (ssl) Could not shutdown ssl socket\n", __FILE__, __LINE__);
				return failure;
			}

			assert(res == 1);
		}
	}

	SSL_free(ssl);

	assert(close(sockfd) == 0);

	(void)uri;
	return failure;
}

int main(void)
{
	int i;

	#define APIKEY_LEN 39
	char apikey[APIKEY_LEN + 1] = {0};

	FILE* apikey_file = fopen("youtube-api.key", "r");
	if(apikey_file == NULL)
	{
		printf("ERROR:%s:%d: Could not open apikey file: %s\n",
				__FILE__, __LINE__, strerror(errno));
		return failure;
	}

	for(i = 0; i < APIKEY_LEN; ++i)
	{
		i32 c = fgetc(apikey_file);
		if(c == EOF)
		{
			printf("ERROR:%s:%d: Api key is incomplete, read %d chars out of %d\n",
					__FILE__, __LINE__, i, APIKEY_LEN-1);
			return failure;
		}
		apikey[i] = (char)c;
	}

	{
		char uri[26+APIKEY_LEN+58+1] = {0};
		for(i = 0; i < 26; ++i)
		{
			char str[] = "/youtube/v3/playlists?key=";
			uri[i] = str[i];
		}

		for(i = 0; i < APIKEY_LEN; ++i)
		{
			uri[26+i] = apikey[i];
		}

		for(i = 0; i < 58; ++i)
		{
			char str[] = "&id=PLd23Y4uu3SslprRLNuBitQft8kb4a7R3q&part=contentDetails";
			uri[26+APIKEY_LEN+i] = str[i];
		}

		if(fetchs("www.googleapis.com", uri))
		{
			return failure;
		}
	}

	(void)fetch;
	(void)fetchs;

	return success;
}
