#pragma once
#ifndef SVGVIEWSHOW_H
#define SVGVIEWSHOW_H

#include <QGraphicsView>
#include <QDomNode>
#include <QPushButton>
#include <QTime>

#include "item.h"
#include "svgview.h"
#include "setattributedialog.h"
#include "setmiledialog.h"
#include "common.h"
#include "Libraries/include/imgAnno.h"

QT_BEGIN_NAMESPACE
class QGraphicsSvgItem;
class QSvgRenderer;
class QWheelEvent;
class QPaintEvent;
class GraphicsRectItem;
class GraphicsPolygonItem;
class GraphicsLineItem;
QT_END_NAMESPACE

class SvgViewShow : public QGraphicsView
{
	Q_OBJECT
public:
	explicit SvgViewShow(QPixmap image, QWidget *parent = nullptr);
	~SvgViewShow();
public:
	static SvgViewShow* Instance;
	// 多边形属性集合
	QList<TipsWithPos> m_TipsPosList;
	// 重置背景
	void resetBack();
	// 增加多边形边框
	void addPolygonFrame(const QVector<QPointF> &points, const QColor color);
	// 保存
	void onSaveSvg();
	// 移除选择标记框
	void removeCurPolygonRect();
	// 添加多边形
	void addPolygon(const QColor color, const QString content);
	// 增加文字标签
	void addText(const QColor color, const QString content, const QString front);
	// 添加永久线段
	void addLine(const QColor color, const double width);
	// 修改编辑状态
	void changeEditState(const EditType type);
	// 保存里程编辑
	void saveMile(const qreal start, const qreal end, const QString unit, const bool isCheck = true);
	// 清除里程标记
	void removeMileData();
	// 设置点属性对话框
	SetMileDialog *m_pMileDialog;
	// 清除文本位置标签
	void removeTextPosFlag();
	// 清除里程坐标
	void clearMilePos();
	// 修改Item属性
	bool modifyItem(const QString id, const EditType type, const NodeAttribute attr);
	// 移除Item
	bool deleteItem(const QString id, const EditType type);
	// 清除临时Item
	bool clearTmpItem();
	// 清除里程标记
	void clearMileFlag();
	// 加载附加顶层图元
	void loadExtratItem();

	void insertPoint(std::vector<_out_pt> src)
	{
		for (auto p : src)
		{
			QPoint point(p.x, p.y);
			m_selectPointVec.push_back(point);
		}
	}
signals:
	void SIGNAL_Add(const QPoint&, const QPoint&, const qreal, const QSize);
	void SIGNAL_Add(const QVector<QPoint>&, const NodeAttribute, const QSize);
	void SIGNAL_Add(const QPointF&, const NodeAttribute, const QSize, TextTagType);
	void SIGNAL_Add(const QVector<QPoint>&, const NodeAttribute, const QSize, const double strokeWidth);
	void Remove(QList<QString>);
private slots:
	void SLOT_onMouseSingleClick();
public slots:
	void SLOT_OnHandlerImagSelect();
protected:
	void keyPressEvent(QKeyEvent *event) override;
	void wheelEvent(QWheelEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	void mouseReleaseEvent(QMouseEvent * event) override;
	void mousePressEvent(QMouseEvent * event) override;
	void mouseMoveEvent(QMouseEvent * event) override;
	void mouseDoubleClickEvent(QMouseEvent * event) override;
private:
	// 当前背景图片
	QPixmap m_pixmap;
	// 背景图元
	QGraphicsPixmapItem* m_pPixmapItem;
	// 当前画线
	QGraphicsLineItem* m_pCurLineItem;
	// 当前画线
	QGraphicsRectItem* m_pCurRectItem;
	// 开始编辑标志位
	//bool m_startEdit;
	// 设置点属性对话框
	SetAttributeDialog *m_pDialog;
	// 文本编辑坐标
	QPointF m_TextPos;
	// 单双击时间区分定时器
	QTimer *m_pTimer;
	// 单击坐标及按键
	QPointF m_pos;
	Qt::MouseButton m_button;
	// 选择显示区域起点
	QPoint m_rectStart;
	// 编辑类型
	EditType m_editType;
	// 编辑结束标志位
	bool m_endEdit;
	// 设置里程属性
	SetMileWithPos setMileInfo;
	// 最后一次刷新时间
	QDateTime m_lastRefreshTime;
	// 里程起点位置标记
	GraphicsPixmapItem *m_pStartFlag;
	// 里程终点位置标记
	GraphicsPixmapItem *m_pEndFlag;
	// 文本标签框位置标记
	QGraphicsPixmapItem *m_pTextPosItem;
	// 里程起点公里
	QStringList m_startMile;
	// 里程终点公里
	QStringList m_endMile;
	// 里程起始点
	QPoint m_lastStartPos;
	// 里程终点
	QPoint m_lastEndPos;
	// 背景缓存图
	QPixmap *m_pImage;
	// 设置里程计数器,奇数设置起点，偶数设置终点
	uint m_setMileCnt;
	// 节点图片
	QPixmap m_nodePix;
	// 多边形
	QMap<QString, GraphicsPolygonItem *> m_polygonMap;
	// 文字标签
	QMap<QString, GraphicsTextItem *> m_textMap;
	// 实时画线集合
	QList<QGraphicsLineItem*> m_tempLineList;
	// 线段集合
	QMap<QString, GraphicsLineItem*> m_LineMap;
	// 多边形选中框
	QList<QGraphicsLineItem*> m_selectPolygonRectList;
	// 当前编辑的点
	QVector<QPoint> m_selectPointVec;
	// 节点图片
	QList<QGraphicsPixmapItem*> m_nodeItemList;
	// 顶层矩形框大小
	uint m_minX;
	uint m_minY;
	uint m_maxX;
	uint m_maxY;
private:
	// 清除当前选中的点
	void clearSelectStatus();
	// 添加临时实时线段
	void addLine(const QPoint start, const QPointF end);
	// 移除多边形
	void removePolygon();
	// 移除文字标签
	void removeText();
	// 移除线段
	void removeLines();
	// 撤回
	void back();
	// 清除当前选择区域
	bool clearShowRectSize();
    // 更新当前画线
	void updateCurLine(const QPointF pos);
	// 更新当前选择矩形框大小
	void updateCurRect(const QPointF pos);
	// 更新滚动条
	void updateScrollBar(const QPointF pos);
	// 更新顶图矩形大小
	void updateRectSize(const TipsWithPos::Point pos);
	// 初始化UI
	void initUI();
	// 注册信号槽
	void registSignal2Slots();
};

#endif // SVGVIEWSHOW_H
