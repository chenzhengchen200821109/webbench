/*
 * Simple forking WWW Server benchmark:
 *
 * Usage:
 *   webbench --help
 *
 * Return codes:
 *    0 - sucess
 *    1 - benchmark failed (server is not on-line)
 *    2 - bad param
 *    3 - internal error, fork failed
 * 
 */ 
#include "socket.h"
#include <unistd.h>
#include <sys/param.h>
#include <rpc/types.h>
#include <getopt.h>
#include <strings.h>
#include <time.h>
#include <signal.h>

/* values */
volatile int timerexpired = 0;
int success = 0; /* the number of successful http requests */
int failed = 0;  /* the number of failures */
size_t bytes = 0;

/* http/1.0 is the default supported version */
int http10 = 1; /* 0 - http/0.9, 1 - http/1.0, 2 - http/1.1 */

/* Allow: GET, HEAD, OPTIONS, TRACE */
#define METHOD_GET 0
#define METHOD_HEAD 1
#define METHOD_OPTIONS 2
#define METHOD_TRACE 3
#define PROGRAM_VERSION "1.5"

int method = METHOD_GET;
int clients = 1;
int force = 0;
int force_reload = 0; /* no cache */
int proxyport = 80;
char *proxyhost = NULL;
int benchtime = 30;

/* internal */
int mypipe[2]; /* pipe file descriptors */
char host[MAXHOSTNAMELEN];
#define REQUEST_SIZE 2048
char request[REQUEST_SIZE];

/*
 * Below struct come from unistd.h
 *
 * struct option {
 *   const char* name;
 *   int has_arg;
 *   int* flag;
 *   int val;
 * };
 */

static const struct option long_options[]=
{
    { "force", no_argument, &force, 1 },
    { "reload", no_argument, &force_reload, 1 },
    { "time", required_argument, NULL, 't' },
    { "help", no_argument, NULL, '?' },
    { "http09", no_argument, NULL, '9' },
    { "http10", no_argument, NULL, '1' },
    { "http11", no_argument, NULL, '2' },
    { "get", no_argument, &method, METHOD_GET },
    { "head", no_argument, &method, METHOD_HEAD },
    { "options", no_argument, &method, METHOD_OPTIONS },
    { "trace", no_argument, &method, METHOD_TRACE },
    { "version", no_argument, NULL, 'V' },
    { "proxy", required_argument, NULL,'p' },
    { "clients", required_argument, NULL,'c' },
    { NULL, 0, NULL, 0 }
};

extern int Socket(const char* host, int port);

/* prototypes */
static void benchcore(const char* host, const int port, const char *request);
static int bench(void);
static void build_request(const char *url);

static void alarm_handler(int signal)
{
   timerexpired = 1;
}	

/* useage information */
static void usage(void)
{
   fprintf(stderr,
	"webbench [option]... URL\n"
	"  -f|--force               Don't wait for reply from server.\n"
	"  -r|--reload              Send reload request - Pragma: no-cache.\n"
	"  -t|--time <sec>          Run benchmark for <sec> seconds. Default 30.\n"
	"  -p|--proxy <server:port> Use proxy server for request.\n"
	"  -c|--clients <n>         Run <n> HTTP clients at once. Default one.\n"
	"  -9|--http09              Use HTTP/0.9 style requests.\n"
	"  -1|--http10              Use HTTP/1.0 protocol.\n"
	"  -2|--http11              Use HTTP/1.1 protocol.\n"
	"  --get                    Use GET request method.\n"
	"  --head                   Use HEAD request method.\n"
	"  --options                Use OPTIONS request method.\n"
	"  --trace                  Use TRACE request method.\n"
	"  -?|-h|--help             This information.\n"
	"  -V|--version             Display program version.\n"
	);
};

int main(int argc, char *argv[])
{
    int opt = 0;
    int options_index = 0;
    char *tmp = NULL;

    /* no arguments */
    if(argc == 1) {
        usage();
        return 2;
    } 

    /*
     * getopt_long() function is used to parse command-line options.
     * "912Vfrt:p:c:?h" is optstring, which is a string containing the legitimate
     * option characters. If such a character is followed by a colon, the option
     * requries an argument. getopt_long() function places a pointer to the following
     * text in the same argv-element in optarg.
     */
    while((opt = getopt_long(argc, argv, "912Vfrt:p:c:?h", long_options, &options_index)) != EOF) {
        switch(opt) {
            case  0 : break;
            case 'f': force = 1; break;
            case 'r': force_reload = 1; break; 
            case '9': http10 = 0; break;
            case '1': http10 = 1; break;
            case '2': http10 = 2; break;
            case 'V': printf(PROGRAM_VERSION"\n"); exit(0);
            case 't': benchtime = atoi(optarg); break;	     
            case 'p': 
	            /* proxy server parsing server:port */
	            tmp = strrchr(optarg, ':');
	            proxyhost = optarg;
	            if(tmp == NULL) {
                    break;
	            }
	            if(tmp == optarg) {
		            fprintf(stderr,"Error in option --proxy %s: Missing hostname.\n",optarg);
		            return 2;
	            }
	            if(tmp == optarg + strlen(optarg) - 1) {
		            fprintf(stderr,"Error in option --proxy %s Port number is missing.\n",optarg);
		            return 2;
	            }
	            *tmp = '\0'; // insert '\0' as a delimiter
	            proxyport = atoi(tmp + 1); break;
            case ':':
            case 'h':
            case '?': usage(); return 2 ; break; // not an option
            case 'c': clients = atoi(optarg); break;
        }
    }
 
    /*
     * The variable optind is the index of the next element to be processed in argv.
     * The system initializes this value to 1.
     */
    if(optind == argc) {
        fprintf(stderr,"webbench: Missing URL!\n");
		usage();
		return 2;
    }

    if(clients == 0) clients = 1;
    if(benchtime == 0) benchtime = 60;

    /* Copyright */
    fprintf(stderr,"Webbench - Simple Web Benchmark "PROGRAM_VERSION"\n");

    /* argv[optind] now points to URL */
    build_request(argv[optind]);
    /* print bench info */
    printf("\nBenchmarking: ");
    switch(method)
    {
        case METHOD_GET:
	    default:
            printf("GET"); break;
	    case METHOD_OPTIONS:
		    printf("OPTIONS"); break;
	    case METHOD_HEAD:
	        printf("HEAD"); break;
	    case METHOD_TRACE:
		    printf("TRACE"); break;
    }
    printf(" %s", argv[optind]);
    switch(http10) {
	    case 0: printf(" (using HTTP/0.9)"); break;
	    case 2: printf(" (using HTTP/1.1)"); break;
    }
    printf("\n");

    if(clients == 1)
        printf("1 client");
    else
        printf("%d clients", clients);
    printf(", running %d sec", benchtime);
    if(force) 
        printf(", early socket close");
    if(proxyhost != NULL) 
        printf(", via proxy server %s:%d", proxyhost, proxyport);
    if(force_reload) 
        printf(", forcing reload");
    printf(".\n");
    return bench();
}

/*
 * build http request message
 */
void build_request(const char* url)
{
    char tmp[10];
    int i;

    bzero(host, MAXHOSTNAMELEN);
    bzero(request, REQUEST_SIZE);

    if(force_reload && proxyhost != NULL && http10 < 1) 
        http10 = 1;
    if(method == METHOD_HEAD && http10 < 1) 
        http10 = 1;
    if(method == METHOD_OPTIONS && http10< 2) 
        http10 = 2;
    if(method == METHOD_TRACE && http10 < 2) 
        http10 = 2;

    switch(method) {
        default:
	    case METHOD_GET: 
            strcpy(request, "GET"); 
            break;
	    case METHOD_HEAD: 
            strcpy(request, "HEAD");
            break;
	    case METHOD_OPTIONS: 
            strcpy(request,"OPTIONS");
            break;
	    case METHOD_TRACE: 
            strcpy(request,"TRACE");
            break;
    }
		  
    strcat(request, " ");
    /*
     * URL syntax: <scheme>://<user>:<password>@<host>::<port>/path;<params>?<query>#<frag>.
     */
    if(NULL == strstr(url, "://")) {
	    fprintf(stderr, "\n%s: is not a valid URL.\n", url);
	    exit(2);
    }
    if(strlen(url) > 1500) {
        fprintf(stderr,"URL is too long.\n");
	    exit(2);
    }
    if(proxyhost == NULL) {
        if(0 != strncasecmp("http://", url, 7)) {
            fprintf(stderr,"\nOnly HTTP protocol is directly supported, set --proxy for others.\n");
            exit(2);
        }
        /* protocol/host delimiter */
        i = strstr(url, "://") - url + 3;
        // printf("%d\n", i);
        if(strchr(url + i,'/') == NULL) {
            fprintf(stderr,"\nInvalid URL syntax - hostname don't ends with '/'.\n");
            exit(2);
        }
    }
    if(proxyhost == NULL) {
        /* get port from hostname */
        if(index(url + i, ':') != NULL && index(url + i, ':') < index(url + i, '/')) {
            strncpy(host, url + i, strchr(url + i,':')- url - i);
	        bzero(tmp,10);
	        strncpy(tmp, index(url + i,':') + 1, strchr(url + i,'/') - index(url + i,':') - 1);
	        // printf("tmp=%s\n", tmp); 
	        proxyport = atoi(tmp);
	        if(proxyport == 0) 
                proxyport = 80;
        } else {
            strncpy(host, url + i, strcspn(url + i, "/"));
        } 
        // printf("Host = %s\n", host);
        strcat(request + strlen(request), url + i + strcspn(url + i, "/"));
    } else {
        // printf("ProxyHost = %s\nProxyPort = %d\n", proxyhost, proxyport);
        strcat(request, url);
    }
    if(http10 == 1)
	    strcat(request, " HTTP/1.0");
    else if(http10==2)
	    strcat(request, " HTTP/1.1");
    strcat(request, "\r\n");
    if(http10 > 0) /* version: http/1.0+ */
        /* User-Agent should be randomly chosen */
	    strcat(request, "User-Agent: WebBench "PROGRAM_VERSION"\r\n");
    if(proxyhost == NULL && http10 > 0) {
        strcat(request, "Host: ");
	    strcat(request, host);
	    strcat(request, "\r\n");
    }
    if(force_reload && proxyhost != NULL)
    {
        /* no cache */
        //strcat(request, "Pragma: no-cache\r\n");
        strcat(request, "Cache-Control: no-cache\r\n");
    }
    if(http10 > 1) /* version: http/1.1 */
        strcat(request, "Connection: close\r\n");
    /* add empty line at end */
    if(http10 > 0) /* version: http/1.0+ */
        strcat(request, "\r\n"); 
    // printf("Req = %s\n", request);
}

/* benchmark */
static int bench(void)
{
    int sockfd;
    int i, j, k;	
    pid_t pid = 0;
    FILE *f;

    /* check avaibility of target server */
    sockfd = Socket(proxyhost == NULL ? host : proxyhost, proxyport);
    if(sockfd < 0) {
        fprintf(stderr,"\nConnect to server failed. Aborting benchmark.\n");
        return 1;
    }
    close(sockfd);
    /* create pipe */
    if(pipe(mypipe)) {
        perror("pipe failed.");
	    return 3;
    }

    /* fork children */
    for(i = 0; i < clients; i++) { 
        pid = fork();
	    if(pid <= (pid_t)0) { /* child process or error */
		    break;
	    }
        sleep(1);
    }

    /* fork failed */
    if(pid < (pid_t)0) {
        fprintf(stderr,"problems forking worker no. %d\n", i);
	    perror("fork failed.");
	    return 3;
    }

    if(pid == (pid_t)0) { /* child process */
        if(proxyhost == NULL)
            benchcore(host, proxyport, request);
        else
            benchcore(proxyhost, proxyport, request);
        /* write results to pipe */
	    f = fdopen(mypipe[1], "w");
	    if(f == NULL) {
            perror("open pipe for writing failed.");
		    return 3;
	    }
	    // fprintf(stderr, "Child - %d %d\n", speed, failed);
	    fprintf(f, "%d %d %d\n", success, failed, bytes);
	    fclose(f);
	    return 0;
    } else { /* parent process */
	  f = fdopen(mypipe[0], "r");
	  if(f == NULL) {
          perror("open pipe for reading failed.");
		  return 3;
	  }
	  setvbuf(f, NULL, _IONBF, 0);
	  success = 0;
      failed = 0;
      bytes = 0;

	  while(1) { /* read from all clients */
          pid = fscanf(f, "%d %d %d", &i, &j, &k);
		  if(pid < 2) {
              fprintf(stderr,"Some of our childrens died.\n");
              break;
          }
		  success += i;
		  failed += j;
		  bytes += k;
		  // fprintf(stderr, "*Knock* %d %d read=%d\n", speed, failed, pid);
		  if(--clients == 0) 
              break;
	  }
	  fclose(f);

      printf("\nSpeed=%d pages/min, %d bytes/sec.\nRequests: %d succeed, %d failed.\n", (int)((success + failed)/(benchtime/60.0f)),
		     (int)(bytes/(float)benchtime),
		     success,
		     failed);
    }
    return i;
}

void benchcore(const char *host,const int port,const char *req)
{
    size_t rlen; // request length
    char buf[1500];
    int sockfd;
    size_t i;
    struct sigaction sa;

    /* setup alarm signal handler */
    sa.sa_handler = alarm_handler;
    sa.sa_flags = 0;
    /* sigaction() returns 0 on success; On error, -1 is returned */
    if(sigaction(SIGALRM, &sa, NULL))
        exit(3);
    alarm(benchtime);

    rlen = strlen(req);
nexttry:
    while(1) {
        if(timerexpired) {
            if(failed > 0) { /* some system call might be interrupted by ALARM signal */
                /* fprintf(stderr,"Correcting failed by signal\n"); */
                failed--;
            }
            /* benchcore() returned from here */
            return;
        }
        /* Tcp connection has established */
        sockfd = Socket(host, port);                          
        if(sockfd < 0) { 
            failed++; 
            continue; 
        } 
        if(rlen != write(sockfd, req, rlen)) {
            failed++;
            close(sockfd);
            continue;
        }
        /* version http/0.9 */
        if(http10 == 0) { 
            /* further transmissions will be disallowed */
	        if(shutdown(sockfd, SHUT_WR)) { 
                failed++; 
                close(sockfd);
                continue;
            }
        }
        /* don't wait for reply from server if force equals 1 */
        if(force == 0) {
            /* read all available data from socket */
            while(1) {
                if(timerexpired) 
                    break; 
	            i = read(sockfd, buf, 1500);
                /* fprintf(stderr,"%d\n",i); */
	            if(i < 0) { 
                    failed++;
                    close(sockfd);
                    goto nexttry;
                } else if(i == 0) 
                    break;
		       else
			       bytes += i;
            }
        }
        // close() returns zero on success. On error, -1 is returned
        if(close(sockfd)) {
            failed++;
            continue;
        }
        success++;
    }
}
