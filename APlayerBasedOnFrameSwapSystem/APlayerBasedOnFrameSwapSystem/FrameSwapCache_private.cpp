#include "FrameSwapCache.h"

//  Dynamic Loading
int FrameSwapCache::_setFrameBlock(int iCurrentFrame, int iTargetBlock)
{
	_loadFrame(iCurrentFrame, iTargetBlock);
	_setFrameIndexOfCacheBlock(iCurrentFrame, iTargetBlock);

	return 0;
}

int FrameSwapCache::_resetFrameBlock(int iTargetBlock)
{
	//qDebug() << "cache Index remove: " << iTargetBlock<<'\n';
	_setFrameIndexOfCacheBlock(-1, iTargetBlock);

	return 0;
}

//  Dynamic Loading
int FrameSwapCache::_setFrameIndexOfCacheBlock(int iCurrentFrame, int iTargetBlock)
{
	m_pFrameIndexOfCacheBlock[iTargetBlock] = iCurrentFrame;
	return 0;
}

//  Dynamic Loading
int FrameSwapCache::_loadFrame(int iCurrentFrameNum, int iTargetBlock)
{
	//qDebug() << "_loadFrame";
	FILE *pf = NULL;
	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	const std::string root = m_sRootFolder;// "D:\\Downloads\\London\\London\\LondonOne\\LondonOne";
	const std::string suffix = m_sVideoSuffix;// ".rgb";

	DWORD * pFrame = m_pFrameCache[iTargetBlock];

	std::string sFileNum = std::to_string(iCurrentFrameNum);

	fulFillZero(sFileNum);

	std::string sFullFilePath = root + "\\" + m_sVideoName + sFileNum + suffix;


	if ((pf = fopen(sFullFilePath.data(), "rb")) == NULL)
	{
		printf("File coulkd not be opened ");
		qDebug() << sFullFilePath.data() << "\n";
		return -1;
	}

	int n = fread(pRData, uiFrameSize_pixel, 1, pf);
	n = fread(pGData, uiFrameSize_pixel, 1, pf);
	n = fread(pBData, uiFrameSize_pixel, 1, pf);

	if (n == 1)
	{
		for (unsigned int i = 0; i < uiFrameSize_pixel; ++i)
		{
			BYTE *pb = (BYTE *)(pFrame + i);
			pFrame[i] = pBData[i];
			pb[1] = pGData[i];
			pb[2] = pRData[i];
			pb[3] = 255;
		}
	}
	else
	{
		printf("_loadFrame: fread failed!\n");
		return -1;
	}
	fclose(pf);

	//qDebug() << "targetBlock: "<< iTargetBlock << ' ' << "currentFrameNum:  "<<iCurrentFrameNum;
	return 0;
}


int FrameSwapCache::_forwardLoadFrameSeq()
{
	int iNextFrameNum = (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] + 1) % (m_iMaxFrame + 1);


	if (iNextFrameNum == 0) //&& (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] != -1))
	{
		if (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] == -1)
		{
			if (m_iTail == (m_iHead + 1) % m_iCacheSize)
			{
				printf("it is true, ");
			}
			printf("forwardLoadFrame race condition \n");
			qDebug() << "last Frame in Map is: " << m_FrameCacheMap.lastIndex() + 1;
			iNextFrameNum = (m_FrameCacheMap.lastIndex() + 1) + 1;
		}
		else
		{
			printf("forwardLoadFrame end \n");
			iNextFrameNum += 1;
			return 0;
		}
	}
	//printf("current Frame: %d, current head: %d, current tail: %d, last loaded frame: %d, loading Frame: %d | ", m_iCurrentFrame, m_iHead, m_iTail, m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize], iNextFrameNum);
	//printf("current Frame: %d, current head frame: %d, current tail frame: %d, loading Frame: %d \n ", m_iCurrentFrame, m_pFrameIndexOfCacheBlock[m_iHead], m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize],iNextFrameNum);

	if ((iNextFrameNum - 1) != m_FrameCacheMap.lastIndex() + 1) // ID + 1 = true frame number
	{
		qDebug() << "forward function error!\n: " << "iNextFrameNum - 1: " << iNextFrameNum - 1 << " m_FrameCacheMap.lastIndex() + 1" << m_FrameCacheMap.lastIndex() + 1;
		return -2;
		//_clear();
	}
	m_FrameCacheMap.append(append(iNextFrameNum));

	//qDebug() << "forward: map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
	return 0;
}


void FrameSwapCache::_startThread()
{
	if (!(isFull()) && !(isAlmostFull()))
	{
		loaderThread.start();
	}
}