#ifndef  __MAIN_H
#define  __MAIN_H



#define prompt(argv) \
	fprintf(stderr,"usage: %s [--daemon] --<major/trans> <options>\n" , argv[0])

#define major_prompt(argv) \
	fprintf(stderr , "usage: %s --major <-d domain_id> <-r ring_id> <-v vid1[,vid2]> <-m mainport_number> <-s secondport_number>\n" , argv[-1])

#define transport_prompt(argv) \
	fprintf(stderr , "usage: %s --transport <-d domain_id> <-r ring_id> <-v vid1[,vid2]> <-m mainport_number> <-s secondport_number>\n" , argv[-1])


extern char * optarg;
int getopt(int argc, char * const argv[],const char *optstring);







#endif

