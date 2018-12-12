#include "Frame.h"

#include <typeinfo>


void fulFillZero(std::string & sNum)
{
	//qDebug() << "sNum: " << sNum.data() << endl;
	if (sNum.size() == 1)
	{
		std::string sFill("000");
		sNum = sFill + sNum;
	}
	else if( sNum.size() == 2)
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

Frame::Frame( QWidget *parent) :
	QWidget(parent)
{
	timer.setTimerType(Qt::PreciseTimer);
	m_caliTimer.setTimerType(Qt::PreciseTimer);
	connect(&timer, SIGNAL(timeout()), this, SLOT(updateCurrentFrame()));
	connect(this, SIGNAL(currentFrameUpdated(int)), this, SLOT(repaint()));
	connect(&m_caliTimer, SIGNAL(timeout()), this, SLOT(calibrationTimer()));
	//connect(this, SIGNAL(rootFolderIsSet()), this, SLOT(LoadVideo()));  //signal: "rootFolderIsSet()" used to be emitted by setRootFolder()

	//connect(&loadTimer, SIGNAL(timeout()), this, SLOT(startThread()));
	loaderThread.setFrameObject(this);

	audioPlayer = new QMediaPlayer(this, QMediaPlayer::LowLatency);
	connect(audioPlayer, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), this, SLOT(updateAudioStatus(QMediaPlayer::MediaStatus)));
	m_iCurrentFrame = 1;


}

void Frame::setBasic(int iMaxframe, int iFrameWidth, int iFrameHeight, int iFps,int iCacheSize, int iInitialLoadedFrameSize )
{
	m_iMaxFrame = iMaxframe;
	m_iFrameWidth = iFrameWidth;
	m_iFrameHeight = iFrameHeight;
	m_iInterval = 1000/iFps;
	m_iCacheSize = iCacheSize;
	m_iInitialLoadedFrameSize = iInitialLoadedFrameSize;
	//timer.start(m_iInterval);
	//m_caliTimer.start(1000);

}

void Frame::Init()
{
	initCacheSystem(m_iCacheSize);

}

void Frame::LoadVideo(int startFrame)
{
	if (m_sRootFolder.compare("") == 0)
	{
		QMessageBox::warning(this, "Error", "Please specify path to video");
		return;
	}

	m_sVideoName_old = m_sVideoName;
	if (m_bIsStopped == false)
	{
		QMessageBox::StandardButton reply;
		reply = QMessageBox::critical(this, tr("Load Video Warning"), " Please Stop Video First!");
		return;
	}

	m_FrameCacheMap.clear();
	_clear();
	m_bAudioLoaded = false;
	if (startFrame != -1)
	{
		m_iCurrentFrame = startFrame;
		loadInitialFrame(startFrame,m_iInitialLoadedFrameSize);
	}
	else
	{
		m_iCurrentFrame = 1;
		loadInitialFrame(m_iInitialLoadedFrameSize);
	}
	m_bVideoIsLoaded = true;
	LoadAudio();
	qDebug() << "setCurrentLink finished";
	qDebug() << "current Frame: " << m_iCurrentFrame;
	emit currentFrameUpdated(m_iCurrentFrame);
	qDebug() << "LoadVideo Finished";
	emit videoLoaded(true);
}


void Frame::reloadVideo()
{
	m_sVideoName_old = m_sVideoName;
	emit videoLoaded(true);
}

Frame::~Frame()
{
	//free(pRData);
	//free(pGData);
	//free(pBData);
	//free(pFrame);
	//freeMemory();
	freeCacheSystemMemory();
	delete audioPlayer;
}

int Frame::initMemory(int iFrameNum, int width, int height)
{
	pFrames = (DWORD**)malloc(iFrameNum * sizeof(DWORD*));
	if (pFrames == NULL)
	{
		return -1;
	}

	DWORD uiFrameSize_pixel = width * height;

	for (int i = 0; i < iFrameNum; i++)
	{
		pFrames[i] = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	}

	/*pFramesLoadFlag = new bool[iFrameNum];

	memset(pFramesLoadFlag, false, iFrameNum );*/

	pFrame = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	pRData = (BYTE*)malloc(uiFrameSize_pixel);
	pGData = (BYTE*)malloc(uiFrameSize_pixel);
	pBData = (BYTE*)malloc(uiFrameSize_pixel);
}

int Frame::initMemory()
{
	pFrames = (DWORD**)malloc(m_iMaxFrame * sizeof(DWORD*));
	if (pFrames == NULL)
	{
		return -1;
	}

	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	for (int i = 0; i < m_iMaxFrame; i++)
	{
		pFrames[i] = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
		if (pFrames[i] == NULL)
		{
			return -1;
		}
	}

	//pFramesLoadFlag = new bool[m_iMaxFrame];
	/*if (pFramesLoadFlag == NULL)
	{
		return -1;
	}

	memset(pFramesLoadFlag, false, m_iMaxFrame);*/

	pFrame = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	pRData = (BYTE*)malloc(uiFrameSize_pixel);
	pGData = (BYTE*)malloc(uiFrameSize_pixel);
	pBData = (BYTE*)malloc(uiFrameSize_pixel);

	return 0;
}

int Frame::freeMemory()
{
	for (int i = 0; i < m_iMaxFrame; i++)
	{
		free(pFrames[i]);
	}

	free(pFrames);
	return 0;
}

void Frame:: LoadAllFrame()
{
	for (int i = 0; i < m_iMaxFrame; i++)
	{
		LoadFrame(i + 1);
		if (i % 100 == 0)
		{
			printf(">");
		}
		if (i % 1000 == 0)
		{
			printf("\n");
		}
	}
	printf("\nLOAD ALL FRAME SUCCEED!\n");
}


void Frame::LoadFrame( int iFrameNum )
{
	FILE *pf = NULL;
	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	const std::string root = m_sRootFolder;// "D:\\Downloads\\London\\London\\LondonOne\\LondonOne";
	const std::string suffix = m_sVideoSuffix;// ".rgb";

	DWORD * pFrame = pFrames[iFrameNum - 1];

	std::string sFileNum = std::to_string(iFrameNum);

	fulFillZero(sFileNum);

	std::string sFullFilePath = root + "\\" + m_sVideoName + sFileNum + suffix;


	if ((pf = fopen(sFullFilePath.data(), "rb")) == NULL)
	{
		printf("File coulkd not be opened ");
		qDebug() << sFullFilePath.data() << "\n";
		return;
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
	fclose(pf);
}

void Frame::LoadAudio()
{
	QString audioFileName = QString::fromStdString(m_sRootFolder+"\\" + m_sVideoName + m_sAudioSuffix);
	audioPlayer->setMedia(QMediaContent());
	audioPlayer->setMedia(QUrl::fromLocalFile(audioFileName));
	audioPlayer->setVolume(0);
	audioPlayer->play();
	m_bAudioLoaded = false;
	qDebug() << "LoadAudio finished";
}


void Frame::paintEvent(QPaintEvent *e)
{
	if (m_bVideoIsLoaded == false)
	{
		return;
	}
	paint.begin(this);

	DWORD * pFrame = fetchFrameBlock(m_iCurrentFrame); //pFrames[m_iCurrentFrame-1];

	QImage image = QImage((uchar*)pFrame, m_iFrameWidth, m_iFrameHeight, QImage::Format_RGB32);
	QSize size = image.size().scaled(this->rect().size(), Qt::KeepAspectRatio);
	QRect topPortion = QRect(QPoint((this->rect().size().width() - size.width()) / 2, (this->rect().size().height() - size.height()) / 2), size);
	paint.setRenderHint(QPainter::SmoothPixmapTransform, 1);
	paint.drawImage(topPortion, image);

	paint.end();

}

void Frame::setFileList(QString path)
{
	dirPath.setPath(path);
	fileList.append(dirPath.entryInfoList());
	/*for (int i = 0; i < fileList.size(); i++)
	{
		qDebug() << "Filepath" << fileList[i].path()<< "Filename" << fileList[i].fileName()<<endl;
	}*/
}

void Frame::setRootFolder(QString rootFolderPath)
{
	m_sRootFolder = rootFolderPath.toStdString();
	QDir dir;
	dir.setPath(rootFolderPath);
	m_sVideoName = dir.dirName().toStdString();
	printf(" rootfolder: %s, VideoName: %s\n", m_sRootFolder.data(), m_sVideoName.data());

	//emit rootFolderIsSet();
}

void Frame::setRootFolderForEditor(QString rootFolderPath)
{
	m_sVideoName_old = m_sVideoName;
	
	setRootFolder(rootFolderPath);
}

void Frame::setVideoName(QString videoName )
{
	m_sVideoName = videoName.toStdString();
}

//  Dynamic Loading
int Frame::initCacheSystem(int iCacheSize)
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

	//m_pFrameStateFlag = new bool[m_iMaxFrame];

	//memset(m_pFrameStateFlag, false, m_iMaxFrame);

	m_iHead = 0;
	m_iTail = 0;
	m_bIsFull = false;
	m_bIsAlmostFull = false;

	pFrame = (DWORD*)malloc(uiFrameSize_pixel * sizeof(DWORD));
	pRData = (BYTE*)malloc(uiFrameSize_pixel);
	pGData = (BYTE*)malloc(uiFrameSize_pixel);
	pBData = (BYTE*)malloc(uiFrameSize_pixel);

	return 0;
}

//  Dynamic Loading
void Frame:: freeCacheSystemMemory()
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
DWORD * Frame::fetchFrameBlock(int iCurrentFrame)
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
			_prepop();

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
int Frame::checkFrameExisted(int iCurrentFrame)
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
			_clear();
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
			_clear();
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
int Frame::checkAndLoadFrame(int iCurrentFrame)
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
			_clear();
			m_FrameCacheMap.clear();
			m_FrameCacheMap.insert(iCurrentFrameId, _append(iCurrentFrame));
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
			if (_isFull())
			{
				if (!m_FrameCacheMap.isFull())
				{
					printf("checkAndLoadFrame: fatal error: _isFull() conflicts! \n");
				}
				int iBlockIndex = m_FrameCacheMap.takeLast();//remove the head item
				if (iBlockIndex != m_iTail-1)
				{
					printf("checkAndLoadFrame: fatal error: m_iTail coflicts! \n");
				}
				_backpop();
			}
			m_FrameCacheMap.prepend(_prepend(iCurrentFrame));
		}
		else if (iCurrentFrameId == m_FrameCacheMap.lastIndex() + 1)
		{
			//printf("insert next back!!!\n");
			if (_isFull())
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
				_prepop();
			}
			m_FrameCacheMap.append(_append(iCurrentFrame));
			//qDebug() << "append: ";
			//qDebug() << "map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
			//qDebug() << "blockIndex range: " << "head: " << m_iHead << " to " << "tail: " << m_iTail << " |  Content : " << m_pFrameIndexOfCacheBlock[m_iHead] << " to " << m_pFrameIndexOfCacheBlock[m_iTail - 1] << '\n';
		}
		else
		{
			_clear();
			m_FrameCacheMap.insert(iCurrentFrameId, _append(iCurrentFrame));
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
	startThread();
	return 0;
}

//  Dynamic Loading
int Frame::_clear()
{
	//printf("clear\n");
	m_iHead = 0;
	m_iTail = 0;
	m_bIsFull = false;
	m_bIsAlmostFull = false;
	return 0;
}

//  Dynamic Loading
bool Frame::_isFull()
{
	return m_bIsFull;
}

//  Dynamic Loading
bool Frame::isFull()
{
	return m_bIsFull;
}

//  Dynamic Loading
bool Frame::isAlmostFull()
{
	return m_bIsAlmostFull;
}
//  Dynamic Loading
int Frame::_prepop()
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
int Frame::_backpop()
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
int Frame::_prepend(int iFrameNum)  // return 1: queue is full
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
int Frame::_append(int iFrameNum)
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

//  Dynamic Loading
int Frame::_setFrameBlock(int iCurrentFrame, int iTargetBlock)
{
	_loadFrame(iCurrentFrame, iTargetBlock);
	_setFrameIndexOfCacheBlock(iCurrentFrame, iTargetBlock);

	return 0;
}

int Frame::_resetFrameBlock(int iTargetBlock)
{
	//qDebug() << "cache Index remove: " << iTargetBlock<<'\n';
	_setFrameIndexOfCacheBlock(-1, iTargetBlock);

	return 0;
}

//  Dynamic Loading
int Frame::_setFrameIndexOfCacheBlock(int iCurrentFrame, int iTargetBlock)
{
	m_pFrameIndexOfCacheBlock[iTargetBlock] = iCurrentFrame;
	return 0;
}

//  Dynamic Loading
int Frame::_loadFrame(int iCurrentFrameNum, int iTargetBlock)
{
	//qDebug() << "_loadFrame";
	FILE *pf = NULL;
	DWORD uiFrameSize_pixel = m_iFrameWidth * m_iFrameHeight;

	const std::string root = m_sRootFolder;// "D:\\Downloads\\London\\London\\LondonOne\\LondonOne";
	const std::string suffix = m_sVideoSuffix;// ".rgb";

	DWORD * pFrame = m_pFrameCache[iTargetBlock];

	std::string sFileNum = std::to_string(iCurrentFrameNum);

	fulFillZero(sFileNum);

	std::string sFullFilePath = root +"\\"+ m_sVideoName+ sFileNum + suffix;


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


int Frame::forwardLoadFrameSeq()
{
	int iNextFrameNum = (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] + 1) % (m_iMaxFrame + 1);


	if (iNextFrameNum == 0) //&& (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] != -1))
	{
		if (m_pFrameIndexOfCacheBlock[(m_iTail - 1 + m_iCacheSize) % m_iCacheSize] == -1)
		{
			if (m_iTail == (m_iHead + 1)%m_iCacheSize)
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
	m_FrameCacheMap.append(_append(iNextFrameNum));

	//qDebug() << "forward: map index range: " << "first: " << m_FrameCacheMap.firstIndex() << " to " << "last: " << m_FrameCacheMap.lastIndex() << "  |  Content : " << m_FrameCacheMap.first() << " to " << m_FrameCacheMap.last();
	return 0;
}

//Dynamic Loading
void Frame::loadInitialFrame(int iInitialNum)
{
	for (int i = 0; i < iInitialNum; i++)
	{
		m_FrameCacheMap.append(_append(i + 1));
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

void Frame::loadInitialFrame(int startFrame, int iInitialNum)
{
	qDebug() << "loadFrames";
	for (int i = startFrame; i <= startFrame + iInitialNum; i++)
	{
		//qDebug() << startFrame;
		m_FrameCacheMap.insert(i - 1, _append(i));// !!!!! i-1 cause the index in QContiguous starts from 0!!!
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
void Frame::_loadFrames(int startFrame, int iInitialNum)
{
	qDebug() << "loadFrames";
	for (int i = startFrame ; i <= startFrame + iInitialNum; i++)
	{
		//qDebug() << startFrame;
		m_FrameCacheMap.insert(i-1 , _append(i ));// !!!!! i-1 cause the index in QContiguous starts from 0!!!
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
