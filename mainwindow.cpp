#include <QGridLayout>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDebug>
#include <QScrollBar>
#include <QDialogButtonBox>
#include <QImageReader>
#include <QImageWriter>
#include <Assert.h>
#include <QJsonParseError>

#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "config.h"
#include "Log.h"
#include "Libraries/include/imgAnno.h"

MainWindow* MainWindow::Instance; MainWindow::MainWindow(QWidget *parent) :QMainWindow(parent), ui(new Ui::MainWindow)
{
	Instance = this;
	ui->setupUi(this);
	this->setWindowTitle(QL("Svg编辑器"));

	initUI();
	registSignal2Slots();
	m_lastBackPath = "./";
	m_lastTopPath = "./";
	setActionShowStatus();
}

MainWindow::~MainWindow()
{
	delete m_pDialogLine;
	m_pDialogLine = nullptr;
	delete m_pSaveTopImg;
	m_pSaveTopImg = nullptr;
	delete m_pHelpMessageBox;
	m_pHelpMessageBox = nullptr;
	if (m_pSvg)
	{
		delete m_pSvg;
		m_pSvg = nullptr;
	}
	delete m_pDialogPolygon;
	m_pDialogPolygon = nullptr;
	delete ui;
}

void MainWindow::initUI()
{
	QFile file(Config::Instance->GetHelpPath());
	QString declare = nullptr;
	if (file.open(QIODevice::ReadOnly | QFile::Text))
	{
		declare = file.readAll();
	}
	else
	{
		declare = "敬请期待!";
	}

	m_pHelpMessageBox = new QMessageBox(QMessageBox::Icon::NoIcon, QL("使用说明"), declare, QMessageBox::NoButton, this);
	m_pHelpMessageBox->setVisible(false);
	m_pHelpMessageBox->addButton(QL("关闭"), QMessageBox::ButtonRole::YesRole);

	//默认设置为首页
	ui->stackedWidget->setCurrentIndex(0);
	m_pDialogPolygon = new SetAttributeDialog(nullptr, this, EditType::EditType_Polygon);
	m_pDialogLine = new SetAttributeDialog(nullptr, this, EditType::EditType_Line);
	m_pSaveTopImg = new SaveTopImgDialog();

	m_pSvg = nullptr;
	m_pSvgShow = nullptr;

	setStatusMessage(QL("欢迎使用SVG编辑器!"));

	m_str2ColorTypeMap["Red"] = ToRedImag;
	m_str2ColorTypeMap["Green"] = ToGreenImag;
	m_str2ColorTypeMap["Blue"] = ToBlueImag;
	m_str2ColorTypeMap["Gray"] = ToGrayImag;
	m_str2ColorTypeMap["RGBA"] = ToRawImag;
	ui->m_imgFileList->setVisible(false);
}

void MainWindow::registSignal2Slots()
{
	connect(ui->m_selectBackFileBtn, &QPushButton::clicked, this, &MainWindow::SLOT_on_m_selectBackFileBtn_clicked);
	connect(ui->m_openNewBack, &QAction::triggered, this, &MainWindow::SLOT_on_m_selectBackFileBtn_clicked);
	connect(ui->m_selectTopBtn, &QPushButton::clicked, this, &MainWindow::SLOT_on_m_selectTopBtn_clicked);
	connect(ui->m_openNewTop, &QAction::triggered, this, &MainWindow::SLOT_on_m_selectTopBtn_clicked);
	connect(ui->m_loadFileBtn, &QPushButton::clicked, this, &MainWindow::SLOT_on_m_loadFileBtn_clicked);
	connect(ui->m_save, &QAction::triggered, this, &MainWindow::SLOT_OnSaveClick);
	connect(ui->m_refresh, &QAction::triggered, this, &MainWindow::SLOT_OnRefreshClick);
	connect(ui->m_help, &QAction::triggered, this, &MainWindow::SLOT_OnHelpSelect);

	connect(ui->m_editPolygon, &QAction::triggered, this, &MainWindow::SLOT_OnEditPolygonSelect);
	connect(ui->m_editText, &QAction::triggered, this, &MainWindow::SLOT_OnEditTextSelect);
	connect(ui->m_editMile, &QAction::triggered, this, &MainWindow::SLOT_OnEditMileSelect);
	connect(ui->m_forbidEdit, &QAction::triggered, this, &MainWindow::SLOT_OnForbidEditSelect);
	connect(ui->m_editLine, &QAction::triggered, this, &MainWindow::SLOT_OnEditLineSelect);

	connect(ui->m_saveTopImg, &QAction::triggered, this, &MainWindow::SLOT_OnSaveTopImgSelect);

	connect(ui->m_openFolder, &QAction::triggered, this, &MainWindow::SLOT_OnOpenFolderSelect);

	connect(ui->m_imgFileList, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(SLOT_OnFileItemClicked(QListWidgetItem*)));
}

void MainWindow::SLOT_OnSaveClick()
{
	if (m_pSvgShow == nullptr)
		return;
	m_pSvgShow->onSaveSvg();
	m_pDialogPolygon->clear();
}

// 重置背景
void MainWindow::SLOT_OnRefreshClick()
{
	if (m_pSvgShow == nullptr)
		return;
	m_imageShow.load(Config::Instance->GetBackPngPath());
	m_pSvgShow->resetBack();
}

void MainWindow::SLOT_OnHelpSelect()
{
	m_pHelpMessageBox->setVisible(true);
}

void MainWindow::setStatusMessage(QString msg)
{
	ui->statusBar->showMessage(msg);
}

void MainWindow::SLOT_on_m_selectBackFileBtn_clicked()
{
	m_lastBackPath = QFileDialog::getOpenFileName(this, tr("Open File"), m_lastBackPath, tr("svg,png (*.svg *.png *.jpg *.bmp *.tif *.tiff)"));
	if (m_lastBackPath.isNull())
	{
		return;
	}
	updateBackImag();
}

void MainWindow::updateBackImag()
{
	QString name = m_lastBackPath.split("/").last().split(".").first();
	if (m_lastBackPath.endsWith("svg"))
	{
		m_backSvgPathStr = m_lastBackPath;
		Config::Instance->SetBackPngPath("./" + name + ".png");
	}
	else
	{
		Config::Instance->SetBackPngPath(m_lastBackPath);
	}
	this->setWindowTitle(m_lastBackPath);
	m_lastTopPath = m_lastBackPath.split(".")[0] + "_Top.svg";
	Config::Instance->SetTopSvgPath(m_lastTopPath);
	m_pSaveTopImg->setSaveImagPath(m_lastBackPath.split(".")[0] + "_mask.bmp");
	ui->m_svgPath->setText(m_lastBackPath);
	ui->m_svgTopPath->setText(Config::Instance->GetTopSvgPath());
	if (m_pSvgShow != nullptr)
	{
		m_pSvgShow->resetBack();
	}
}

void MainWindow::SLOT_on_m_selectTopBtn_clicked()
{
	m_lastTopPath = QFileDialog::getOpenFileName(this, tr("Open File"), m_lastTopPath, tr("svg (*.svg)"));
	Config::Instance->SetTopSvgPath(m_lastTopPath);
	ui->m_svgTopPath->setText(m_lastTopPath);
	if (m_pSvgShow != nullptr)
	{
		m_pSvgShow->resetBack();
	}
}

void MainWindow::SLOT_on_m_loadFileBtn_clicked()
{
	QString backPngPath = Config::Instance->GetBackPngPath();
	if (!QFileInfo::exists(m_backSvgPathStr) && !QFileInfo::exists(backPngPath))
	{
		QMessageBox megBox(QMessageBox::Warning, QL("警告"), QL("底图不能为空!"), QMessageBox::Yes, this);
		megBox.setButtonText(QMessageBox::Yes, QL("确定"));
		megBox.exec();
		return;
	}

	ui->stackedWidget->setCurrentIndex(1);
	setStatusMessage(QL("正在加载图片,请等待..."));
	if (m_backSvgPathStr != nullptr)
	{
		m_pSvg = new SvgView();
		m_pSvg->openFile(MainWindow::m_backSvgPathStr);
		m_pSvg->SLOT_setViewBackground(true);
		QGraphicsScene *s = m_pSvg->scene();

		m_image = QPixmap(s->width(), s->height());
		m_image.fill(qRgba(255, 255, 255, 0));

		m_pPainter = new QPainter(&m_image);
		m_pPainter->setRenderHint(QPainter::Antialiasing);
		m_pPainter->begin(&m_image);
		s->render(m_pPainter);
		m_imageShow = m_image.copy();
		m_image.save(Config::Instance->GetBackPngPath(), nullptr, 100);
	}
	else if (QFileInfo::exists(Config::Instance->GetBackPngPath()))
	{

		QImage image;
		readImage(Config::Instance->GetBackPngPath(), image);
		m_imageShow = QPixmap::fromImage(image);
	}
	m_pSvgShow = new SvgViewShow(m_imageShow);
	connect(ui->m_handlerImag, &QAction::triggered, m_pSvgShow, &SvgViewShow::SLOT_OnHandlerImagSelect);
	m_pSvgShow->loadExtratItem();
	ui->m_svgEdit->setWidget(m_pSvgShow);

	ui->m_selectTopBtn->setVisible(false);
	ui->m_svgTopPath->setVisible(false);
	ui->menuBar->setVisible(true);

	setStatusMessage(QL("图片加载成功!"));
}

void MainWindow::SLOT_OnEditTextSelect()
{
	if (m_pSvgShow == nullptr)
		return;
	m_pSvgShow->changeEditState(EditType::EditType_Text);
}

void MainWindow::SLOT_OnEditPolygonSelect()
{
	if (m_pSvgShow == nullptr)
		return;
	m_pSvgShow->changeEditState(EditType::EditType_Polygon);
}

void MainWindow::SLOT_OnEditMileSelect()
{
	if (m_pSvgShow == nullptr)
		return;
	m_pSvgShow->changeEditState(EditType::EditType_Mile);
}

void MainWindow::SLOT_OnForbidEditSelect()
{
	if (m_pSvgShow == nullptr)
		return;
	m_pSvgShow->changeEditState(EditType::EditType_None);
}

void MainWindow::SLOT_OnEditLineSelect()
{
	if (m_pSvgShow == nullptr)
		return;
	//m_dialogLine->exec();
	m_pSvgShow->changeEditState(EditType::EditType_Line);
}

//
// (x - 1, y + 1) (x, y + 1) (x + 1, y + 1)
// (x - 1, y)     (x, y)     (x + 1, y)
// (x - 1, y - 1) (x, y - 1) (x + 1, y - 1)
//
QRgb checkColor(QImage src, QPoint center)
{
	return src.pixel(center);
	// 将周围九个点RGB值取出来
	QList<QRgb> colorList;
	for (int startX = (center.x() > 0 ? center.x() : 1) - 1; startX <= center.x() + 1; ++startX)
	{
		for (int startY = (center.y() > 0 ? center.y() : 1) - 1; startY <= center.y() + 1; ++startY)
		{
			colorList.push_back(src.pixel(startX, startY));
		}
	}

	// 点颜色出现次数计数
	QList<uint> cntList;
	for (auto cellLeft : colorList)
	{
		uint cnt = 0;
		for (auto cellRight : colorList)
		{
			if (cellLeft == cellRight)
			{
				++cnt;
			}
		}
		cntList.push_back(cnt);
	}

	// 计算颜色次数最多的点
	auto ptr = std::max_element(cntList.begin(), cntList.end());
	uint index = cntList.indexOf(*ptr);
	return colorList[index];
}

std::tuple<QImage, QImage> MainWindow::modifyImag(QImage image, modifyImagColor type)
{
	int height = image.height();
	int width = image.width();
	QImage ret(width, height, QImage::Format_Indexed8);
	QImageReader reader(Config::Instance->GetBackPngPath());
	QImage srcImag;
	if (!readImage(Config::Instance->GetBackPngPath(), srcImag))
	{
		assert(false);
	}
	srcImag = srcImag.convertToFormat(QImage::Format_RGB888);
	QImage retRgb(srcImag.width(), srcImag.height(), QImage::Format_RGB888);
	int counts = srcImag.width() * srcImag.height();
	for (int i = 0; i < counts; ++i)
	{
		if (false)
		{
			retRgb.bits()[i * 3 + 0] = srcImag.bits()[i];
			retRgb.bits()[i * 3 + 1] = srcImag.bits()[i];
			retRgb.bits()[i * 3 + 2] = srcImag.bits()[i];
		}
		else
		{
			retRgb.bits()[i * 3 + 0] = srcImag.bits()[i * 3 + 0];
			retRgb.bits()[i * 3 + 1] = srcImag.bits()[i * 3 + 1];
			retRgb.bits()[i * 3 + 2] = srcImag.bits()[i * 3 + 2];
		}
	}

	ret.setColorCount(256);
	for (int i = 0; i < 256; i++)
	{
		ret.setColor(i, qRgb(i, i, i));
	}
	switch (image.format())
	{
	case QImage::Format_Indexed8:
		for (int i = 0; i < height; i++)
		{
			const uchar *pSrc = (uchar *)image.constScanLine(i);
			uchar *pDest = (uchar *)ret.scanLine(i);
			memcpy(pDest, pSrc, width);
		}
		break;
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32:
	case QImage::Format_ARGB32_Premultiplied:
		for (int i = 0; i < height; i++)
		{
			uchar *pDest = (uchar *)ret.scanLine(i);
			uchar *pDestRgb = (uchar *)retRgb.scanLine(i);
			for (int j = 0; j < width; j++)
			{
				if (image.pixel(j, i) == -1)
				{
					pDest[j] = 0;
					continue;
				}
				QRgb color = checkColor(image, QPoint(j, i));
				switch (type)
				{
				case ToRedImag:
				{
					pDest[j] = (color >> 16 & 0xff);
					pDestRgb[j * 3] = maxChannelValue;
					if (pDestRgb[j * 3 + 1] > maxChannelValue)
					{
						pDestRgb[j * 3 + 1] = minChannelValue;
					}
					if (pDestRgb[j * 3 + 2] > maxChannelValue)
					{
						pDestRgb[j * 3 + 2] = minChannelValue;
					}
				}
				break;
				case ToGreenImag:
				{
					pDest[j] = (color >> 8 & 0xff);
					pDestRgb[j * 3 + 1] = maxChannelValue;
				}
				break;
				case ToBlueImag:
				{
					pDest[j] = (color & 0xff);
					pDestRgb[j * 3 + 2] = maxChannelValue;
				}
				break;
				case ToGrayImag:
				{
					ret.setPixel(j, i, image.pixel(i, j));
				}
				break;
				default:
					break;
				}
			}
		}
		break;
	}
	return std::make_tuple(ret, retRgb);;
}

void MainWindow::SLOT_OnSaveTopImgSelect()
{
	if (m_pSvgShow == nullptr)
		return;
	if (!QFileInfo::exists(Config::Instance->GetTopSvgPath()))
	{
		setStatusMessage(QL("请先绘图，再保存!"));
		return;
	}
	std::tuple<QString, QString> ret = m_pSaveTopImg->getSaveAAttribute();
	QString path = std::get<0>(ret);
	if (path.isEmpty())
	{
		setStatusMessage(QL("请先选择保存路径，再保存!"));
		return;
	}
	QString color = std::get<1>(ret);
	saveTopImage(path, color);
}

bool MainWindow::saveTopImage(QString path, QString color)
{
	modifyImagColor type = ToRawImag;

	if (m_str2ColorTypeMap.contains(color))
	{
		type = m_str2ColorTypeMap[color];
	}
	SvgView* svg = new SvgView();
	svg->openFile(Config::Instance->GetTopSvgPath());
	QImage image;
	readImage(Config::Instance->GetBackPngPath(), image);
	m_imageShow = QPixmap::fromImage(image);
	QPixmap imagSave = QPixmap(m_imageShow.width(), m_imageShow.height());
	imagSave.fill(qRgba(255, 255, 255, 0));

	QPainter* savePainter = new QPainter(&imagSave);
	savePainter->begin(&imagSave);

	XmlNodeAction::loadXml(m_pSvgShow->m_TipsPosList);
	for (auto value : m_pSvgShow->m_TipsPosList)
	{
		if (value.type == EditType::EditType_Polygon)
		{
			QVector<QPointF> posVec;
			for (auto pos : value.points)
			{
				QPointF *point = new QPointF(pos.x, pos.y);
				posVec.push_back(*point);
			}

            //savePainter->setBrush(value.color);
			savePainter->setPen(value.color);
            savePainter->drawPolygon(posVec);
		}
		else if (value.type == EditType::EditType_Line)
		{
			QVector<QPointF> posVec;
			for (int i = 0; i < value.points.size(); ++i)
			{
				// (起点，终点)
				// (A, B)
				// (B, C)
				// (C, D)  取A B C D;
				if (i == 0 || i % 2 == 1)
				{
					TipsWithPos::Point pos = value.points[i];
					QPointF *point = new QPointF(pos.x, pos.y);
					posVec.push_back(*point);
				}
			}

			QPen pen;
			pen.setColor(value.color);
			pen.setCapStyle(Qt::RoundCap);
			pen.setJoinStyle(Qt::RoundJoin);
			pen.setWidthF(value.strokeWidth.toDouble());
			savePainter->setPen(pen);
			for (int i = 1; i < posVec.size(); ++i)
			{
				savePainter->drawLine(posVec[i - 1], posVec[i]);
			}
		}
		else if (value.type == EditType::EditType_Text)
		{
		}
	}
	QImage imageGray;
	QImage imageRgb;
	if (!path.endsWith("bmp"))
	{
		path.append(".bmp");
	}
	if (type == ToRawImag)
	{
		imageGray = imagSave.toImage();
	}
	else
	{
		std::tuple<QImage, QImage> ret = modifyImag(imagSave.toImage(), type);
		imageGray = std::get<0>(ret);
		imageRgb = std::get<1>(ret);
	}
	imageGray.save(path, "bmp");
	imageRgb.save(path.split(".")[0] + "_rgb.bmp", "bmp");
	setStatusMessage(path);
	delete svg;
	svg = nullptr;
	return true;
}

QFileInfoList MainWindow::getFileList(QString path)
{
	QDir dir(path);
	QFileInfoList file_list = dir.entryInfoList(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
	QFileInfoList folder_list = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

	for (int i = 0; i < folder_list.count(); ++i)
	{
		QFileInfo info = folder_list.at(i);
		QString name = info.absoluteFilePath();
		QFileInfoList child_file_list = getFileList(name);
		file_list.append(child_file_list);
	}
	for (int i = 0; i < file_list.count(); ++i)
	{
		QFileInfo info = file_list.at(i);
		QString filter = info.suffix();
		if ((filter != "tif") && (filter != "png") && (filter != "bmp") && (filter != "jpg"))
		{
			file_list.removeAt(i);
		}
	}
	return file_list;
}

void MainWindow::SLOT_OnOpenFolderSelect()
{
	QString path = QFileDialog::getExistingDirectory(this, tr("Open File"), "./");
	m_imagePathList = getFileList(path);
	updateFileList();
}

void MainWindow::updateFileList()
{
	ui->m_imgFileList->setVisible(true);
	ui->m_imgFileList->clear();
	uint max = 0;
	for (uint i = 0; i < m_imagePathList.size(); ++i)
	{
		QString path = m_imagePathList[i].filePath();
		QString filename = path.split(".")[0];
		if (filename.endsWith("mask") || filename.endsWith("rgb"))
			continue;
		if (path.size() > max)
			max = path.size();
		auto cell = new QListWidgetItem(path);
		ui->m_imgFileList->addItem(cell);
	}
	ui->m_imgFileList->setMaximumWidth(max * 7);
	SLOT_OnFileItemClicked(ui->m_imgFileList->item(0));
}

void MainWindow::changeEditIcon(EditType type)
{
	if (type == EditType::EditType_None)
	{
		ui->m_editLine->setEnabled(true);
		ui->m_editPolygon->setEnabled(true);
		ui->m_editText->setEnabled(true);
		ui->m_editMile->setEnabled(true);
		ui->m_forbidEdit->setEnabled(false);
	}
	else if (type == EditType::EditType_Polygon)
	{
		ui->m_editLine->setEnabled(true);
		ui->m_editPolygon->setEnabled(false);
		ui->m_editText->setEnabled(true);
		ui->m_editMile->setEnabled(true);
		ui->m_forbidEdit->setEnabled(true);
	}
	else if (type == EditType::EditType_Text)
	{
		ui->m_editLine->setEnabled(true);
		ui->m_editPolygon->setEnabled(true);
		ui->m_editText->setEnabled(false);
		ui->m_editMile->setEnabled(true);
		ui->m_forbidEdit->setEnabled(true);
	}
	else if (type == EditType::EditType_Mile)
	{
		ui->m_editLine->setEnabled(true);
		ui->m_editPolygon->setEnabled(true);
		ui->m_editText->setEnabled(true);
		ui->m_editMile->setEnabled(false);
		ui->m_forbidEdit->setEnabled(true);
	}
	else if (type == EditType::EditType_Line)
	{
		ui->m_editLine->setEnabled(false);
		ui->m_editPolygon->setEnabled(true);
		ui->m_editText->setEnabled(true);
		ui->m_editMile->setEnabled(true);
		ui->m_forbidEdit->setEnabled(true);
	}
}

bool MainWindow::readImage(const QString path, QImage& dst)
{
	if (!QFileInfo::exists(path))
	{
		return false;
	}
	QByteArray pData;
	QFile *file = new QFile(path);
	file->open(QIODevice::ReadOnly);
	pData = file->readAll();
	dst.loadFromData(pData);
	delete file;
	file = nullptr;
	return true;
}

void MainWindow::setActionShowStatus()
{
	QFile loadFile(Config::Instance->GetShowConfigPath());
	if (!loadFile.open(QIODevice::ReadOnly))
	{
		qDebug() << "could't open projects json";
		return;
	}

	QByteArray allData = loadFile.readAll();
	loadFile.close();

	QJsonParseError json_error;
	QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));

	if (json_error.error != QJsonParseError::NoError)
	{
		qDebug() << "json error!";
		return;
	}

	QJsonObject rootObj = jsonDoc.object();
	if (rootObj.contains("MainWindow"))
	{
		QJsonObject subObj = rootObj.value("MainWindow").toObject();

		ui->m_save->setVisible(subObj["m_save"].toBool());
		ui->m_forbidEdit->setVisible(subObj["m_forbidEdit"].toBool());
		ui->m_editPolygon->setVisible(subObj["m_editPolygon"].toBool());
		ui->m_editText->setVisible(subObj["m_editText"].toBool());
		ui->m_editMile->setVisible(subObj["m_editMile"].toBool());
		ui->m_editLine->setVisible(subObj["m_editLine"].toBool());
		ui->m_saveTopImg->setVisible(subObj["m_saveTopImg"].toBool());
	}
}

void MainWindow::SLOT_OnFileItemClicked(QListWidgetItem* item)
{
	if (m_lastBackPath != "./")
	{
		std::tuple<QString, QString> ret = m_pSaveTopImg->getDefaultAAttribute();
		QString path = std::get<0>(ret);
		if (path.isEmpty())
		{
			setStatusMessage(QL("请先选择保存路径，再保存!"));
			return;
		}
		QString color = std::get<1>(ret);
		saveTopImage(path, color);
	}
	m_lastBackPath = item->text();
	updateBackImag();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
	return;
	if (event->key() == Qt::Key_F12)
	{
		for (uint i = 0; i < ui->m_imgFileList->count(); ++i)
		{
			SLOT_OnFileItemClicked(ui->m_imgFileList->item(i));
		}
	}
}
