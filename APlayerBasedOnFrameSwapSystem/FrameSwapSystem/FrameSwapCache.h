#pragma once
#include <QtWidgets/QMainWindow>
#include <QObject>
#include <QString>
#include <QDebug>
#include <cstring>
#include <string>
#include <time.h> 
#include <QPainterPath>
#include <iostream>
#include "multiThread.h"

typedef unsigned short WORD;
typedef unsigned char BYTE;
typedef unsigned int DWORD;

class FrameSwapCache
{
private:
	BYTE *pRData;
	BYTE *pGData;
	BYTE *pBData;
	DWORD * pFrame;
	DWORD ** m_pFrameCache;
	int * m_pFrameIndexOfCacheBlock;

	CFramesLoaderThread loaderThread;

	int m_iCacheSize;
	int m_iFrameWidth = 0;
	int m_iFrameHeight = 0;
	int m_iStorageThreshold = 8; // 保留一部分使用过的图像
	int m_iLoadTimerInterval = 300;
	int m_iMaxFrame;

	std::string m_sRootFolder = "";
	std::string m_sVideoSuffix = ".rgb";
	std::string m_sVideoName = "";


	int m_iHead;
	int m_iTail;
	int m_iCurrentMaxIndex;

	bool m_bIsFull;
	bool m_bIsAlmostFull;
	bool m_bIsEmpty;

	int _resetFrameBlock(int iTargetBlock);
	int _setFrameBlock(int, int);
	int _setFrameIndexOfCacheBlock(int, int);
	int _loadFrame(int iCurrentFrameNum, int iTargetBlock);
	void _startThread();

public:
	FrameSwapCache(int, int, int);
	~FrameSwapCache();

	int initCacheSystem(int);
	void freeCacheSystemMemory();

	void setRootFolder(std::string temp);
	void setVideoName(std::string temp);

	void loadInitialFrame(int iInitialNum);
	void loadInitialFrame(int, int);
	void loadFrames(int startFrameNum, int iInitialNum);
	
	bool isFull();
	bool isAlmostFull();
	bool isEmpty();
	
	int checkAndLoadFrame(int iFrameNum);
	int checkFrameExisted(int iFrameNum);
	
	int prepop();
	int backpop();
	int prepend(int iFrameNum);
	int append(int iFrameNum);
	int insert(int iIndex, int iFrameNum);
	int clear();

	
	bool containsIndex(int iIndex);
	int firstIndex();
	int lastIndex();
	int currentMaxIndex();
	DWORD * first();
	DWORD * last();
	DWORD * takeFirst();
	DWORD * takseLast();
	DWORD * at(int);

	DWORD * fetchFrameBlock(int iFrameNum);
	int _forwardLoadFrameSeq();

	void startThread();
	void stopThread();
};

void fulFillZero(std::string & sNum);