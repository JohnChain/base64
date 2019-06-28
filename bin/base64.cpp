#include <stddef.h>	// size_t
#include <stdio.h>	// fopen()
#include <string.h>	// strlen()
#include <getopt.h>
#include "../include/libbase64.h"
#include <chrono>
#include <dirent.h>
#include <vector>		//vector
#include <string>
#include <algorithm>	//sort
#include "base64_find.h"
using namespace std;
using namespace std::chrono;

#define BUFSIZE 100 * 1024 * 1024
int decode = 0;
int baseEngin = 1;
int printTime = 0;
std::string path = "";
std::string keyword = "";
static char buf[BUFSIZE];
static char out[(BUFSIZE * 5) / 3];	// Technically 4/3 of input, but take some margin
size_t nread;
size_t nout;
int (*decodeFun) (const char *, size_t, char*, size_t*, int);
void (*encodeFun) (const char *, size_t, char*, size_t*, int);
#define GLOG_INFO printf

struct readData{
	char sourceName[256];
	char * data;
	int size;
};

static const struct option long_option[] = {
	{"decode", required_argument, NULL, 'd'},
	{"encode", required_argument, NULL, 'e'},
	{"path", required_argument, NULL, 'p'},
	{"keyword", required_argument, NULL, 'k'},
	{"time", required_argument, NULL, 't'},
	{"baseengin", required_argument, NULL, 'b'},
	{"help", required_argument, NULL, 'h'},
	{"version", required_argument, NULL, 'v'},
	{NULL, 0, NULL, 0},
};

void showUsage(){
	GLOG_INFO("[USAGE]: ./base64 -d/-e -p dir_path_to_files -k file_keyword\n");
}

int getFileList(string &basePath, string &keyword, vector<string> &fileList)
{
	DIR *dir;
	struct dirent *ptr;
	if ((dir=opendir(basePath.c_str())) == NULL){
		GLOG_INFO("Open %s error... %s[%d]\n", basePath.c_str(), strerror(errno), errno);
		return -1;
	}

	while ((ptr=readdir(dir)) != NULL){
		string fileName = basePath;
		if(strcmp(ptr->d_name,".")==0 || strcmp(ptr->d_name,"..")==0)
			continue;
		else if(ptr->d_type == 8){
			fileName = fileName + "/" + ptr->d_name;
			if(fileName.find(keyword) != string::npos){
				fileList.push_back(fileName);
			}
		}else if(ptr->d_type == 10){
			//Link file
		}else if(ptr->d_type == 4){
			//Dir file
		}
	}
	closedir(dir);
	sort(fileList.begin(), fileList.end(), less<string>());
	int pos = fileList.size();
	for(int i = 0; i < pos; i++){
		//GLOG_INFO("[%d] %s\n", i, fileList[i].c_str());
	}

	return 0;
}

int readFile(const char *file, char* buf, size_t *nread)
{
	int ret = 1;
	int tempLen = 0;
	const int tempBufLen = 1024 * 1024;
	char tempBuf[1024 * 1024] = {'\0'};
	FILE *fp = fopen(file, "rb");
	while ((tempLen = fread(tempBuf, 1, tempBufLen, fp)) > 0) {
		if(*nread >= BUFSIZE){
			GLOG_INFO("too big file size\n");
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
enc (vector <struct readData*> &readList)
{
	int ret = 1;
	steady_clock::time_point t1 = steady_clock::now();
	int size = readList.size();
	for(int i = 0; i < size; i++)
    {
		nout = 0;
		memset(out, 0, (BUFSIZE * 5) / 3);
		(*encodeFun)(readList[i]->data, readList[i]->size, out, &nout, 0);

		if (nout) {
			fwrite(out, nout, 1, stdout);
			if(size > 1) GLOG_INFO("#########\n");
		}else{
			GLOG_INFO("encode failed\n");
		}
	}
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	if(printTime){
		GLOG_INFO("It took me %lf\n", time_span.count());
	}
	fclose(stdout);
	return ret;
}

static int
dec (vector <struct readData*> &readList)
{
	int ret = 1;
	int size = readList.size();
	steady_clock::time_point t1 = steady_clock::now();
	for(int i = 0; i < size; i++)
    {
		memset(out, 0, (BUFSIZE * 5) / 3);
		ret = (*decodeFun)(readList[i]->data, readList[i]->size, out, &nout, 0);
		if(printTime) printf("decode OK: %s\n", readList[i]->sourceName);
    }
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	if (!ret || !nout)
	{
		fprintf(stderr, "decoding error\n");
	}
	if(printTime)
	{
		GLOG_INFO("It took me %lf\n", time_span.count());
	}
	else
	{
		if (nout)
		{
			fwrite(out, nout, 1, stdout);
		}
		else
		{
			fprintf(stderr, "nout error\n");
		}
	}

	fclose(stdout);
	return ret;
}

int parseArgs(int argc, char *argv[])
{
	int opt=0;
	int ret = 0;
	while((opt=getopt_long(argc,argv,"dep:k:b:thv",long_option,NULL))!=-1){
		switch(opt){
			case 'd':
				decode = 1;
				ret = 1;
				break;
			case 'e':
				decode = 0;
				ret = 1;
				break;
			case 'p':
				path = optarg;
				ret = 1;
				break;
			case 'k':
				keyword = optarg;
				ret = 1;
				break;
			case 'b':
				baseEngin = stoi(optarg, nullptr, 10);
				ret = 1;
				break;
			case 't':
				printTime = 1;
				break;
			case 'h':
			case 'v':
				showUsage();
				break;
			default:
				break;
		}
	}
	return ret;
}

int
main (int argc, char **argv)
{
	if(!parseArgs(argc, argv))
	{
		showUsage();
		return 0;
	}

	vector<string> fileList;
	vector<struct readData*> readList;
    getFileList(path, keyword, fileList);
	int size = fileList.size();
	for(int i = 0; i < size; i++)
	{
		size_t ll = 0;
		memset(buf, 0, BUFSIZE);
		int ret = readFile(fileList[i].c_str(), buf, &ll);
		struct readData *data = (struct readData *)malloc(sizeof(struct readData));
		data->data = (char*)malloc(ll);
		memcpy(data->data, buf, ll);
		data->size = ll;
		memset(data->sourceName, 0, 256);
		memcpy(data->sourceName, fileList[i].c_str(), strlen(fileList[i].c_str()));
		readList.push_back(data);
	}

	switch(baseEngin)
	{
		case 1:
		{
			if(decode)
			{
				decodeFun = &base64_decode;
			}
			else
			{
				encodeFun = &base64_encode;
			}
			break;
		}
		case 2:
		{
			if(decode)
			{
				decodeFun = &base64_decode_find;
			}
			else
			{
				encodeFun = &base64_encode_find;
			}
			break;
		}
	}

	// Invert return codes to create shell return code:
	int ret = (decode) ? !dec(readList) : !enc(readList);
	for(int i = 0; i < size; i++)
	{
		free(readList[i]->data);
		free(readList[i]);
	}
	readList.clear();
	return ret;
}
