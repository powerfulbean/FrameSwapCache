#include "FrameSwapCache.h"

void fulFillZero(std::string & sNum)
{
	//qDebug() << "sNum: " << sNum.data() << endl;
	if (sNum.size() == 1)
	{
		std::string sFill("000");
		sNum = sFill + sNum;
	}
	else if (sNum.size() == 2)
	{
		std::string sFill("00");
		sNum = sFill + sNum;
	}
	else if (sNum.size() == 3)
	{
		std::string sFill("0");
		sNum = sFill + sNum;
	}
	else if (sNum.size() == 4)
	{
		return;
	}
}

FrameSwapCache::FrameSwapCache( int iExpectedTotalFrame, int iWidth, int iHeight)
{
	m_bIsAlmostFull = false;
	m_bIsFull = false;
	m_bIsEmpty = true;
	m_iMaxFrame = iExpectedTotalFrame;
	loaderThread.setFrameObject(this);
	m_iFrameWidth = iWidth;
	m_iFrameHeight = iHeight;
}

FrameSwapCache::~FrameSwapCache()
{
	freeCacheSystemMemory();
}

void FrameSwapCache::setRootFolder(std::string temp)
{
	m_sRootFolder = temp;
}

void FrameSwapCache::setVideoName(std::string temp)
{
	m_sVideoName = temp;
}

//  Dynamic Loading
int FrameSwapCache::initCacheSystem(int iCacheSize)
{
	m_iCacheSize = iCacheSize;
	m_pFrameCache = (DWORD**)malloc(m_iCacheSize * sizeof(DWORD*));
	if (m_pFrameCache == NULL)
	{
		return -1;
	}

	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;
	for (int i = 0; i < m_iCacheSize; i++)
	{
		m_pFrameCache[i] = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	}

	m_pFrameIndexOfCacheBlock = new int[m_iCacheSize];

	for (int i = 0; i < m_iCacheSize; i++)
	{
		m_pFrameIndexOfCacheBlock[i] = 0;
	}

	m_iHead = 0;
	m_iTail = 0;
	m_iCurrentMaxIndex = 0;
	m_bIsFull = false;
	m_bIsAlmostFull = false;

	pFrame = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	pRData = (BYTE*)malloc(uiFrameSize_pixel);
	pGData = (BYTE*)malloc(uiFrameSize_pixel);
	pBData = (BYTE*)malloc(uiFrameSize_pixel);

	return 0;
}

//  Dynamic Loading
void FrameSwapCache::freeCacheSystemMemory()
{
	for (int i = 0; i < m_iCacheSize; i++)
	{
		free(m_pFrameCache[i]);
	}

	free(m_pFrameCache);

	free(pFrame);
	free(pRData);
	free(pGData);
	free(pBData);

	delete m_pFrameIndexOfCacheBlock;
}

//  Dynamic Loading
DWORD * FrameSwapCache::fetchFrameBlock(int iCurrentFrame)
{
	//loaderThread.interrupt();
	//while (!loaderThread.isStoped())
	//{
	//	qDebug() << "still Running";
	//	//loaderThread.exit();
	//}

	if (containsIndex(iCurrentFrame))//&& (m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)] == iCurrentFrame))
	{
		if (iCurrentFrame - firstIndex() >= m_iStorageThreshold)
		{
			/*qDebug() << "Map remove: " << m_FrameCacheMap.first();
			qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1];*/
			prepop();

			//loaderThread.start(QThread::HighPriority);
			//startThread();
			return at(iCurrentFrame);
		}
		else
		{
			//loaderThread.start(QThread::HighPriority);
			//startThread();
			return at(iCurrentFrame);
		}

	}
	else
	{
		printf("fetchFrameBlock: fatal error! iCurrentFrame: %d\n", iCurrentFrame);
		return NULL;
	}

}

//  Dynamic Loading
int FrameSwapCache::checkFrameExisted(int iCurrentFrame)
{
	if (containsIndex(iCurrentFrame))
	{
		;
	}
	else if (!containsIndex(iCurrentFrame))
	{
		printf("\n \n");
		qDebug() << "checkFrameId: " << iCurrentFrame<< "  doesn't hit!  ";
		if (iCurrentFrame == firstIndex() - 1) // add new item at the head
		{
			return 1;
		}
		else if (iCurrentFrame == lastIndex() + 1)
		{
			return 2;
		}
		else
		{
			loaderThread.interrupt();
			while (!loaderThread.isStoped())
			{
				//printf("!");
				loaderThread.exit();
			}
			clear();
			return -1;
		}
	}
	else
	{
		printf("checkFrameExisted: fatal error! \n");
		return NULL;
	}
}

//  Dynamic Loading
int FrameSwapCache::checkAndLoadFrame(int iCurrentFrame)
{
	loaderThread.interrupt();
	/*while (!loaderThread.isStoped())
	{
	printf("!");
	loaderThread.exit();
	}*/

	if (containsIndex(iCurrentFrame))
	{
		;
	}
	else 
	{
		qDebug() << "checkAndLoadFrame: FrameId: " << iCurrentFrame << "  doesn't hit!  ";
		while (!loaderThread.isStoped())
		{
			//printf("!");
			loaderThread.exit();
		}
		if (iCurrentFrame == firstIndex() - 1) // add new item at the head
		{
			//printf("insert next front!!!\n");
			if (isFull())
			{
				backpop();
			}
			prepend(iCurrentFrame);
		}
		else if (iCurrentFrame == lastIndex() + 1)
		{
			//printf("insert next back!!!\n");
			if (isFull())
			{
				prepop();
			}
			append(iCurrentFrame);
			//qDebug() << "append: ";
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1] << '\n';
		}
		else
		{
			clear();
			append(iCurrentFrame);
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail<< "  |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to "  << m_pFrameIndexOfCacheBlock[m_iTail - 1]<<'\n';
		}
	}
	_startThread();
	return 0;
}

//  Dynamic Loading
bool FrameSwapCache::isFull()
{
	return m_bIsFull;
}

//  Dynamic Loading
bool FrameSwapCache::isAlmostFull()
{
	return m_bIsAlmostFull;
}

bool FrameSwapCache::isEmpty()
{
	return m_bIsEmpty;
}

//  Dynamic Loading
int FrameSwapCache::clear()
{
	//printf("clear\n");
	m_iHead = 0;
	m_iTail = 0;
	m_bIsFull = false;
	m_bIsAlmostFull = false;
	m_bIsEmpty = true;
	return 0;
}

//  Dynamic Loading
int FrameSwapCache::prepop()
{
	// check is Empty
	if ((m_iHead == m_iTail) && (!m_bIsFull))
	{
		printf("error: Queue is empty! \n");
		return -1;
	}

	// release head block
	_resetFrameBlock(m_iHead);
	m_iHead = (m_iHead + 1) % m_iCacheSize;

	m_bIsFull = false;

	// disable almost full
	if (((m_iTail + 2) % m_iCacheSize != m_iHead) && (m_bIsFull == false))
	{
		m_bIsAlmostFull = false;
	}
	return 0;
}

//  Dynamic Loading
int FrameSwapCache::backpop()
{
	if ((m_iHead == m_iTail) && (!m_bIsFull))
	{
		printf("error: Queue is empty! \n");
		return -1;
	}
	m_iTail = (m_iTail - 1 + m_iCacheSize) % m_iCacheSize;
	_resetFrameBlock(m_iTail);

	m_bIsFull = false;

	if (((m_iTail + 2) % m_iCacheSize != m_iHead) && (m_bIsFull == false))
	{
		m_bIsAlmostFull = false;
	}
	return 0;
}

//  Dynamic Loading
int FrameSwapCache::prepend(int iFrameNum)  // return 1: queue is full
{
	if (containsIndex(iFrameNum))
	{
		printf("the frame to be appended exist in cache! \n");
		return (m_iHead + iFrameNum - m_pFrameIndexOfCacheBlock[m_iHead]) % m_iCacheSize;
	}
	else if ((iFrameNum != firstIndex() - 1) && (!((m_iHead == m_iTail) && (!m_bIsFull))))
	{
		printf("append a discontiguous frame,the cache has to be cleared up\n ");
		clear();
	}

	if ((m_iHead == m_iTail) && (m_bIsFull))
	{
		printf("_prepend: Queue is full! \n");
		return 1;
	}

	int iNextHead = (m_iHead - 1 + m_iCacheSize) % m_iCacheSize;

	if (iNextHead == m_iTail) // chcek if the cache is full
	{
		m_bIsFull = true;
	}

	if (iNextHead == (m_iTail + 2) % m_iCacheSize) // check if the cache is almost full
	{
		m_bIsAlmostFull = true;
	}

	m_iHead = iNextHead;
	_setFrameBlock(iFrameNum, m_iHead);

	return m_iHead;
}

//  Dynamic Loading
int FrameSwapCache::append(int iFrameNum)
{
	if (containsIndex(iFrameNum))
	{
		printf("the frame to be appended exist in cache! \n");
		return (m_iHead + iFrameNum - m_pFrameIndexOfCacheBlock[m_iHead]) % m_iCacheSize;
	}
	else if ( (iFrameNum != lastIndex() + 1) && (! ((m_iHead == m_iTail) && (!m_bIsFull)) ))
	{
		printf("append a discontiguous frame,the cache has to be cleared up\n ");
		clear();
	}
	
	if ((m_iTail == m_iHead) && (m_bIsFull))
	{
		printf("_append: Queue is full! \n");
		return 1;
	}

	//qDebug() << "head:  "<< m_iHead  << ' '<<"tail:  "<< (m_iTail + 1) % m_iCacheSize ;
	if ((m_iTail + 1) % m_iCacheSize == m_iHead)
	{
		m_bIsFull = true;
	}

	if ((m_iTail + 3) % m_iCacheSize == m_iHead)
	{
		m_bIsAlmostFull = true;
	}
	
	_setFrameBlock(iFrameNum, m_iTail);

	int iPastTail = m_iTail;

	m_iTail = (m_iTail + 1) % m_iCacheSize;

	return iPastTail;

}

//Dynamic Loading
void FrameSwapCache::loadInitialFrame(int iInitialNum)
{
	for (int i = 0; i < iInitialNum; i++)
	{
		append(i + 1);
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	printf("\nLOAD Initial FRAME SUCCEED!\n");
}

void FrameSwapCache::loadInitialFrame(int startFrame, int iInitialNum)
{
	qDebug() << "loadFrames";
	for (int i = startFrame; i <= startFrame + iInitialNum; i++)
	{
		//qDebug() << startFrame;
		append(i);// !!!!! i-1 cause the index in QContiguous starts from 0!!!
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
	printf("LOAD FRAMES SUCCEED!\n");
}

//Dynamic Loading
void FrameSwapCache::loadFrames(int startFrame, int iInitialNum)
{
	qDebug() << "loadFrames";
	for (int i = startFrame; i <= startFrame + iInitialNum; i++)
	{
		//qDebug() << startFrame;
		append(i);// !!!!! i-1 cause the index in QContiguous starts from 0!!!
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
	printf("LOAD FRAMES SUCCEED!\n");
}

int FrameSwapCache::firstIndex()
{
	if ((m_iHead == m_iTail) && (!m_bIsFull))
	{
		return -1;
	}
	return m_pFrameIndexOfCacheBlock[m_iHead];
}

int FrameSwapCache::lastIndex()
{
	if ((m_iHead == m_iTail) && (!m_bIsFull))
	{
		return -1;
	}
	return m_pFrameIndexOfCacheBlock[m_iTail-1];
}

DWORD * FrameSwapCache::first()
{
	return m_pFrameCache[m_iHead];
}

DWORD * FrameSwapCache::last()
{
	return m_pFrameCache[m_iTail - 1];
}


DWORD * FrameSwapCache::at(int iIndex)
{
	if (!containsIndex(iIndex))
	{
		printf("the cache system does not possess this frame");
		return NULL;
	}
	return m_pFrameCache[(m_iHead + iIndex - m_pFrameIndexOfCacheBlock[m_iHead]) % m_iCacheSize];
}

bool FrameSwapCache::containsIndex(int iIndex)
{
	if ((m_iHead == m_iTail) && (!m_bIsFull)) // is empty
	{
		return false;
	}

	if (iIndex >= firstIndex() && iIndex <= lastIndex())
	{
		return true;
	}
	else
	{
		return false;
	}
}

void FrameSwapCache::startThread()
{
	if (!(isFull()) && !(isAlmostFull()))
	{
		loaderThread.start();
	}
}

void FrameSwapCache::stopThread()
{
	loaderThread.exit();
}

