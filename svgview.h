#pragma once
#ifndef SVGVIEW_H
#define SVGVIEW_H

#include <QGraphicsView>
#include <QDomNode>
#include <QPushButton>

QT_BEGIN_NAMESPACE
class QGraphicsSvgItem;
class QSvgRenderer;
class QWheelEvent;
class QPaintEvent;
class GraphicsRectItem;
class GraphicsPolygonItem;
QT_END_NAMESPACE

class SvgView : public QGraphicsView
{
	Q_OBJECT
public:
	enum RendererType { Native, OpenGL, Image };

	explicit SvgView(QWidget *parent = nullptr);

	bool openFile(const QString &fileName);
	void setRenderer(RendererType type = Native);
	void drawBackground(QPainter *p, const QRectF &rect) override;

	QSize svgSize() const;
	QSvgRenderer *renderer() const;

public slots:
	void SLOT_setHighQualityAntialiasing(bool highQualityAntialiasing);
	void SLOT_setViewBackground(bool enable);
protected:
	void wheelEvent(QWheelEvent *event);
	void paintEvent(QPaintEvent *event);
private:
	QGraphicsSvgItem *m_svgItem;
    RendererType m_renderer;
	QGraphicsRectItem *m_backgroundItem;
	QGraphicsRectItem *m_outlineItem;

	QImage m_image;
};
#endif // SVGVIEW_H
