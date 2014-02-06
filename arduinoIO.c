static char *id __attribute__ ((__unused__)) 
  = "@(#) centeotl:/home/morcha/src/MATLAB/Arduino/testVariables.c;"
    "Friday January 31, 2014 (09:56);"
    "Mark Orchard-Webb;"
    "Compiled: "__DATE__ " (" __TIME__ ")" ;

/*
 * Friday January 31, 2014 (09:56)
 * ===============================
 * 
 * check if variables in MATLAB are conserved between calls
 * 
 */

#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <termios.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include "mex.h"

double value = 0;
pid_t matlab_pid, server_pid = 0;
int connected = 0;
int debug = 1;

struct data {
  int toServer, fromServer, arduino, maxfd;
  pid_t server_pid;
};

const char *debug_fd (const char *str, const int fd)
{
  int rc;
  rc = fcntl(fd,F_GETFL);
  if (rc < 0) { fprintf(stderr,"%s", str); perror("perror(F_GETFL):"); return("fcntl(F_GETFL) failed"); }
  fprintf(stderr,"%s File descriptor flags: %x", str, rc);
#define FOO(X) if (rc & X) fprintf(stderr, " " #X)
  FOO(O_RDONLY);
  FOO(O_WRONLY);
  FOO(O_RDWR);
  fprintf(stderr,"\n");
  rc = fcntl(fd,F_GETPIPE_SZ);
  if (rc < 0) { fprintf(stderr,"%s", str); perror("perror(F_GETPIPE_SZ):"); return("fcntl(F_GETPIPE_SZ) failed"); }
  fprintf(stderr,"%s fcntl(F_GETPIPE_SZ) = %d\n", str, rc);
  return NULL;
}

void sig_handler (int arg) {
  fprintf(stderr,"sig_handler: arg = %d\n", arg);
}

int openArduino(void) {
  struct termios termios;
  char buf[64];
  int i, fd, rc;
  for (i = 0; i < 10; i++) {
    sprintf(buf, "/dev/ttyACM%d",i);
    fd = open(buf,O_RDWR);
    if (fd < 0) continue;
    if(debug)fprintf(stderr,"found Arduino at %s\n", buf);
    break;
  }
  if (fd < 0) mexErrMsgTxt("Unable to find Arduino, is it connected to USB?");
  rc = tcgetattr(fd,&termios);
  cfsetispeed(&termios,B115200);
  cfsetospeed(&termios,B115200);
  tcsetattr(fd,TCSANOW, &termios);
  return fd;
}

short int getShortInt (const char *bigBuf) {
  char buf[5];
  short int si;
  int rc;
  strncpy(buf,bigBuf,4);
  buf[4] = 0;
  if (sscanf(buf,"%x",&si) != 1) { fprintf(stderr,"getShortInt() failed\n"); exit(1); }
  return si;
}

void processComment(char *bigBuf,int *bufLen, int *mode) {
  char *ptr, *ptr2;
  int count;
  for (ptr = bigBuf; *ptr; ptr++) if (*ptr == '\n') break;
  if (!ptr) return;
  for (ptr = bigBuf; *ptr; ptr++) if (*ptr != '\n') break;
  if (strlen(ptr) < 5) return;
  if (getShortInt(ptr) != 0) return;
  ptr2 = strchr(ptr+4,'\n');
  if (!ptr2) return;
  *ptr2 = 0;
  fprintf(stderr,"arduinoIO debug: %s\n", ptr+4);
  *ptr2 = '\n';
  count = strlen(ptr2);
  memmove(bigBuf, ptr2, count);
  bigBuf[*bufLen = count] = 0;
  *mode = 1;
}

void processCommand(char *bigBuf, int *bufLen, int fromServer) {
  char *ptr, *ptr2;
  int count;
  for (ptr = bigBuf; *ptr; ptr++) if (*ptr == '\n') break;
  if (!ptr) return;
  for (ptr = bigBuf; *ptr; ptr++) if (*ptr != '\n') break;
  if (strlen(ptr) < 8) return;
  if (getShortInt(ptr) == 0) return;
  count = getShortInt(ptr+4);
  if (strlen(ptr) < 4*(count+2)) return;
  count = write(fromServer,ptr,4*(count+2));
  fprintf(stderr, "wrote %d bytes fromServer\n", count);
  *bufLen = strlen(ptr2);
  memmove(bigBuf,ptr2,*bufLen);
  bigBuf[*bufLen] = 0;
}

int getSize(int value) {
  if ((value & 0x7f) == value) return 2;
  if ((value & 0x3fff) == value) return 4;
  if ((value & 0x1fffff) == value) return 6;
  if ((value & 0xfffffff) == value) return 8;
  fprintf(stderr,"getSize(value=%d) too large", value);
  exit(1);
}

int encode_int (char *buf, int value) {
  switch (getSize(value)) {
  case 2:
    sprintf(buf,"%02x",value);
    return 2;
  case 4:
    sprintf(buf,"%02x%02x",0x80 | (value >> 7), value & 0x7f);
    return 4;
  case 6:
    sprintf(buf,"%02x%02x%02x",0x80 | (value >> 14), 0x80 | ((value >> 7) & 0x7f), value & 0x7f);
    return 6;
  case 8:
    sprintf(buf,"%02x%02x%02x%02x",0x80 | (value >> 21), 0x80 | ((value >> 14) & 0x7f), 0x80 | ((value >> 7) & 0x7f), value & 0x7f);
    return 8;
  default:
    fprintf(stderr,"Can not get here can we!");
  }
}

int send_command (struct data *data, int command, int count, int *args) {
  char *buf, *ptr;
  const char *errStr;
  int rc, i, size = getSize(command);
  if (debug) fprintf(stderr,"send_command()\n");
  if (debug) fprintf(stderr,"send_command() toServer = %d, fromServer = %d\n", data->toServer, data->fromServer);
  if(debug)debug_fd("MATLAB:send_command():toServer:",data->toServer);
  size += getSize(count);
  for (i = 0; i < count; i++) size += getSize(args[i]);
  buf = malloc(size+1);
  if (!buf) { fprintf(stderr,"MATLAB:send_command(): malloc(%d) failed\n", size+1); exit(1); }
  if(debug)fprintf(stderr,"MATLAB:send_command(): malloc'd %d bytes\n",size+1);
  ptr = buf;
  ptr += encode_int(ptr, command);
  ptr += encode_int(ptr, count);
  for (i = 0; i < count; i++) ptr += encode_int(ptr, args[i]);
  if(debug)fprintf(stderr,"MATLAB:send_command(): buf = \"%s\"\n",buf);
  if(debug)debug_fd("MATLAB:send_command():toServer:",data->toServer);
  rc = write(data->toServer,&size,sizeof(int));
  if (rc != sizeof(int)) { fprintf(stderr,"write(toServer,&size,sizeof(int))\n"); exit(1); }
  if(debug)fprintf(stderr,"wrote %d toServer\n", size);
  rc = write(data->toServer,buf,size);
  if (rc != size) { fprintf(stderr,"write(toServer,buf,size)\n"); exit(1); }
  if(debug)fprintf(stderr,"wrote %d bytes toServer (\"%s\")\n", size, buf);
  if (debug) fprintf(stderr,"send_command() returns\n");
  return 0;
}

int getHexByte (const char *bigBuf, int *byte) {
  char buf[3];
  if(debug)fprintf(stderr,"Server:getHexByte() entered\n");
  strncpy(buf,bigBuf,3);
  buf[2] = 0;
  if (sscanf(buf,"%x",byte) != 1) { if(debug)fprintf(stderr,"sscanf(\"%s\") failed\n", buf); return -1; }
  if(debug)fprintf(stderr,"Server:getHexByte() buf = \"%s\", byte = %d\n", buf, *byte);
  return 0;
}

int getNumber(char *buf, int *number) {
  int byte, count = 0;
  *number = 0;
  do {
    if (strlen(buf+count) < 2) { if(debug)fprintf(stderr,"Server:getNumber(): short read, at offset %d, buf =\"%s\"\n", count, buf); return 0; }
    if (getHexByte(buf+count, &byte)) return -1;
    *number <<= 7;
    *number |= (byte & 0x7f);
    count+=2;
  } while (byte & 0x80);
  if(debug)fprintf(stderr,"Server:getNumber() read %d (%d bytes)\n", *number, count);
  return count;
}

int recv_command (struct data *data, int *command, int *count, int **args) {
  int rc, size, i;
  char buf[65536], *ptr;
  rc = read(data->fromServer,&size,sizeof(int));
  if (rc != sizeof(int)) { fprintf(stderr,"MATLAB:recv_command(): read(data->fromServer,&size,sizeof(int)) returned %d\n", rc); exit(1); }
  rc = read(data->fromServer,buf,size);
  if (rc != size) { fprintf(stderr,"MATLAB:recv_command(): read(data->fromServer,buf,%d) returned %d\n", size, rc); exit(1); }
  ptr = buf;
  rc = getNumber(ptr,command);
  if (!(rc>0)) { fprintf(stderr,"MATLAB:recv_command(): getNumber(ptr,command) failed\n"); exit(1); }
  ptr += rc;
  rc = getNumber(ptr,count);
  if (!(rc>0)) { fprintf(stderr,"MATLAB:recv_command(): getNumber(ptr,count) failed\n"); exit(1); }
  ptr += rc;
  if (count) {
    *args = malloc(*count*sizeof(int));
    if (!*args) { fprintf(stderr,"MATLAB:recv_command(): malloc(%d*sizeof(int)) failed\n",*count); exit(1); }
    for (i = 0; i < *count; i++) {
      rc = getNumber(ptr,(*args)+i);
      if (!(rc>0)) { fprintf(stderr,"MATLAB:recv_command(): getNumber(ptr,(*args)+i) failed\n"); exit(1); }
      ptr += rc;
    }
  } else *args = NULL;
  return 0;
}

int process_buffer(char *buf, int *len, int fromServer) {
  char *firstNewline, *secondNewline, *afterNewline, *afterCommand;
  int rc, command, count, i;
  if(debug)fprintf(stderr,"Server:process_buffer(\"%s\", %d, %d) entered\n",buf,*len,fromServer);
  firstNewline = strchr(buf,'\n');
  if (!firstNewline) return 0;
  afterNewline = firstNewline + 1;
  while (*afterNewline == '\n') afterNewline++;
  rc = getNumber(afterNewline,&command);
  if (rc < 0) return 0;
  if (rc == 0) return 0;
  afterCommand = afterNewline + rc;
  if (command == 0) {
    secondNewline = strchr(afterNewline+rc,'\n');
    if (!secondNewline) return 0;
    *secondNewline = 0;
    fprintf(stderr,"Arduino debugging statement: %s\n", afterCommand);
    *secondNewline = '\n';
  } else {
    char *nextNumber;
    rc = getNumber(afterCommand,&count);
    if (rc < 0) return 0;
    if (rc == 0) return 0;
    nextNumber = afterCommand + rc;
    for (i = 0; i < count; i++) {
      rc = getNumber(nextNumber,&command); /* burn the command, just a dummy variable */
      if (rc < 0) return 0;
      if (rc == 0) return 0;
      nextNumber += rc;
    }
    count = nextNumber - afterNewline;
    rc = write(fromServer,&count,sizeof(int));
    rc = write(fromServer,afterNewline,count);    
    secondNewline = nextNumber;
  }
  *len = strlen(secondNewline);
  if (*len) memmove(buf, secondNewline, *len);
  buf[*len] = 0;
  return 1;
}

void writeArduino (char *buf, int size, int fd) {
  int rc, i;
  char tbuf[2];
  time_t start;
  if(debug)fprintf(stderr,"Server: attempt to write %d bytes to arduino (\"%s\")\n", size, buf);
  rc = write(fd,"\n",1);
  if (rc != 1) { fprintf(stderr,"write(arduino,\"\n\",1) failed\n"); exit(1); }
  for (i = 0; i < size; i++) {
    rc = write(fd,buf+i,1);
    if (rc != 1) { fprintf(stderr,"write(arduino,\"%s\",%d) failed at %d\n",buf,size,i); exit(1); }
    time(&start);
    start+=2;
    while(read(fd,tbuf,1) != 1) {
      if (time(NULL) > start) {
	fprintf(stderr,"read(arduino,tbuf,1) timeout at %d\n",i);
	write(fd,"X",1);
	return;
      }
    }
    if (tbuf[0] != buf[i]) {
      fprintf(stderr,"character mismatch at %d, sent '%c', got '%c'\n", i, buf[i], tbuf[0]);
      write(fd,"X",1);
      return;
    }
  }
  if(debug)fprintf(stderr,"Server: wrote %d bytes to arduino (\"%s\")\n", size, buf);
}

int server (struct data *data) {
  int rc;
  char *bigBuf;
  int bufLen = 0;
  bigBuf = malloc(65536);
  int mode = 0;
  if(debug)fprintf(stderr,"Server: toServer = %d, fromServer = %d\n",data->toServer,data->fromServer);
  for (;;) {
    fd_set read_fds[data->maxfd], write_fds[data->maxfd], except_fds[data->maxfd];
    struct timeval timeout = { 0, 100000 }; /* 100 ms */
    FD_ZERO(read_fds);
    FD_ZERO(write_fds);
    FD_ZERO(except_fds);
    FD_SET(data->arduino, read_fds);
    FD_SET(data->toServer, read_fds);
    FD_SET(data->arduino, except_fds);
    FD_SET(data->toServer, except_fds);
    FD_SET(data->fromServer, except_fds);
    rc = select(data->maxfd,read_fds,write_fds,except_fds, &timeout);
    if (debug && rc) fprintf(stderr,"select returns %d\n", rc);
    if (FD_ISSET(data->arduino,read_fds)) {
      if(debug)fprintf(stderr,"Arduino is ready to read\n");
      rc = read(data->arduino,bigBuf + bufLen,65536 - bufLen);
      if (rc < 0) {
	perror("read(arduino) failed:");
	exit(1);
      }
      if (debug && rc) fprintf(stderr, "Server: read %d bytes from arduino\n", rc);
      bufLen += rc;
      bigBuf[bufLen] = 0;
      while (process_buffer(bigBuf,&bufLen,data->fromServer)) mode = 1;
      if(debug)fprintf(stderr,"Server:after process_buffer()\n");
    }
    if (mode && FD_ISSET(data->toServer,read_fds)) {
      char buf[65536];
      int rc;
      int size;
      if(debug)fprintf(stderr,"toServer is ready to read\n");
      rc = read(data->toServer,&size,sizeof(int));
      if (rc != sizeof(int)) { fprintf(stderr,"read(data->toServer,&size,sizeof(int)) = %d",rc); exit(1); }
      rc = read(data->toServer,buf,size);
      if (rc != size) { fprintf(stderr,"read(data->toServer,buf,size) = %d",rc); exit(1); }
      buf[rc] = 0;
      if(debug)fprintf(stderr,"Server: read %d bytes toServer: (\"%s\")\n",rc, buf);
      if (!strncmp(buf,"00",2)) { /* special command */
	if(debug)fprintf(stderr,"Server: special command \"%s\"\n", buf);
	write(data->fromServer,&size,sizeof(int));
	write(data->fromServer,buf,size);
	if (!strcmp(buf,"000101")) { /* close server */
	  sleep(1);
	  if(debug)fprintf(stderr,"Server: closing\n");
	  exit(0);
	}
      } else {
	writeArduino(buf,size,data->arduino);
      }
    }
    if (FD_ISSET(data->arduino,except_fds)) fprintf(stderr,"Arduino is exceptional\n");
    if (FD_ISSET(data->toServer,except_fds)) fprintf(stderr,"toServer is exceptional\n");
    if (FD_ISSET(data->fromServer,except_fds)) fprintf(stderr,"fromServer is exceptional\n");
  }
}

struct data *data = NULL;

void mexFunction( int nlhs, mxArray **plhs, int nrhs, const mxArray **prhs)     
{ 
  int rc, i, rows, cols;
  int command, count, *args;
  const char *errStr;
  if (nrhs != 1) mexErrMsgTxt("Usage: arduino ( vector ).");
  if (nlhs > 2) mexErrMsgTxt("Returns one vector"); 
  rows = mxGetM(prhs[0]);
  cols = mxGetN(prhs[0]);
  if (rows != 1) mexErrMsgTxt("There must be a single row");
  if (debug && 1) fprintf(stderr,"arduinoIO:mexFunction: rows = %d, cols = %d\n", rows, cols);
  if (cols < 2) mexErrMsgTxt("vector must have at least two columns, command and count");
  command = mxGetPr(prhs[0])[0];
  count = mxGetPr(prhs[0])[1];
  if (cols != (count + 2)) mexErrMsgTxt("mismatch between count and number of arguments");
  if (count) {
    args = malloc(count*sizeof(int));
    if (!args) mexErrMsgTxt("malloc of args failed!");
    for (i = 0; i < count; i++) args[i] = mxGetPr(prhs[0])[2+i];
  }

  if ((command == 0) && (count == 1) && (args[0] == 0)) { /* special case for arduino_connect */
    int fromfd[2],tofd[2], rc;
    if (connected) mexErrMsgTxt("already connected to arduino, call arduino_disconnect() before calling arduino_connect()");
    data = malloc(sizeof(struct data));
    if (!data) mexErrMsgTxt("count not malloc storage for connection data");
    data->arduino = openArduino();
    rc = pipe(fromfd);
    if (rc < 0) { perror("perror (from):"); mexErrMsgTxt("pipe failed"); }
    rc = pipe(tofd);
    if (rc < 0) { perror("perror (to):"); mexErrMsgTxt("pipe failed"); }
    if(debug)fprintf(stderr,"tofd = [%d,%d], fromfd = [%d,%d]\n",tofd[0],tofd[1],fromfd[0],fromfd[1]);
    data->server_pid = fork();
    switch (data->server_pid) {
    case -1:
      mexErrMsgTxt("fork failed");
      return;
    case 0:
      data->toServer = tofd[0];
      data->fromServer = fromfd[1];
      /*
      if ((errStr = debug_fd("Server: arduino:",data->arduino))) { fprintf(stderr,"Server: arduino: %s", errStr); exit(1); }
      if ((errStr = debug_fd("Server: toServer:",data->toServer))) { fprintf(stderr,"Server: toServer: %s", errStr); exit(1); }
      if ((errStr = debug_fd("Server: fromServer:",data->fromServer))) { fprintf(stderr,"Server: fromServer: %s", errStr); exit(1); }
      */
      data->maxfd = data->arduino;
      if (data->toServer > data->maxfd) data->maxfd = data->toServer;
      if (data->fromServer > data->maxfd) data->maxfd = data->fromServer;
      data->maxfd++;
      rc = write(data->fromServer,"\n",1);
      if(debug)fprintf(stderr,"server wrote %d bytes to fromServer\n");
      server(data);
      return;
    default:
      close(data->arduino);
      data->toServer = tofd[1];
      data->fromServer = fromfd[0];
      /*
      if ((errStr = debug_fd("Server: toServer:",data->toServer))) { fprintf(stderr,"MATLAB: toServer: %s", errStr); exit(1); }
      if ((errStr = debug_fd("Server: fromServer:",data->fromServer))) { fprintf(stderr,"MATLAB: fromServer: %s", errStr); exit(1); }
      */
      data->maxfd = data->toServer;
      if (data->fromServer > data->maxfd) data->maxfd = data->fromServer;
      data->maxfd++;
      if(debug)fprintf(stderr,"Waiting for server\n");
      do {
	char buf[1];
        rc = read(data->fromServer,buf,1);
	if(debug)fprintf(stderr,".",rc);
	if(debug)fflush(stderr);
      } while (rc != 1);
      if(debug)fprintf(stderr,"\nserver established\n");
      connected = 1;
    }
  }

  if (!connected) mexErrMsgTxt("arduino is not connected, call arduino_connect() before attempting to use other functions");

  send_command (data, command, count, args);
  free (args);
  rc = recv_command (data, &command, &count, &args);
  if (rc) mexErrMsgTxt("recv_command failed");
  plhs[0] = mxCreateDoubleMatrix(1, count+2, mxREAL);
  mxGetPr(plhs[0])[0] = command;
  mxGetPr(plhs[0])[1] = count;
  for (i = 0; i < count; i++) mxGetPr(plhs[0])[2+i] = args[i];
  if ((command == 0) && (count == 1) && (args[0] == 1)) connected = 0;
  if (count) free(args);
}

