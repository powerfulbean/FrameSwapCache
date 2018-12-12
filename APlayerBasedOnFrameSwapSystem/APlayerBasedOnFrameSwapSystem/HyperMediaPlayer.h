#pragma once

#include <QWidget>
#include <QLayout>
#include <QFileDialog>
#include <QProxyStyle>
#include <QStack>
#include "ui_Player.h"
#include "Frame.h"

struct History
{
	std::string sHistoryFilePath;
	int iHistoryFrame;
	History()
	{
		;
	}

	History(std::string a, int b)
	{
		sHistoryFilePath = a;
		iHistoryFrame = b;
	}
};

class HyperMediaPlayer : public QWidget
{
	Q_OBJECT

public:
	HyperMediaPlayer(int iFrameNum, int iWidth, int iHeight,int iFps = 30, QWidget *parent = Q_NULLPTR);
	~HyperMediaPlayer();
private:
	Ui::Form ui;
	int m_iFrameNum;
	int m_iWidth;
	int m_iHeight;
	int m_iFps;
	QString frame2time(int iFrameNum, QString  string);
	QFileDialog *videoFolderDialog;
	QStack<History> frameStack;
	void initialFrame();

signals:
	void linkOutputUpdated(QString);
	
public slots:

	void updateTime(int i)
	{
		ui.label->setText(frame2time(i, ui.label->text()));
	}


	void jumpToAnotherFrame(std::string targetVideoName, int targetFrame)
	{
		History history(ui.widget->m_sRootFolder, ui.widget->m_iCurrentFrame);
		frameStack.append(history);
		//qDebug() << "targetFrame: "<<targetFrame;

		std::cout << ui.widget->m_sRootFolder << endl;
		std::size_t found = ui.widget->m_sRootFolder.find_last_of("/\\");
		std::string parentDirectory = ui.widget->m_sRootFolder.substr(0, found);
		parentDirectory = parentDirectory.append("/");
		std::cout << " path: " << parentDirectory << '\n';

		QString temp = QString::fromStdString(parentDirectory) + QString::fromStdString(targetVideoName); //"D:\\Downloads\\London\\London\\";
		//temp += QString::fromStdString(targetVideoName);
		ui.lineEdit->setText(temp);
		ui.widget->setRootFolder(temp);
		ui.widget->LoadVideo(targetFrame);
	}

	void backwardToHistoryFrame()
	{
		if (frameStack.empty())
		{
			printf("warning: there is no history recorded!\n");
			return;
		}
		ui.widget->framePause();
		ui.widget->m_bIsStopped = true;
		History history = frameStack.pop();
		QString temp = QString::fromStdString(history.sHistoryFilePath);
		ui.lineEdit->setText(temp);
		ui.widget->setRootFolder(temp);
		ui.widget->LoadVideo(history.iHistoryFrame);
		ui.widget->m_bIsStopped = false;
	}

	void enablePlayerUI(bool enable)
	{
		ui.pushButton->setEnabled(enable);
		ui.pushButton_2->setEnabled(enable);
		ui.pushButton_5->setEnabled(enable);
	}
};



class MyStyle : public QProxyStyle
{
public:
	using QProxyStyle::QProxyStyle;

	int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0, const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
	{
		if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
			return (Qt::LeftButton);// | Qt::MidButton | Qt::RightButton);
		return QProxyStyle::styleHint(hint, option, widget, returnData);
	}
};
