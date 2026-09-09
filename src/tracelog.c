#include "tracelog.h"
// 默认日志最大100KB大小
#define MAXFILELEN 102400
char logtime[20];
char filepath[MAXFILEPATH] = "/tmp/scutclient.log";
FILE *logfile;
LOGLEVEL cloglev = INF;

static const char *LogLevelText[] =
		{ "NONE", "ERROR", "INF", "DEBUG", "TRACE" };
static const char *LogTypeText[] =
		{ "ALL", "INIT", "8021X", "DRCOM" };

static unsigned long get_file_size(const char *path) {
	struct stat statbuff;
	if (stat(path, &statbuff) < 0) {
		return 0;
	}
	return statbuff.st_size;
}

/*
 *获取时间
 * */
static void settime() {
	time_t timer = time(NULL);
	strftime(logtime, 20, "%Y-%m-%d %H:%M:%S", localtime(&timer));
}

static int initlog(LOGTYPE logtype, LOGLEVEL loglevel) {
	//获取日志时间
	settime();

	// 判定是否大于指定的大小，超过则轮转为备份文件
	if (get_file_size(filepath) > MAXFILELEN) {
		char backup[MAXFILEPATH + 16];
		snprintf(backup, sizeof(backup), "%s.backup.log", filepath);
		if (logfile != NULL) {
			fclose(logfile);
			logfile = NULL;
		}
		// 使用 rename() 取代 system("mv ...")，避免起 shell 及符号链接攻击
		rename(filepath, backup);
	}

	// 日志句柄常驻，避免每行 fopen/fclose 造成大量系统调用与闪存写放大
	if (logfile == NULL) {
		if ((logfile = fopen(filepath, "a+")) == NULL) {
			perror("Unable to open log file");
			return -1;
		}
	}
	//写入日志级别，日志时间
	fprintf(logfile, "[%s][%-5s][%-3s]:[", logtime, LogTypeText[logtype], LogLevelText[loglevel]);
	printf("[%s][%-5s][%-3s]:[", logtime, LogTypeText[logtype], LogLevelText[loglevel]);
	return 0;
}

/*
 *日志写入
 * */
int LogWrite(LOGTYPE logtype, LOGLEVEL loglevel, char *format, ...) {
	va_list args;

	if (loglevel > cloglev)
		return 0;
	//初始化日志
	if (initlog(logtype, loglevel) != 0)
		return -1;
	//打印日志信息
	va_start(args, format);
	vfprintf(logfile, format, args);
	va_end(args);
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	fprintf(logfile, "]\n");
	printf("]\n");
	//文件刷出（句柄保持打开，供后续复用）
	fflush(logfile);
	return 0;
}
