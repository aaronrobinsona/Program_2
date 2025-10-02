//peer.c
//Program_2 Group 17
//Aaron Robinson almazan
//James C Oppy
/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT "5432" // This must match on client and server
#define BUF_SIZE 256 // This can be smaller. What size?

/*
 * Lookup a host IP address and connect to it using service. Arguments match the first two
 * arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible for closing
 * the returned socket.
 */
static void trim_newline(char *s) { //Need this because user hits enter after typing for ex. "JOIN" adds "\n" to string = "JOIN\n"
    size_t n = strlen(s);
    if (n && s[n-1] == '\n') {
        s[n-1] = '\0';
    }
}

int connect_to_registry( const char *host, const char *service );

int main( int argc, char *argv[] ) {
        char *registry_host;
	char *registry_port;
	char *peer_id;
        char buf[BUF_SIZE];
        int s;
        int len;
        uint32_t a, b;
        uint32_t answer;

        if ( argc == 4 ) {
                registry_host = argv[1];
		registry_port = argv[2];
		peer_id = argv[3];
        }
        else {
                fprintf( stderr, "usage: %s host\n", argv[0] );
                exit( 1 );
        }

        /* Lookup IP and connect to server */
        if ( ( s = connect_to_registry( registry_host, registry_port) ) < 0 ) {
                exit( 1 );
        }
	
        while(1) {

          printf("Enter a command: ");
          fgets(buf, BUF_SIZE, stdin);
          trim_newline(buf);

          if(strcmp(buf, "JOIN") == 0)
            {
              // build JOIN message and send
            }
          else if(strcmp(buf, "PUBLISH") == 0)
            {
              // build PUBLISH message (scan ./SharedFiles) and send
            }
          else if(strcmp(buf, "SEARCH") == 0)
            {
              // ask user for filename, build SEARCH request, send
              //then recv the 10-byte response and print result
            }
          else if (strcmp(buf, "EXIT") == 0)
            {
              break;
            }
        }

        close( s );

        return 0;
}



int connect_to_registry( const char *host, const char *service ) {
        struct addrinfo hints;
        struct addrinfo *rp, *result;
        int s;

        /* Translate host name into peer's IP address */
        memset( &hints, 0, sizeof( hints ) );
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = 0;
        hints.ai_protocol = 0;

        if ( ( s = getaddrinfo( host, service, &hints, &result ) ) != 0 ) {
                fprintf( stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror( s ) );
                return -1;
        }

        /* Iterate through the address list and try to connect */
        for ( rp = result; rp != NULL; rp = rp->ai_next ) {
                if ( ( s = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) == -1 ) {
                        continue;
                }

                if ( connect( s, rp->ai_addr, rp->ai_addrlen ) != -1 ) {
                        break;
                }

                close( s );
        }
        if ( rp == NULL ) {
                perror( "stream-talk-client: connect" );
                return -1;
        }
        freeaddrinfo( result );

        return s;
}
