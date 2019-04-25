#include <QSvgRenderer>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QGraphicsSvgItem>
#include <QPaintEvent>
#include <qmath.h>
#include <QToolTip>
#include <QPushButton>
#include <QSvgGenerator>
#include <QFileDialog>
#include <QTimer>
#include <QScrollBar>
#include <stdint.h>

#include "xmlnodeaction.h"
#include "svgviewshow.h"
#include "mainwindow.h"
#include "config.h"

#include <QDebug>
#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

SvgViewShow* SvgViewShow::Instance;
SvgViewShow::SvgViewShow(QPixmap image, QWidget *parent) :
    QGraphicsView(parent), m_pCurLineItem(nullptr), m_pCurRectItem(nullptr)
{
	Instance = this;
	setScene(new QGraphicsScene(this));
	setTransformationAnchor(AnchorUnderMouse);
	setViewportUpdateMode(FullViewportUpdate);
	// setDragMode(ScrollHandDrag);
	setMouseTracking(true);

	m_setMileCnt = 1;
	m_minX = UINT_FAST32_MAX;
	m_minY = UINT_FAST32_MAX;
	m_maxX = 0;
	m_maxY = 0;
	m_endEdit = false;
	setAcceptDrops(true);
	m_pixmap = image.copy();
	setMileInfo.Reset();
	initUI();
	QGraphicsScene *s = scene();
	s->addItem(m_pStartFlag);
	s->addItem(m_pEndFlag);
	s->addItem(m_pTextPosItem);
	s->addItem(m_pPixmapItem);
	s->addItem(m_pCurRectItem);
	registSignal2Slots();
	changeEditState(EditType::EditType_None);
}

SvgViewShow ::~SvgViewShow()
{
	delete m_pCurRectItem;
	m_pCurRectItem = nullptr;
	delete m_pPixmapItem;
	m_pPixmapItem = nullptr;
	delete m_pImage;
	m_pImage = nullptr;
	delete m_pTextPosItem;
	m_pTextPosItem = nullptr;
	delete m_pStartFlag;
	m_pStartFlag = nullptr;
    delete m_pEndFlag;
	m_pEndFlag = nullptr;
	delete m_pDialog;
	m_pDialog = nullptr;
	if (m_pCurLineItem != nullptr)
	{
		delete m_pCurLineItem;
		m_pCurLineItem = nullptr;
	}
}
void SvgViewShow::initUI()
{
    m_pDialog = new SetAttributeDialog(nullptr, nullptr, EditType::EditType_Text);
	m_pTimer = new QTimer();

	m_pMileDialog = new SetMileDialog();

	QString flagPath = Config::Instance->GetFlagPicPath();
	m_pStartFlag = new GraphicsPixmapItem(flagPath);
	m_pStartFlag->setVisible(false);

	m_pEndFlag = new GraphicsPixmapItem(flagPath);
	m_pEndFlag->setVisible(false);

	m_pTextPosItem = new QGraphicsPixmapItem();
	QPixmap imgPos(Config::Instance->GetTextPosPicPath());
	QPixmap retPos = imgPos.scaled(25, 25);
	m_pTextPosItem->setPixmap(retPos);
	m_pTextPosItem->setVisible(false);
	m_pTextPosItem->setZValue(3);

	m_pCurRectItem = new QGraphicsRectItem();
	m_pCurRectItem->setZValue(1);
	QPen pen(Qt::red, 2, Qt::DashLine);
	m_pCurRectItem->setPen(pen);

	m_pImage = new QPixmap(Config::Instance->GetBackPngPath());

	m_nodePix = QPixmap(Config::Instance->GetCrossPicPath());

	m_pPixmapItem = new QGraphicsPixmapItem();
	m_pPixmapItem->setPixmap(m_pixmap);
	m_pPixmapItem->setZValue(-1);
}

void SvgViewShow::registSignal2Slots()
{
	qRegisterMetaType<NodeAttribute>("NodeAttribute");
	qRegisterMetaType<QList<QString>>("QList<QString>");
	qRegisterMetaType<QVector<QPoint>>("QVector<QPoint>");
	qRegisterMetaType<TextTagType>("TextTagType");
	connect(this, SIGNAL(Remove(QList<QString>)),
		XmlNodeAction::Instance, SLOT(SLOT_Remove(QList<QString>)), Qt::DirectConnection);
	connect(this, SIGNAL(SIGNAL_Add(const QPoint&, const QPoint&, const qreal, const QSize)),
		XmlNodeAction::Instance, SLOT(SLOT_Add(const QPoint&, const QPoint&, const qreal, const QSize)), Qt::QueuedConnection);
	connect(this, SIGNAL(SIGNAL_Add(const QVector<QPoint>&, const NodeAttribute, const QSize)),
		XmlNodeAction::Instance, SLOT(SLOT_Add(const QVector<QPoint>&, const NodeAttribute, const QSize)), Qt::QueuedConnection);
	connect(this, SIGNAL(SIGNAL_Add(const QPointF&, const NodeAttribute, const QSize, TextTagType)),
		XmlNodeAction::Instance, SLOT(SLOT_Add(const QPointF&, const NodeAttribute, const QSize, TextTagType)), Qt::QueuedConnection);
	connect(this, SIGNAL(SIGNAL_Add(const QVector<QPoint>&, const NodeAttribute, const QSize, const double)),
		XmlNodeAction::Instance, SLOT(SLOT_Add(const QVector<QPoint>&, const NodeAttribute, const QSize, const double)), Qt::QueuedConnection);

    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(SLOT_onMouseSingleClick()));
}

void SvgViewShow::paintEvent(QPaintEvent *event)
{
	QGraphicsView::paintEvent(event);
}

void SvgViewShow::wheelEvent(QWheelEvent *event)
{
	qreal factor = qPow(1.2, event->delta() / 240.0);
	scale(factor, factor);
	event->accept();
}

void SvgViewShow::loadExtratItem()
{
	XmlNodeAction::loadXml(m_TipsPosList);
	QGraphicsScene *s = scene();
	removePolygon();
	removeText();
	removeLines();
	m_pCurRectItem->setRect(0, 0, 0, 0);
	for (auto value : m_TipsPosList)
	{
		if (value.type == EditType::EditType_Polygon)
		{
			QVector<QPointF> posVec;
			for (auto pos : value.points)
			{
				updateRectSize(pos);
				QPointF *point = new QPointF(pos.x, pos.y);
				posVec.push_back(*point);
			}
			GraphicsPolygonItem* item = new GraphicsPolygonItem(posVec, value.color, value.content, value.id);
			s->addItem(item);
			m_polygonMap[value.id] = item;
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
					updateRectSize(pos);
					QPointF *point = new QPointF(pos.x, pos.y);
					posVec.push_back(*point);
				}
			}
			GraphicsLineItem* item = new GraphicsLineItem(value.id, posVec, value.color, value.strokeWidth.toDouble());
			s->addItem(item);
			m_LineMap[value.id] = item;
		}
		else if (value.type == EditType::EditType_Text)
		{
			QPointF *point = new QPointF(value.points.first().x, value.points.first().y);
			if (value.content.startsWith(QL("起点")))
			{
				updateRectSize(value.points[0]);
				m_startMile = value.content.split(":")[1].split(" ");
				if (m_startMile.count() != 2)
					continue;
				m_pStartFlag->setPos(QPointF(value.points[0].x, value.points[0].y));
				m_pStartFlag->setVisible(true);
				m_pStartFlag->setDeclare(value.content);
				m_pStartFlag->setID(value.id);
				m_pMileDialog->setStartMile(m_startMile[0]);
				m_pMileDialog->setUnitText(m_startMile[1]);
				setMileInfo.startPos.setX(value.points[0].x);
				setMileInfo.startPos.setY(value.points[0].y);
				m_lastStartPos.setX(value.points[0].x);
				m_lastStartPos.setY(value.points[0].y);
			}
			else if (value.content.startsWith(QL("终点")))
			{
				m_endMile = value.content.split(":")[1].split(" ");
				updateRectSize(value.points[0]);
				if (m_endMile.count() != 2)
					continue;
				m_pEndFlag->setPos(QPointF(value.points[0].x, value.points[0].y));
				m_pEndFlag->setVisible(true);
				m_pEndFlag->setDeclare(value.content);
				m_pEndFlag->setID(value.id);
				m_pMileDialog->setEndMile(m_endMile[0]);
				setMileInfo.endPos.setX(value.points[0].x);
				setMileInfo.endPos.setY(value.points[0].y);
				m_lastEndPos.setX(value.points[0].x);
				m_lastEndPos.setY(value.points[0].y);
			}
			else
			{
				GraphicsTextItem *item = new GraphicsTextItem(*point, value.color, value.content, value.id, QString::number(value.front));
				m_textMap[value.id] = item;
				s->addItem(item);
			}
		}
	}
	update();
}

void SvgViewShow::keyPressEvent(QKeyEvent *event)
{
	// F5 刷新(2s CD)
	if (event->key() == Qt::Key_F5)
	{
		QDateTime now = QDateTime::currentDateTime();
		if ((now.toTime_t() - m_lastRefreshTime.toTime_t()) < 2)
		{
			return;
		}
		m_lastRefreshTime = QDateTime::currentDateTime();
		resetBack();
		MainWindow::Instance->setStatusMessage(QL("刷新成功!"));
	}
	// Ctrl+S 保存
	else if (event->modifiers() == Qt::ControlModifier)
	{
		if (event->key() == Qt::Key_S)
		{
			onSaveSvg();
		}
	}
}

void SvgViewShow::mouseReleaseEvent(QMouseEvent * event)
{
	QGraphicsView::mouseReleaseEvent(event);
	if (!(m_editType == EditType::EditType_Polygon || m_editType == EditType::EditType_Line) || m_endEdit)
		return;
	if (event->button() == Qt::RightButton)
	{
		back();
		mouseMoveEvent(event);
	}
	else
	{
		QPointF pos = mapToScene(event->pos());
		if (m_selectPointVec.count() >= 1)
		{
			addLine(m_selectPointVec.last(), pos);
		}
		//QGraphicsScene *s = scene();
		/*QGraphicsPixmapItem *pix = new QGraphicsPixmapItem();
		pix->setPixmap(m_nodePix);
		pix->setPos(QPointF(pos.x() - m_nodePix.width() / 2, pos.y() - m_nodePix.height() / 2));
		s->addItem(pix);
		m_nodeItem.push_back(pix);*/
		m_selectPointVec << pos.toPoint();
	}
	//event->accept();
}

void SvgViewShow::mousePressEvent(QMouseEvent * event)
{
	m_pos = mapToScene(event->pos());
	m_button = event->button();
	m_pTimer->start(50);
	QGraphicsView::mousePressEvent(event);
}

void SvgViewShow::SLOT_onMouseSingleClick()
{
	m_pTimer->stop();
	if (m_editType == EditType::EditType_Mile && m_button == Qt::LeftButton)
	{
		if (m_setMileCnt % 2)
		{
			setMileInfo.startPos.setX(m_pos.x());
			setMileInfo.startPos.setY(m_pos.y());
			m_pStartFlag->setPos(setMileInfo.startPos);
			m_pStartFlag->setVisible(true);
			MainWindow::Instance->setStatusMessage(QL("设置起点成功"));
		}
		else
		{
			setMileInfo.endPos.setX(m_pos.x());
			// 同一水平线
			setMileInfo.endPos.setY(setMileInfo.startPos.y());
			m_pEndFlag->setPos(setMileInfo.endPos);
			m_pEndFlag->setVisible(true);
			MainWindow::Instance->setStatusMessage(QL("设置终点成功"));
		}
		if ((setMileInfo.startPos.x() && setMileInfo.startPos.x() != m_lastStartPos.x()) && (setMileInfo.endPos.x() && setMileInfo.endPos.x() != m_lastEndPos.x()))
		{
			m_pMileDialog->clear();
			m_pMileDialog->exec();
		}
		++m_setMileCnt;
	}
	if (m_editType == EditType::EditType_Text && m_button == Qt::LeftButton)
	{
		if (m_TextPos.x() == 0 || m_TextPos.y() == 0)
		{
			m_TextPos = m_pos;
		}
		m_pTextPosItem->setPos(QPointF(m_TextPos.x() - 12.5, m_TextPos.y() - 12.5));
		m_pTextPosItem->setVisible(true);
		m_pDialog->exec();
		return;
	}
	if ((m_editType == EditType::EditType_Polygon || m_editType == EditType::EditType_Line) && m_selectPointVec.isEmpty() && m_button == Qt::LeftButton)
	{
		setCursor(Qt::CrossCursor);
	}
}

void SvgViewShow::mouseDoubleClickEvent(QMouseEvent * event)
{
	m_pTimer->stop();
	if (m_editType == EditType::EditType_Polygon)
	{
		if (event->button() == Qt::LeftButton && m_selectPointVec.count() > 2)
		{
			addLine(m_selectPointVec.first(), m_selectPointVec.last());
			MainWindow::Instance->m_pDialogPolygon->clear();
			MainWindow::Instance->m_pDialogPolygon->exec();
		}
	}
	else if (m_editType == EditType::EditType_Line)
	{
		if (event->button() == Qt::LeftButton && m_selectPointVec.count() >= 2)
		{
			/*addLine(m_selectPoint.first(), m_selectPoint.last());
			m_selectPoint.push_back(m_selectPoint.first());*/
			MainWindow::Instance->m_pDialogLine->exec();
		}
	}
	else
	{
		if (event->button() == Qt::LeftButton && m_editType == EditType::EditType_None)
		{
			QGraphicsView::mouseDoubleClickEvent(event);
		}
	}
	setCursor(Qt::ArrowCursor);
}

void SvgViewShow::mouseMoveEvent(QMouseEvent * event)
{
	QGraphicsView::mouseMoveEvent(event);
	/*QString tip = QString::number(event->pos().x()) + "/" + QString::number(event->pos().y());
	setToolTip(tip);*/
	if (!(m_editType == EditType::EditType_Polygon || m_editType == EditType::EditType_Line) || m_endEdit)
	{
		if (m_pCurLineItem != nullptr)
		{
			m_pCurLineItem->setVisible(false);
		}
	}
	QPointF pos = mapToScene(event->pos());

	// 多边形编辑 || 画线 显示实时画线
	if (m_editType == EditType_Polygon || m_editType == EditType_Line)
	{
		updateCurLine(pos);
	}
	// 滚动条实时滚动
	/*if (m_editType != EditType_None)
		updateScrollBar(pos);*/
}

void SvgViewShow::onSaveSvg()
{
	if (m_editType == EditType::EditType_Polygon)
	{
		if (m_selectPointVec.count() > 2)
		{
			NodeAttribute attr = MainWindow::Instance->m_pDialogPolygon->getAttribute();
			emit SIGNAL_Add(m_selectPointVec, attr, m_pPixmapItem->pixmap().size());
		}
		clearSelectStatus();
	}
	else if (m_editType == EditType::EditType_Line)
	{
		if (m_selectPointVec.count() >= 2)
		{
			NodeAttribute attr = MainWindow::Instance->m_pDialogLine->getAttribute();
			emit SIGNAL_Add(m_selectPointVec, attr, m_pPixmapItem->pixmap().size(), 0.0);
		}
		clearSelectStatus();
	}
	else if (m_editType == EditType::EditType_Text)
	{
		emit SIGNAL_Add(m_TextPos, m_pDialog->getAttribute(), m_pPixmapItem->pixmap().size(), TextTagType::Normal);
		m_pDialog->clear();
	}
	clearTmpItem();
	MainWindow::Instance->setStatusMessage(QL("保存成功!"));
}

void SvgViewShow::resetBack()
{
	if (!QFileInfo::exists(Config::Instance->GetBackPngPath()))
		return;
	QGraphicsScene *s = scene();
	s->removeItem(m_pPixmapItem);
	QImage image;
	QByteArray pData;
	QFile *file = new QFile(Config::Instance->GetBackPngPath());
	file->open(QIODevice::ReadOnly);
	pData = file->readAll();
	image.loadFromData(pData);
	*m_pImage = QPixmap::fromImage(image);
	m_pixmap = m_pImage->copy();
	m_pPixmapItem->setPixmap(m_pixmap);
	m_pPixmapItem->setZValue(-1);
	s->addItem(m_pPixmapItem);
	clearTmpItem();
	loadExtratItem();
	clearSelectStatus();
	removeTextPosFlag();
	s->setSceneRect(0, 0, m_pImage->width(), m_pImage->height());
	this->resize(m_pImage->width(), m_pImage->height());
	delete file;
	file = nullptr;
}

void SvgViewShow::addPolygon(const QColor color, const QString content)
{
	if (m_selectPointVec.count() < 3)
		return;
	QGraphicsScene *s = scene();
	QVector<QPointF> posVec;
	for (auto pos : m_selectPointVec)
	{
		QPointF *point = new QPointF(pos.x(), pos.y());
		posVec.push_back(*point);
	}
	QString maxKey = XmlNodeAction::getNewID();
	GraphicsPolygonItem* item = new GraphicsPolygonItem(posVec, color, content, maxKey);
	for (auto pos : posVec)
	{
		updateRectSize(TipsWithPos::Point(pos.x(), pos.y()));
	}
	s->addItem(item);
	m_polygonMap[maxKey] = item;
}

void SvgViewShow::addText(const QColor color, const QString content, const QString front)
{
	if (m_editType != EditType::EditType_Text || (m_TextPos.x() == 0 && m_TextPos.y() == 0))
		return;
	QGraphicsScene *s = scene();
	QString maxKey = XmlNodeAction::getNewID();
	GraphicsTextItem* item = new GraphicsTextItem(m_TextPos, color, content, maxKey, front);
	updateRectSize(TipsWithPos::Point(m_TextPos.x(), m_TextPos.y()));
	s->addItem(item);
	m_textMap[maxKey] = item;
}

void SvgViewShow::addLine(const QPoint start, const QPointF end)
{
	if (m_selectPointVec.count() < 1)
		return;
	QGraphicsScene *s = scene();
	QGraphicsLineItem *line = new QGraphicsLineItem(start.x(), start.y(), end.x(), end.y());
	QPen pen;
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setColor(QColor(255, 0, 0, 255));
	if (m_editType == EditType::EditType_Line)
	{
		NodeAttribute attr = MainWindow::Instance->m_pDialogLine->getAttribute();
		pen.setWidth(attr.strokeWidth.toDouble());
	}
	line->setPen(pen);
	m_tempLineList.push_back(line);
	s->addItem(line);
}

void SvgViewShow::addLine(const QColor color, const double width)
{
	QGraphicsScene *s = scene();
	QString maxKey = XmlNodeAction::getNewID();

	QVector<QPointF> posVec;
	for (auto pos : m_selectPointVec)
	{
		QPointF *point = new QPointF(pos.x(), pos.y());
		posVec.push_back(*point);
	}

	GraphicsLineItem *line = new GraphicsLineItem(maxKey, posVec, color, width);
	for (auto pos : posVec)
	{
		updateRectSize(TipsWithPos::Point(pos.x(), pos.y()));
	}
	m_LineMap[maxKey] = line;
	s->addItem(line);
}

void SvgViewShow::removePolygon()
{
	QGraphicsScene *s = scene();
	for (auto item : m_polygonMap)
	{
		s->removeItem(item);
		delete item;
		item = nullptr;
	}
	m_polygonMap.clear();
}

void SvgViewShow::removeText()
{
	QGraphicsScene *s = scene();
	for (auto item : m_textMap)
	{
		s->removeItem(item);
		delete item;
		item = nullptr;
	}
	m_textMap.clear();
}

void SvgViewShow::removeLines()
{
	QGraphicsScene *s = scene();
	for (auto item : m_LineMap)
	{
		s->removeItem(item);
		delete item;
		item = nullptr;
	}
	m_LineMap.clear();
}

// 撤回
void SvgViewShow::back()
{
	QGraphicsScene *s = scene();
	if (m_tempLineList.isEmpty())
	{
		if (!m_selectPointVec.isEmpty())
		{
			m_selectPointVec.clear();
		}
		if (!m_nodeItemList.isEmpty())
		{
			for (auto item : m_nodeItemList)
			{
				s->removeItem(item);
				delete item;
			}
			m_nodeItemList.clear();
		}
		return;
	}
	s->removeItem(m_tempLineList.last());
	delete m_tempLineList.last();
	/*s->removeItem(m_nodeItem.last());
	delete m_nodeItem.last();*/
	m_tempLineList.removeLast();
	m_selectPointVec.removeLast();
	//m_nodeItem.removeLast();
	repaint();
}

void SvgViewShow::clearSelectStatus()
{
	setCursor(Qt::ArrowCursor);
}

void SvgViewShow::addPolygonFrame(const QVector<QPointF> &points, const QColor color)
{
	QGraphicsScene *s = scene();
	uint count = points.count();
	for (uint index = 0; index < count; ++index)
	{
		QPointF start, end;
		if (index == (count - 1))
		{
			start = points[index];
			end = points[0];
		}
		else
		{
			start = points[index];
			end = points[index + 1];
		}
		int r, g, b;
		if (color.red() < 128) r = 255; else r = 0;
		if (color.green() < 128) g = 255; else g = 0;
		if (color.blue() < 128) b = 255; else b = 0;
		QGraphicsLineItem *line = new QGraphicsLineItem(start.x(), start.y(), end.x(), end.y());
		line->setPen(QPen(QColor(r, g, b, 255), 2));
		line->setZValue(1);
		m_selectPolygonRectList.push_back(line);
		s->addItem(line);
	}
}

void SvgViewShow::removeCurPolygonRect()
{
	QGraphicsScene *s = scene();
	for (auto line : m_selectPolygonRectList)
	{
		s->removeItem(line);
		delete line;
		line = nullptr;
	}
	m_selectPolygonRectList.clear();
}

void SvgViewShow::changeEditState(const EditType type)
{
	setCursor(Qt::ArrowCursor);
	if (m_editType == EditType::EditType_Polygon || m_editType == EditType::EditType_Line)
	{
		clearTmpItem();
	}
	else if (m_editType == EditType::EditType_Mile)
	{
		clearMilePos();
	}
	if (type == EditType::EditType_None)
	{
		clearMilePos();
		MainWindow::Instance->setStatusMessage(QL("请先选择编辑目标"));
	}
	else if (type == EditType::EditType_Polygon)
	{
		removeTextPosFlag();
		MainWindow::Instance->setStatusMessage(QL("编辑图形"));
		m_endEdit = false;
	}
	else if (type == EditType::EditType_Text)
	{
		MainWindow::Instance->setStatusMessage(QL("编辑文字标签"));
	}
	else if (type == EditType::EditType_Mile)
	{
		MainWindow::Instance->setStatusMessage(QL("编辑里程"));
	}
	else if (type == EditType::EditType_Line)
	{
		MainWindow::Instance->setStatusMessage(QL("编辑折线"));
	}
	else
		return;
	clearShowRectSize();
	setMileInfo.Reset();
	MainWindow::Instance->changeEditIcon(type);
	m_editType = type;
}

void SvgViewShow::saveMile(const qreal start, const qreal end, const QString unit, const bool isCheck)
{
	Q_UNUSED(isCheck);
	removeMileData();
	MainWindow::Instance->setStatusMessage(QL("开始编辑里程"));
	setMileInfo.startMile = start;
	setMileInfo.endMile = end;
	QGraphicsScene *s = scene();
	/*g_stepLen = (setMileInfo.endPos.x() - setMileInfo.startPos.x()) / (end - start);
	if (g_stepLen <= 0)
	return;
	if (isCheck)*/
	if (false)
	{
		qreal stepLen = Config::Instance->GetStepLen();
		uint all = (setMileInfo.endPos.x() - setMileInfo.startPos.x()) / stepLen;
		for (uint index = 0; index <= all; ++index)
		{
			uint mile = start + index;
			GraphicsTextItem* item = new GraphicsTextItem(QPointF(setMileInfo.startPos.x() + stepLen * index, setMileInfo.startPos.y()), Qt::black,
				mile % 10 ? QString::number(mile % 10) : ("YDK" + QString::number(mile / 10)), QString::number(m_textMap.lastKey().toInt() + 1), "9", true);
			item->setAcceptHoverEvents(true);
			s->addItem(item);
			m_textMap[QString::number(m_textMap.lastKey().toInt() + 1)] = item;
		}
		emit SIGNAL_Add(setMileInfo.startPos, setMileInfo.endPos, setMileInfo.startMile, m_pPixmapItem->pixmap().size());
	}
	else
	{
		NodeAttribute startAttr;
		QString maxKey = XmlNodeAction::getNewID();
		startAttr.content = QL("起点:") + QString::number(start) + " " + unit;
		m_pStartFlag->setDeclare(startAttr.content);
		m_pStartFlag->setID(maxKey);
		m_lastStartPos.setX(setMileInfo.startPos.x());
		m_lastStartPos.setY(setMileInfo.startPos.y());
		updateRectSize(TipsWithPos::Point(setMileInfo.startPos.x(), setMileInfo.startPos.y()));

		NodeAttribute endAttr;
		endAttr.content = QL("终点:") + QString::number(end) + " " + unit;
		m_pEndFlag->setDeclare(endAttr.content);
		m_pEndFlag->setID(QString::number(maxKey.toInt() + 1));
		m_lastEndPos.setX(setMileInfo.endPos.x());
		m_lastEndPos.setY(setMileInfo.endPos.y());
		updateRectSize(TipsWithPos::Point(setMileInfo.endPos.x(), setMileInfo.endPos.y()));

		emit SIGNAL_Add(setMileInfo.startPos, startAttr, m_pPixmapItem->pixmap().size(), TextTagType::StartMile);
		emit SIGNAL_Add(setMileInfo.endPos, endAttr, m_pPixmapItem->pixmap().size(), TextTagType::EndMile);
	}
}

void SvgViewShow::clearMileFlag()
{
	m_pStartFlag->setVisible(false);
	m_pStartFlag->setDeclare(nullptr);
	m_pEndFlag->setVisible(false);
	m_pEndFlag->setDeclare(nullptr);
	clearMilePos();
}

void SvgViewShow::clearMilePos()
{
	setMileInfo.startPos.setX(m_lastStartPos.x());
	setMileInfo.startPos.setY(m_lastStartPos.y());
	setMileInfo.endPos.setX(m_lastEndPos.x());
	setMileInfo.endPos.setY(m_lastEndPos.y());
	if (m_pStartFlag != nullptr && m_pEndFlag != nullptr)
	{
		if (m_lastEndPos.x() != 0 && m_lastEndPos.y() != 0)
		{
			m_pStartFlag->setPos(m_lastStartPos);
			m_pEndFlag->setPos(m_lastEndPos);
		}
		else
		{
			m_pStartFlag->setVisible(false);
			m_pEndFlag->setVisible(false);
		}
	}
	m_setMileCnt = 1;
}

void SvgViewShow::removeMileData()
{
	QList<QString> idList;
	if (m_pStartFlag != nullptr)
	{
		idList << m_pStartFlag->getID();
	}
	if (m_pEndFlag != nullptr)
	{
		idList << m_pEndFlag->getID();
	}
	emit Remove(idList);
}

void SvgViewShow::removeTextPosFlag()
{
	this->m_pTextPosItem->setVisible(false);
	this->m_TextPos.setX(0);
	this->m_TextPos.setY(0);
}

bool SvgViewShow::modifyItem(QString id, EditType type, NodeAttribute attr)
{
	if (type == EditType::EditType_Text && m_textMap.contains(id))
	{
		m_textMap[id]->modifyAttr(attr);
	}
	else if (type == EditType::EditType_Polygon && m_polygonMap.contains(id))
	{
		m_polygonMap[id]->modifyAttr(attr);
	}
	else if (type == EditType::EditType_Line && m_LineMap.contains(id))
	{
		m_LineMap[id]->modifyAttr(attr);
	}
	else
		return false;
	return true;
}

bool SvgViewShow::deleteItem(const QString id, const EditType type)
{
	if (type != EditType::EditType_None && type != EditType::EditType_Mile)
	{
		QGraphicsScene *s = scene();
		if (type == EditType::EditType_Text && m_textMap.contains(id))
		{
			s->removeItem(m_textMap[id]);
			delete m_textMap[id];
			m_textMap.remove(id);
		}
		else if (type == EditType::EditType_Polygon && m_polygonMap.contains(id))
		{
			s->removeItem(m_polygonMap[id]);
			delete m_polygonMap[id];
			m_polygonMap.remove(id);
		}
		else if (type == EditType::EditType_Line && m_LineMap.contains(id))
		{
			s->removeItem(m_LineMap[id]);
			delete m_LineMap[id];
			m_LineMap.remove(id);
		}
		else
			return false;
		return true;
	}
	else
		return false;
}

bool SvgViewShow::clearTmpItem()
{
	QGraphicsScene *s = scene();
	while (!m_tempLineList.isEmpty())
	{
		s->removeItem(m_tempLineList.last());
		delete m_tempLineList.last();
		m_tempLineList.removeLast();
		if (!m_selectPointVec.isEmpty())
		{
			m_selectPointVec.removeLast();
		}
	}

	while (!m_nodeItemList.isEmpty())
	{
		s->removeItem(m_nodeItemList.last());
		delete m_nodeItemList.last();
		m_nodeItemList.removeLast();
	}

	m_selectPointVec.clear();
	return true;
}

bool SvgViewShow::clearShowRectSize()
{
	m_rectStart.setX(0);
	m_rectStart.setY(0);
	return true;
}

void SvgViewShow::updateCurLine(const QPointF pos)
{
	if (m_selectPointVec.count() <= 0)
	{
		if (m_pCurLineItem != nullptr)
		{
			m_pCurLineItem->setVisible(false);
		}
		return;
	}
	if (m_pCurLineItem == nullptr)
	{
		QGraphicsScene *s = scene();
		m_pCurLineItem = new QGraphicsLineItem(m_selectPointVec.last().x(), m_selectPointVec.last().y(), pos.x(), pos.y());
		m_pCurLineItem->setZValue(1);
		s->addItem(m_pCurLineItem);
	}

	QPen pen;
	if (m_editType == EditType::EditType_Line)
	{
		NodeAttribute attr = MainWindow::Instance->m_pDialogLine->getAttribute();
		pen.setWidth(attr.strokeWidth.toDouble());
	}
	pen.setColor(QColor(255, 0, 0));
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	m_pCurLineItem->setPen(pen);
	m_pCurLineItem->setVisible(true);
	m_pCurLineItem->setLine(m_selectPointVec.last().x(), m_selectPointVec.last().y(), pos.x(), pos.y());
}

void SvgViewShow::updateCurRect(const QPointF pos)
{
	// 暂时屏蔽
	return;
	if (m_rectStart.x() == 0 && m_rectStart.y() == 0)
	{
		if (m_pCurRectItem)
		{
			m_pCurRectItem->setVisible(false);
		}
		return;
	}
	if (m_pCurRectItem == nullptr)
	{
		QGraphicsScene *s = scene();
		m_pCurRectItem = new QGraphicsRectItem();
		m_pCurRectItem->setZValue(1);
		s->addItem(m_pCurRectItem);
	}

	QPen pen;
	pen.setColor(QColor(255, 0, 0, 255));
	m_pCurRectItem->setPen(pen);
	m_pCurRectItem->setVisible(true);
	m_pCurRectItem->setRect(m_rectStart.x(), m_rectStart.y(), pos.x() - m_rectStart.x(), pos.y() - m_rectStart.y());
}

void SvgViewShow::updateScrollBar(const QPointF pos)
{
	int barWidth = this->horizontalScrollBar()->width();
	int barHeight = this->verticalScrollBar()->height();
	int curW = this->horizontalScrollBar()->value() + barWidth;
	int curH = this->verticalScrollBar()->value() + barHeight;
	double rateW = (curW - pos.x())*1.0 / barWidth;
	double rateH = (curH - pos.y())*1.0 / barHeight;

	if (rateW > 0.8)
	{
		this->horizontalScrollBar()->setSliderPosition(this->horizontalScrollBar()->value() - 20);
	}
	else if (rateW < 0.2)
	{
		this->horizontalScrollBar()->setSliderPosition(this->horizontalScrollBar()->value() + 20);
	}
	if (rateH > 0.8)
	{
		this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->value() - 20);
	}
	else if (rateH < 0.2)
	{
		this->verticalScrollBar()->setSliderPosition(this->verticalScrollBar()->value() + 20);
	}
}

void SvgViewShow::updateRectSize(const TipsWithPos::Point pos)
{
	// 暂时屏蔽
	return;
	if (pos.x > m_maxX)
		m_maxX = pos.x;
	if (pos.y > m_maxY)
		m_maxY = pos.y;
	if (pos.x < m_minX)
		m_minX = pos.x;
	if (pos.y < m_minY)
		m_minY = pos.y;
	m_pCurRectItem->setRect(m_minX, m_minY, m_maxX - m_minX, m_maxY - m_minY);
}

void SvgViewShow::SLOT_OnHandlerImagSelect()
{
	changeEditState(EditType_Polygon);
	vector<vector<_out_pt>> out;
	QString backPngPath = Config::Instance->GetBackPngPath();
    int ret = GetAutoAnnoInfo(const_cast<char*>(backPngPath.toStdString().c_str()), &out, 50);
	if (ret != 0)
		return;
	for (auto p : out)
	{
		for (auto cell : p)
		{
			QPoint point(cell.x, cell.y);
			m_selectPointVec.push_back(point);
		}
		onSaveSvg();
	}
	changeEditState(EditType_None);
}
