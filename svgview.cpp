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

#include "svgview.h"

#define  QT_NO_OPENGL

#include <QDebug>
#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

SvgView::SvgView(QWidget *parent) : QGraphicsView(parent), m_svgItem(nullptr), m_renderer(Native), m_backgroundItem(nullptr), m_outlineItem(nullptr)
{
	setScene(new QGraphicsScene(this));
	setTransformationAnchor(AnchorUnderMouse);
	setDragMode(ScrollHandDrag);
	setViewportUpdateMode(FullViewportUpdate);

	setRenderHint(QPainter::Antialiasing, false);
	setRenderHint(QPainter::SmoothPixmapTransform, false);
}

void SvgView::drawBackground(QPainter *p, const QRectF &)
{
	p->save();
	p->resetTransform();
	p->drawTiledPixmap(viewport()->rect(), backgroundBrush().texture());
	p->restore();
}

QSize SvgView::svgSize() const
{
	return m_svgItem ? m_svgItem->boundingRect().size().toSize() : QSize();
}

bool SvgView::openFile(const QString &fileName)
{
	QGraphicsScene *s = scene();
	const bool drawBackground = (m_backgroundItem ? m_backgroundItem->isVisible() : false);
	const bool drawOutline = (m_outlineItem ? m_outlineItem->isVisible() : true);

	QScopedPointer<QGraphicsSvgItem> svgItem(new QGraphicsSvgItem(fileName));
	if (!svgItem->renderer()->isValid())
		return false;

	s->clear();
	resetTransform();
	m_svgItem = svgItem.take();
	m_svgItem->setFlags(QGraphicsItem::ItemClipsToShape);
	m_svgItem->setCacheMode(QGraphicsItem::NoCache);
	m_svgItem->setZValue(0);

	// 背景
	if (m_backgroundItem)
	{
		s->removeItem(m_backgroundItem);
		delete m_backgroundItem;
	}
	m_backgroundItem = new QGraphicsRectItem(m_svgItem->boundingRect());
	m_backgroundItem->setBrush(Qt::white);
	m_backgroundItem->setPen(Qt::NoPen);
	m_backgroundItem->setVisible(drawBackground);
	m_backgroundItem->setZValue(-1);

	// 边界线
	if (m_outlineItem)
	{
		s->removeItem(m_outlineItem);
		delete m_outlineItem;
	}
	m_outlineItem = new QGraphicsRectItem(m_svgItem->boundingRect());
	QPen outline(Qt::black, 2, Qt::DashLine);
	outline.setCosmetic(true);
	m_outlineItem->setPen(outline);
	m_outlineItem->setBrush(Qt::NoBrush);
	m_outlineItem->setVisible(drawOutline);
	m_outlineItem->setZValue(1);

	s->addItem(m_svgItem);
	s->addItem(m_backgroundItem);
	//s->addItem(m_outlineItem);
	s->setSceneRect(m_outlineItem->boundingRect().adjusted(0, 0, 0, 0));
	update();
	return true;
}

void SvgView::setRenderer(RendererType type)
{
	m_renderer = type;

	if (m_renderer == OpenGL) {
#ifndef QT_NO_OPENGL
		setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif
	}
	else {
		setViewport(new QWidget);
	}
}

void SvgView::SLOT_setHighQualityAntialiasing(bool highQualityAntialiasing)
{
#ifndef QT_NO_OPENGL
	setRenderHint(QPainter::HighQualityAntialiasing, highQualityAntialiasing);
#else
	Q_UNUSED(highQualityAntialiasing);
#endif
}

void SvgView::SLOT_setViewBackground(bool enable)
{
	if (!m_backgroundItem)
		return;
	m_backgroundItem->setVisible(enable);
}

void SvgView::paintEvent(QPaintEvent *event)
{
	if (m_renderer == Image) {
		if (m_image.size() != viewport()->size()) {
			m_image = QImage(viewport()->size(), QImage::Format_ARGB32_Premultiplied);
		}

		QPainter imagePainter(&m_image);
		QGraphicsView::render(&imagePainter);
		imagePainter.end();

		QPainter p(viewport());
		p.drawImage(0, 0, m_image);
	}
	else
	{
		QGraphicsView::paintEvent(event);
	}
}

void SvgView::wheelEvent(QWheelEvent *event)
{
	qreal factor = qPow(1.2, event->delta() / 240.0);
	scale(factor, factor);
	event->accept();
}

QSvgRenderer *SvgView::renderer() const
{
	if (m_svgItem)
		return m_svgItem->renderer();
	return nullptr;
}
