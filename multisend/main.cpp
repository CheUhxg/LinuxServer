#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUFFER_SIZE 1024

static const char* status_line[2] = {
        "200 OK",
        "500 Internal server error"
};

void server(const char* ip, const short port, const char* filename);

int main(int argc, char* argv[]) {
  if(argc != 4) {
    printf("Usage: %s <ip> <port> <filename>\n", basename(argv[0]));
    return 1;
  }
  server(argv[1], atoi(argv[2]), argv[3]);
  return 0;
}

void server(const char* ip, const short port, const char* filename) {
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  inet_pton(AF_INET, ip, &addr.sin_addr);
  addr.sin_port = htons(port);

  int sock = socket(PF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (sockaddr *) &addr, sizeof(addr));
  assert(ret >= 0);

  ret = listen(sock, 5);
  assert(ret >= 0);

  sockaddr_in cli_addr;
  socklen_t cli_addr_len = sizeof(cli_addr);
  int conn = accept(sock, (sockaddr*)&cli_addr, &cli_addr_len);
  assert(conn >= 0);

  char* filebuf = nullptr;
  bool is_filevalid = false;

  struct stat filestat;
  if(stat(filename, &filestat) < 0 ||
     S_ISDIR(filestat.st_mode)) {
    printf("Invalid file name\n");
  } else if(filestat.st_mode & S_IROTH) {
    int fd = open(filename, O_RDONLY);
    filebuf = new char[filestat.st_size + 1];
    bzero(filebuf, sizeof(filebuf));
    if(read(fd, filebuf, filestat.st_size) < 0) {
      printf("Read file failed\n");
    } else {
      is_filevalid = true;
    }
  }

  char headerbuf[BUFFER_SIZE] = {0};
  int len = 0;
  if(is_filevalid) {
    ret = snprintf(headerbuf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[0]);
    len += ret;
    ret = snprintf(headerbuf+len, BUFFER_SIZE - 1 - len, "Content-Length:%ld\r\n", filestat.st_size);
    len += ret;
    ret = snprintf(headerbuf+len, BUFFER_SIZE - 1 - len, "%s", "\r\n");

    iovec msg_iov[2];
    msg_iov[0].iov_base = headerbuf;
    msg_iov[0].iov_len = strlen(headerbuf);
    msg_iov[1].iov_base = filebuf;
    msg_iov[1].iov_len = strlen(filebuf);

    ret = writev(conn, msg_iov, 2);
  } else {
    ret = snprintf(headerbuf, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", status_line[1]);
    len += ret;
    ret = snprintf(headerbuf+len, BUFFER_SIZE - 1 - len, "%s", "\r\n");

    send(conn, headerbuf, strlen(headerbuf), 0);
  }
  close(conn);
  close(sock);
  delete[] filebuf;
}