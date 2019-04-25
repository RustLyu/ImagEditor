#include <QPainter>
#include <QRectF>
#include <QDebug>
#include <QGraphicsSceneHoverEvent>
#include <QtMath>
#include <QMouseEvent>
#include <QToolTip>

#include "item.h"
#include "svgviewshow.h"
#include "config.h"
#include "xmlnodeaction.h"

/// 不规则图形
GraphicsPolygonItem::GraphicsPolygonItem(const QVector<QPointF> point, const QColor color, const QString content, const QString id)
{
	m_pointVec = point;
	m_color = color;
	m_tips = content;
	m_id = id;
	setAcceptedMouseButtons(Qt::LeftButton);
    m_pDialog = new SetAttributeDialog(m_id, nullptr, EditType::EditType_Polygon);
	setAcceptHoverEvents(true);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setZValue(1);
	m_nodePix = QPixmap(Config::Instance->GetCrossPicPath());
	m_nodePix = m_nodePix.scaled(QSize(25,25), Qt::KeepAspectRatio);
}

QPainterPath GraphicsPolygonItem::shape() const
{
	QPainterPath *path = new QPainterPath();
	path->addPolygon(m_pointVec);
	return *path;
}

QRectF GraphicsPolygonItem::boundingRect() const
{
	std::vector<QPointF> tmp = m_pointVec.toStdVector();
	size_t count = tmp.size();
	if (count <= 1)
	{
		return QRectF(0, 0, 0, 0);
	}
	std::stable_sort(tmp.begin(), tmp.end(), CompairX);
	qreal xMax = tmp[0].x();
	qreal xMin = tmp[count - 1].x();
	std::stable_sort(tmp.begin(), tmp.end(), CompairY);
	qreal yMax = tmp[0].y();
	qreal yMin = tmp[count - 1].y();
	return QRectF(xMin, yMin, xMax - xMin, yMax - yMin);
}

void GraphicsPolygonItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	//painter->setBrush(m_color);
	painter->setPen(m_color);
	painter->setRenderHint(QPainter::Antialiasing);
	painter->drawPolygon(m_pointVec);
//	for (auto pos : m_pointVec)
	{
		//painter->drawPixmap(QPointF(pos.x() - m_nodePix.width() / 2, pos.y() - m_nodePix.height() / 2), m_nodePix);
	}
}

void GraphicsPolygonItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);
	QToolTip::showText(cursor().pos(), m_tips);
}

void GraphicsPolygonItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
	Q_UNUSED(event);
	QToolTip::hideText();
}

size_t getMostNearPoint(QVector<QPointF> dataVec, QPointF pos)
{
	float dis = 9999999999;
    size_t ret = 0;
    for (size_t index = 0; index < dataVec.size(); ++index)
	{
		QPointF data = dataVec[index];
		float absD = pow(data.x() - pos.x(), 2) + pow(data.y() - pos.y(), 2);
		if (dis > absD)
		{
			ret = index;
			dis = absD;
		}
	}
	return ret;
}

void GraphicsPolygonItem::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
	if (event->buttons() == Qt::LeftButton)
	{
		QPointF pos = event->pos();
        size_t index = getMostNearPoint(m_pointVec, pos);
		m_pointVec[index] = pos;
		event->accept();
		update();
		XmlNodeAction::SLOT_UpdatePoints(m_id, m_pointVec, EditType_Polygon);
	}
}

void GraphicsPolygonItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	m_pDialog->fillAttr(m_tips, m_color);
	if (!m_pDialog->isVisible())
	{
		SvgViewShow::Instance->addPolygonFrame(m_pointVec, m_color);
		m_pDialog->exec();
	}
}

void GraphicsPolygonItem::modifyAttr(const NodeAttribute attr)
{
	m_tips = attr.content;
	m_color.setRed(attr.red.toInt());
	m_color.setGreen(attr.green.toInt());
	m_color.setBlue(attr.blue.toInt());
	m_color.setAlpha(attr.alpha.toInt());
}

GraphicsPolygonItem ::~GraphicsPolygonItem()
{
	delete m_pDialog;
	m_pDialog = nullptr;
}

/// 文字标签
GraphicsTextItem::GraphicsTextItem(const QPointF point, const QColor color, const QString content, const QString id, const QString front, const bool isMile)
{
	m_point = point;
	m_color = color;
	m_showText = content;
	m_id = id;
	m_front = front;
	setAcceptedMouseButtons(Qt::LeftButton);
    m_pDialog = new SetAttributeDialog(m_id, nullptr, EditType::EditType_Text);
	setAcceptHoverEvents(true);
	setZValue(2);
	m_width = 10;
	m_height = 5;
	m_isMile = isMile;
}

QRectF GraphicsTextItem::boundingRect() const
{
	return QRectF(m_point.x(), m_point.y() - m_height, m_width, m_height);
}

void GraphicsTextItem::setIsMile(const bool isMile)
{
	m_isMile = isMile;
}

bool GraphicsTextItem::IsMile()
{
	return m_isMile;
}

bool GraphicsTextItem::setContent(const QString content)
{
	m_showText = content;
	return true;
}

bool GraphicsTextItem::setPosition(const QPointF point)
{
	m_point = point;
	return true;
}

QString GraphicsTextItem::getID() const
{
	return m_id;
}

void GraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->setBrush(m_color);
	painter->setPen(m_color);
	QFont font = painter->font();
	font.setPointSize(m_front.toDouble());
	painter->setFont(font);
	painter->setRenderHint(QPainter::Antialiasing);
	painter->drawText(m_point, m_showText);
	if (!m_isMile)
	{
		QPixmap img(Config::Instance->GetCrossPicPath());
		QPixmap ret = img.scaled(25, 25);
		painter->drawPixmap(m_point.x() - 11, m_point.y() - 13, ret);
	}
	QFontMetrics fm = painter->fontMetrics();
	m_width = fm.width(m_showText);
	m_height = fm.height();
}

void GraphicsTextItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	if (!m_isMile)
	{
		m_pDialog->fillAttr(m_showText, m_color, m_front);
		if (!m_pDialog->isVisible())
		{
			m_pDialog->exec();
		}
	}
}

void GraphicsTextItem::modifyAttr(const NodeAttribute attr)
{
	m_showText = attr.content;
	m_color.setRed(attr.red.toInt());
	m_color.setGreen(attr.green.toInt());
	m_color.setBlue(attr.blue.toInt());
	m_color.setAlpha(attr.alpha.toInt());
	m_front = attr.front;
}

GraphicsTextItem ::~GraphicsTextItem()
{
	delete m_pDialog;
	m_pDialog = nullptr;
}

/// 图片
GraphicsPixmapItem::GraphicsPixmapItem(const QString filePath)
{
	m_pPoint = new QPointF(0, 0);
	m_imgPath = filePath;
	setAcceptedMouseButtons(Qt::LeftButton);
	setAcceptHoverEvents(true);
	setZValue(3);
	m_declare = "";
}

QRectF GraphicsPixmapItem::boundingRect() const
{
	return QRectF(m_pPoint->x(), m_pPoint->y(), 36, 36);
}

void GraphicsPixmapItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->setRenderHint(QPainter::Antialiasing);
	QPixmap img(m_imgPath);
	QPixmap ret = img.scaled(36, 36);
	painter->drawPixmap(*m_pPoint, ret);
	painter->drawText(*m_pPoint, m_declare);
}

void GraphicsPixmapItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	SvgViewShow::Instance->m_pMileDialog->exec();
}

void GraphicsPixmapItem::setPixmapPos(const QPointF pos)
{
	m_pPoint->setX(pos.x());
	m_pPoint->setY(pos.y());
}

void GraphicsPixmapItem::setDeclare(const QString declare)
{
	m_declare = declare;
}

QString GraphicsPixmapItem::getID() const
{
	return m_id;
}

void GraphicsPixmapItem::setID(const QString id)
{
	m_id = id;
}

GraphicsPixmapItem ::~GraphicsPixmapItem()
{
	delete m_pPoint;
	m_pPoint = nullptr;
}

/// 线段
GraphicsLineItem::GraphicsLineItem(const QString id, const QVector<QPointF> points, const QColor color, const double width)
{
	m_id = id;
	m_pointVec = points;
	m_color = color;
	m_strokeWidth = width;
	setAcceptedMouseButtons(Qt::LeftButton);
	setAcceptHoverEvents(true);
	setZValue(4);
    m_pDialog = new SetAttributeDialog(m_id, nullptr, EditType::EditType_Line);
}

QPainterPath GraphicsLineItem::shape() const
{
	QPainterPath *path = new QPainterPath();
	path->addPolygon(m_pointVec);
	QPainterPathStroker stroker;
	stroker.setWidth(m_strokeWidth);
	return stroker.createStroke(*path);
}

QRectF GraphicsLineItem::boundingRect() const
{
	std::vector<QPointF> tmp = m_pointVec.toStdVector();
	size_t count = tmp.size();
	if (count <= 1)
	{
		return QRectF(0, 0, 0, 0);
	}
	std::stable_sort(tmp.begin(), tmp.end(), CompairX);
	qreal xMax = tmp[0].x();
	qreal xMin = tmp[count - 1].x();
	std::stable_sort(tmp.begin(), tmp.end(), CompairY);
	qreal yMax = tmp[0].y();
	qreal yMin = tmp[count - 1].y();
	return QRectF(xMin, yMin, xMax - xMin, yMax - yMin);
}

void GraphicsLineItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
	Q_UNUSED(option);
	Q_UNUSED(widget);
	painter->setRenderHint(QPainter::HighQualityAntialiasing, true);
	QPen pen;
	pen.setColor(m_color);
	pen.setCapStyle(Qt::RoundCap);
	pen.setJoinStyle(Qt::RoundJoin);
	pen.setWidthF(m_strokeWidth);
	painter->setPen(pen);
	for (int i = 1; i < m_pointVec.size(); ++i)
	{
		painter->drawLine(m_pointVec[i - 1], m_pointVec[i]);
	}
}

void GraphicsLineItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
	Q_UNUSED(event);
	m_pDialog->fillAttr(nullptr, m_color, nullptr, QString::number(m_strokeWidth));
	if (!m_pDialog->isVisible())
	{
		m_pDialog->exec();
	}
}

void GraphicsLineItem::modifyAttr(const NodeAttribute attr)
{
	m_color.setRed(attr.red.toInt());
	m_color.setGreen(attr.green.toInt());
	m_color.setBlue(attr.blue.toInt());
	m_color.setAlpha(attr.alpha.toInt());
	m_strokeWidth = attr.strokeWidth.toDouble();
}

QString GraphicsLineItem::getID() const
{
	return m_id;
}

void GraphicsLineItem::setID(const QString id)
{
	m_id = id;
}

GraphicsLineItem ::~GraphicsLineItem()
{
	delete m_pDialog;
	m_pDialog = nullptr;
}
