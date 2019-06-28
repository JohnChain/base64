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
using namespace std;
using namespace std::chrono;

#define BUFSIZE 100 * 1024 * 1024
int decode = 0;
std::string path = "";
std::string keyword = "";
static char buf[BUFSIZE];
static char out[(BUFSIZE * 5) / 3];	// Technically 4/3 of input, but take some margin
size_t nread;
size_t nout;

struct readData{
	char * data;
	int size;
};

static const struct option long_option[] = {
	{"decode", required_argument, NULL, 'd'},
	{"encode", required_argument, NULL, 'e'},
	{"path", required_argument, NULL, 'p'},
	{"keyword", required_argument, NULL, 'k'},
	{"help", required_argument, NULL, 'h'},
	{"version", required_argument, NULL, 'v'},
	{NULL, 0, NULL, 0},
};

void showUsage(){
	printf("[USAGE]: ./base64 -d/-e -p dir_path_to_files -k file_keyword");
}

int getFileList(string &basePath, string &keyword, vector<string> &fileList)
{
	DIR *dir;
	struct dirent *ptr;
	printf("basePath = %s\n", basePath.c_str());
	if ((dir=opendir(basePath.c_str())) == NULL){
		printf("Open %s error... %s[%d]\n", basePath.c_str(), strerror(errno), errno);
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
		printf("[%d] %s\n", i, fileList[i].c_str());
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
enc (vector <struct readData*> &readList)
{
	int ret = 1;
	steady_clock::time_point t1 = steady_clock::now();
	int size = readList.size();
	for(int i = 0; i < size; i++)
    {
		memset(out, 0, (BUFSIZE * 5) / 3);
		base64_encode(readList[i]->data, readList[i]->size, out, &nout, 0);
		if (nout) {
			fwrite(out, nout, 1, stdout);
			printf("======================\n");
		}
	}
	steady_clock::time_point t2 = steady_clock::now();
	duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
	printf("It took me %lf\n", time_span.count());
	fclose(stdout);
	return ret;
}

static int
dec (vector <struct readData*> &readList)
{
	int ret = 1;
	//for (vector<std::string>::const_iterator iter = readList.cbegin(); iter != readList.cend(); iter++)
    //{
        //steady_clock::time_point t1 = steady_clock::now();
		//ret = base64_decode(iter->c_str(), iter->size(), out, &nout, 0);
		//steady_clock::time_point t2 = steady_clock::now();
		//duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
		//printf("It took me %lf\n", time_span.count());
		//if (!ret) {
			//fprintf(stderr, "decoding error\n");
			//ret = 0;
			//goto out;
		//}
    //}

	////if (nout) {
		////fwrite(out, nout, 1, stdout);
	////}
//out:
	//fclose(stdout);
	return ret;
}

int parseArgs(int argc, char *argv[])
{
	int opt=0;
	int ret = 0;
	while((opt=getopt_long(argc,argv,"dep:k:hv",long_option,NULL))!=-1){
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
		readList.push_back(data);
	}

	// Invert return codes to create shell return code:
	return (decode) ? !dec(readList) : !enc(readList);
}
