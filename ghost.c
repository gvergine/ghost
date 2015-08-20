#include <th.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

// transparent:
// start ghost as root and the browser as normal user
// sudo iptables -t nat -A OUTPUT -p tcp -m owner ! --uid-owner root --dport 80 -j REDIRECT --to-port 9999

struct connection * l;

char * listen_ip;
unsigned short int listen_port;

int th_evt_init(int argc, char* argv[])
{
    printf("Init - active on %s:%d\n",listen_ip,listen_port);
    th_act_startlistening(listen_ip,listen_port);
    return 0;
}

void th_evt_fini()
{
	printf("Fini\n");
}

void th_evt_error(struct connection * c, const char* domain)
{
	printf("Error - socket %d: %s: %s\n",c->fd, domain,(char*)strerror(errno));
}

void th_evt_newconnection(struct connection * c)
{
	//printf("New connection - socket %d\n",c->fd);
	if (c->role == 0)
	{
		//printf("Socket %d is the listen socket\n",c->fd);
		l = c;
	}
	else
	{
		//printf("New connection on socket %d\n",c->fd);
		printf("src: %s:%d\n",inet_ntoa(c->src.sin_addr),ntohs(c->src.sin_port));
		printf("dst: %s:%d\n",inet_ntoa(c->dst.sin_addr),ntohs(c->dst.sin_port));
		if (c->user == 0) // is new incoming connection
		{
			th_act_createconnection(&c->dst, c);
			//printf("Is new incoming connection, i am %p and i will send to %p\n",c,c->user);
		}
		else
		{
			// is manually created
			struct connection * left = (struct connection*)c->user;
			left->user = c; // now both are bound
			//printf("Is manually created, i am %p and i will send to %p\n",c,c->user);
		}

	}
}

void th_evt_data(struct connection * c, char* buf, ssize_t len)
{
	printf("New data - socket %d (len=%d)\n",c->fd,len);
	struct connection * otherEnd = (struct connection*)c->user;
	if (otherEnd != 0)
        th_act_send(otherEnd,buf,len);
}

void th_evt_closedconnection(struct connection *c)
{
	th_act_destroyconnection(c);
	//printf("Closed connection - socket %d\n",c->fd);
}


int main(int argc, char* argv[])
{
    listen_ip = argv[1];
    listen_port = atoi(argv[2]);
    return th_service();
}