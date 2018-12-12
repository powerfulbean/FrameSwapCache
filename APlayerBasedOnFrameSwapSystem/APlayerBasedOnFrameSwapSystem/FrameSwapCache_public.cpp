#include "FrameSwapCache.h"
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

	m_FrameCacheMap.setCapacity(m_iCacheSize);

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

	m_pContiguousEleArray = new int[m_iCacheSize];

	//m_pFrameStateFlag = new bool[m_iMaxFrame];

	//memset(m_pFrameStateFlag, false, m_iMaxFrame);

	m_iHead = 0;
	m_iTail = 0;
	m_iFirstIndex = -1;
	m_iLastIndex = -1;
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
	int iCurrentFrameId = iCurrentFrame - 1;

	if (m_FrameCacheMap.containsIndex(iCurrentFrameId))//&& (m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)] == iCurrentFrame))
	{
		if (iCurrentFrameId - m_FrameCacheMap.firstIndex() >= m_iStorageThreshold)
		{
			/*qDebug() << "Map remove: " << m_FrameCacheMap.first();
			qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1];*/
			m_FrameCacheMap.takeFirst();
			prepop();

			//loaderThread.start(QThread::HighPriority);
			//startThread();
			return m_pFrameCache[m_FrameCacheMap.at(iCurrentFrameId)];
		}
		else
		{
			//loaderThread.start(QThread::HighPriority);
			//startThread();
			return m_pFrameCache[m_FrameCacheMap.at(iCurrentFrameId)];
		}

	}
	else
	{
		printf("fetchFrameBlock: fatal error! iCurrentFrameId: %d\n", iCurrentFrameId);
		return NULL;
	}

}

//  Dynamic Loading
int FrameSwapCache::checkFrameExisted(int iCurrentFrame)
{
	int iCurrentFrameId = iCurrentFrame - 1;

	if (m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{
		if (m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)] == iCurrentFrame)
		{
			;
		}
		else
		{
			printf("\n existedFrameID confliction: conflicted block! iCurrentFrame: %d, FrameIndexOfCacheBlock  %d\n", iCurrentFrame, m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)]);
			loaderThread.interrupt();
			while (!loaderThread.isStoped())
			{
				//printf("!");
				loaderThread.exit();
			}
			clear();
			m_FrameCacheMap.clear();
			return -2;
		}
		return 0;
	}
	else if (!m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{
		printf("\n \n");
		qDebug() << "checkFrameId: " << iCurrentFrameId << "  doesn't hit!  ";
		if (iCurrentFrameId == m_FrameCacheMap.firstIndex() - 1) // ��ǰ��������Ԫ�� add new item at the head
		{
			return 1;
		}
		else if (iCurrentFrameId == m_FrameCacheMap.lastIndex() + 1)
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
			m_FrameCacheMap.clear();
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
	//qDebug() << "checkAndLoadFrame";
	loaderThread.interrupt();
	/*while (!loaderThread.isStoped())
	{
	printf("!");
	loaderThread.exit();
	}*/
	int iCurrentFrameId = iCurrentFrame - 1;

	if (m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{
		//printf("hit!\n");
		if (m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)] == iCurrentFrame)
		{
			//printf("hit! \n");
		}
		else
		{
			printf("\n existedFrameID confliction: conflicted block! iCurrentFrame: %d, FrameIndexOfCacheBlock  %d\n", iCurrentFrame, m_pFrameIndexOfCacheBlock[m_FrameCacheMap.at(iCurrentFrameId)]);
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1]<<'\n';
			while (!loaderThread.isStoped())
			{
				//printf("!");
				loaderThread.exit();
			}
			clear();
			m_FrameCacheMap.clear();
			m_FrameCacheMap.insert(iCurrentFrameId, append(iCurrentFrame));
		}
	}
	else if (!m_FrameCacheMap.containsIndex(iCurrentFrameId))
	{

		qDebug() << "checkAndLoadFrame: FrameId: " << iCurrentFrameId << "  doesn't hit!  ";
		while (!loaderThread.isStoped())
		{
			//printf("!");
			loaderThread.exit();
		}
		if (iCurrentFrameId == m_FrameCacheMap.firstIndex() - 1) // ��ǰ��������Ԫ�� add new item at the head
		{
			//printf("insert next front!!!\n");
			if (isFull())
			{
				if (!m_FrameCacheMap.isFull())
				{
					printf("checkAndLoadFrame: fatal error: _isFull() conflicts! \n");
				}
				int iBlockIndex = m_FrameCacheMap.takeLast();//remove the head item
				if (iBlockIndex != m_iTail - 1)
				{
					printf("checkAndLoadFrame: fatal error: m_iTail coflicts! \n");
				}
				backpop();
			}
			m_FrameCacheMap.prepend(prepend(iCurrentFrame));
		}
		else if (iCurrentFrameId == m_FrameCacheMap.lastIndex() + 1)
		{
			//printf("insert next back!!!\n");
			if (isFull())
			{
				if (!m_FrameCacheMap.isFull())
				{
					printf("checkAndLoadFrame: fatal error: _isFull() conflicts! \n");
				}
				int iBlockIndex = m_FrameCacheMap.takeFirst();//remove the head item
				if (iBlockIndex != m_iHead)
				{
					printf("checkAndLoadFrame: fatal error: m_iHead coflicts! iBlockIndex: %d, m_iHead: %d \n", iBlockIndex, m_iHead);
				}
				prepop();
			}
			m_FrameCacheMap.append(append(iCurrentFrame));
			//qDebug() << "append: ";
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1] << '\n';
		}
		else
		{
			clear();
			m_FrameCacheMap.insert(iCurrentFrameId, append(iCurrentFrame));
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail<< "  |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to "  << m_pFrameIndexOfCacheBlock[m_iTail - 1]<<'\n';
		}
	}
	else
	{
		printf("checkAndLoadFrame: fatal error! \n");
		return NULL;
	}
	//loaderThread.start(QThread::HighPriority);
	_startThread();
	return 0;
}

//  Dynamic Loading
int FrameSwapCache::clear()
{
	//printf("clear\n");
	m_iHead = 0;
	m_iTail = 0;
	m_bIsFull = false;
	m_bIsAlmostFull = false;
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
//  Dynamic Loading
int FrameSwapCache::prepop()
{
	if ((m_iHead == m_iTail) && (!m_bIsFull))
	{
		printf("error: Queue is empty! \n");
		return -1;
	}

	_resetFrameBlock(m_iHead);
	m_iHead = (m_iHead + 1) % m_iCacheSize;

	m_bIsFull = false;

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

	if ((m_iHead == m_iTail) && (m_bIsFull))
	{
		printf("_prepend: Queue is full! \n");
		return 1;
	}

	int iNextHead = (m_iHead - 1 + m_iCacheSize) % m_iCacheSize;

	if (iNextHead == m_iTail)
	{
		m_bIsFull = true;
	}

	if (iNextHead == (m_iTail + 2) % m_iCacheSize)
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
		m_FrameCacheMap.append(append(i + 1));
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
		m_FrameCacheMap.insert(i - 1, append(i));// !!!!! i-1 cause the index in QContiguous starts from 0!!!
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
		m_FrameCacheMap.insert(i - 1, append(i));// !!!!! i-1 cause the index in QContiguous starts from 0!!!
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