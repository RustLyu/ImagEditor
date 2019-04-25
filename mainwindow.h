#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QListWidgetItem>

#include "svgviewshow.h"
#include "xmlnodeaction.h"
#include "savetopimgdialog.h"
#include "common.h"

// 手动调整图像通道最大值
const int maxChannelValue = 200;
// 手动调整图像通道最小值
const int minChannelValue = 150;

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
    ~MainWindow() override;
	void setStatusMessage(QString msg);
	void changeEditIcon(EditType type);

private slots:
	void SLOT_OnSaveClick();
	void SLOT_OnRefreshClick();
	void SLOT_OnHelpSelect();
	void SLOT_on_m_selectBackFileBtn_clicked();
	void SLOT_on_m_selectTopBtn_clicked();
	void SLOT_on_m_loadFileBtn_clicked();
	void SLOT_OnEditTextSelect();
	void SLOT_OnEditPolygonSelect();
	void SLOT_OnEditMileSelect();
	void SLOT_OnForbidEditSelect();
	void SLOT_OnEditLineSelect();
	void SLOT_OnSaveTopImgSelect();
	void SLOT_OnOpenFolderSelect();
	void SLOT_OnFileItemClicked(QListWidgetItem* item);

public:
	SetAttributeDialog* m_pDialogPolygon;
	SetAttributeDialog* m_pDialogLine;
	static MainWindow* Instance;

private:
	Ui::MainWindow *ui;
	SvgView *m_pSvg;
	SvgViewShow *m_pSvgShow;
	QPixmap m_image;
	QPixmap m_imageShow;
	QPainter *m_pPainter;
	SaveTopImgDialog* m_pSaveTopImg;
	// 帮助
	QMessageBox *m_pHelpMessageBox;
	// svg顶图文件路径
	QString m_backSvgPathStr;
	QMap<QString, modifyImagColor> m_str2ColorTypeMap;
	QString m_lastBackPath;
	QString m_lastTopPath;
	QFileInfoList m_imagePathList;

private:
	std::tuple<QImage, QImage> modifyImag(QImage image, modifyImagColor type);
	QFileInfoList getFileList(QString path);
	void initUI();
	void registSignal2Slots();
	bool readImage(const QString path, QImage& dst);
	void setActionShowStatus();
	void updateFileList();
	void updateBackImag();
	bool saveTopImage(QString path, QString color);

protected:
	void keyPressEvent(QKeyEvent *event) override;
};

#endif // MAINWINDOW_H
