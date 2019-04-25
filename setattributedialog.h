#pragma once
#ifndef SETATTRIBUTEDIALOG_H
#define SETATTRIBUTEDIALOG_H

#include <QDialog>
#include "svgview.h"
#include "common.h"

namespace Ui {
	class SetAttributeDialog;
}

class SetAttributeDialog : public QDialog
{
	Q_OBJECT

public:
	explicit SetAttributeDialog(QString id, QWidget *parent = 0, EditType type = EditType::EditType_None);
	~SetAttributeDialog();
public:
	Ui::SetAttributeDialog *ui;
public:
	NodeAttribute getAttribute() const;
	void clear();
	void fillAttr(QString content, QColor color, QString front = nullptr, QString lineWidth = nullptr);
private:
	QString m_id;
	EditType m_type;
	QString m_content;
	QColor m_color;
	QString m_front;
	QString m_lineWidth;
private:
	bool diff();
	void initUI();
	void registerSignal2Slots();
private slots:
	void SLOT_OnSaveBtnClick();
	void SLOT_OnCancleBtnClick();
	void SLOT_OnDeleteBtnClick();
	void SLOT_OnSelectColorBtnClick();
protected:
	void closeEvent(QCloseEvent *event) override;
signals:
	void SIGNAL_Remove(const QString);
	void SIGNAL_UpdateNode(const QString, const NodeAttribute);
};

#endif // SETATTRIBUTEDIALOG_H
