#include "Frame.h"
#include <typeinfo>


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
	//initCacheSystem(m_iCacheSize);
	frameCache = new FrameSwapCache(m_iMaxFrame, m_iFrameWidth, m_iFrameHeight);
	frameCache->initCacheSystem(m_iCacheSize);
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

	frameCache->clear();
	m_bAudioLoaded = false;
	if (startFrame != -1)
	{
		m_iCurrentFrame = startFrame;
		frameCache->loadInitialFrame(startFrame,m_iInitialLoadedFrameSize);
	}
	else
	{
		m_iCurrentFrame = 1;
		frameCache->loadInitialFrame(m_iInitialLoadedFrameSize);
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
	delete audioPlayer;
	delete frameCache;
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

	DWORD * pFrame = frameCache->fetchFrameBlock(m_iCurrentFrame); //pFrames[m_iCurrentFrame-1];

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
	frameCache->setRootFolder(m_sRootFolder);
	frameCache->setVideoName(m_sVideoName);
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
