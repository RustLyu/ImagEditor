#include <QDomElement>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QMessageBox>
#include <QColorDialog>

#include "setattributedialog.h"
#include "ui_setattributedialog.h"
#include "xmlnodeaction.h"
#include "svgviewshow.h"
#include "config.h"
#include "mainwindow.h"

SetAttributeDialog::SetAttributeDialog(QString id, QWidget *parent, EditType type) :QDialog(parent), ui(new Ui::SetAttributeDialog)
{
	ui->setupUi(this);

	m_id = id;
	m_type = type;

	registerSignal2Slots();
	initUI();
	clear();
}

SetAttributeDialog::~SetAttributeDialog()
{
	delete ui;
}

void SetAttributeDialog::initUI()
{
	ui->m_red->setToolTip("R:0-255");
	ui->m_green->setToolTip("G:0-255");
	ui->m_blue->setToolTip("B:0-255");
	ui->m_alpha->setToolTip("A:0-255");
	ui->m_strokeWidth->setToolTip("W:0-50");
	ui->m_red->setValidator(new QIntValidator(0, 255, this));
	ui->m_green->setValidator(new QIntValidator(0, 255, this));
	ui->m_blue->setValidator(new QIntValidator(0, 255, this));
	ui->m_alpha->setValidator(new QIntValidator(0, 255, this));
	ui->m_strokeWidth->setValidator(new QIntValidator(0, 50, this));
	ui->m_strokeWidth->setEnabled(false);

	QString title;
	if (m_id.isNull() || !m_id.compare("99999"))
	{
		if (m_type == EditType::EditType_Text)
		{
			title = QL("设置标签属性");
		}
		else if (m_type == EditType::EditType_Polygon)
		{
			title = QL("设置图形属性");
		}
		else if (m_type == EditType::EditType_Line)
		{
			title = QL("设置折线属性");
		}
		ui->m_deleteBtn->setVisible(false);
		ui->id_label->setVisible(false);
		ui->m_id->setVisible(false);
	}
	else
	{
		if (m_type == EditType::EditType_Text)
		{
			title = QL("修改标签属性");
		}
		else if (m_type == EditType::EditType_Polygon)
		{
			title = QL("修改图形属性");
		}
		else if (m_type == EditType::EditType_Line)
		{
			title = QL("修改折线属性");
		}
		ui->m_id->setEnabled(false);
		ui->m_id->setToolTip(QL("系统自动生成，不可修改！"));
	}
	// 根据类型设置显隐
	if (m_type == EditType::EditType_Text)
	{
		;
	}
	else if (m_type == EditType::EditType_Polygon)
	{
		ui->m_selectFrontSize->setVisible(false);
		ui->m_size->setVisible(false);
	}
	else if (m_type == EditType::EditType_Line)
	{
		ui->m_content->setVisible(false);
		ui->m_attrTitle->setVisible(false);
		ui->m_strokeWidth->setEnabled(true);
		ui->m_size->setVisible(false);
		ui->m_selectFrontSize->setVisible(false);
	}
	this->setWindowTitle(title);
	QStringList items;
	for (uint index = 6; index <= 12; ++index)
	{
		items << QString::number(index);
	}
	for (uint index = 14; index <= 28; index += 2)
	{
		items << QString::number(index);
	}
	items << "36" << "48" << "72";
	ui->m_selectFrontSize->addItems(items);
	ui->m_selectFrontSize->setCurrentText("9");
}

void SetAttributeDialog::registerSignal2Slots()
{
	connect(ui->m_saveBtn, SIGNAL(clicked()), this, SLOT(SLOT_OnSaveBtnClick()));
	connect(ui->m_cancleBtn, SIGNAL(clicked()), this, SLOT(SLOT_OnCancleBtnClick()));
	connect(ui->m_deleteBtn, SIGNAL(clicked()), this, SLOT(SLOT_OnDeleteBtnClick()));
	connect(ui->m_colorSelect, SIGNAL(clicked()), this, SLOT(SLOT_OnSelectColorBtnClick()));
	qRegisterMetaType<NodeAttribute>("NodeAttribute");
	connect(this, SIGNAL(SIGNAL_Remove(const QString)), XmlNodeAction::Instance, SLOT(SLOT_Remove(const QString)), Qt::QueuedConnection);
	connect(this, SIGNAL(SIGNAL_UpdateNode(const QString, const NodeAttribute)), XmlNodeAction::Instance, 
		SLOT(SLOT_Update(const QString, const NodeAttribute)), Qt::QueuedConnection);
}

NodeAttribute SetAttributeDialog::getAttribute() const
{
	NodeAttribute attr;
	attr.content = ui->m_content->toPlainText().trimmed();
	attr.red = ui->m_red->text();
	attr.green = ui->m_green->text();
	attr.blue = ui->m_blue->text();
	attr.alpha = ui->m_alpha->text();
	attr.strokeWidth = ui->m_strokeWidth->text();
	attr.front = ui->m_selectFrontSize->currentText();
	return attr;
}

void SetAttributeDialog::SLOT_OnSaveBtnClick()
{
	if (!diff())
	{
		this->close();
		return;
	}
	NodeAttribute attr = this->getAttribute();
	if (!m_id.isNull())
	{
		emit SIGNAL_UpdateNode(m_id, attr);
		SvgViewShow::Instance->modifyItem(m_id, m_type, attr);
	}
	else
	{
		SvgViewShow::Instance->removeCurPolygonRect();
		QColor color(attr.red.toInt(), attr.green.toInt(), attr.blue.toInt(), attr.alpha.toInt());
		if (m_type == EditType::EditType_Polygon)
		{
			SvgViewShow::Instance->addPolygon(color, attr.content);
		}
		else if (m_type == EditType::EditType_Line)
		{
			SvgViewShow::Instance->addLine(color, attr.strokeWidth.toDouble());
		}
		else
		{
			SvgViewShow::Instance->addText(color, attr.content, attr.front);
		}
		SvgViewShow::Instance->onSaveSvg();
	}
	this->close();
}

void SetAttributeDialog::SLOT_OnCancleBtnClick()
{
	SvgViewShow::Instance->clearTmpItem();
	clear();
	this->close();
}

void SetAttributeDialog::SLOT_OnDeleteBtnClick()
{
	MainWindow::Instance->setStatusMessage("");
	QMessageBox::StandardButton ret = QMessageBox::question(this, QL("确认"), QL("确定移除该图元吗?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
	if (ret == QMessageBox::Yes && !m_id.isNull())
	{
		if (m_id.compare("99999"))
		{
			emit SIGNAL_Remove(m_id);
		}					 
		SvgViewShow::Instance->deleteItem(m_id, m_type);
		SvgViewShow::Instance->removeCurPolygonRect();
	}
}

void SetAttributeDialog::SLOT_OnSelectColorBtnClick()
{
	QColor color = QColorDialog::getColor(Qt::red, this, QL("选择颜色"));
	if (color.isValid())
	{
		ui->m_red->setText(QString::number(color.red()));
		ui->m_green->setText(QString::number(color.green()));
		ui->m_blue->setText(QString::number(color.blue()));
		ui->m_alpha->setText("255");
	}
}

void SetAttributeDialog::clear()
{
	ui->m_content->clear();
	ui->m_red->setText("255");
	ui->m_green->setText("0");
	ui->m_blue->setText("0");
	ui->m_alpha->setText("255");
	ui->m_strokeWidth->setText("12");
}

void SetAttributeDialog::fillAttr(QString content, QColor color, QString front, QString lineWidth)
{
	m_content = content;
	m_color = color;
	m_front = front;
	m_lineWidth = lineWidth;
	ui->m_content->setPlainText(content);
	ui->m_red->setText(QString::number(color.red()));
	ui->m_green->setText(QString::number(color.green()));
	ui->m_blue->setText(QString::number(color.blue()));
	ui->m_alpha->setText(QString::number(color.alpha()));
	ui->m_selectFrontSize->setCurrentText(front);
	ui->m_id->setText(m_id);
	ui->m_strokeWidth->setText(lineWidth);
}

bool SetAttributeDialog::diff()
{
	if (ui->m_content->toPlainText().compare(m_content)
		|| ui->m_red->text().compare(QString::number(m_color.red()))
		|| ui->m_green->text().compare(QString::number(m_color.green()))
		|| ui->m_blue->text().compare(QString::number(m_color.blue()))
		|| ui->m_alpha->text().compare(QString::number(m_color.alpha()))
		|| ui->m_selectFrontSize->currentText().compare(m_front)
		|| ui->m_strokeWidth->text().compare(m_lineWidth))
		return true;
	return false;
}

void SetAttributeDialog::closeEvent(QCloseEvent *event)
{
	Q_UNUSED(event);
	if (m_id == nullptr && m_type != EditType::EditType_Line)
	{
		clear();
	}
	SvgViewShow::Instance->clearTmpItem();
	if (m_type == EditType::EditType_Polygon)
	{
		SvgViewShow::Instance->removeCurPolygonRect();
	}
	else if (m_type == EditType::EditType_Text)
	{
		SvgViewShow::Instance->removeTextPosFlag();
	}
}
