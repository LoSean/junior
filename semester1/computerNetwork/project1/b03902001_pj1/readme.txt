Platform: Linux
Language: C

How to Compile: 
	$make

How to Execute:
	$./client [-n number] [-t timeout] server1:port server2:port ...
	$./server listen_port

How to Delete the Files:
	$make clean

Client:
1.Send/Receive Messages to Server:
	Implementation: send()/recv()
	Implement at: void my_ping(int fd, int pingCount, int pingTimeout, char *address, int server_port);

2.Translate Host Name to IP:
	Implementation: gethostbyname()
	Implement at: void *client_start(void *ptr);

3.Support -n -t Command:
	Implement at: int set_parameter(char **par, int *num, int *timeout);

4.Print Output According to P.9:
	Implementation:
		(1) setsockopt() for timeout
		(2) clock_gettime()
	Implement at: void my_ping(int fd, int pingCount, int pingTimeout, char *address, int server_port);

Bonus:
5.Client Can Ping Multiple Server Simultaneously:
	Implementation: pthread

Server:
1.Send/Receive Messages from Multiple Client:
	Implementation: 
		(1) fork()
		(2) send()/recv()

2.Print Output According to P.11:
	Implementation at: void *handle_request(void *ptr);
	
