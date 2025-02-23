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
} IPv4_addr;


#define SB_STRINGS_LEN 32
typedef struct {
	char* strings[SB_STRINGS_LEN];
	n8 string_count;
} StringBuilder;

static n64 SB_get_length(StringBuilder sb)
{
	n64 len = 1;
	n8 i;

	for(i = 0; i < sb.string_count; ++i)
	{
		len += strlen(sb.strings[i]);
	}

	return len;
}

static void SB_string_append(StringBuilder* sb, char* str)
{
	assert(sb->string_count < SB_STRINGS_LEN);
	sb->strings[sb->string_count++] = str;
}

static void SB_strings_merge(StringBuilder sb, n32 outstr_len, char* outstr)
{
	n8 i;

	n32 offset = 0;
	n64 len = SB_get_length(sb);
	assert(outstr_len >= len);

	for(i = 0; i < sb.string_count; ++i)
	{
		n8 j;

		char* str = sb.strings[i];
		for(j = 0; j < strlen(str); ++j)
		{
			outstr[offset] = str[j];
			offset++;
		}
	}

	outstr[offset] = '\0';
}


static error ipv4_lookup(const char* hostname, IPv4_addr* addr)
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

	*addr = *((IPv4_addr*)hostlookup->h_addr_list[0]);

#if DEBUG_ENABLE
	printf("Resolved %s to %u.%u.%u.%u\n", hostname,
			addr->byte.A, addr->byte.B, addr->byte.C, addr->byte.D);
#endif

	return success;
}


static error fetchs(char* hostname, char* uri)
{
	SSL* ssl = NULL;
	SSL_CTX* ssl_ctx = NULL;

	struct sockaddr_in addr = {0};
	IPv4_addr ipaddr = {0};

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
#define REQ_MAX_LEN 2048
		char request[REQ_MAX_LEN] = {0};
		StringBuilder req_builder = {0};

		n64 totsent_b = 0;
		n64 req_len;

		SB_string_append(&req_builder, "GET ");
		SB_string_append(&req_builder, hostname);
		SB_string_append(&req_builder, uri);
		SB_string_append(&req_builder, " HTTP/1.1\r\n\r\nAccept: application/json\r\nUser-Agent: urmom\r\n");
		req_len = SB_get_length(req_builder);

		SB_strings_merge(req_builder, REQ_MAX_LEN, request);

#if DEBUG_ENABLE
		printf("------------------------------ Writing ------------------------------\n");
#endif
		while(totsent_b < req_len)
		{
			size_t sent_b = 0;
			if(SSL_write_ex(ssl, request, req_len, &sent_b) == 0)
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
			printf("%.*s", (int)sent_b, request);
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

static i32 file_slurp(char* fpath, i32 buff_len, char* buffer)
{
	i32 i;

	FILE* file = fopen(fpath, "r");
	if(file == NULL)
	{
		printf("ERROR:%s:%d: Could not open file %s: %s\n",
				__FILE__, __LINE__, fpath, strerror(errno));
		return -1;
	}

	for(i = 0; i < buff_len; ++i)
	{
		i32 c = fgetc(file);

		if(c == EOF)
		{
			fclose(file);
			return i;
		}

		buffer[i] = (char)c;
	}

	fclose(file);
	return i;
}

int main(void)
{
#define APIKEY_LEN 39
	char apikey[APIKEY_LEN + 1] = {0};

#define URI_LEN 26+APIKEY_LEN+58+1
	char uri[URI_LEN] = {0};

	StringBuilder uri_builder = {0};

	assert(file_slurp("youtube-api.key", APIKEY_LEN, apikey) == APIKEY_LEN);

	SB_string_append(&uri_builder, "/youtube/v3/playlists?key=");
	SB_string_append(&uri_builder, apikey);
	SB_string_append(&uri_builder, "&id=PLd23Y4uu3SslprRLNuBitQft8kb4a7R3q&part=contentDetails");
	SB_strings_merge(uri_builder, URI_LEN, uri);

	if(fetchs("www.googleapis.com", uri))
	{
		return failure;
	}

	return success;
}
