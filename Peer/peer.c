//peer.c
//Program_2 Group 17
//Aaron Robinson Almazan
//James C Oppy
/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#define BUF_SIZE 256

static void trim_newline(char *s) { //Need this because user hits enter after typing for ex. "JOIN" adds "\n" to string = "JOIN\n"
    size_t n = strlen(s);
    if (n && s[n-1] == '\n') {
        s[n-1] = '\0';
    }
}
/*                                                                                                                                                                                                                  
 * Lookup a host IP address and connect to it using service. Arguments match the first two                                                                                                                          
 * arguments to getaddrinfo(3).                                                                                                                                                                                     
 *                                                                                                                                                                                                                  
 * Returns a connected socket descriptor or -1 on error. Caller is responsible for closing                                                                                                                          
 * the returned socket.                                                                                                                                                                                            
 */
int connect_to_registry( const char *host, const char *service );

int main( int argc, char *argv[] ) {
        char *registry_host;
	char *registry_port;
	char *peer_id;
        char buf[BUF_SIZE];
        int s;
  
        if ( argc == 4 ) {
                registry_host = argv[1];
		registry_port = argv[2];
		peer_id = argv[3];
        }
        else {
                fprintf(stderr, "usage: %s <registry_host> <registry_port> <peer_id>\n", argv[0]);
                exit( 1 );
        }

        // Lookup IP and connect to server 
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
	      uint8_t msg[5];    
	      msg[0] = 0;                  

	      uint32_t peer_id_net = htonl((uint32_t)atoi(peer_id));
	      memcpy(msg + 1, &peer_id_net, 4);

	      ssize_t sent = send(s, msg, sizeof(msg), 0);
	      if (sent != sizeof(msg))
		{
		  perror("JOIN send failed");
		}
	      else
		{
		  printf("JOIN request sent.\n");
		}
            }
	  // PUBLISH
          else if(strcmp(buf, "PUBLISH") == 0)
            {
              // build PUBLISH message (scan ./SharedFiles) and send
	      DIR *dir = opendir("SharedFiles");
	      if (!dir)
		{
		  perror("opendir");
		  continue;
		}

	      uint8_t msg[1200];
	      msg[0] = 1;
	      uint32_t count = 0;
	      size_t pos = 5;

	      struct dirent *entry;
	      while((entry = readdir(dir)) != NULL)
		{
		  if(entry->d_type == DT_REG)
		    {
		      size_t len = strlen(entry->d_name) + 1;
		      if (pos + len > sizeof(msg))
			{
			  break;
			}
		      memcpy(msg + pos, entry->d_name, len);
		      pos += len;
		      count++;
		    }
		}
	      closedir(dir);
	      uint32_t count_net = htonl(count);
	      memcpy(msg +1, &count_net, 4);

	      ssize_t sent = send(s, msg, pos, 0);
	      if (sent != (ssize_t)pos)
		{
		  perror("PUBLISH send failed");
		}
	      else
		{
		  printf("PUBLISH request sent (%u files)\n", count);
		}
            }
	  // SEARCH Implementation Start 
          else if (strcmp(buf, "SEARCH") == 0)
            {
              // ask user for filename, build SEARCH request, send
              // then recv the 10-byte response and print result
	      printf("Enter a file name: ");
	      if (!fgets(buf, sizeof(buf), stdin))
		{
		  continue;
		}
	      trim_newline(buf);
	      size_t name_len = strlen(buf) + 1;
	      if (name_len > 100)
		{
		  fprintf(stderr, "filename too long\n");
		  continue;
		}
	      uint8_t msg[1 + 101];
	      msg[0] = 2;
	      memcpy(msg + 1, buf, name_len);

	      if (send(s, msg, 1 + name_len, 0) < 0)
		{
		  perror("SEARCH send failed");
		  continue;
		}
	      uint8_t resp[10];
	      ssize_t r = recv(s, resp, sizeof(resp), 0);
	      if (r != sizeof(resp))
		{
		  perror("SEARCH recv failed");
		  continue;
		}
	      uint32_t pid_net, ip_net;
	      uint16_t port_net;
	      memcpy(&pid_net, resp, 4);
	      memcpy(&ip_net, resp + 4, 4);
	      memcpy(&port_net, resp + 8, 2);
	      uint32_t pid_host = ntohl(pid_net);
	      uint32_t ip_host = ntohl(ip_net);
	      uint16_t port_host = ntohs(port_net);
	      if (pid_host == 0 && ip_host == 0 && port_host == 0)
		{
		  printf("File not indexed by registry\n");
		}
	      else
		{
		  struct in_addr addr;
		  addr.s_addr = htonl(ip_host);
		  char ipstr[INET_ADDRSTRLEN];
		  if (!inet_ntop(AF_INET, &addr, ipstr, sizeof(ipstr)))
		    {
		      strcpy(ipstr, "<?>");
		    }
		  printf("File found at\nPeer %u\n%s:%u\n", pid_host, ipstr, port_host);
		}
            } 
	  // EXIT Implementation Start
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
