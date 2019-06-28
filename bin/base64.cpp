#include <stddef.h>	// size_t
#include <stdio.h>	// fopen()
#include <string.h>	// strlen()
#include <getopt.h>
#include "../include/libbase64.h"
#include <chrono>
using namespace std::chrono;
#define BUFSIZE 100 * 1024 * 1024

static char buf[BUFSIZE];
static char out[(BUFSIZE * 5) / 3];	// Technically 4/3 of input, but take some margin
size_t nread;
size_t nout;

int readFile(FILE *fp, char* buf, size_t *nread){
	int ret = 1;
	int tempLen = 0;
	const int tempBufLen = 1024 * 1024;
	char tempBuf[1024 * 1024] = {'\0'};
	while ((tempLen = fread(tempBuf, 1, tempBufLen, fp)) > 0) {
		if(*nread >= BUFSIZE){
			printf("too big file size\n");
			ret = 0;
			goto out;
		}
		memcpy(buf + *nread, tempBuf, tempLen);
		*nread += tempLen;
		memset(tempBuf, 0, tempBufLen);
		if (feof(fp)) {
			break;
		}
	}

	if (ferror(fp)) {
		fprintf(stderr, "read error\n");
		ret = 0;
		goto out;
	}
out:	fclose(fp);
	return ret;
}

static int
enc (FILE *fp)
{
	int ret = 1;
	ret = readFile(fp, buf, &nread);
	//printf("nout = %d, nread = %d, ret = %d\n", nout, nread, ret);
	if(ret > 0){
		steady_clock::time_point t1 = steady_clock::now();
		base64_encode(buf, nread, out, &nout, 0);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		printf("It took me %lf seconds\n", time_span.count());
	}

	//if (nout) {
		//fwrite(out, nout, 1, stdout);
	//}
	fclose(stdout);
	return ret;
}

static int
dec (FILE *fp)
{
	int ret = 1;
	ret = readFile(fp, buf, &nread);
	if(ret > 0){
		steady_clock::time_point t1 = steady_clock::now();
		ret = base64_decode(buf, nread, out, &nout, 0);
		steady_clock::time_point t2 = steady_clock::now();
		duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		printf("It took me %lf\n", time_span.count());
		if (!ret) {
			fprintf(stderr, "decoding error\n");
			ret = 0;
			goto out;
		}
	}

	//if (nout) {
		//fwrite(out, nout, 1, stdout);
	//}
out:
	fclose(stdout);
	return ret;
}

int
main (int argc, char **argv)
{
	char *file;
	FILE *fp;
	int decode = 0;

	// Parse options:
	for (;;)
	{
		int c;
		int opt_index = 0;
		static struct option opt_long[] = {
			{ "decode", 0, 0, 'd' },
			{ 0, 0, 0, 0 }
		};
		if ((c = getopt_long(argc, argv, "d", opt_long, &opt_index)) == -1) {
			break;
		}
		switch (c)
		{
			case 'd':
				decode = 1;
				break;
		}
	}

	// No options left on command line? Read from stdin:
	if (optind >= argc) {
		fp = stdin;
	}

	// One option left on command line? Treat it as a file:
	else if (optind + 1 == argc) {
		file = argv[optind];
		if (strcmp(file, "-") == 0) {
			fp = stdin;
		}
		else if ((fp = fopen(file, "rb")) == NULL) {
			printf("cannot open %s\n", file);
			return 1;
		}
	}

	// More than one option left on command line? Syntax error:
	else {
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	// Invert return codes to create shell return code:
	return (decode) ? !dec(fp) : !enc(fp);
}
