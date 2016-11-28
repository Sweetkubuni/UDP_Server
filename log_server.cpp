#include <array>
#include <iostream>
#include <fstream>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
using namespace std;

#define PORT 8767
#define BUFSIZE 512;
#define FOREVER 1
int main(int argc, char* argv[])
{
	pid_t process_id = 0;
	pid_t sid = 0;

	// Create child process
	process_id = fork();
	// Indication of fork() failure
	if (process_id < 0)
	{
		cerr << "fork failed!\n";
		// Return failure in exit status
		exit(1);
	}

	// PARENT PROCESS. Need to kill it.
	if (process_id > 0)
	{
		cout << "process_id of child process" << process_id << endl;
		// return success in exit status
		exit(0);
	}

	//unmask the file mode
	umask(0);
	//set new session
	sid = setsid();
	if (sid < 0)
	{
		// Return failure
		exit(1);
	}
	// Change the current working directory to root.
	chdir("/");
	// Close stdin. stdout and stderr
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);

	/* CHILD PROCESS PROCEDURE ONLY! */

	//logging file
	ofstream log;
	log.open("server_log.txt", ios::out | ios::app);

	struct sockaddr_in myaddr;      // our address 
	struct sockaddr_in remaddr;     // remote address
	socklen_t addrlen = sizeof(remaddr);
	int recvlen;
	int fd;                         // server socket
	array<uint8_t> buf[BUFSIZE];     // receive buffer
	

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(PORT);

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("cannot create socket");
		return 0;
	}

	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		std::time_t result = std::time(nullptr);
		log << "Error: bind failed " << "TIMESTAMP: " << std::asctime(std::localtime(&result)) << endl;
		log.close();
		return 0;
	}

	while (FOREVER) {
		recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&remaddr, &addrlen);
		if (recvlen > 0) {
			string data(buf.begin(), buf.end());
			log << inet_ntoa(remaddr.sin_addr) << " : " << data << '\n';

			if (data.compare("STOP") == 0)
				break;
		}
	}// end of FOREVER loop

	close(fd);
	log.close();
	return (0);
}