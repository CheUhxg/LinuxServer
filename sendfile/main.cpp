#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/sendfile.h>

int main(int argc, char* argv[]) {
  if(argc <= 3) {
    printf("usage: %s <ip> <port> <filename>\n", basename(argv[0]));
    return 1;
  }
  const char* ip = argv[1];
  int port = atoi(argv[2]);
  const char* file_name = argv[3];

  int fd = open(file_name, O_RDONLY);
  assert(fd > 0);

  struct stat stat_buf;
  fstat(fd, &stat_buf);

  sockaddr_in addr = {0};
  inet_pton(AF_INET, ip, &addr.sin_addr);
  addr.sin_port = htons(port);
  addr.sin_family = AF_INET;

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  assert(sock >= 0);

  int ret = bind(sock, (sockaddr*)&addr, sizeof(addr));
  assert(ret >= 0);

  ret = listen(sock, 5);
  assert(ret >= 0);

  sockaddr_in cli_addr = {0};
  socklen_t cli_addr_len = sizeof(cli_addr);
  int conn = accept(sock, (sockaddr*)&addr, &cli_addr_len);
  assert(conn >= 0);

  ssize_t send_len = sendfile(conn, fd, NULL, stat_buf.st_size);
  printf("File size: %ldB\nSend size: %ldB\nAlready send %.3f\n",
         stat_buf.st_size, send_len, send_len * 100.0 / stat_buf.st_size);
  close(conn);
  close(sock);
  return 0;
}
