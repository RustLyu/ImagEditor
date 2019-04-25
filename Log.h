#ifndef LOG_H    
#define LOG_H    

//logÎÄ¼þÂ·¾¶  
#define LOG_FILE_NAME "./log.txt" 

#include <fstream>    
#include <string>    
#include <ctime>    
using namespace std;
#define DEBUG(x)          CLog::WriteDebug(x); 
#define LOG(x)			  CLog::WriteLog(x);     
#define INFO(x)           CLog::WriteInfo(x);  
#define TRACE(x)          CLog::WriteTrace(x); 
#define WARNING(x)		  CLog::WriteWarning(x);
#define ERROR(x)          CLog::WriteError(x); 

const uint MAX_PATH = 1000;

class CLog
{
public:
	template <class T>
	static void WriteLog(T x)
	{
		ofstream fout(LOG_FILE_NAME, ios::app);
		fout.seekp(ios::end);
		fout << GetSystemTime() << " [LOG] " << x << endl;
		fout.close();
	}

	template <class T>
	static void WriteDebug(T x)
	{
		qDebug() << x;
		ofstream fout(LOG_FILE_NAME, ios::app);
		fout.seekp(ios::end);
		fout << GetSystemTime() << " [DEBUG] " << x << endl;
		fout.close();
	}

	template <class T>
	static void WriteInfo(T x)
	{
		qDebug() << x;
		ofstream fout(LOG_FILE_NAME, ios::app);
		fout.seekp(ios::end);
		fout << GetSystemTime() << " [INFO] " << x << endl;
		fout.close();
	}

	template <class T>
	static void WriteTrace(T x)
	{
		qDebug() << x;
		ofstream fout(LOG_FILE_NAME, ios::app);
		fout.seekp(ios::end);
		fout << GetSystemTime() << " [TRACE] " << x << endl;
		fout.close();
	}

	template <class T>
	static void WriteError(T x)
	{
		qDebug() << x;
		ofstream fout(LOG_FILE_NAME, ios::app);
		fout.seekp(ios::end);
		fout << GetSystemTime() << " [ERROR] " << x << endl;
		fout.close();
	}

	template <class T>
	static void WriteWarning(T x)
	{
		qDebug() << x;
		ofstream fout(LOG_FILE_NAME, ios::app);
		fout.seekp(ios::end);
		fout << GetSystemTime() << " [WARNING] " << x << endl;
		fout.close();
	}

private:
	static string GetSystemTime()
	{
		time_t tNowTime;
		time(&tNowTime);
		tm* tLocalTime = localtime(&tNowTime);
		char szTime[30] = { '\0' };
		strftime(szTime, 30, "[%Y-%m-%d %H:%M:%S] ", tLocalTime);
		string strTime = szTime;
		return strTime;
	}
};
#endif  
