#ifndef IMAG_ANNO_H
#define IMAG_ANNO_H

#ifdef IMGANNO_EXPORTS
#define IMGANNO_API __declspec(dllexport)
#else
#define IMGANNO_API __declspec(dllimport)
#endif


#include <vector>
using namespace std;

// 此类是从 imgAnno.dll 导出的
struct _out_pt 
{
	int x;
	int y;
};

extern IMGANNO_API int nimgAnno;

IMGANNO_API int fnimgAnno(void);
IMGANNO_API int GetAutoAnnoInfo(char* imgPath, vector<vector<_out_pt> > *pOutPoly, int g_nThresh);
#endif // IMAG_ANNO_H